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

#define BUFLEN 4096
#define WIN32_LEAN_AND_MEAN
#define _WIN32_IE 0x0501
#undef __STRICT_ANSI__
//#define BENCH_MODE

//{ Includes

// Windows
#include <windows.h>
#include <commdlg.h>        // for GetOpenFileName()
#include <setupapi.h>       // for SetupDiGetClassDescription()
#include <shlwapi.h>        // for StrStrIW
#include <shobjidl.h>       // for ShowProgressInTaskbar()
#include <shellapi.h>       // for CommandLineToArgvW [x64]
#include <shlobj.h>         // for SHBrowseForFolder()
#include "SRRestorePtAPI.h" // for RestorePoint

typedef WINBOOL (__cdecl *WINAPI5t_SRSetRestorePointW)(PRESTOREPOINTINFOW pRestorePtSpec,PSTATEMGRSTATUS pSMgrStatus);
#ifndef _WIN64
#define DISPLAY_DEVICE_ACTIVE         1
#define DISPLAY_DEVICE_ATTACHED       2
#endif

// C
#include <stdio.h>      // for _wfopen
#include <time.h>       // for time
#include <math.h>       // for sqrt
#include <signal.h>     // for signal
#include <process.h>    // for _beginthreadex
#include <direct.h>     // for _wmkdir

// C++
#include <unordered_map>
#include <queue>
#include <vector>
#include <memory>

// webp
#include <webp\decode.h>

// 7-zip
extern "C"
{
#include "7z.h"
#include "7zAlloc.h"
#include "7zCrc.h"
#include "7zFile.h"
#include "7zVersion.h"
#include "LzmaEnc.h"
#include "Lzma86.h"
}

#ifndef BENCH_MODE
#define StrStrW wcsstr
#define StrCmpIW wcsicmp
#define StrCmpW wcscmp
#endif

// SDI
#include "svnrev.h"
#include "resources.h"

#include "common.h"
#include "indexing.h"
#include "guicon.h"
#include "enum.h"
#include "matcher.h"
#include "manager.h"
#include "theme.h"
#include "install.h"
#include "draw.h"
#include "cli.h"
#include "update.h"

#include "debug_new.h"
//}

//{ Defines

// Misc
#define APPTITLE            L"Snappy Driver Installer v0.2"
#define VER_MARKER          "SDW"
#define VER_STATE           0x102
#define VER_INDEX           0x204

// Global flags
enum FLAG
{
    COLLECTION_FORCE_REINDEXING = 0x00000001,
    COLLECTION_USE_LZMA         = 0x00000002,
    COLLECTION_PRINT_INDEX      = 0x00000004,
    FLAG_NOGUI                  = 0x00000010,
    FLAG_CHECKUPDATES           = 0x00000020,
    FLAG_DISABLEINSTALL         = 0x00000040,
    FLAG_AUTOINSTALL            = 0x00000080,
    FLAG_FAILSAFE               = 0x00000100,
    FLAG_AUTOCLOSE              = 0x00000200,
    FLAG_NORESTOREPOINT         = 0x00000400,
    FLAG_NOLOGFILE              = 0x00000800,
    FLAG_NOSNAPSHOT             = 0x00001000,
    FLAG_NOSTAMP                = 0x00002000,
    FLAG_NOVIRUSALERTS          = 0x00004000,
    FLAG_PRESERVECFG            = 0x00008000,
    FLAG_EXTRACTONLY            = 0x00010000,
    FLAG_KEEPUNPACKINDEX        = 0x00020000,
    FLAG_KEEPTEMPFILES          = 0x00040000,
    FLAG_SHOWDRPNAMES1          = 0x00080000,
    FLAG_DPINSTMODE             = 0x00100000,
    FLAG_SHOWCONSOLE            = 0x00200000,
    FLAG_DELEXTRAINFS           = 0x00400000,
    FLAG_SHOWDRPNAMES2          = 0x00800000,
    FLAG_ONLYUPDATES            = 0x01000000,
    FLAG_AUTOUPDATE             = 0x02000000,
    FLAG_FILTERSP               = 0x04000000,
    FLAG_OLDSTYLE               = 0x08000000,
};

// Mode
enum STATEMODE
{
    STATEMODE_REAL      =0,
    STATEMODE_EMUL      =1,
    STATEMODE_EXIT      =2,
};

// Mode
enum INVALIDATE
{
    INVALIDATE_INDEXES  =1,
    INVALIDATE_SYSINFO  =2,
    INVALIDATE_DEVICES  =4,
    INVALIDATE_MANAGER  =8,
};

// Popup window
enum FLOATING_TYPE
{
    FLOATING_NONE       =0,
    FLOATING_TOOLTIP    =1,
    FLOATING_SYSINFO    =2,
    FLOATING_CMPDRIVER  =3,
    FLOATING_DRIVERLST  =4,
    FLOATING_ABOUT      =5,
    FLOATING_DOWNLOAD   =6,
};

// kb panels
enum KB_ID
{
    KB_NONE           =  0,
    KB_FIELD          =  1,
    KB_INSTALL        =  2,
    KB_LANG           =  3,
    KB_THEME          =  4,
    KB_EXPERT         =  5,
    KB_ACTIONS        =  6,
    KB_PANEL1         =  7,
    KB_PANEL2         =  8,
    KB_PANEL3         =  9,
    KB_PANEL_CHK      = 10,
};

// Mouse state
enum MOUSE_STATE
{
    MOUSE_NONE         = 0,
    MOUSE_CLICK        = 1,
    MOUSE_MOVE         = 2,
    MOUSE_SCROLL       = 3,
};

// Left panel types
enum panel_type
{
    TYPE_GROUP         = 1,
    TYPE_TEXT          = 2,
    TYPE_CHECKBOX      = 3,
    TYPE_BUTTON        = 4,
    TYPE_GROUP_BREAK   = 5,
};

// Messages
enum MessagesWND
{
    WM_BUNDLEREADY     = WM_APP+1,
    WM_UPDATELANG      = WM_APP+2,
    WM_UPDATETHEME     = WM_APP+3,
};

// Left panel IDs
enum install_mode
{
    MODE_NONE          = 0,
    MODE_INSTALLING    = 1,
    MODE_STOPPING      = 2,
    MODE_SCANNING      = 3,
};
//}

//{ Global variables

// Manager
extern Manager *manager_g;
extern int volatile installmode;
extern int ctrl_down;
extern int space_down;
extern int shift_down;
extern int invaidate_set;
extern int num_cores;

// Window
extern HINSTANCE ghInst;
extern int main1x_c,main1y_c;
extern int mainx_c,mainy_c;
extern CRITICAL_SECTION sync;
extern HFONT hFont;
extern HWND hPopup,hMain,hField,hLang,hTheme;

// Window helpers
extern int floating_type;
extern int floating_itembar;
extern int floating_x,floating_y;
extern int horiz_sh;
extern int hideconsole;
extern int offset_target;
extern int kbpanel,kbitem[KB_PANEL_CHK+1];
extern int ret_global;

// Settings
extern wchar_t drp_dir   [BUFLEN];
extern wchar_t drpext_dir[BUFLEN];
extern wchar_t index_dir [BUFLEN];
extern wchar_t data_dir  [BUFLEN];
extern wchar_t log_dir   [BUFLEN];

extern wchar_t state_file[BUFLEN];
extern wchar_t finish    [BUFLEN];
extern wchar_t finish_upd[BUFLEN];
extern wchar_t finish_rb [BUFLEN];
extern wchar_t HWIDs     [BUFLEN];

extern int flags;
extern int statemode;
extern int expertmode;
extern int hintdelay;
extern int filters;
extern int virtual_os_version;
extern int virtual_arch_type;

// Windows version
#define NUM_OS 8
extern const wchar_t *windows_name[NUM_OS];
extern int windows_ver[NUM_OS];
//}

// Settings
void settings_parse(const wchar_t *str,int ind);
void settings_save();
int  settings_load(const wchar_t *filename);

// Main
//int WINAPI WinMain(HINSTANCE hInst,HINSTANCE hinst,LPSTR pStr,int nCmd);
void gui(int nCmd);

//Bundle
class Bundle
{
    State state;
    Collection collection;
    Matcher matcher;

private:
    static unsigned int __stdcall thread_scandevices(void *arg);
    static unsigned int __stdcall thread_loadindexes(void *arg);
    static unsigned int __stdcall thread_getsysinfo(void *arg);

public:
    Matcher *getMatcher(){return &matcher;}

    static unsigned int __stdcall thread_loadall(void *arg);

    void bundle_init();
    void bundle_prep();
    void bundle_load(Bundle *pbundle);
    void bundle_lowprioirity();
};

// Subroutes
void CALLBACK drp_callback(const wchar_t *szFile,DWORD action,LPARAM lParam);
void lang_refresh();
void theme_refresh();
void snapshot();
void extractto();
void selectDrpDir();
void invaidate(int v);

// Scrollbar
void setscrollrange(int y);
int  getscrollpos();
void setscrollpos(int pos);

// Misc
void get_resource(int id,void **data,int *size);
void mkdir_r(const wchar_t *path);
void escapeAmpUrl(wchar_t *buf,const wchar_t *source);
void escapeAmp(wchar_t *buf,const wchar_t *source);

// GUI Helpers
HWND CreateWindowM(const wchar_t *type,const wchar_t *name,HWND hwnd,HMENU id);
HWND CreateWindowMF(const wchar_t *type,const wchar_t *name,HWND hwnd,HMENU id,DWORD f);
void GetRelativeCtrlRect(HWND hWnd,RECT *rc);
void checktimer(const wchar_t *str,long long t,int uMsg);
void redrawfield();
void redrawmainwnd();
void tabadvance(int v);
void arrowsAdvance(int v);

// GUI
LRESULT CALLBACK WndProcCommon(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
LRESULT CALLBACK WindowGraphProcedure(HWND,UINT,WPARAM,LPARAM);
LRESULT CALLBACK PopupProcedure(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK LicenseProcedure(HWND hwnd,UINT Message,WPARAM wParam,LPARAM lParam);
