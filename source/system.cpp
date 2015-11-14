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
#include "common.h"
#include "settings.h"
#include "logging.h"
#include "manager.h"

#include <windows.h>
#ifdef _MSC_VER
#include <commdlg.h>
#include <direct.h>
#include <shellapi.h>
#endif

#include <process.h>
#include <errno.h>
#include <setupapi.h>       // for SHELLEXECUTEINFO
#include <shlwapi.h>        // for PathFileExists
#include <shlobj.h>         // for SHBrowseForFolder

#include "main.h"
#include "system.h"

SystemImp System;
int monitor_pause=0;
HFONT CLIHelp_Font;

static BOOL CALLBACK EnumLanguageGroupsProc(
    LGRPID LanguageGroup,
    LPTSTR lpLanguageGroupString,
    LPTSTR lpLanguageGroupNameString,
    DWORD dwFlags,
    LONG_PTR  lParam
    )
{
    UNREFERENCED_PARAMETER(lpLanguageGroupString);
    UNREFERENCED_PARAMETER(lpLanguageGroupNameString);
    UNREFERENCED_PARAMETER(dwFlags);

    LGRPID* plLang=(LGRPID*)(lParam);
    //Log.print_con("lang %d,%ws,%ws,%d\n",LanguageGroup,lpLanguageGroupString,lpLanguageGroupNameString,dwFlags);
    if(*plLang==LanguageGroup)
    {
        *plLang=0;
        return false;
    }
    return true;
}

bool SystemImp::IsLangInstalled(int group)
{
    LONG lLang=group;
    EnumSystemLanguageGroups(EnumLanguageGroupsProc,LGRPID_INSTALLED,(LONG_PTR)&lLang);
    return lLang==0;
}

unsigned SystemImp::GetTickCountWr()
{
    return GetTickCount();
}

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
void get_resource(int id,void **data,size_t *size)
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
void StrFormatSize(long long val,wchar_t *buf,int len)
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
        if(_wmkdir(buf)<0&&errno!=EEXIST&&wcslen(buf)>2)
            Log.print_err("ERROR in mkdir_r(): failed _wmkdir(%S,%d)\n",buf,errno);
        *p=L'\\';
        p++;
    }
    if(_wmkdir(buf)<0&&errno!=EEXIST&&wcslen(buf)>2)
        Log.print_err("ERROR in mkdir_r(): failed _wmkdir(%S,%d)\n",buf,errno);
}

//{ Event
class EventImp:public Event
{
    EventImp(const EventImp&)=delete;
	EventImp &operator = (const EventImp&) = delete;

    HANDLE h;

public:
    EventImp(bool manual):
        h(CreateEvent(nullptr,manual?1:0,0,nullptr))
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
    void reset()
    {
        ResetEvent(h);
    }
};

Event *CreateEventWr(bool manual)
{
    return new EventImp{manual};
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
        if(h)
        {
            if(!CloseHandle(h))
                Log.print_err("ERROR in ThreadImpS(): failed CloseHandle\n");
        }
    }
};
ThreadAbs *CreateThread()
{
    return new ThreadImp;
}

void SystemImp::UnregisterClass_log(const wchar_t *lpClassName,const wchar_t *func,const wchar_t *obj)
{
    if(!UnregisterClass(lpClassName,ghInst))
        Log.print_err("ERROR in %S(): failed UnregisterClass(%S)\n",func,obj);
}

bool SystemImp::FileExists(const wchar_t *path)
{
    return PathFileExists(path)!=0;
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
	wchar_t    dir[BUFLEN];
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
    FilemonImp(const wchar_t *szDirectory,int subdirs,FileChangeCallback callback);
    ~FilemonImp();
};

Filemon *CreateFilemon(const wchar_t *szDirectory,int subdirs,FileChangeCallback callback)
{
    return new FilemonImp(szDirectory,subdirs,callback);
}

FilemonImp::FilemonImp(const wchar_t *szDirectory, int subdirs_, FileChangeCallback callback_)
{
	wcscpy(data.dir,szDirectory);

	data.hDir=CreateFile(szDirectory,FILE_LIST_DIRECTORY,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
	                            nullptr,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OVERLAPPED,nullptr);

	if(data.hDir!=INVALID_HANDLE_VALUE)
	{
		data.ol.hEvent    = CreateEvent(nullptr,TRUE,FALSE,nullptr);
		data.notifyFilter = FILE_NOTIFY_CHANGE_LAST_WRITE|FILE_NOTIFY_CHANGE_FILE_NAME;
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

	wchar_t szFile[MAX_PATH];
	PFILE_NOTIFY_INFORMATION pNotify;
	FilemonDataPOD *pMonitor=reinterpret_cast<FilemonDataPOD*>(lpOverlapped);

	if(dwErrorCode==ERROR_SUCCESS)
	{
        size_t offset=0;
        do
		{
			pNotify=(PFILE_NOTIFY_INFORMATION)&pMonitor->buffer[offset];
			offset+=pNotify->NextEntryOffset;

            wcsncpy(szFile,pNotify->FileName,pNotify->FileNameLength/sizeof(wchar_t)+1);

			if(!monitor_pause)
			{
                FILE *f;
                wchar_t buf[BUFLEN];
                int m=0,flag;
                size_t sz=0;

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
                    _fseeki64(f,0,SEEK_END);
                    sz=static_cast<size_t>(_ftelli64(f));
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

                if(flag)pMonitor->callback(szFile,pNotify->Action,(int)pMonitor->lParam);
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
    tm1=GetTickCountWr();
    for(i=0;i<1024*1024;i++)wsprintfA(buf,"Test str %ws",L"wchar str");
    tm1=GetTickCountWr()-tm1;
    tm2=GetTickCountWr();
    for(i=0;i<1024*1024;i++)sprintf(buf,"Test str %ws",L"wchar str");
    tm2=GetTickCountWr()-tm2;
    Log.print_con("%c wsprintfA \t%ld\n",tm1<tm2?'+':' ',tm1);
    Log.print_con("%c sprintf   \t%ld\n\n",tm1>tm2?'+':' ',tm2);

    tm1=GetTickCountWr();
    for(i=0;i<1024*1024;i++)wsprintfW(bufw,L"Test str %s",L"wchar str");
    tm1=GetTickCountWr()-tm1;
    tm2=GetTickCountWr();
    for(i=0;i<1024*1024;i++)swprintf(bufw,L"Test str %s",L"wchar str");
    tm2=GetTickCountWr()-tm2;
    Log.print_con("%c wsprintfW \t%ld\n",tm1<tm2?'+':' ',tm1);
    Log.print_con("%c swprintf  \t%ld\n\n",tm1>tm2?'+':' ',tm2);

    tm1=GetTickCountWr();
    for(i=0;i<1024*1024*5;i++)_strcmpi("Test str %ws","wchar str");
    tm1=GetTickCountWr()-tm1;
    tm2=GetTickCountWr();
    for(i=0;i<1024*1024*5;i++)strcmpi("Test str %ws","wchar str");
    tm2=GetTickCountWr()-tm2;
    Log.print_con("%c _strcmpi \t%ld\n",tm1<tm2?'+':' ',tm1);
    Log.print_con("%c strcmpi  \t%ld\n\n",tm1>tm2?'+':' ',tm2);

    tm1=GetTickCountWr();
    for(i=0;i<1024*1024*5;i++)strcasecmp("Test str %ws","wchar str");
    tm1=GetTickCountWr()-tm1;
    tm2=GetTickCountWr();
    for(i=0;i<1024*1024*5;i++)strcmpi("Test str %ws","wchar str");
    tm2=GetTickCountWr()-tm2;
    Log.print_con("%c strcasecmp \t%ld\n",tm1<tm2?'+':' ',tm1);
    Log.print_con("%c strcmpi  \t%ld\n\n",tm1>tm2?'+':' ',tm2);

    tm1=GetTickCountWr();
    for(i=0;i<1024*1024*5;i++)StrCmpIW(L"Test str %ws",L"wchar str");
    tm1=GetTickCountWr()-tm1;
    tm2=GetTickCountWr();
    for(i=0;i<1024*1024*5;i++)wcsicmp(L"Test str %ws",L"wchar str");
    tm2=GetTickCountWr()-tm2;
    Log.print_con("%c StrCmpIW \t%ld\n",tm1<tm2?'+':' ',tm1);
    Log.print_con("%c wcsicmp  \t%ld\n\n",tm1>tm2?'+':' ',tm2);

    tm1=GetTickCountWr();
    for(i=0;i<1024*1024*5;i++)StrCmpIA("Test str %ws","wchar str");
    tm1=GetTickCountWr()-tm1;
    tm2=GetTickCountWr();
    for(i=0;i<1024*1024*5;i++)strcmpi("Test str %ws","wchar str");
    tm2=GetTickCountWr()-tm2;
    Log.print_con("%c StrCmpIA \t%ld\n",tm1<tm2?'+':' ',tm1);
    Log.print_con("%c strcmpi  \t%ld\n\n",tm1>tm2?'+':' ',tm2);

    tm1=GetTickCountWr();
    for(i=0;i<1024*1024*5;i++)StrCmpW(L"Test str %ws",bufw); // StrCmpW == lstrcmp
    tm1=GetTickCountWr()-tm1;
    tm2=GetTickCountWr();
    for(i=0;i<1024*1024*5;i++)wcscmp(L"Test str %ws",bufw);
    tm2=GetTickCountWr()-tm2;
    Log.print_con("%c StrCmpW \t%ld\n",tm1<tm2?'+':' ',tm1);
    Log.print_con("%c wcscmp  \t%ld\n\n",tm1>tm2?'+':' ',tm2);

    tm1=GetTickCountWr();
    for(i=0;i<1024*1024*5;i++)a=StrCmpA("Test str %ws",buf);
    tm1=GetTickCountWr()-tm1;
    tm2=GetTickCountWr();
    for(i=0;i<1024*1024*5;i++)a=strcmp("Test str %ws",buf);
    tm2=GetTickCountWr()-tm2;
    Log.print_con("%c StrCmpA \t%ld\n",tm1<tm2?'+':' ',tm1);
    Log.print_con("%c strcmp \t%ld\n\n",tm1>tm2?'+':' ',tm2);

    tm1=GetTickCountWr();
    for(i=0;i<1024*1024*5;i++)StrStrA("Test str %ws",buf);
    tm1=GetTickCountWr()-tm1;
    tm2=GetTickCountWr();
    for(i=0;i<1024*1024*5;i++)strstr("Test str %ws","Test str %ws");
    tm2=GetTickCountWr()-tm2;
    Log.print_con("%c StrStrA \t%ld\n",tm1<tm2?'+':' ',tm1);
    Log.print_con("%c strstr \t%ld\n\n",tm1>tm2?'+':' ',tm2);

    tm1=GetTickCountWr();
    for(i=0;i<1024*1024*5;i++)StrStrW(L"Test str %ws",bufw);
    tm1=GetTickCountWr()-tm1;
    tm2=GetTickCountWr();
    for(i=0;i<1024*1024*5;i++)wcsstr(L"Test str %ws",bufw);
    tm2=GetTickCountWr()-tm2;
    Log.print_con("%c StrStrW \t%ld\n",tm1<tm2?'+':' ',tm1);
    Log.print_con("%c wcsstr  \t%ld\n\n",tm1>tm2?'+':' ',tm2);

    tm1=GetTickCountWr();
    for(i=0;i<1024*1024*5*2;i++)lstrlenA(buf);
    tm1=GetTickCountWr()-tm1;
    tm2=GetTickCountWr();
    for(i=0;i<1024*1024*5*2;i++)strlen(buf);
    tm2=GetTickCountWr()-tm2;
    Log.print_con("%c lstrlenA \t%ld\n",tm1<tm2?'+':' ',tm1);
    Log.print_con("%c strlen   \t%ld\n\n",tm1>tm2?'+':' ',tm2);

    tm1=GetTickCountWr();
    for(i=0;i<1024*1024*5*2;i++)lstrlenW(bufw);
    tm1=GetTickCountWr()-tm1;
    tm2=GetTickCountWr();
    for(i=0;i<1024*1024*5*2;i++)wcslen(bufw);
    tm2=GetTickCountWr()-tm2;
    Log.print_con("%c lstrlenW \t%ld\n",tm1<tm2?'+':' ',tm1);
    Log.print_con("%c wcslen   \t%ld\n\n",tm1>tm2?'+':' ',tm2);

    tm1=GetTickCountWr();
    for(i=0;i<1024*1024*5;i++)StrCpyA(buf,"Test str %ws");
    tm1=GetTickCountWr()-tm1;
    tm2=GetTickCountWr();
    for(i=0;i<1024*1024*5;i++)strcpy(buf,"Test str %ws");
    tm2=GetTickCountWr()-tm2;
    Log.print_con("%c StrCpyA \t%ld\n",tm1<tm2?'+':' ',tm1);
    Log.print_con("%c strcpy \t%ld\n\n",tm1>tm2?'+':' ',tm2);
}
#endif
//}

//{ Virus detection
void viruscheck(const wchar_t *szFile,int action,int lParam)
{
    UNREFERENCED_PARAMETER(szFile);
    UNREFERENCED_PARAMETER(action);
    UNREFERENCED_PARAMETER(lParam);

    int type;
    int update=0;

    if(Settings.flags&FLAG_NOVIRUSALERTS)return;
    type=GetDriveType(nullptr);

    // autorun.inf
    if(type!=DRIVE_CDROM)
    {
        if(System.FileExists(L"\\autorun.inf"))
        {
            FILE *f;
            f=_wfopen(L"\\autorun.inf",L"rb");
            if(f)
            {
                char buf[BUFLEN];
                fread(buf,BUFLEN,1,f);
                fclose(f);
                buf[BUFLEN-1]=0;
                if(!StrStrIA(buf,"[NOT_A_VIRUS]")&&StrStrIA(buf,"open"))
                    manager_g->itembar_setactive(SLOT_VIRUS_AUTORUN,update=1);
            }
            else
                Log.print_con("NOTE: cannot open autorun.inf [error: %d]\n",errno);
        }
    }

    // RECYCLER
    if(type==DRIVE_REMOVABLE)
        if(System.FileExists(L"\\RECYCLER")&&!System.FileExists(L"\\RECYCLER\\not_a_virus.txt"))
            manager_g->itembar_setactive(SLOT_VIRUS_RECYCLER,update=1);

    // Hidden folders
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind=FindFirstFile(L"\\*.*",&FindFileData);
    if(type==DRIVE_REMOVABLE)
    while(FindNextFile(hFind,&FindFileData)!=0)
    {
        if(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
        {
            if(lstrcmp(FindFileData.cFileName,L"..")==0)continue;
            if(lstrcmpi(FindFileData.cFileName,L"System Volume Information")==0)continue;

            if(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN)
            {
                WStringShort bufw;
                bufw.sprintf(L"\\%ws\\not_a_virus.txt",FindFileData.cFileName);
                if(System.FileExists(bufw.Get()))continue;
                Log.print_con("VIRUS_WARNING: hidden folder '%S'\n",FindFileData.cFileName);
                manager_g->itembar_setactive(SLOT_VIRUS_HIDDEN,update=1);
            }
        }
    }
    FindClose(hFind);
    if(update)
    {
        manager_g->setpos();
        SetTimer(MainWindow.hMain,1,1000/60,nullptr);
    }
}
//}

static BOOL CALLBACK ShowHelpProcedure(HWND hwnd,UINT Message,WPARAM wParam,LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    HWND hEditBox;
    LPCSTR s;
    size_t sz;

    switch(Message)
    {
    case WM_INITDIALOG:
        SetWindowTextW(hwnd,L"Command Line options help");
        hEditBox=GetDlgItem(hwnd,0);
        SetWindowTextW(hEditBox,L"Command Line options");
        //ShowWindow(hEditBox,SW_HIDE);
        hEditBox=GetDlgItem(hwnd,IDOK);
        ShowWindow(hEditBox,SW_HIDE);
        hEditBox=GetDlgItem(hwnd,IDCANCEL);
        SetWindowTextW(hEditBox,L"Close");

        get_resource(IDR_CLI_HELP,(void **)&s,&sz);
        hEditBox=GetDlgItem(hwnd,IDC_EDIT1);

        SendMessage(hEditBox,WM_SETFONT,(WPARAM)CLIHelp_Font,0);

        SetWindowTextA(hEditBox,s);
        SendMessage(hEditBox,EM_SETREADONLY,1,0);

        return TRUE;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            EndDialog(hwnd,IDOK);
            return TRUE;

        case IDCANCEL:
            EndDialog(hwnd,IDCANCEL);
            return TRUE;

        default:
            break;
        }
        break;

    default:
        break;
    }
    return false;
}

void ShowHelp()
{
    CLIHelp_Font=CreateFont(-12,0,0,0,FW_DONTCARE,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,
                            CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,VARIABLE_PITCH,L"Consolas");

    DialogBox(ghInst,MAKEINTRESOURCE(IDD_DIALOG1),0,(DLGPROC)ShowHelpProcedure);
    DeleteObject(CLIHelp_Font);
}
