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

#define BUFLEN              4096

//{ Includes
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <setupapi.h>
#include <shlwapi.h>
#include <shobjidl.h>
#include <shellapi.h>
#include <shlobj.h>
#include <winerror.h>
#include "SRRestorePtAPI.h"
typedef WINBOOL (__cdecl *WINAPI5t_SRSetRestorePointW)(PRESTOREPOINTINFOW pRestorePtSpec,PSTATEMGRSTATUS pSMgrStatus);

#include <stdio.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>
#include <process.h>
#include <direct.h>
#include <locale.h>
#include <vector>

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

#include <webp\decode.h>

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

//{ Defines

// Misc
#define APPTITLE            L"Snappy Driver Installer v0.2"
#define VER_MARKER          "SDW"
#define VER_STATE           0x102
#define VER_INDEX           0x204

// Mode
#define STATEMODE_LOAD      2
#define STATEMODE_EXIT      3

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
extern manager_t *manager_g;
extern int volatile installmode;
extern int driverpackpath;
extern CRITICAL_SECTION sync;
extern int ctrl_down;
extern int space_down;

// Window
extern HINSTANCE ghInst;
extern int main1x_c,main1y_c;
extern int mainx_c,mainy_c;
extern HFONT hFont;
extern HWND hPopup,hMain,hField;

// Window helpers
extern int floating_type;
extern int floating_itembar;
extern int floating_x,floating_y;
extern int horiz_sh;
extern int ret_global;
extern int offset_target;
extern int kbpanel,kbitem[KB_PANEL_CHK+1];

// Settings
extern WCHAR drp_dir   [BUFLEN];
extern WCHAR index_dir [BUFLEN];
extern WCHAR drpext_dir[BUFLEN];
extern WCHAR data_dir  [BUFLEN];
extern WCHAR log_dir   [BUFLEN];
extern WCHAR state_file[BUFLEN];
extern WCHAR finish    [BUFLEN];
extern WCHAR finish_upd[BUFLEN];
extern WCHAR finish_rb [BUFLEN];
extern WCHAR HWIDs     [BUFLEN];
extern int hintdelay;
extern int filters;
extern int flags;
extern int statemode;
extern int expertmode;
extern int virtual_os_version;
extern int virtual_arch_type;
//}

//{ Structs
typedef struct _bundle_t
{
    State state;
    Collection collection;
    matcher_t matcher;
}bundle_t;
//}

// Main
void settings_parse(const WCHAR *str,int ind);
void settings_save();
int  settings_load(const WCHAR *filename);
void SignalHandler(int signum);
void CALLBACK drp_callback(const WCHAR *szFile,DWORD action,LPARAM lParam);
void checkupdates();
//int WINAPI WinMain(HINSTANCE hInst,HINSTANCE hinst,LPSTR pStr,int nCmd);

// Threads
unsigned int __stdcall thread_scandevices(void *arg);
unsigned int __stdcall thread_loadindexes(void *arg);
unsigned int __stdcall thread_getsysinfo(void *arg);
unsigned int __stdcall thread_loadall(void *arg);

// Bundle
void bundle_init(bundle_t *bundle);
void bundle_prep(bundle_t *bundle);
void bundle_free(bundle_t *bundle);
void bundle_load(bundle_t *bundle);
void bundle_lowprioirity(bundle_t *bundle);

// Windows
HWND CreateWindowM(const WCHAR *type,const WCHAR *name,HWND hwnd,HMENU id);
HWND CreateWindowMF(const WCHAR *type,const WCHAR *name,HWND hwnd,HMENU id,DWORD f);
void setfont();
void redrawfield();
void redrawmainwnd();
void lang_refresh();
void theme_refresh();
void setscrollrange(int y);
int  getscrollpos();
void setscrollpos(int pos);

// Helpers
void get_resource(int id,void **data,int *size);
const WCHAR *get_winverstr(manager_t *manager);
void mkdir_r(const WCHAR *path);
void snapshot();
void extractto();

// GUI
void tabadvance(int v);
void gui(int nCmd);
void checktimer(const WCHAR *str,long long t,int uMsg);
LRESULT CALLBACK WndProcCommon(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
LRESULT CALLBACK WindowGraphProcedure(HWND,UINT,WPARAM,LPARAM);
LRESULT CALLBACK PopupProcedure(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);
void GetRelativeCtrlRect(HWND hWnd,RECT *rc);
BOOL CALLBACK LicenseProcedure(HWND hwnd,UINT Message,WPARAM wParam,LPARAM lParam);

//new
void str_unicode2ansi(char *a);
void set_rstpnt(int checked);
void drvdir();
const WCHAR *getHWIDby(int id,int num);
void escapeAmpUrl(WCHAR *buf,WCHAR *source);
void escapeAmp(WCHAR *buf,WCHAR *source);
void contextmenu3(int x,int y);
void contextmenu2(int x,int y);
void contextmenu(int x,int y);
