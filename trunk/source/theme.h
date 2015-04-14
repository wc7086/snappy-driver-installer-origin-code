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

#include "themelist.h"
#include "langlist.h"

// Theme/lang
#define STR(A) (language[A].val?(WCHAR *)language[A].val:L"")
#define D(A) theme[A].val

typedef struct _entry_t
{
    const WCHAR *name;
    intptr_t val;
    int init;
}entry_t;

class Vault
{
public:
    entry_t *entry;
    int num;
    WCHAR namelist[64][250];
    WCHAR *data,*odata;
    hashtable_t strs;

public:
    void init1(entry_t *entry,int num);
    void free1();
    void parse(WCHAR *data);
    void loadfromfile(WCHAR *filename);
    void loadfromres(int id);
};

extern entry_t language[STR_NM];
extern entry_t theme[THEME_NM];
extern Vault vLang,vTheme;
extern int monitor_pause;

// Monitor
typedef void (CALLBACK *FileChangeCallback)(const WCHAR *,DWORD,LPARAM);
typedef struct _monitor_t
{
	OVERLAPPED ol;
	HANDLE     hDir;
	BYTE       buffer[32*1024];
	LPARAM     lParam;
	DWORD      notifyFilter;
	BOOL       fStop;
	WCHAR      dir[BUFLEN];
	int        subdirs;
	FileChangeCallback callback;
}*monitor_t;

// Vault
void vault_startmonitors();
void vault_stopmonitors();
void vault_init();
void vault_free();

void *vault_loadfile(const WCHAR *filename,int *sz);
int  vault_findvar(hashtable_t *t,WCHAR *str);
int  vault_readvalue(const WCHAR *str);
intptr_t  vault_findstr(WCHAR *str);


// Lang/theme
int  lang_enum(HWND hwnd,const WCHAR *path,int locale);
void theme_enum(HWND hwnd,const WCHAR *path);
void lang_set(int i);
void theme_set(int i);
void CALLBACK lang_callback(const WCHAR *szFile,DWORD action,LPARAM lParam);
void CALLBACK theme_callback(const WCHAR *szFile,DWORD action,LPARAM lParam);

// Monitor
monitor_t      monitor_start(LPCTSTR szDirectory,DWORD notifyFilter,int subdirs,FileChangeCallback callback);
int           monitor_refresh(monitor_t pMonitor);
void CALLBACK monitor_callback(DWORD dwErrorCode,DWORD dwNumberOfBytesTransfered,LPOVERLAPPED lpOverlapped);
void          monitor_stop(monitor_t pMonitor);
