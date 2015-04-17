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
#define STR(A) (language[A].valstr?(wchar_t *)language[A].valstr:L"")
#define D(A) theme[A].val
#define D_STR(A) theme[A].valstr

struct entry_t
{
    const wchar_t *name;
    union
    {
        int val;
        wchar_t *valstr;
    };
    int init;
};

typedef std::unordered_map <std::wstring,int> lookuptbl_t;

class Vault
{
public:
    entry_t *entry;
    int num;
    wchar_t namelist[64][250];
    wchar_t *data,*odata;
lookuptbl_t *lookuptbl;
    int res;

private:
    int  findvar(wchar_t *str);
    wchar_t *findstr(wchar_t *str);
    int  readvalue(const wchar_t *str);
    void parse(wchar_t *data);
    wchar_t *loadFromEncodedFile(const wchar_t *filename,int *sz);
    void loadFromFile(wchar_t *filename);
    void loadFromRes(int id);

public:
    void init1(entry_t *entry,int num,int res);
    void free1();
    void load(int i);

    friend void lang_enum(HWND hwnd,const wchar_t *path,int locale);
    friend void theme_enum(HWND hwnd,const wchar_t *path);
};

extern entry_t language[STR_NM];
extern entry_t theme[THEME_NM];
extern Vault vLang,vTheme;
extern int monitor_pause;

// FileMonitor
typedef void (CALLBACK *FileChangeCallback)(const wchar_t *,DWORD,LPARAM);
void CALLBACK monitor_callback(DWORD dwErrorCode,DWORD dwNumberOfBytesTransfered,LPOVERLAPPED lpOverlapped);

class monitor_t
{
public:
	OVERLAPPED ol;
	HANDLE     hDir;
	BYTE       buffer[32*1024];
	LPARAM     lParam;
	DWORD      notifyFilter;
	BOOL       fStop;
	wchar_t      dir[BUFLEN];
	int        subdirs;
	FileChangeCallback callback;

    int           monitor_refresh();
    void          monitor_stop();
};
monitor_t     *monitor_start(LPCTSTR szDirectory,DWORD notifyFilter,int subdirs,FileChangeCallback callback);

// Lang/theme
void lang_set(int i);
void theme_set(int i);
void lang_enum(HWND hwnd,const wchar_t *path,int locale);
void theme_enum(HWND hwnd,const wchar_t *path);

void vault_startmonitors();
void vault_stopmonitors();
void CALLBACK lang_callback(const wchar_t *szFile,DWORD action,LPARAM lParam);
void CALLBACK theme_callback(const wchar_t *szFile,DWORD action,LPARAM lParam);
