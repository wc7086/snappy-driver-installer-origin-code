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

#ifndef SYSTEM_H
#define SYSTEM_H

#ifdef _MSC_VER
#include <shlwapi.h>
#else
#ifndef _INC_SHLWAPI
#define _INC_SHLWAPI
  extern "C" __stdcall char *StrStrIA(const char *lpFirst,const char *lpSrch);
  extern "C" __stdcall wchar_t *StrStrIW(const wchar_t *lpFirst,const wchar_t *lpSrch);
  extern "C" __stdcall int StrCmpIW(const wchar_t *psz1,const wchar_t *psz2);
#endif
#endif

class Event
{
public:
    virtual ~Event(){}
    virtual void wait()=0;
    virtual bool isRaised()=0;
    virtual void raise()=0;
};
Event *CreateEvent();

typedef unsigned ( __stdcall *threadCallback)(void *arg);

class ThreadAbs
{
public:
    virtual ~ThreadAbs(){}
    virtual void start(threadCallback callback,void *arg)=0;
    virtual void join()=0;
};
ThreadAbs *CreateThread();

void get_resource(int id,void **data,int *size);
void mkdir_r(const wchar_t *path);
void StrFormatSize(long long val,wchar_t *buf,int len);
void ShowHelp();

class SystemImp
{
public:
    int canWrite(const wchar_t *path);
    int run_command(const wchar_t* file,const wchar_t* cmd,int show,int wait);
    void benchmark();

    void deletefile(const wchar_t *filename);
    bool FileExists(const wchar_t *filename);
    void ExpandEnvVar(const wchar_t *source,wchar_t *dest,int bufsize);
    bool ChooseDir(wchar_t *path,const wchar_t *title);
    bool ChooseFile(wchar_t *filename,const wchar_t *strlist,const wchar_t *ext);
    void CreateDir(const wchar_t *filename);
    void fileDelSpec(wchar_t *filename);

    void getClassDesc(const GUID *guid,wchar_t *bufw);
    void CloseHandle_log(HANDLE h,const wchar_t *func,const wchar_t *obj);
    void UnregisterClass_log(const wchar_t *lpClassName,HINSTANCE hInstance,const wchar_t *func,const wchar_t *obj);
};
extern SystemImp System;

// FileMonitor
class Filemon
{
public:
    virtual ~Filemon(){}
};

typedef void (*FileChangeCallback)(const wchar_t *szFile,int Action,int lParam);
Filemon *CreateFilemon(const wchar_t *szDirectory,int subdirs,FileChangeCallback callback);
extern int monitor_pause;

#endif
