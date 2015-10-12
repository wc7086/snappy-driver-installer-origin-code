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
#include "common.h"
#include "matcher.h"
#include "settings.h"
#include "cli.h"
#include "indexing.h"
#include "manager.h"
#include "update.h"
#include "install.h"
#include "gui.h"

#include <windows.h>
#include <setupapi.h>       // for CommandLineToArgvW
#ifdef _MSC_VER
#include <shellapi.h>
#include <process.h>
#include <signal.h>
#endif

#include "system.h"
#include "enum.h"
#include "main.h"
#include "model.h"
#include "draw.h"
#include "theme.h"

//{ Global variables

Manager manager_v[2];
Manager *manager_g=&manager_v[0];
Console_t *Console;

volatile int deviceupdate_exitflag=0;
Event *deviceupdate_event;
HINSTANCE ghInst;
CRITICAL_SECTION sync;
int manager_active=0;
int bundle_display=1;
int bundle_shadow=0;

// Windows name
const wchar_t *getWindowsName(int a)
{
    static const wchar_t *windows_name[NUM_OS]=
    {
        L"Windows 2000",
        L"Windows XP",
        L"Windows Vista",
        L"Windows 7",
        L"Windows 8",
        L"Windows 8.1",
        L"Windows 10",
        L"Unknown OS"
    };

    return windows_name[a];
}

int getWindowsVer(int a)
{
    static int windows_ver[NUM_OS]={50,51,60,61,62,63,100,0};

    return windows_ver[a];
}
//}

//{ Objects
Popup_t Popup;
MainWindow_t MainWindow;
Settings_t Settings;
//}

class Console1:public Console_t
{
public:
    Console1()
    {
        DWORD dwProcessId;
        GetWindowThreadProcessId(GetConsoleWindow(),&dwProcessId);
        if(GetCurrentProcessId()!=dwProcessId)MainWindow.hideconsole=SW_SHOWNOACTIVATE;
        ShowWindow(GetConsoleWindow(),MainWindow.hideconsole);
    }
    ~Console1()
    {
        ShowWindow(GetConsoleWindow(),1);
    }
    void Show()
    {
        ShowWindow(GetConsoleWindow(),1);
    }
    void Hide()
    {
        ShowWindow(GetConsoleWindow(),0);
    }
};

class Console2:public Console_t
{
public:
    ~Console2()
    {
        FreeConsole();
    }
    void Show()
    {
        AllocConsole();
        freopen("CONIN$","r",stdin);
        freopen("CONOUT$","w",stdout);
        freopen("CONOUT$","w",stderr);
    }
    void Hide()
    {
        FreeConsole();
    }
};

//{  Main
int WINAPI WinMain(HINSTANCE hInst,HINSTANCE hinst,LPSTR pStr,int nCmd)
{
    UNREFERENCED_PARAMETER(hinst);
    UNREFERENCED_PARAMETER(pStr);
    ghInst=hInst;

    Timers.start(time_total);

    // Hide the console window as soon as possible
#ifdef _MSC_VER
    Console=new Console2;
#else
    Console=new Console1;
#endif

    // Determine number of CPU cores
    SYSTEM_INFO siSysInfo;
    GetSystemInfo(&siSysInfo);
    num_cores=siSysInfo.dwNumberOfProcessors;

    // 7-zip
    registerall();

    // Check if the mouse present
    if(!GetSystemMetrics(SM_MOUSEPRESENT))MainWindow.kbpanel=KB_FIELD;

    // Runtime error handlers
    start_exception_hadnlers();
    HMODULE backtrace=LoadLibraryA("backtrace.dll");
    if(!backtrace)signal(SIGSEGV,SignalHandler);

    // Load settings
    init_CLIParam();
    if(!Settings.load_cfg_switch(GetCommandLineW()))
    if(!Settings.load(L"sdi.cfg"))
        Settings.load(L"tools\\SDI\\settings.cfg");

    Settings.parse(GetCommandLineW(),1);
    RUN_CLI();

    // Reset paths for GUI-less version of the app
    #ifdef CONSOLE_MODE
    Settings.flags|=FLAG_NOGUI;
    Settings.license=1;
    wcscpy(Settings.drp_dir,Settings.log_dir);
    wcscpy(Settings.index_dir,Settings.log_dir);
    wcscpy(Settings.output_dir,Settings.log_dir);
    #endif

    // Close the app if the work is done
    if(Settings.statemode==STATEMODE_EXIT)
    {
        if(backtrace)FreeLibrary(backtrace);
        delete Console;
        return ret_global;
    }

    // Bring back the console window
    #ifndef CONSOLE_MODE
    if((Settings.expertmode&&Settings.flags&FLAG_SHOWCONSOLE)?SW_SHOWNOACTIVATE:MainWindow.hideconsole)
        Console->Show();
    else
        Console->Hide();
    #endif

    // Start logging
    ExpandEnvironmentStrings(Settings.logO_dir,Settings.log_dir,BUFLEN);
    Log.start(Settings.log_dir);
    Settings.loginfo();
    #ifndef NDEBUG
    Log.print_con("Debug info present\n");
    if(backtrace)Log.print_con("Backtrace is loaded\n");
    #endif

    #ifdef BENCH_MODE
    System.benchmark();
    #endif

    // Make dirs
    mkdir_r(Settings.drp_dir);
    mkdir_r(Settings.index_dir);
    mkdir_r(Settings.output_dir);

    // Load text
    vLang=CreateVaultLang(language,STR_NM,IDR_LANG);
    vTheme=CreateVaultTheme(theme,THEME_NM,IDR_THEME);
    vLang->load(0);

    // Allocate resources
    Bundle bundle[2];
    manager_v[0].init(bundle[bundle_display].getMatcher());
    manager_v[1].init(bundle[bundle_display].getMatcher());
    deviceupdate_event=CreateEvent();

    // Start device/driver scan
    bundle[bundle_display].bundle_prep();
    invalidate(INVALIDATE_DEVICES|INVALIDATE_SYSINFO|INVALIDATE_INDEXES|INVALIDATE_MANAGER);
    HANDLE thr=(HANDLE)_beginthreadex(nullptr,0,&Bundle::thread_loadall,&bundle[0],0,nullptr);

    // Check updates
    #ifdef USE_TORRENT
    Updater=CreateUpdater();
    #endif

    // Start folder monitors
    Filemon *mon_drp=CreateFilemon(Settings.drp_dir,FILE_NOTIFY_CHANGE_LAST_WRITE|FILE_NOTIFY_CHANGE_FILE_NAME,1,drp_callback);
    virusmonitor_start();
    viruscheck(L"",0,0);

    // MAIN GUI LOOP
    MainWindow.gui(nCmd);

    // Wait till the device scan thread is finished
    if(MainWindow.hMain)deviceupdate_exitflag=1;
    deviceupdate_event->raise();
    WaitForSingleObject(thr,INFINITE);
    System.CloseHandle_log(thr,L"WinMain",L"thr");
    delete deviceupdate_event;

    // Stop libtorrent
    #ifdef USE_TORRENT
    delete Updater;
    #endif

    // Free allocated resources
    int i;
    for(i=0;i<BOX_NUM;i++)box[i].Release();
    for(i=0;i<ICON_NUM;i++)icon[i].Release();
    delete vLang;
    delete vTheme;

    // Save settings
    #ifndef CONSOLE_MODE
    Settings.save();
    #endif

    // Stop folder monitors
    delete mon_drp;
    virusmonitor_stop();

    // Bring the console window back
    ShowWindow(GetConsoleWindow(),SW_SHOWNOACTIVATE);

    // Stop runtime error handlers
    /*if(backtrace)
        FreeLibrary(backtrace);
    else
        signal(SIGSEGV,SIG_DFL);*/

    // Stop logging
    //time_total=GetTickCount()-time_total;
    Timers.print();
    Log.stop();
    delete Console;

    // Exit
    return ret_global;
}

void MainWindow_t::gui(int nCmd)
{
    if((Settings.flags&FLAG_NOGUI)&&(Settings.flags&FLAG_AUTOINSTALL)==0)return;

    // Register classMain
    WNDCLASSEX wcx;
    memset(&wcx,0,sizeof(WNDCLASSEX));
    wcx.cbSize=         sizeof(WNDCLASSEX);
    wcx.lpfnWndProc=    WndProc;
    wcx.hInstance=      ghInst;
    wcx.hIcon=          LoadIcon(ghInst,MAKEINTRESOURCE(IDI_ICON1));
    wcx.hCursor=        LoadCursor(nullptr,IDC_ARROW);
    wcx.lpszClassName=  classMain;
    wcx.hbrBackground=  (HBRUSH)(COLOR_WINDOW+1);
    if(!RegisterClassEx(&wcx))
    {
        Log.print_err("ERROR in gui(): failed to register '%S' class\n",wcx.lpszClassName);
        return;
    }

    // Register classPopup
    wcx.lpfnWndProc=PopupProcedure;
    wcx.lpszClassName=classPopup;
    wcx.hIcon=nullptr;
    if(!RegisterClassEx(&wcx))
    {
        Log.print_err("ERROR in gui(): failed to register '%S' class\n",wcx.lpszClassName);
        System.UnregisterClass_log(classMain,ghInst,L"gui",L"classMain");
        return;
    }

    // Register classField
    wcx.lpfnWndProc=WindowGraphProcedure;
    wcx.lpszClassName=classField;
    if(!RegisterClassEx(&wcx))
    {
        Log.print_err("ERROR in gui(): failed to register '%S' class\n",wcx.lpszClassName);
        System.UnregisterClass_log(classMain,ghInst,L"gui",L"classMain");
        System.UnregisterClass_log(classPopup,ghInst,L"gui",L"classPopup");
        return;
    }

    // Main windows
    hMain=CreateWindowEx(WS_EX_LAYERED,
                        classMain,
                        APPTITLE,
                        WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,
                        CW_USEDEFAULT,CW_USEDEFAULT,D(MAINWND_WX),D(MAINWND_WY),
                        nullptr,nullptr,ghInst,nullptr);
    if(!hMain)
    {
        Log.print_err("ERROR in gui(): failed to create '%S' window\n",classMain);
        return;
    }

    // license dialog
    if(!Settings.license)
        DialogBox(ghInst,MAKEINTRESOURCE(IDD_DIALOG1),0,(DLGPROC)LicenseProcedure);

    // Enable updates notifications
    if(Settings.license==2)
    {
        /*int f;
        f=lang_enum(hLang,L"langs",manager_g->matcher->state->locale);
        Log.print_con("lang %d\n",f);
        lang_set(f);*/

        //if(MessageBox(0,STR(STR_UPD_DIALOG_MSG),STR(STR_UPD_DIALOG_TITLE),MB_YESNO|MB_ICONQUESTION)==IDYES)
        {
            Settings.flags|=FLAG_CHECKUPDATES;
            #ifdef USE_TORRENT
            Updater->checkUpdates();
            #endif
            invalidate(INVALIDATE_MANAGER);
        }
    }

    if(Settings.license)
    {
        //time_test=GetTickCount()-time_total;log_times();
        ShowWindow(hMain,(Settings.flags&FLAG_NOGUI)?SW_HIDE:nCmd);

        int done=0;
        while(!done)
        {
            while(WAIT_IO_COMPLETION==MsgWaitForMultipleObjectsEx(0,nullptr,INFINITE,QS_ALLINPUT,MWMO_ALERTABLE));

            MSG msg;
            while(PeekMessage(&msg,nullptr,0,0,PM_REMOVE))
            {
                if(msg.message==WM_QUIT)
                {
                    done=TRUE;
                    break;
                }else
                if(msg.message==WM_KEYDOWN)
                {
                    if(!(msg.lParam&(1<<30)))
                    {
                        if(msg.wParam==VK_CONTROL||msg.wParam==VK_SPACE)
                        {
                            POINT p;
                            GetCursorPos(&p);
                            SetCursorPos(p.x+1,p.y);
                            SetCursorPos(p.x,p.y);
                        }
                        if(msg.wParam==VK_CONTROL)ctrl_down=1;
                        if(msg.wParam==VK_SPACE)  space_down=1;
                        if(msg.wParam==VK_SHIFT||msg.wParam==VK_LSHIFT||msg.wParam==VK_RSHIFT)  space_down=shift_down=1;
                    }
                    if(msg.wParam==VK_SPACE&&kbpanel)
                    {
                        if(kbpanel==KB_FIELD)
                        {
                            SendMessage(hField,WM_LBUTTONDOWN,0,0);
                            SendMessage(hField,WM_LBUTTONUP,0,0);
                        }
                        else
                        {
                            SendMessage(hMain,WM_LBUTTONDOWN,0,0);
                            SendMessage(hMain,WM_LBUTTONUP,0,0);
                        }
                    }
                    if((msg.wParam==VK_LEFT||msg.wParam==VK_RIGHT)&&kbpanel==KB_INSTALL)
                    {
                        arrowsAdvance(msg.wParam==VK_LEFT?-1:1);
                    }
                    if((msg.wParam==VK_LEFT)&&kbpanel==KB_FIELD)
                    {
                        int index,nop;
                        manager_g->hitscan(0,0,&index,&nop);
                        manager_g->expand(index,EXPAND_MODE::COLLAPSE);
                    }
                    if((msg.wParam==VK_RIGHT)&&kbpanel==KB_FIELD)
                    {
                        int index,nop;
                        manager_g->hitscan(0,0,&index,&nop);
                        manager_g->expand(index,EXPAND_MODE::EXPAND);
                    }
                    if(msg.wParam==VK_UP)arrowsAdvance(-1);else
                    if(msg.wParam==VK_DOWN)arrowsAdvance(1);

                    if(msg.wParam==VK_TAB&&shift_down)
                    {
                        tabadvance(-1);
                    }
                    if(msg.wParam==VK_TAB&&!shift_down)
                    {
                        tabadvance(1);
                    }
                }else
                if(msg.message==WM_KEYUP)
                {
                    if(msg.wParam==VK_CONTROL||msg.wParam==VK_SPACE)
                    {
                        drawpopup(-1,FLOATING_NONE,0,0,hField);
                    }
                    if(msg.wParam==VK_CONTROL)ctrl_down=0;
                    if(msg.wParam==VK_SPACE)  space_down=0;
                    if(msg.wParam==VK_SHIFT||msg.wParam==VK_LSHIFT||msg.wParam==VK_RSHIFT)  space_down=shift_down=0;
                }

                if(!(msg.message==WM_SYSKEYDOWN&&msg.wParam==VK_MENU))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        }
    }

    System.UnregisterClass_log(classMain,ghInst,L"gui",L"classMain");
    System.UnregisterClass_log(classPopup,ghInst,L"gui",L"classPopup");
    System.UnregisterClass_log(classField,ghInst,L"gui",L"classField");
}
//}

//{ Subroutes
void drp_callback(const wchar_t *szFile,int action,int lParam)
{
    UNREFERENCED_PARAMETER(action);
    UNREFERENCED_PARAMETER(lParam);

    if(StrStrIW(szFile,L".7z")&&Updater->isPaused())invalidate(INVALIDATE_INDEXES);
}

const wchar_t MainWindow_t::classMain[]= L"classSDIMain";
const wchar_t MainWindow_t::classField[]=L"classSDIField";
const wchar_t MainWindow_t::classPopup[]=L"classSDIPopup";
MainWindow_t::MainWindow_t()
{
    hFont=new Font;

    Popup.floating_itembar=-1;
    Popup.floating_x=1;
    Popup.floating_y=1;
    hideconsole=SW_HIDE;

    mousex=-1;
    mousey=-1;
    mousedown=MOUSE_NONE;
    kbpanel=KB_NONE;
}

MainWindow_t::~MainWindow_t()
{
    delete hFont;
}

void MainWindow_t::lang_refresh()
{
    if(!hMain||!hField)
    {
        Log.print_err("ERROR in lang_refresh(): hMain is %d, hField is %d\n",hMain,hField);
        return;
    }

    rtl=language[STR_RTL].val;
    setMirroring(hMain);
    setMirroring(hLang);
    setMirroring(hTheme);
    setMirroring(hField);
    setMirroring(Popup.hPopup);

    RECT rect;
    GetWindowRect(hMain,&rect);
    MoveWindow(hMain,rect.left,rect.top,D(MAINWND_WX),D(MAINWND_WY)+1,1);
    MoveWindow(hMain,rect.left,rect.top,D(MAINWND_WX),D(MAINWND_WY),1);
    InvalidateRect(Popup.hPopup,nullptr,0);
}

void MainWindow_t::theme_refresh()
{
    hFont->SetFont(D_STR(FONT_NAME),D(FONT_SIZE));
    Popup.hFontP->SetFont(D_STR(FONT_NAME),D(POPUP_FONT_SIZE));
    Popup.hFontBold->SetFont(D_STR(FONT_NAME),D(POPUP_FONT_SIZE),true);

    SendMessage(hTheme,WM_SETFONT,(WPARAM)hFont->get(),MAKELPARAM(FALSE,0));
    SendMessage(hLang,WM_SETFONT,(WPARAM)hFont->get(),MAKELPARAM(FALSE,0));

    if(!hMain||!hField)
    {
        Log.print_err("ERROR in theme_refresh(): hMain is %d, hField is %d\n",hMain,hField);
        return;
    }

    // Resize window
    RECT rect;
    GetWindowRect(hMain,&rect);
    MoveWindow(hMain,rect.left,rect.top,D(MAINWND_WX),D(MAINWND_WY)+1,1);
    MoveWindow(hMain,rect.left,rect.top,D(MAINWND_WX),D(MAINWND_WY),1);
}

void MainWindow_t::snapshot()
{
    if(System.ChooseFile(Settings.state_file,STR(STR_OPENSNAPSHOT),L"snp"))
    {
        Settings.statemode=STATEMODE_EMUL;
        invalidate(INVALIDATE_DEVICES|INVALIDATE_SYSINFO|INVALIDATE_MANAGER);
    }
}

void MainWindow_t::extractto()
{
    wchar_t dir[BUFLEN];

    if(System.ChooseDir(dir,STR(STR_EXTRACTFOLDER)))
    {
        int argc;
        wchar_t buf[BUFLEN];
        wchar_t **argv=CommandLineToArgvW(GetCommandLineW(),&argc);
        wsprintf(buf,L"%s\\drv.exe",dir);
        if(!CopyFile(argv[0],buf,0))
            Log.print_err("ERROR in extractto(): failed CopyFile(%S,%S)\n",argv[0],buf);
        LocalFree(argv);

        wcscat(dir,L"\\drivers");
        wcscpy(extractdir,dir);
        manager_g->install(OPENFOLDER);
    }
}

void MainWindow_t::selectDrpDir()
{
    if(System.ChooseDir(Settings.drpext_dir,STR(STR_DRVDIR)))
    {
        invalidate(INVALIDATE_INDEXES|INVALIDATE_MANAGER);
    }
}

void invalidate(int v)
{
    invaidate_set|=v;
    deviceupdate_event->raise();
}
//}

//{ Scrollbar
void MainWindow_t::setscrollrange(int y)
{
    if(!hField)
    {
        Log.print_err("ERROR in setscrollrange(): hField is 0\n");
        return;
    }

    RECT rect;
    GetClientRect(hField,&rect);

    SCROLLINFO si;
    si.cbSize=sizeof(si);
    si.fMask =SIF_RANGE|SIF_PAGE;
    si.nMin  =0;
    si.nMax  =y;
    si.nPage =rect.bottom;
    scrollvisible=rect.bottom>y;
    SetScrollInfo(hField,SB_VERT,&si,TRUE);
}

int MainWindow_t::getscrollpos()
{
    if(!hField)
    {
        Log.print_err("ERROR in getscrollpos(): hField is 0\n");
        return 0;
    }

    SCROLLINFO si;
    si.cbSize=sizeof(si);
    si.fMask=SIF_POS;
    si.nPos=0;
    GetScrollInfo(hField,SB_VERT,&si);
    return si.nPos;
}

void MainWindow_t::setscrollpos(int pos)
{
    if(!hField)
    {
        Log.print_err("ERROR in setscrollpos(): hField is 0\n");
        return;
    }

    SCROLLINFO si;
    si.cbSize=sizeof(si);
    si.fMask=SIF_POS;
    si.nPos=pos;
    SetScrollInfo(hField,SB_VERT,&si,TRUE);
}
//}

//{ Misc

void escapeAmpUrl(wchar_t *buf,const wchar_t *source)
{
    while(*source)
    {
        *buf=*source;
        if(*buf==L'&')
        {
            *buf++=L'%';
            *buf++=L'2';
            *buf=L'6';
        }
        if(*buf==L'\\')
        {
            *buf++=L'%';
            *buf++=L'5';
            *buf=L'C';
        }
        buf++;source++;
    }
    *buf=0;
}

void escapeAmp(wchar_t *buf,const wchar_t *source)
{
    while(*source)
    {
        *buf=*source;
        if(*buf==L'&')*(++buf)=L'&';
        buf++;source++;
    }
    *buf=0;
}
//}

//{ GUI Helpers
HWND CreateWindowM(const wchar_t *type,const wchar_t *name,HWND hwnd,HMENU id)
{
    return CreateWindow(type,name,WS_CHILD|WS_VISIBLE,0,0,0,0,hwnd,id,ghInst,NULL);
}

HWND CreateWindowMF(const wchar_t *type,const wchar_t *name,HWND hwnd,HMENU id,DWORD f)
{
    return CreateWindow(type,name,WS_CHILD|WS_VISIBLE|f,0,0,0,0,hwnd,id,ghInst,NULL);
}

void GetRelativeCtrlRect(HWND hWnd,RECT *rc)
{
    GetWindowRect(hWnd,rc);
    ScreenToClient(GetParent(hWnd),(LPPOINT)&((LPPOINT)rc)[0]);
    ScreenToClient(GetParent(hWnd),(LPPOINT)&((LPPOINT)rc)[1]);
    //MapWindowPoints(nullptr,hWnd,(LPPOINT)&rc,2);
    rc->right-=rc->left;
    rc->bottom-=rc->top;
}

void setMirroring(HWND hwnd)
{
    LONG v=GetWindowLong(hwnd,GWL_EXSTYLE);
    if(rtl)v|=WS_EX_LAYOUTRTL;else v&=~WS_EX_LAYOUTRTL;
    SetWindowLong(hwnd,GWL_EXSTYLE,v);
}

void checktimer(const wchar_t *str,long long t,int uMsg)
{
    if(GetTickCount()-t>20&&Log.isAllowed(LOG_VERBOSE_LAGCOUNTER))
        Log.print_con("GUI lag in %S[%X]: %ld\n",str,uMsg,GetTickCount()-t);
}

void MainWindow_t::redrawfield()
{
    if(Settings.flags&FLAG_NOGUI)return;
    if(!hField)
    {
        Log.print_err("ERROR in redrawfield(): hField is 0\n");
        return;
    }
    InvalidateRect(hField,nullptr,0);
}

void MainWindow_t::redrawmainwnd()
{
    if(Settings.flags&FLAG_NOGUI)return;
    if(!hMain)
    {
        Log.print_err("ERROR in redrawmainwnd(): hMain is 0\n");
        return;
    }
    InvalidateRect(hMain,nullptr,0);
}

void MainWindow_t::tabadvance(int v)
{
    while(1)
    {
        kbpanel+=v;
        if(!kbpanel)kbpanel=KB_PANEL_CHK;
        if(kbpanel>KB_PANEL_CHK)kbpanel=KB_FIELD;

        if(kbpanel==KB_PANEL_CHK&&!D(PANEL12_WY))continue;
        if(!Settings.expertmode&&kbpanel>=KB_ACTIONS&&kbpanel<=KB_PANEL3)continue;
        break;
    }

    if(kbpanel==KB_LANG)
        SetFocus(hLang);
    else if(kbpanel==KB_THEME)
        SetFocus(hTheme);
    else
        SetFocus(hMain);

    redrawfield();
    redrawmainwnd();
}

extern int setaa;
void MainWindow_t::arrowsAdvance(int v)
{
    if(!kbpanel)return;

    if(kbpanel==KB_INSTALL)
    {
        kbitem[kbpanel]+=v;
        if(kbitem[kbpanel]<0)kbitem[kbpanel]=2;
        redrawmainwnd();
        return;
    }
    if(kbpanel==KB_FIELD)
    {
        kbitem[kbpanel]+=v;
        setaa=1;
        redrawfield();
        return;
    }

    //for(int i=0;i<NUM_PANELS;i++)panels[i].keybAdvance(v);
    redrawmainwnd();
}
//}

//{ GUI
LRESULT CALLBACK MainWindow_t::WndProcCommon(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    return MainWindow.WndProcCommon2(hwnd,uMsg,wParam,lParam);
}

int MainWindow_t::WndProcCommon2(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);

    RECT rect;
    short x,y;

    x=LOWORD(lParam);
    y=HIWORD(lParam);
    switch(uMsg)
    {
        case WM_MOUSELEAVE:
            ShowWindow(Popup.hPopup,SW_HIDE);
            InvalidateRect(hwnd,nullptr,0);
            break;

        case WM_MOUSEHOVER:
            InvalidateRect(Popup.hPopup,nullptr,0);
            ShowWindow(Popup.hPopup,Popup.floating_type==FLOATING_NONE?SW_HIDE:SW_SHOWNOACTIVATE);
            break;

        case WM_ACTIVATE:
            InvalidateRect(hwnd,nullptr,0);
            break;

        case WM_MOUSEMOVE:
            if(mousedown==MOUSE_CLICK||mousedown==MOUSE_MOVE)
            {
                GetWindowRect(hMain,&rect);
                if(mousedown==MOUSE_MOVE||abs(mousex-x)>2||abs(mousey-y)>2)
                {
                    mousedown=MOUSE_MOVE;
                    MoveWindow(hMain,rect.left+(x-mousex)*(rtl?-1:1),rect.top+y-mousey,
                               rect.right-rect.left,rect.bottom-rect.top,1);
                }
            }
            return 1;

        case WM_LBUTTONDOWN:
            if(kbpanel&&x&&y)
            {
                kbpanel=KB_NONE;
                redrawmainwnd();
            }
            SetFocus(hMain);
            if(!IsZoomed(hMain))
            {
                mousex=x;
                mousey=y;
                mousedown=MOUSE_CLICK;
                SetCapture(hwnd);
            }
            break;

        case WM_CANCELMODE:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
            mousex=-1;
            mousey=-1;
            SetCursor(LoadCursor(nullptr,IDC_ARROW));
            ReleaseCapture();
            mouseclick=uMsg==WM_LBUTTONUP&&mousedown!=MOUSE_MOVE?1:0;
            mousedown=MOUSE_NONE;
            return 1;

        default:
            return 1;
    }
    return 0;
}

LRESULT CALLBACK MainWindow_t::WndProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    return MainWindow.WndProc2(hwnd,uMsg,wParam,lParam);
}

LRESULT MainWindow_t::WndProc2(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    RECT rect;
    short x,y;

    int i;
	LRESULT f;
    int wp;
    long long timer=GetTickCount();

    x=LOWORD(lParam);
    y=HIWORD(lParam);

    if(WndProcCommon2(hwnd,uMsg,wParam,lParam))
    switch(uMsg)
    {
        case WM_CREATE:
            // Canvas
            canvasMain=new Canvas;
            hMain=hwnd;

            // Field
            hField=CreateWindowMF(classField,nullptr,hwnd,nullptr,WS_VSCROLL);

            // Popup
            Popup.hPopup=CreateWindowEx(WS_EX_LAYERED|WS_EX_NOACTIVATE|WS_EX_TOPMOST|WS_EX_TRANSPARENT,
                classPopup,L"",WS_POPUP,
                0,0,0,0,hwnd,(HMENU)nullptr,ghInst,nullptr);

            // Lang
            hLang=CreateWindowMF(WC_COMBOBOX,L"",hwnd,(HMENU)ID_LANG,CBS_DROPDOWNLIST|CBS_HASSTRINGS|WS_OVERLAPPED|WS_VSCROLL);
            PostMessage(hwnd,WM_UPDATELANG,0,0);

            // Theme
            hTheme=CreateWindowMF(WC_COMBOBOX,L"",hwnd,(HMENU)ID_THEME,CBS_DROPDOWNLIST|CBS_HASSTRINGS|WS_OVERLAPPED|WS_VSCROLL);
            PostMessage(hwnd,WM_UPDATETHEME,1,0);

            // Misc
            vLang->startmonitor();
            vTheme->startmonitor();
            DragAcceptFiles(hwnd,1);

            manager_g->populate();
            manager_g->filter(Settings.filters);
            manager_g->setpos();
            break;

        case WM_CLOSE:
            if(installmode==MODE_NONE||(Settings.flags&FLAG_AUTOCLOSE))
                DestroyWindow(hwnd);
            else if(MessageBox(hMain,STR(STR_INST_QUIT_MSG),STR(STR_INST_QUIT_TITLE),MB_YESNO|MB_ICONQUESTION)==IDYES)
            {
                installmode=MODE_STOPPING;
                #ifdef USE_TORRENT
                Updater->pause();
                #endif
            }
            break;

        case WM_DESTROY:
            GetWindowRect(hwnd,&rect);
            Settings.wndwx=rect.right-rect.left;
            Settings.wndwy=rect.bottom-rect.top;

            vLang->stopmonitor();
            vTheme->stopmonitor();
            delete canvasMain;
            PostQuitMessage(0);
            break;

        case WM_UPDATELANG:
            SendMessage(hLang,CB_RESETCONTENT,0,0);
            vLang->enumfiles(hLang,L"langs",manager_g->matcher->getState()->getLocale());
            f=SendMessage(hLang,CB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)Settings.curlang);
            if(f==CB_ERR)f=SendMessage(hLang,CB_GETCOUNT,0,0)-1;
            vLang->switchdata((int)f);
            SendMessage(hLang,CB_SETCURSEL,f,0);
            lang_refresh();
            break;

        case WM_UPDATETHEME:
            SendMessage(hTheme,CB_RESETCONTENT,0,0);
            vTheme->enumfiles(hTheme,L"themes");
            f=SendMessage(hTheme,CB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)Settings.curtheme);
            if(f==CB_ERR)f=vTheme->pickTheme();
			vTheme->switchdata((int)f);
            if(Settings.wndwx)D(MAINWND_WX)=Settings.wndwx;
            if(Settings.wndwy)D(MAINWND_WY)=Settings.wndwy;
            SendMessage(hTheme,CB_SETCURSEL,f,0);
            theme_refresh();

            // Move to the center of the screen
            if(!wParam)break;
            GetWindowRect(GetDesktopWindow(),&rect);
            rect.left=(rect.right-D(MAINWND_WX))/2;
            rect.top=(rect.bottom-D(MAINWND_WY))/2;
            MoveWindow(hwnd,rect.left,rect.top,D(MAINWND_WX),D(MAINWND_WY),1);
            break;

        case WM_BUNDLEREADY:
            {
                Bundle *bb=reinterpret_cast<Bundle *>(wParam);
                Manager *manager_prev=manager_g;
                Log.print_con("{Sync");
                EnterCriticalSection(&sync);
                Log.print_con("...\n");
                manager_active++;
                manager_active&=1;
                manager_g=&manager_v[manager_active];
                manager_g->matcher=bb->getMatcher();
                manager_g->restorepos1(manager_prev);
            }
            break;

        case WM_DROPFILES:
            {
                wchar_t lpszFile[MAX_PATH]={0};
                UINT uFile=0;
                HDROP hDrop=(HDROP)wParam;

                uFile=DragQueryFile(hDrop,0xFFFFFFFF,nullptr,0);
                if(uFile!=1)
                {
                    //MessageBox(0,L"Dropping multiple files is not supported.",NULL,MB_ICONERROR);
                    DragFinish(hDrop);
                    break;
                }

                lpszFile[0] = '\0';
                if(DragQueryFile(hDrop,0,lpszFile,MAX_PATH))
                {
                    uFile=GetFileAttributes(lpszFile);
                    if(uFile!=INVALID_FILE_ATTRIBUTES&&uFile&FILE_ATTRIBUTE_DIRECTORY)
                    {
                        wcscpy(Settings.drpext_dir,lpszFile);
                        invalidate(INVALIDATE_INDEXES|INVALIDATE_MANAGER);
                    }
                    else if(StrStrIW(lpszFile,L".snp"))
                    {
                        wcscpy(Settings.state_file,lpszFile);
                        Settings.statemode=STATEMODE_EMUL;
                        invalidate(INVALIDATE_DEVICES|INVALIDATE_SYSINFO|INVALIDATE_MANAGER);
                    }
                    //else
                    //    MessageBox(NULL,lpszFile,NULL,MB_ICONINFORMATION);
                }
                DragFinish(hDrop);
            }
            break;

        case WM_WINDOWPOSCHANGING:
            {
                WINDOWPOS *wpos=(WINDOWPOS*)lParam;

                SystemParametersInfo(SPI_GETWORKAREA,0,&rect,0);
                if(rect.right<D(MAINWND_WX)||rect.bottom<D(MAINWND_WY))
                  if(rect.right<wpos->cx||rect.bottom<wpos->cy)
                {
                    wpos->cx=rect.right;
                    wpos->cy=rect.bottom;
                    wpos->x=0;
                    wpos->y=0;
                }
            }
            break;

        case WM_SIZING:
            {
                RECT *r=(RECT *)lParam;
                int minx=D(MAINWND_MINX);
                int miny=D(MAINWND_MINY);

                switch(wParam)
                {
                    case WMSZ_LEFT:
                    case WMSZ_TOPLEFT:
                    case WMSZ_BOTTOMLEFT:
                        if(r->right-r->left<minx)r->left=r->right-minx;
                        break;

                    case WMSZ_BOTTOM:
                    case WMSZ_RIGHT:
                    case WMSZ_TOP:
                    case WMSZ_BOTTOMRIGHT:
                    case WMSZ_TOPRIGHT:
                        if(r->right-r->left<minx)r->right=r->left+minx;
                        break;

                    default:
                        break;
                }
                switch(wParam)
                {
                    case WMSZ_TOP:
                    case WMSZ_TOPLEFT:
                    case WMSZ_TOPRIGHT:
                        if(r->bottom-r->top<miny)r->top=r->bottom-miny;
                        break;

                    case WMSZ_BOTTOM:
                    case WMSZ_BOTTOMLEFT:
                    case WMSZ_BOTTOMRIGHT:
                    case WMSZ_LEFT:
                    case WMSZ_RIGHT:
                        if(r->bottom-r->top<miny)r->bottom=r->top+miny;
                        break;

                    default:
                        break;
                }
                break;
            }

        case WM_KEYUP:
            if(ctrl_down&&wParam==L'A'){SelectAllCommand c;c.LeftClick();}
            if(ctrl_down&&wParam==L'N'){SelectNoneCommand c;c.LeftClick();}
            if(ctrl_down&&wParam==L'I'){InstallCommand c;c.LeftClick();}
            if(ctrl_down&&wParam==L'P')
            {
                ClickVisiter cv{ID_RESTPNT};
                wPanels->Accept(cv);
            }
            if(ctrl_down&&wParam==L'R')
            {
                ClickVisiter cv{ID_REBOOT};
                wPanels->Accept(cv);
            }
            if(wParam==VK_F5&&ctrl_down)
                invalidate(INVALIDATE_DEVICES);else
            if(wParam==VK_F5)
                invalidate(INVALIDATE_DEVICES|INVALIDATE_SYSINFO|INVALIDATE_INDEXES|INVALIDATE_MANAGER);
            if(wParam==VK_F6&&ctrl_down)
            {
                manager_g->testitembars();
                manager_g->setpos();
                redrawfield();
            }
            if(wParam==VK_F7)
            {
                save_wndinfo();
                MessageBox(hMain,L"Windows data recorded into the log.",L"Message",0);
            }
            if(wParam==VK_F8)
            {
                switch(Settings.flags&(FLAG_SHOWDRPNAMES1|FLAG_SHOWDRPNAMES2))
                {
                    case FLAG_SHOWDRPNAMES1:
                        Settings.flags^=FLAG_SHOWDRPNAMES1;
                        Settings.flags^=FLAG_SHOWDRPNAMES2;
                        break;

                    case FLAG_SHOWDRPNAMES2:
                        Settings.flags^=FLAG_SHOWDRPNAMES2;
                        break;

                    case 0:
                        Settings.flags^=FLAG_SHOWDRPNAMES1;
                        break;

                    default:
                        break;
                }
                manager_g->filter(Settings.filters);
                manager_g->setpos();
                redrawfield();
            }
            break;

        case WM_SYSKEYDOWN:
            if(wParam==VK_MENU)break;
            return DefWindowProc(hwnd,uMsg,wParam,lParam);

        case WM_DEVICECHANGE:
            if(installmode==MODE_INSTALLING)break;
            Log.print_con("WM_DEVICECHANGE(%x,%x)\n",wParam,lParam);
            invalidate(INVALIDATE_DEVICES);
            break;

        case WM_SIZE:
            SetLayeredWindowAttributes(hMain,0,(BYTE)D(MAINWND_TRANSPARENCY),LWA_ALPHA);
            SetLayeredWindowAttributes(Popup.hPopup,0,(BYTE)D(POPUP_TRANSPARENCY),LWA_ALPHA);

            GetWindowRect(hwnd,&rect);
            main1x_c=x;
            main1y_c=y;

            i=D(PNLITEM_OFSX)+D(PANEL_LIST_OFSX);
            f=D(PANEL_LIST_OFSX)?4:0;
            MoveWindow(hField,Xm(D(DRVLIST_OFSX),D(DRVLIST_WX)),Ym(D(DRVLIST_OFSY)),XM(D(DRVLIST_WX),D(DRVLIST_OFSX)),YM(D(DRVLIST_WY),D(DRVLIST_OFSY)),TRUE);

            wPanels->arrange();
            manager_g->setpos();

            redrawmainwnd();
            break;

        case WM_TIMER:
            if(manager_g->animate())
                redrawfield();
            else
                KillTimer(hwnd,1);
            break;

        case WM_PAINT:
            GetClientRect(hwnd,&rect);
            canvasMain->begin(hwnd,rect.right,rect.bottom);

            canvasMain->DrawWidget(0,0,rect.right+1,rect.bottom+1,BOX_MAINWND);
            canvasMain->SetFont(hFont);
            drawnew(*canvasMain);
            canvasMain->end();
            break;

        case WM_ERASEBKGND:
            return 1;

        case WM_MOUSEMOVE:
            {
                HoverVisiter hv{x,y};
                wPanels->Accept(hv);
            }
            break;

        case WM_LBUTTONUP:
            if(mouseclick)
            {
                ClickVisiter cv{x,y};
                wPanels->Accept(cv);
            }
            break;

        case WM_RBUTTONUP:
            {
                ClickVisiter cv{x,y,true};
                wPanels->Accept(cv);
            }
            break;

        case WM_MOUSEWHEEL:
            i=GET_WHEEL_DELTA_WPARAM(wParam);
            if(space_down)
            {
                Popup.horiz_sh-=i/5;
                if(Popup.horiz_sh>0)Popup.horiz_sh=0;
                InvalidateRect(Popup.hPopup,nullptr,0);
            }
            else
                SendMessage(hField,WM_VSCROLL,MAKELONG(i>0?SB_LINEUP:SB_LINEDOWN,0),0);
            break;

        case WM_COMMAND:
            wp=LOWORD(wParam);
            switch(wp)
            {
                case ID_SCHEDULE:
                    manager_g->toggle(Popup.floating_itembar);
                    redrawfield();
                    break;

                case ID_SHOWALT:
                    if(Popup.floating_itembar==SLOT_RESTORE_POINT)
                    {
                        System.run_command(L"cmd",L"/c %windir%\\Sysnative\\rstrui.exe",SW_HIDE,0);
                        System.run_command(L"cmd",L"/c %windir%\\system32\\Restore\\rstrui.exe",SW_HIDE,0);
                    }
                    else
                    {
                        manager_g->expand(Popup.floating_itembar,EXPAND_MODE::TOGGLE);
                    }
                    break;

                case ID_OPENINF:
                case ID_LOCATEINF:
                    manager_g->getINFpath(wp);
                    break;

                case ID_DEVICEMNG:
                    System.run_command(L"devmgmt.msc",nullptr,SW_SHOW,0);
                    break;

                case ID_EMU_32:
                    Settings.virtual_arch_type=32;
                    invalidate(INVALIDATE_SYSINFO|INVALIDATE_MANAGER);
                    break;

                case ID_EMU_64:
                    Settings.virtual_arch_type=64;
                    invalidate(INVALIDATE_SYSINFO|INVALIDATE_MANAGER);
                    break;

                case ID_DIS_INSTALL:
                    Settings.flags^=FLAG_DISABLEINSTALL;
                    break;

                case ID_DIS_RESTPNT:
                    Settings.flags^=FLAG_NORESTOREPOINT;
                    manager_g->itembar_setactive(SLOT_RESTORE_POINT,(Settings.flags&FLAG_NORESTOREPOINT)?0:1);
                    manager_g->set_rstpnt(0);
                    break;

                default:
                    break;
            }
            if(wp>=ID_WIN_2000&&wp<=ID_WIN_10)
            {
                Settings.virtual_os_version=getWindowsVer(wp-ID_WIN_2000);
                invalidate(INVALIDATE_SYSINFO|INVALIDATE_MANAGER);
            }
            if(wp>=ID_HWID_CLIP&&wp<=ID_HWID_WEB+100)
            {
                int id=wp%100;
                if(wp>=ID_HWID_WEB)
                {
                    wchar_t buf[BUFLEN];
                    wchar_t buf2[BUFLEN];
                    const wchar_t *str=manager_g->getHWIDby(id);
                    //wsprintf(buf,L"https://www.google.com/#q=%s",str);
                    wsprintf(buf,L"http://catalog.update.microsoft.com/v7/site/search.aspx?q=%s",str);
                    escapeAmpUrl(buf2,buf);
                    System.run_command(L"iexplore.exe",buf2,SW_SHOW,0);

                }
                else
                {
                    const wchar_t *str=manager_g->getHWIDby(id);
                    size_t len=wcslen(str)*2+2;
                    HGLOBAL hMem=GlobalAlloc(GMEM_MOVEABLE,len);
                    memcpy(GlobalLock(hMem),str,len);
                    GlobalUnlock(hMem);
                    OpenClipboard(nullptr);
                    EmptyClipboard();
                    SetClipboardData(CF_UNICODETEXT,hMem);
                    CloseClipboard();
                }
            }

            if(HIWORD(wParam)==CBN_SELCHANGE)
            {
                if(wp==ID_LANG)
                {
					LRESULT j=SendMessage((HWND)lParam,CB_GETCURSEL,0,0);
                    SendMessage((HWND)lParam,CB_GETLBTEXT,j,(LPARAM)Settings.curlang);
                    vLang->switchdata((int)j);
                    lang_refresh();
                }

                if(wp==ID_THEME)
                {
					LRESULT j=SendMessage((HWND)lParam,CB_GETCURSEL,0,0);
                    SendMessage((HWND)lParam,CB_GETLBTEXT,j,(LPARAM)Settings.curtheme);
					vTheme->switchdata((int)j);
                    theme_refresh();
                }
            }
            break;

        default:
            {
                LRESULT j=DefWindowProc(hwnd,uMsg,wParam,lParam);
                checktimer(L"MainD",timer,uMsg);
                return j;
            }
    }
    checktimer(L"Main",timer,uMsg);
    return 0;
}

void RestPointCheckboxCommand::RightClick(int x,int y)
{
    Popup.floating_itembar=SLOT_RESTORE_POINT;
    manager_g->contextmenu(x-Xm(D(DRVLIST_OFSX),D(DRVLIST_WX)),y-Ym(D(DRVLIST_OFSY)));
}

//{ Buttons
void OpenLogsCommand::LeftClick(bool)
{
    ShellExecute(MainWindow.hMain,L"explore",Settings.log_dir,nullptr,nullptr,SW_SHOW);
}

void SnapshotCommand::LeftClick(bool)
{
    MainWindow.snapshot();
}

void ExtractCommand::LeftClick(bool)
{
    MainWindow.extractto();
}

void DrvDirCommand::LeftClick(bool)
{
    MainWindow.selectDrpDir();
}

void InstallCommand::LeftClick(bool)
{
    if(installmode==MODE_NONE)
    {
        if((Settings.flags&FLAG_EXTRACTONLY)==0)
        wsprintf(extractdir,L"%s\\SDI",manager_g->matcher->getState()->textas.get(manager_g->matcher->getState()->getTemp()));
        manager_g->install(INSTALLDRIVERS);
    }
}

void SelectAllCommand::LeftClick(bool)
{
    manager_g->selectall();
    MainWindow.redrawmainwnd();
    MainWindow.redrawfield();
}

void SelectNoneCommand::LeftClick(bool)
{
    manager_g->selectnone();
    MainWindow.redrawmainwnd();
    MainWindow.redrawfield();
}
//}

LRESULT CALLBACK MainWindow_t::WindowGraphProcedure(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    return MainWindow.WindowGraphProcedure2(hwnd,uMsg,wParam,lParam);
}

LRESULT MainWindow_t::WindowGraphProcedure2(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
    SCROLLINFO si;
    RECT rect;
    int x,y;
    long long timer=GetTickCount();
    int i;

    x=LOWORD(lParam);
    y=HIWORD(lParam);
    if(WndProcCommon2(hwnd,message,wParam,lParam))
    switch(message)
    {
        case WM_CREATE:
            canvasField=new Canvas;
            break;

        case WM_PAINT:
            y=getscrollpos();

            GetClientRect(hwnd,&rect);
            canvasField->begin(hwnd,rect.right,rect.bottom);
            canvasField->CopyCanvas(canvasMain,Xm(D(DRVLIST_OFSX),D(DRVLIST_WX)),Ym(D(DRVLIST_OFSY)));
            manager_g->draw(*canvasField,y);
            canvasField->end();
            break;

        case WM_DESTROY:
            delete canvasField;
            break;

        case WM_ERASEBKGND:
            return 1;

        case WM_SIZE:
            mainx_c=x;
            mainy_c=y;
            if(scrollvisible)mainx_c-=GetSystemMetrics(SM_CXVSCROLL);
            break;

        case WM_VSCROLL:
            si.cbSize=sizeof(si);
            si.fMask=SIF_ALL;
            si.nPos=getscrollpos();
            GetScrollInfo(hwnd,SB_VERT,&si);
            switch(LOWORD(wParam))
            {
                case SB_LINEUP:si.nPos-=35;break;
                case SB_LINEDOWN:si.nPos+=35;break;
                case SB_PAGEUP:si.nPos-=si.nPage;break;
                case SB_PAGEDOWN:si.nPos+=si.nPage;break;
                case SB_THUMBTRACK:si.nPos=si.nTrackPos;break;
                default:break;
            }
            offset_target=0;
            setscrollpos(si.nPos);
            redrawfield();
            break;

        case WM_LBUTTONUP:
            if(!mouseclick)break;
            manager_g->hitscan(x,y,&Popup.floating_itembar,&i);
            if(Popup.floating_itembar==SLOT_SNAPSHOT)
            {
                Settings.statemode=STATEMODE_REAL;
                invalidate(INVALIDATE_DEVICES|INVALIDATE_SYSINFO|INVALIDATE_MANAGER);
            }
            if(Popup.floating_itembar==SLOT_DPRDIR)
            {
                *Settings.drpext_dir=0;
                invalidate(INVALIDATE_INDEXES|INVALIDATE_MANAGER);
            }
            if(Popup.floating_itembar==SLOT_EXTRACTING)
            {
                if(installmode==MODE_INSTALLING)
                    installmode=MODE_STOPPING;
                else if(installmode==MODE_NONE)
                    manager_g->clear();
            }
            if(Popup.floating_itembar==SLOT_DOWNLOAD)
            {
                #ifdef USE_TORRENT
                Updater->openDialog();
                #endif
                break;
            }

            if(Popup.floating_itembar>=0&&(i==1||i==0||i==3))
            {
                manager_g->toggle(Popup.floating_itembar);
                if(wParam&MK_SHIFT&&installmode==MODE_NONE)
                {
                    if((Settings.flags&FLAG_EXTRACTONLY)==0)
                    wsprintf(extractdir,L"%s\\SDI",manager_g->matcher->getState()->textas.get(manager_g->matcher->getState()->getTemp()));
                    manager_g->install(INSTALLDRIVERS);
                }
                redrawfield();
            }
            if(Popup.floating_itembar>=0&&i==2)
            {
                manager_g->expand(Popup.floating_itembar,EXPAND_MODE::TOGGLE);
            }
            break;

        case WM_RBUTTONDOWN:
            manager_g->hitscan(x,y,&Popup.floating_itembar,&i);
            if(Popup.floating_itembar>=0&&(i==0||i==3))
                manager_g->contextmenu(x,y);
            break;

        case WM_MBUTTONDOWN:
            mousedown=MOUSE_SCROLL;
            mousex=x;
            mousey=y;
            SetCursor(LoadCursor(nullptr,IDC_SIZEALL));
            SetCapture(hwnd);
            break;

        case WM_MOUSEMOVE:
            si.cbSize=sizeof(si);
            if(mousedown==MOUSE_SCROLL)
            {
                si.fMask=SIF_ALL;
                si.nPos=0;
                GetScrollInfo(hwnd,SB_VERT,&si);
                si.nPos+=mousey-y;
                si.fMask=SIF_POS;
                SetScrollInfo(hwnd,SB_VERT,&si,TRUE);

                mousex=x;
                mousey=y;
                redrawfield();
            }
            {
                int type=FLOATING_NONE;
                int itembar_i;

                if(space_down&&kbpanel)break;

                if(space_down)type=FLOATING_DRIVERLST;else
                if(ctrl_down||Settings.expertmode)type=FLOATING_CMPDRIVER;

                manager_g->hitscan(x,y,&itembar_i,&i);
                if(i==0&&itembar_i>=RES_SLOTS&&(ctrl_down||space_down||Settings.expertmode))
                    drawpopup(itembar_i,type,x,y,hField);
                else if(itembar_i==SLOT_VIRUS_AUTORUN)
                    drawpopup(STR_VIRUS_AUTORUN_H,FLOATING_TOOLTIP,x,y,hField);
                else if(itembar_i==SLOT_VIRUS_RECYCLER)
                    drawpopup(STR_VIRUS_RECYCLER_H,FLOATING_TOOLTIP,x,y,hField);
                else if(itembar_i==SLOT_VIRUS_HIDDEN)
                    drawpopup(STR_VIRUS_HIDDEN_H,FLOATING_TOOLTIP,x,y,hField);
                else if(itembar_i==SLOT_EXTRACTING&&installmode)
                    drawpopup((instflag&INSTALLDRIVERS)?STR_HINT_STOPINST:STR_HINT_STOPEXTR,FLOATING_TOOLTIP,x,y,hField);
                else if(itembar_i==SLOT_RESTORE_POINT)
                    drawpopup(STR_RESTOREPOINT_H,FLOATING_TOOLTIP,x,y,hField);
                else if(itembar_i==SLOT_DOWNLOAD)
                    drawpopup(-1,FLOATING_DOWNLOAD,x,y,hField);
                else if(i==0&&itembar_i>=RES_SLOTS)
                    drawpopup(STR_HINT_DRIVER,FLOATING_TOOLTIP,x,y,hField);
                else
                    drawpopup(-1,FLOATING_NONE,0,0,hField);

                if(itembar_i!=field_lasti||i!=field_lastz)redrawfield();
                field_lasti=itembar_i;
                field_lastz=i;
            }
            break;

        default:
			{
				LRESULT j=DefWindowProc(hwnd,message,wParam,lParam);
				checktimer(L"ListD",timer,message);
				return j;
			}
    }
    checktimer(L"List",timer,message);
    return 0;
}

LRESULT CALLBACK PopupProcedure(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
    return Popup.PopupProcedure2(hwnd,message,wParam,lParam);
}

Popup_t::Popup_t():
    hFontP(new Font()),
    hFontBold(new Font())
{
}
Popup_t::~Popup_t()
{
    delete hFontP;
    delete hFontBold;
}

LRESULT Popup_t::PopupProcedure2(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
    RECT rect;
    WINDOWPOS *wp;

    switch(message)
    {
        case WM_WINDOWPOSCHANGING:
            if(floating_type!=FLOATING_TOOLTIP)break;

            wp=(WINDOWPOS*)lParam;
            GetClientRect(hwnd,&rect);
            rect.right=D(POPUP_WX);
            rect.bottom=floating_y;

            canvasPopup->SetFont(Popup.hFontP);
            canvasPopup->CalcBoundingBox(STR(floating_itembar),&rect);

            AdjustWindowRectEx(&rect,WS_POPUPWINDOW|WS_VISIBLE,0,0);
            popup_resize(rect.right-rect.left+D(POPUP_OFSX)*2,rect.bottom-rect.top+D(POPUP_OFSY)*2);
            wp->cx=rect.right+D(POPUP_OFSX)*2;
            wp->cy=rect.bottom+D(POPUP_OFSY)*2;
            break;

        case WM_CREATE:
            canvasPopup=new Canvas;
            break;

        case WM_DESTROY:
            delete canvasPopup;
            break;

        case WM_PAINT:
            GetClientRect(hwnd,&rect);
            canvasPopup->begin(hwnd,rect.right,rect.bottom);

            canvasPopup->DrawWidget(0,0,rect.right,rect.bottom,BOX_POPUP);
            switch(floating_type)
            {
                case FLOATING_SYSINFO:
                    canvasPopup->SetFont(Popup.hFontP);
                    manager_g->matcher->getState()->popup_sysinfo(*canvasPopup);
                    break;

                case FLOATING_TOOLTIP:
                    rect.left+=D(POPUP_OFSX);
                    rect.top+=D(POPUP_OFSY);
                    rect.right-=D(POPUP_OFSX);
                    rect.bottom-=D(POPUP_OFSY);
                    canvasPopup->SetFont(Popup.hFontP);
                    canvasPopup->SetTextColor(D(POPUP_TEXT_COLOR));
                    canvasPopup->DrawTextRect(STR(floating_itembar),&rect);
                    break;

                case FLOATING_CMPDRIVER:
                    canvasPopup->SetFont(Popup.hFontP);
                    manager_g->popup_drivercmp(manager_g,*canvasPopup,rect.right,rect.bottom,floating_itembar);
                    break;

                case FLOATING_DRIVERLST:
                    canvasPopup->SetFont(Popup.hFontP);
                    manager_g->popup_driverlist(*canvasPopup,rect.right,rect.bottom,floating_itembar);
                    break;

                case FLOATING_ABOUT:
                    canvasPopup->SetFont(Popup.hFontP);
                    popup_about(*canvasPopup);
                    break;

                case FLOATING_DOWNLOAD:
                    canvasPopup->SetFont(Popup.hFontP);
                    #ifdef USE_TORRENT
                    Updater->showPopup(*canvasPopup);
                    #endif
                    break;

                default:
                    break;
            }

            canvasPopup->end();
            break;

        case WM_ERASEBKGND:
            return 1;

        default:
            return DefWindowProc(hwnd,message,wParam,lParam);
    }
    return 0;
}

BOOL CALLBACK LicenseProcedure(HWND hwnd,UINT Message,WPARAM wParam,LPARAM lParam)
{
    WINDOWPOS *wpos;
    HWND hEditBox;
    RECT rect;
    LPCSTR s;
    int sz;

    switch(Message)
    {
        case WM_INITDIALOG:
            get_resource(IDR_LICENSE,(void **)&s,&sz);
            hEditBox=GetDlgItem(hwnd,IDC_EDIT1);
            SetWindowTextA(hEditBox,s);
            SendMessage(hEditBox,EM_SETREADONLY,1,0);
            return TRUE;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDOK:
                    Settings.license=2;
                    EndDialog(hwnd,IDOK);
                    return TRUE;

                case IDCANCEL:
                    Settings.license=0;
                    EndDialog(hwnd,IDCANCEL);
                    return TRUE;

                default:
                    break;
            }
            break;

        case WM_WINDOWPOSCHANGED:
            wpos=(WINDOWPOS*)lParam;

            SystemParametersInfo(SPI_GETWORKAREA,0,&rect,0);
            if(wpos->cy-rect.bottom>0)
            {
                sz=rect.bottom-20-wpos->cy;
                wpos->y=10;
                wpos->cy=rect.bottom-20;
                MoveWindow(hwnd,wpos->x,wpos->y,wpos->cx,wpos->cy,1);

                GetRelativeCtrlRect(GetDlgItem(hwnd,IDC_EDIT1),&rect);
                rect.bottom+=sz;
                MoveWindow(GetDlgItem(hwnd,IDC_EDIT1),rect.left,rect.top,rect.right,rect.bottom,1);

                GetRelativeCtrlRect(GetDlgItem(hwnd,IDOK),&rect);
                rect.top+=sz;
                MoveWindow(GetDlgItem(hwnd,IDOK),rect.left,rect.top,rect.right,rect.bottom,1);

                GetRelativeCtrlRect(GetDlgItem(hwnd,IDCANCEL),&rect);
                rect.top+=sz;
                MoveWindow(GetDlgItem(hwnd,IDCANCEL),rect.left,rect.top,rect.right,rect.bottom,1);
            }
            return TRUE;

        default:
            break;
    }
    return FALSE;
}
//}
