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
#include "guicon.h"

#include <windows.h>
#include <process.h>
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

//{ Event
class EventImp:public Event
{
    HANDLE h;

public:
    EventImp()
    {
        h=CreateEvent(nullptr,0,0,nullptr);
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
    HANDLE h;

public:
    void start(threadCallback callback,void *arg)
    {
        h=(HANDLE)_beginthreadex(nullptr,0,callback,arg,0,nullptr);
    }
    void join()
    {
        WaitForSingleObject(h,INFINITE);
    }
    ~ThreadImp()
    {
        CloseHandle(h);
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
