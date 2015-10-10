/*
This file is part of Snappy Driver Installer.

Snappy Driver Installer is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Snappy Driver Installer is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Snappy Driver Installer.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "com_header.h"
#include "logging.h"

#include <windows.h>
#include <process.h>
#include <errno.h>
#include <setupapi.h>       // for SHELLEXECUTEINFO
#include <shlwapi.h>        // for PathFileExists
#include <shlobj.h>         // for SHBrowseForFolder

#include "system.h"
#include "main.h"

SystemImp System;

bool SystemImp::ChooseDir(wchar_t *path,const wchar_t *title)
{
    BROWSEINFO lpbi;
    memset(&lpbi,0,sizeof(BROWSEINFO));
    lpbi.hwndOwner=MainWindow.hMain;
    lpbi.pszDisplayName=path;
    lpbi.lpszTitle=title;
    lpbi.ulFlags=BIF_NEWDIALOGSTYLE|BIF_EDITBOX;

    LPITEMIDLIST list=SHBrowseForFolder(&lpbi);
    if(list)
    {
        SHGetPathFromIDList(list,path);
        return true;
    }
    return false;
}

bool SystemImp::ChooseFile(wchar_t *filename,const wchar_t *strlist,const wchar_t *ext)
{
    OPENFILENAME ofn;
    memset(&ofn,0,sizeof(OPENFILENAME));
    ofn.lStructSize=sizeof(OPENFILENAME);
    ofn.hwndOwner  =MainWindow.hMain;
    ofn.lpstrFilter=strlist;
    ofn.nMaxFile   =BUFLEN;
    ofn.lpstrDefExt=ext;
    ofn.lpstrFile  =filename;
    ofn.Flags      =OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;

    if(GetOpenFileName(&ofn))return true;
    return false;
}
void get_resource(int id,void **data,int *size)
{
    HRSRC myResource=FindResource(nullptr,MAKEINTRESOURCE(id),(wchar_t *)RESFILE);
    if(!myResource)
    {
        Log.print_err("ERROR in get_resource(): failed FindResource(%d)\n",id);
        *size=0;
        *data=nullptr;
        return;
    }
    *size=SizeofResource(nullptr,myResource);
    *data=LoadResource(nullptr,myResource);
}
void StrFormatSize(int val,wchar_t *buf,int len)
{
    StrFormatByteSizeW(val,buf,len);
}

void mkdir_r(const wchar_t *path)
{
    if(path[1]==L':'&&path[2]==0)return;
    if(!System.canWrite(path))
    {
        Log.print_err("ERROR in mkdir_r(): Write-protected,'%S'\n",path);
        return;
    }

    wchar_t buf[BUFLEN];
    wcscpy(buf,path);
    wchar_t *p=buf;
    while((p=wcschr(p,L'\\')))
    {
        *p=0;
        if(_wmkdir(buf)<0&&errno!=EEXIST&&lstrlenW(buf)>2)
            Log.print_err("ERROR in mkdir_r(): failed _wmkdir(%S,%d)\n",buf,errno);
        *p=L'\\';
        p++;
    }
    if(_wmkdir(buf)<0&&errno!=EEXIST&&lstrlenW(buf)>2)
        Log.print_err("ERROR in mkdir_r(): failed _wmkdir(%S,%d)\n",buf,errno);
}

//{ Event
class EventImp:public Event
{
    EventImp(const EventImp&)=delete;
    operator=(const EventImp&)=delete;

    HANDLE h;

public:
    EventImp():
        h(CreateEvent(nullptr,0,0,nullptr))
    {
    }
    ~EventImp()
    {
        CloseHandle(h);
    }
    void wait()
    {
        WaitForSingleObject(h,INFINITE);
    }
    bool isRaised()
    {
        return WaitForSingleObject(h,0)==WAIT_OBJECT_0;
    }
    void raise()
    {
        SetEvent(h);
    }
};

Event *CreateEvent()
{
    return new EventImp;
}
//}

class ThreadImp:public ThreadAbs
{
    HANDLE h=nullptr;

public:
    void start(threadCallback callback,void *arg)
    {
        h=(HANDLE)_beginthreadex(nullptr,0,callback,arg,0,nullptr);
    }
    void join()
    {
        if(h)WaitForSingleObject(h,INFINITE);
    }
    ~ThreadImp()
    {
        if(h)CloseHandle(h);
    }
};
ThreadAbs *CreateThread()
{
    return new ThreadImp;
}

void SystemImp::CloseHandle_log(HANDLE h,const wchar_t *func,const wchar_t *obj)
{
    if(!CloseHandle(h))
        Log.print_err("ERROR in %S(): failed CloseHandle(%S)\n",func,obj);
}

void SystemImp::UnregisterClass_log(const wchar_t *lpClassName,HINSTANCE hInstance,const wchar_t *func,const wchar_t *obj)
{
    if(!UnregisterClass(lpClassName,hInstance))
        Log.print_err("ERROR in %S(): failed UnregisterClass(%S)\n",func,obj);
}

bool SystemImp::FileExists(const wchar_t *path)
{
    return PathFileExists(path);
}

void SystemImp::ExpandEnvVar(const wchar_t *source,wchar_t *dest,int bufsize)
{
    ExpandEnvironmentStringsW(source,dest,bufsize);
}

int SystemImp::canWrite(const wchar_t *path)
{
    DWORD flagsv;
    wchar_t drive[4];

    wcscpy(drive,L"C:\\");

    if(path&&wcslen(path)>1&&path[1]==':')
    {
        drive[0]=path[0];
        GetVolumeInformation(drive,nullptr,0,nullptr,nullptr,&flagsv,nullptr,0);
    }
    else
        GetVolumeInformation(nullptr,nullptr,0,nullptr,nullptr,&flagsv,nullptr,0);

    return (flagsv&FILE_READ_ONLY_VOLUME)?0:1;
}

void SystemImp::getClassDesc(GUID *guid,wchar_t *bufw)
{
    SetupDiGetClassDescription(guid,bufw,BUFLEN,nullptr);
}

void SystemImp::CreateDir(const wchar_t *filename)
{
    CreateDirectory(filename,nullptr);
}

void SystemImp::deletefile(const wchar_t *filename)
{
    DeleteFileW(filename);
}

void SystemImp::fileDelSpec(wchar_t *filename)
{
    PathRemoveFileSpec(filename);
}

int SystemImp::run_command(const wchar_t* file,const wchar_t* cmd,int show,int wait)
{
    DWORD ret;

    SHELLEXECUTEINFO ShExecInfo;
    memset(&ShExecInfo,0,sizeof(SHELLEXECUTEINFO));
    ShExecInfo.cbSize=sizeof(SHELLEXECUTEINFO);
    ShExecInfo.fMask=SEE_MASK_NOCLOSEPROCESS;
    ShExecInfo.lpFile=file;
    ShExecInfo.lpParameters=cmd;
    ShExecInfo.nShow=show;

    Log.print_con("Run(%S,%S,%d,%d)\n",file,cmd,show,wait);
    if(!wcscmp(file,L"open"))
        ShellExecute(nullptr,L"open",cmd,nullptr,nullptr,SW_SHOWNORMAL);
    else
        ShellExecuteEx(&ShExecInfo);

    if(!wait)return 0;
    WaitForSingleObject(ShExecInfo.hProcess,INFINITE);
    GetExitCodeProcess(ShExecInfo.hProcess,&ret);
    return ret;
}

//{ FileMonitor
struct FilemonDataPOD
{
	OVERLAPPED ol;
	HANDLE     hDir;
	BYTE       buffer[32*1024];
	LPARAM     lParam;
	DWORD      notifyFilter;
	BOOL       fStop;
	wchar_t      dir[BUFLEN];
	int        subdirs;
	FileChangeCallback callback;
};

class FilemonImp:public Filemon
{
    FilemonDataPOD data;

private:
    static void CALLBACK monitor_callback(DWORD dwErrorCode,DWORD dwNumberOfBytesTransfered,LPOVERLAPPED lpOverlapped);
    static int refresh(FilemonDataPOD &data);

public:
    FilemonImp(const wchar_t *szDirectory,int notifyFilter,int subdirs,FileChangeCallback callback);
    ~FilemonImp();
};

Filemon *CreateFilemon(const wchar_t *szDirectory,int notifyFilter,int subdirs,FileChangeCallback callback)
{
    return new FilemonImp(szDirectory,notifyFilter,subdirs,callback);
}

FilemonImp::FilemonImp(const wchar_t *szDirectory, int notifyFilter_, int subdirs_, FileChangeCallback callback_)
{
	wcscpy(data.dir,szDirectory);

	data.hDir=CreateFile(szDirectory,FILE_LIST_DIRECTORY,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
	                            nullptr,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OVERLAPPED,nullptr);

	if(data.hDir!=INVALID_HANDLE_VALUE)
	{
		data.ol.hEvent    = CreateEvent(nullptr,TRUE,FALSE,nullptr);
		data.notifyFilter = notifyFilter_;
		data.callback     = callback_;
		data.subdirs      = subdirs_;

		if(refresh(data))
		{
			return;
		}
		else
		{
			CloseHandle(data.ol.hEvent);
			CloseHandle(data.hDir);
			data.hDir=INVALID_HANDLE_VALUE;
		}
	}
}

int FilemonImp::refresh(FilemonDataPOD &data1)
{
	return ReadDirectoryChangesW(data1.hDir,data1.buffer,sizeof(data1.buffer),data1.subdirs,
	                      data1.notifyFilter,nullptr,&data1.ol,monitor_callback);
}

void CALLBACK FilemonImp::monitor_callback(DWORD dwErrorCode,DWORD dwNumberOfBytesTransfered,LPOVERLAPPED lpOverlapped)
{
    UNREFERENCED_PARAMETER(dwNumberOfBytesTransfered);

	TCHAR szFile[MAX_PATH];
	PFILE_NOTIFY_INFORMATION pNotify;
	FilemonDataPOD *pMonitor=reinterpret_cast<FilemonDataPOD*>(lpOverlapped);

	if(dwErrorCode==ERROR_SUCCESS)
	{
        size_t offset=0;
        do
		{
			pNotify=(PFILE_NOTIFY_INFORMATION)&pMonitor->buffer[offset];
			offset+=pNotify->NextEntryOffset;

            lstrcpynW(szFile,pNotify->FileName,pNotify->FileNameLength/sizeof(wchar_t)+1);

			if(!monitor_pause)
			{
                FILE *f;
                wchar_t buf[BUFLEN];
                int m=0,sz=-1,flag;

                errno=0;
                wsprintf(buf,L"%ws\\%ws",pMonitor->dir,szFile);
                Log.print_con("{\n  changed'%S'\n",buf);
                f=_wfsopen(buf,L"rb",0x10); //deny read/write mode
                if(f)m=2;
                if(!f)
                {
                    f=_wfopen(buf,L"rb");
                    if(f)m=1;
                }
                if(f)
                {
                    fseek(f,0,SEEK_END);
                    sz=ftell(f);
                    fclose(f);
                }
                /*
                    m0: failed to open
                    m1: opened(normal)
                    m2: opened(exclusive)
                */
                switch(pNotify->Action)
                {
                    case 1: // Added
                        flag=errno?0:1;
                        break;

                    case 2: // Removed
                        flag=1;
                        break;

                    case 3: // Modifed
                        flag=errno?0:1;
                        break;

                    case 4: // Renamed(old name)
                        flag=0;
                        break;

                    case 5: // Renamed(new name)
                        flag=1;
                        break;

                    default:
                        flag=0;
                }

                Log.print_con("  %c a(%d),m(%d),err(%02d),size(%9d)\n",flag?'+':'-',pNotify->Action,m,errno,sz);

                if(flag)pMonitor->callback(szFile,pNotify->Action,pMonitor->lParam);
                Log.print_con("}\n\n");
			}
		}while(pNotify->NextEntryOffset!=0);
	}
	if(!pMonitor->fStop)refresh(*pMonitor);
}

FilemonImp::~FilemonImp()
{
	if(data.hDir!=INVALID_HANDLE_VALUE)
    {
        data.fStop=TRUE;
        CancelIo(data.hDir);
        if(!HasOverlappedIoCompleted(&data.ol))SleepEx(5,TRUE);
        CloseHandle(data.ol.hEvent);
        CloseHandle(data.hDir);
    }
}
//}

#ifdef BENCH_MODE
void SystemImp::benchmark()
{
    char buf[BUFLEN];
    wchar_t bufw[BUFLEN];
    long long i,tm1,tm2;
    int a;

    // wsprintfA is faster but doesn't support %f
    tm1=GetTickCount();
    for(i=0;i<1024*1024;i++)wsprintfA(buf,"Test str %ws",L"wchar str");
    tm1=GetTickCount()-tm1;
    tm2=GetTickCount();
    for(i=0;i<1024*1024;i++)sprintf(buf,"Test str %ws",L"wchar str");
    tm2=GetTickCount()-tm2;
    Log.print_con("%c wsprintfA \t%ld\n",tm1<tm2?'+':' ',tm1);
    Log.print_con("%c sprintf   \t%ld\n\n",tm1>tm2?'+':' ',tm2);

    tm1=GetTickCount();
    for(i=0;i<1024*1024;i++)wsprintfW(bufw,L"Test str %s",L"wchar str");
    tm1=GetTickCount()-tm1;
    tm2=GetTickCount();
    for(i=0;i<1024*1024;i++)swprintf(bufw,L"Test str %s",L"wchar str");
    tm2=GetTickCount()-tm2;
    Log.print_con("%c wsprintfW \t%ld\n",tm1<tm2?'+':' ',tm1);
    Log.print_con("%c swprintf  \t%ld\n\n",tm1>tm2?'+':' ',tm2);

    tm1=GetTickCount();
    for(i=0;i<1024*1024*5;i++)strcasecmp("Test str %ws","wchar str");
    tm1=GetTickCount()-tm1;
    tm2=GetTickCount();
    for(i=0;i<1024*1024*5;i++)strcmpi("Test str %ws","wchar str");
    tm2=GetTickCount()-tm2;
    Log.print_con("%c strcasecmp \t%ld\n",tm1<tm2?'+':' ',tm1);
    Log.print_con("%c strcmpi  \t%ld\n\n",tm1>tm2?'+':' ',tm2);

    tm1=GetTickCount();
    for(i=0;i<1024*1024*5;i++)StrCmpIW(L"Test str %ws",L"wchar str");
    tm1=GetTickCount()-tm1;
    tm2=GetTickCount();
    for(i=0;i<1024*1024*5;i++)wcsicmp(L"Test str %ws",L"wchar str");
    tm2=GetTickCount()-tm2;
    Log.print_con("%c StrCmpIW \t%ld\n",tm1<tm2?'+':' ',tm1);
    Log.print_con("%c wcsicmp  \t%ld\n\n",tm1>tm2?'+':' ',tm2);

    tm1=GetTickCount();
    for(i=0;i<1024*1024*5;i++)StrCmpIA("Test str %ws","wchar str");
    tm1=GetTickCount()-tm1;
    tm2=GetTickCount();
    for(i=0;i<1024*1024*5;i++)strcmpi("Test str %ws","wchar str");
    tm2=GetTickCount()-tm2;
    Log.print_con("%c StrCmpIA \t%ld\n",tm1<tm2?'+':' ',tm1);
    Log.print_con("%c strcmpi  \t%ld\n\n",tm1>tm2?'+':' ',tm2);

    tm1=GetTickCount();
    for(i=0;i<1024*1024*5;i++)StrCmpW(L"Test str %ws",bufw); // StrCmpW == lstrcmp
    tm1=GetTickCount()-tm1;
    tm2=GetTickCount();
    for(i=0;i<1024*1024*5;i++)wcscmp(L"Test str %ws",bufw);
    tm2=GetTickCount()-tm2;
    Log.print_con("%c StrCmpW \t%ld\n",tm1<tm2?'+':' ',tm1);
    Log.print_con("%c wcscmp  \t%ld\n\n",tm1>tm2?'+':' ',tm2);

    tm1=GetTickCount();
    for(i=0;i<1024*1024*5;i++)a=StrCmpA("Test str %ws",buf);
    tm1=GetTickCount()-tm1;
    tm2=GetTickCount();
    for(i=0;i<1024*1024*5;i++)a=strcmp("Test str %ws",buf);
    tm2=GetTickCount()-tm2;
    Log.print_con("%c StrCmpA \t%ld\n",tm1<tm2?'+':' ',tm1);
    Log.print_con("%c strcmp \t%ld\n\n",tm1>tm2?'+':' ',tm2);

    tm1=GetTickCount();
    for(i=0;i<1024*1024*5;i++)StrStrA("Test str %ws",buf);
    tm1=GetTickCount()-tm1;
    tm2=GetTickCount();
    for(i=0;i<1024*1024*5;i++)strstr("Test str %ws","Test str %ws");
    tm2=GetTickCount()-tm2;
    Log.print_con("%c StrStrA \t%ld\n",tm1<tm2?'+':' ',tm1);
    Log.print_con("%c strstr \t%ld\n\n",tm1>tm2?'+':' ',tm2);

    tm1=GetTickCount();
    for(i=0;i<1024*1024*5;i++)StrStrW(L"Test str %ws",bufw);
    tm1=GetTickCount()-tm1;
    tm2=GetTickCount();
    for(i=0;i<1024*1024*5;i++)wcsstr(L"Test str %ws",bufw);
    tm2=GetTickCount()-tm2;
    Log.print_con("%c StrStrW \t%ld\n",tm1<tm2?'+':' ',tm1);
    Log.print_con("%c wcsstr  \t%ld\n\n",tm1>tm2?'+':' ',tm2);

    tm1=GetTickCount();
    for(i=0;i<1024*1024*5*2;i++)lstrlenA(buf);
    tm1=GetTickCount()-tm1;
    tm2=GetTickCount();
    for(i=0;i<1024*1024*5*2;i++)strlen(buf);
    tm2=GetTickCount()-tm2;
    Log.print_con("%c lstrlenA \t%ld\n",tm1<tm2?'+':' ',tm1);
    Log.print_con("%c strlen   \t%ld\n\n",tm1>tm2?'+':' ',tm2);

    tm1=GetTickCount();
    for(i=0;i<1024*1024*5*2;i++)lstrlenW(bufw);
    tm1=GetTickCount()-tm1;
    tm2=GetTickCount();
    for(i=0;i<1024*1024*5*2;i++)wcslen(bufw);
    tm2=GetTickCount()-tm2;
    Log.print_con("%c lstrlenW \t%ld\n",tm1<tm2?'+':' ',tm1);
    Log.print_con("%c wcslen   \t%ld\n\n",tm1>tm2?'+':' ',tm2);

    tm1=GetTickCount();
    for(i=0;i<1024*1024*5;i++)StrCpyA(buf,"Test str %ws");
    tm1=GetTickCount()-tm1;
    tm2=GetTickCount();
    for(i=0;i<1024*1024*5;i++)strcpy(buf,"Test str %ws");
    tm2=GetTickCount()-tm2;
    Log.print_con("%c StrCpyA \t%ld\n",tm1<tm2?'+':' ',tm1);
    Log.print_con("%c strcpy \t%ld\n\n",tm1>tm2?'+':' ',tm2);
}
#endif
//}
