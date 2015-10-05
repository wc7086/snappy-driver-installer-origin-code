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

#ifndef MAIN_H
#define MAIN_H

//{ Defines
class Manager;
class Font;
class Matcher;
class Hwidmatch;
class Canvas;

#include "resources.h"

// Misc
#define APPTITLE            L"Snappy Driver Installer v0.3"
#define VER_MARKER          "SDW"
#define VER_STATE           0x102
#define VER_INDEX           0x205

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

// IDs
enum GUI_ID
{
    ID_SHOW_MISSING     =1,
    ID_SHOW_NEWER       =2,
    ID_SHOW_CURRENT     =3,
    ID_SHOW_OLD         =4,
    ID_SHOW_BETTER      =5,
    ID_SHOW_WORSE_RANK  =6,

    ID_SHOW_NF_MISSING  =7,
    ID_SHOW_NF_UNKNOWN  =8,
    ID_SHOW_NF_STANDARD= 9,

    ID_SHOW_ONE        =10,
    ID_SHOW_DUP        =11,
    ID_SHOW_INVALID    =12,

    ID_INSTALL         =13,
    ID_SELECT_ALL      =14,
    ID_SELECT_NONE     =15,
    ID_EXPERT_MODE     =16,

    ID_LANG            =17,
    ID_THEME           =18,

    ID_OPENLOGS        =19,
    ID_SNAPSHOT        =20,
    ID_EXTRACT         =21,
    ID_DRVDIR          =22,

    ID_SCHEDULE        =23,
    ID_SHOWALT         =24,
    ID_OPENINF         =25,
    ID_LOCATEINF       =26,

    ID_EMU_32          =27,
    ID_EMU_64          =28,
    ID_DEVICEMNG       =29,
    ID_DIS_INSTALL     =30,
    ID_DIS_RESTPNT     =31,

    ID_WIN_2000        =32,
    ID_WIN_XP          =33,
    ID_WIN_VISTA       =34,
    ID_WIN_7           =35,
    ID_WIN_8           =36,
    ID_WIN_81          =37,
    ID_WIN_10          =38,

    ID_RESTPNT         =39,
    ID_REBOOT          =40,

    ID_URL0            =41,
    ID_URL1            =42,
    ID_URL2            =43,
    ID_URL3            =44,
    ID_URL4            =45,

    ID_HWID_CLIP      =100,
    ID_HWID_WEB       =200,
};

// fileter_show
enum fileter_show
{
    FILTER_SHOW_MISSING    = (1<<ID_SHOW_MISSING),
    FILTER_SHOW_NEWER      = (1<<ID_SHOW_NEWER),
    FILTER_SHOW_CURRENT    = (1<<ID_SHOW_CURRENT),
    FILTER_SHOW_OLD        = (1<<ID_SHOW_OLD),
    FILTER_SHOW_BETTER     = (1<<ID_SHOW_BETTER),
    FILTER_SHOW_WORSE_RANK = (1<<ID_SHOW_WORSE_RANK),

    FILTER_SHOW_NF_MISSING = (1<<ID_SHOW_NF_MISSING),
    FILTER_SHOW_NF_UNKNOWN = (1<<ID_SHOW_NF_UNKNOWN),
    FILTER_SHOW_NF_STANDARD= (1<<ID_SHOW_NF_STANDARD),

    FILTER_SHOW_ONE        = (1<<ID_SHOW_ONE),
    FILTER_SHOW_DUP        = (1<<ID_SHOW_DUP),
    FILTER_SHOW_INVALID    = (1<<ID_SHOW_INVALID),
};
//}

//{ Global variables

// Manager
extern Manager *manager_g;
extern int volatile installmode;
extern int invaidate_set;
extern int num_cores;
extern HINSTANCE ghInst;
extern CRITICAL_SECTION sync;
extern int ret_global;

// Window
class Popup_t
{
public:
    HWND hPopup;
    Font *hFontP,*hFontBold;
    Canvas *canvasPopup;

    Popup_t();
    ~Popup_t();

    int floating_type;
    int floating_itembar;
    int floating_x,floating_y;
    int horiz_sh;

    int PopupProcedure2(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);
};
LRESULT CALLBACK PopupProcedure(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);
extern class Popup_t Popup;

// Window
class MainWindow_t
{
public:
    int main1x_c,main1y_c;
    int mainx_c,mainy_c;
    Font *hFont;
    HWND hMain,hField,hLang,hTheme;
    int ctrl_down;
    int space_down;
    int shift_down;

    int hideconsole;
    int offset_target;
    int kbpanel,kbitem[KB_PANEL_CHK+1];

    int panel_lasti=0;
    int field_lasti,field_lastz;

    Canvas *canvasMain;
    Canvas *canvasField;
    static const wchar_t classMain[];
    static const wchar_t classField[];
    static const wchar_t classPopup[];

    int mousex=-1,mousey=-1,mousedown=MOUSE_NONE,mouseclick=0;
    int scrollvisible=0;

public:
    void gui(int nCmd);
    void lang_refresh();
    void theme_refresh();
    void redrawfield();
    void redrawmainwnd();

    void tabadvance(int v);
    void arrowsAdvance(int v);

    // Commands
    void snapshot();
    void extractto();
    void selectDrpDir();

    // Scrollbar
    void setscrollrange(int y);
    int  getscrollpos();
    void setscrollpos(int pos);

    static LRESULT CALLBACK WndProcCommon(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
    static LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
    static LRESULT CALLBACK WindowGraphProcedure(HWND,UINT,WPARAM,LPARAM);

    int WndProcCommon2(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
    int WndProc2(HWND,UINT,WPARAM,LPARAM);
    int WindowGraphProcedure2(HWND,UINT,WPARAM,LPARAM);

    MainWindow_t();
    ~MainWindow_t();
};
extern MainWindow_t MainWindow;

// Settings
class Settings_t
{
public:
    wchar_t curlang   [BUFLEN];
    wchar_t curtheme  [BUFLEN];
    wchar_t logO_dir  [BUFLEN];
    int license;

    wchar_t drp_dir   [BUFLEN];
    wchar_t output_dir[BUFLEN];
    wchar_t drpext_dir[BUFLEN];
    wchar_t index_dir [BUFLEN];
    wchar_t data_dir  [BUFLEN];
    wchar_t log_dir   [BUFLEN];

    wchar_t state_file[BUFLEN];
    wchar_t finish    [BUFLEN];
    wchar_t finish_upd[BUFLEN];
    wchar_t finish_rb [BUFLEN];

    int flags;
    int statemode;
    int expertmode;
    int hintdelay;
    int wndwx,wndwy;
    int filters;
    int virtual_os_version;
    int virtual_arch_type;

public:
    Settings_t();
    void parse(const wchar_t *str,int ind);
    int load(const wchar_t *filename);
    void save();
    void loginfo();

private:
    bool argstr(const wchar_t *s,const wchar_t *cmp,wchar_t *d);
    bool argint(const wchar_t *s,const wchar_t *cmp,int *d);
    bool argopt(const wchar_t *s,const wchar_t *cmp,int *d);
    bool argflg(const wchar_t *s,const wchar_t *cmp,int f);
};
extern Settings_t Settings;

// Windows version
#define NUM_OS 8
const wchar_t *getWindowsName(int a);
int getWindowsVer(int a);
//}

// Subroutes
void drp_callback(const wchar_t *szFile,int action,int lParam);
void invalidate(int v);

// Misc
void get_resource(int id,void **data,int *size);
void mkdir_r(const wchar_t *path);
void escapeAmpUrl(wchar_t *buf,const wchar_t *source);
void escapeAmp(wchar_t *buf,const wchar_t *source);

// GUI Helpers
HWND CreateWindowM(const wchar_t *type,const wchar_t *name,HWND hwnd,HMENU id);
HWND CreateWindowMF(const wchar_t *type,const wchar_t *name,HWND hwnd,HMENU id,DWORD f);
void GetRelativeCtrlRect(HWND hWnd,RECT *rc);
void setMirroring(HWND hwnd);
void checktimer(const wchar_t *str,long long t,int uMsg);

// GUI
BOOL CALLBACK LicenseProcedure(HWND hwnd,UINT Message,WPARAM wParam,LPARAM lParam);

//#include "debug_new.h"
void* operator new(size_t size, const char* file, int line);
void* operator new[](size_t size, const char* file, int line);
/*#define DEBUG_NEW new(__FILE__, __LINE__)
#define new DEBUG_NEW
namespace nvwa {extern size_t total_mem_alloc;}*/
//}

#endif
