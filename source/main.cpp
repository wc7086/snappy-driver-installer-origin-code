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

#include "main.h"

//{ Global variables

// Manager
Manager manager_v[2];
Manager *manager_g=&manager_v[0];
int volatile installmode=MODE_NONE;
int ctrl_down=0;
int space_down=0;
int shift_down=0;
int invaidate_set;
int num_cores;

// Window
HINSTANCE ghInst;
int main1x_c,main1y_c;
int mainx_c,mainy_c;
CRITICAL_SECTION sync;
HFONT hFont=nullptr;
HWND hMain=nullptr,hField=nullptr,hPopup=nullptr,hLang=nullptr,hTheme=nullptr;

// Window helpers
int floating_type=0;
int floating_itembar=-1;
int floating_x=1,floating_y=1;
int horiz_sh=0;
int hideconsole=SW_HIDE;
int offset_target=0;
int kbpanel=KB_NONE,kbitem[]={0,0,0,0,0,0,0,0,0,0,0};
int ret_global=0;

// Window helpers (local)
int panel_lasti=0;
int field_lasti,field_lastz;

Canvas *canvasMain;
Canvas *canvasField;
Canvas *canvasPopup;
const wchar_t classMain[]= L"classSDIMain";
const wchar_t classField[]=L"classSDIField";
const wchar_t classPopup[]=L"classSDIPopup";

int mousex=-1,mousey=-1,mousedown=MOUSE_NONE,mouseclick=0;
volatile int deviceupdate_exitflag=0;
HANDLE deviceupdate_event;
int scrollvisible=0;

int manager_active=0;
int bundle_display=1;
int bundle_shadow=0;

// Settings
wchar_t drp_dir   [BUFLEN]=L"drivers";
wchar_t drpext_dir[BUFLEN]=L"";
wchar_t index_dir [BUFLEN]=L"indexes\\SDI";
wchar_t data_dir  [BUFLEN]=L"tools\\SDI";
wchar_t log_dir   [BUFLEN];

wchar_t state_file[BUFLEN]=L"untitled.snp";
wchar_t finish    [BUFLEN]=L"";
wchar_t finish_upd[BUFLEN]=L"";
wchar_t finish_rb [BUFLEN]=L"";

int flags=COLLECTION_USE_LZMA;
int statemode=STATEMODE_REAL;
int expertmode=0;
int hintdelay=500;
int filters=
    (1<<ID_SHOW_MISSING)+
    (1<<ID_SHOW_NEWER)+
    (1<<ID_SHOW_BETTER)+
    (1<<ID_SHOW_NF_MISSING)+
    (1<<ID_SHOW_ONE);
int virtual_os_version=0;
int virtual_arch_type=0;

// Settings (local)
wchar_t curlang [BUFLEN]=L"";
wchar_t curtheme[BUFLEN]=L"(default)";
wchar_t output_dir[BUFLEN]=L"indexes\\SDI\\txt";
wchar_t logO_dir  [BUFLEN]=L"logs";
int license=0;

// Windows name
const wchar_t *windows_name[NUM_OS]=
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
int windows_ver[NUM_OS]={50,51,60,61,62,63,100,0};
//}

//{ Settings
static bool argstr(const wchar_t *s,const wchar_t *cmp,wchar_t *d)
{
    if(StrStrIW(s,cmp)){wcscpy(d,s+wcslen(cmp));return true;}
    return false;
}

static bool argint(const wchar_t *s,const wchar_t *cmp,int *d)
{
    if(StrStrIW(s,cmp)){*d=_wtoi_my(s+wcslen(cmp));return true;}
    return false;
}

static bool argopt(const wchar_t *s,const wchar_t *cmp,int *d)
{
    if(StrStrIW(s,cmp)){*d=1;return true;}
    return false;
}

static bool argflg(const wchar_t *s,const wchar_t *cmp,int f)
{
    if(!StrCmpIW(s,cmp)){flags|=f;return true;}
    return false;
}

void settings_parse(const wchar_t *str,int ind)
{
    log_con("Args:[%S]\n",str);
    int argc;
    wchar_t **argv=CommandLineToArgvW(str,&argc);
    for(int i=ind;i<argc;i++)
    {
        wchar_t *pr=argv[i];
        if(pr[0]=='/')pr[0]='-';

        if(argstr(pr,L"-drp_dir:",drp_dir))continue;
        if(argstr(pr,L"-index_dir:",index_dir))continue;
        if(argstr(pr,L"-output_dir:",output_dir))continue;
        if(argstr(pr,L"-data_dir:",data_dir))continue;
        if(argstr(pr,L"-log_dir:",logO_dir))continue;

        if(argstr(pr,L"-finish_cmd:",finish))continue;
        if(argstr(pr,L"-finishrb_cmd:",finish_rb))continue;
        if(argstr(pr,L"-finish_upd_cmd:",finish_upd))continue;

        if(argstr(pr,L"-lang:",curlang))continue;
        if(argstr(pr,L"-theme:",curtheme))continue;
        if(argint(pr,L"-hintdelay:",&hintdelay))continue;
        if(argint(pr,L"-filters:",&filters))continue;

        if(argint(pr,L"-port:",&Updater.torrentport))continue;
        if(argint(pr,L"-downlimit:",&Updater.downlimit))continue;
        if(argint(pr,L"-uplimit:",&Updater.uplimit))continue;
        if(argint(pr,L"-connections:",&Updater.connections))continue;

        if(argopt(pr,L"-license",&license))continue;
        if(argopt(pr,L"-expertmode",&expertmode))continue;
        if(argflg(pr,L"-showconsole",FLAG_SHOWCONSOLE))continue;
        if(argflg(pr,L"-norestorepnt",FLAG_NORESTOREPOINT))continue;
        if(argflg(pr,L"-novirusalerts",FLAG_NOVIRUSALERTS))continue;
        if(argflg(pr,L"-preservecfg",FLAG_PRESERVECFG))continue;

        if(argflg(pr,L"-showdrpnames1",FLAG_SHOWDRPNAMES1))continue;
        if(argflg(pr,L"-showdrpnames2",FLAG_SHOWDRPNAMES2))continue;
        if(argflg(pr,L"-oldstyle",FLAG_OLDSTYLE))continue;

        if(argflg(pr,L"-checkupdates",FLAG_CHECKUPDATES))continue;
        if(argflg(pr,L"-onlyupdates",FLAG_NORESTOREPOINT))continue;
        if(!StrCmpIW(pr,L"-7z"))
        {
            wchar_t cmd[BUFLEN];
            wsprintf(cmd,L"7za.exe %s",StrCmpIW(GetCommandLineW(),L"-7z")+4);
            log_con("Executing '%S'\n",cmd);
            registerall();
            ret_global=Extract7z(cmd);
            log_con("Ret: %d\n",ret_global);
            statemode=STATEMODE_EXIT;
            break;
        }
        else
        if(!wcscmp(pr,L"-PATH"))
        {
            wcscpy(drpext_dir,argv[++i]);
            flags|=FLAG_AUTOCLOSE|
                FLAG_AUTOINSTALL|FLAG_NORESTOREPOINT|FLAG_DPINSTMODE|//FLAG_DISABLEINSTALL|
                FLAG_PRESERVECFG;
        }
        else
        if(!wcscmp(pr,L"-install")&&argc-i==3)
        {
            wchar_t buf[BUFLEN];
            log_con("Install '%S' '%s'\n",argv[i+1],argv[i+2]);
            GetEnvironmentVariable(L"TEMP",buf,BUFLEN);
            wsprintf(extractdir,L"%s\\SDI",buf);
            installmode=MODE_INSTALLING;
            driver_install(argv[i+1],argv[i+2],&ret_global,&needreboot);
            log_con("Ret: %X,%d\n",ret_global,needreboot);
            if(needreboot)ret_global|=0x80000000;
            wsprintf(buf,L" /c rd /s /q \"%s\"",extractdir);
            run_command(L"cmd",buf,SW_HIDE,1);
            statemode=STATEMODE_EXIT;
            break;
        }
        else
        if(!StrCmpIW(pr,L"-filtersp"))     {flags|=FLAG_FILTERSP;flags&=~COLLECTION_USE_LZMA;}else
        if(!StrCmpIW(pr,L"-reindex"))      flags|=COLLECTION_FORCE_REINDEXING;else
        if(!StrCmpIW(pr,L"-index_hr"))     flags|=COLLECTION_PRINT_INDEX;else
        if(!StrCmpIW(pr,L"-nogui"))        flags|=FLAG_NOGUI|FLAG_AUTOCLOSE;else
        if(!StrCmpIW(pr,L"-autoinstall"))  flags|=FLAG_AUTOINSTALL;else
        if(!StrCmpIW(pr,L"-autoclose"))    flags|=FLAG_AUTOCLOSE;else
        if(!StrCmpIW(pr,L"-autoupdate"))   flags|=FLAG_AUTOUPDATE;else

        if( StrStrIW(pr,L"-extractdir:"))  {flags|=FLAG_EXTRACTONLY;wcscpy(extractdir,pr+12);}else
        if(!StrCmpIW(pr,L"-keepunpackedindex"))flags|=FLAG_KEEPUNPACKINDEX;else
        if(!StrCmpIW(pr,L"-keeptempfiles"))flags|=FLAG_KEEPTEMPFILES;else
        if(!StrCmpIW(pr,L"-disableinstall"))flags|=FLAG_DISABLEINSTALL;else
        if(!StrCmpIW(pr,L"-failsafe"))     flags|=FLAG_FAILSAFE;else
        if(!StrCmpIW(pr,L"-delextrainfs")) flags|=FLAG_DELEXTRAINFS;else

        if( StrStrIW(pr,L"-ls:"))          {wcscpy(state_file,pr+4);statemode=STATEMODE_EMUL;}else
        if( StrStrIW(pr,L"-verbose:"))     log_verbose=_wtoi_my(pr+9);else
        if(!StrCmpIW(pr,L"-nologfile"))    flags|=FLAG_NOLOGFILE;else
        if(!StrCmpIW(pr,L"-nosnapshot"))   flags|=FLAG_NOSNAPSHOT;else
        if(!StrCmpIW(pr,L"-nostamp"))      flags|=FLAG_NOSTAMP;else

        if(!StrCmpIW(pr,L"-a:32"))         virtual_arch_type=32;else
        if(!StrCmpIW(pr,L"-a:64"))         virtual_arch_type=64;else
        if( StrStrIW(pr,L"-v:"))           virtual_os_version=_wtoi_my(pr+3);else

        if( StrCmpIW(pr,SAVE_INSTALLED_ID_DEF)==0)Parse_save_installed_id_swith(pr);else
        if( StrStrIW(pr,L"-?"))            CLIParam.ShowHelp=TRUE;else
        if( StrCmpIW(pr,HWIDINSTALLED_DEF)==0)Parse_HWID_installed_swith(pr); else
        if( StrCmpIW(pr,GFG_DEF)==0)      continue;
        else
            log_err("Unknown argument '%S'\n",pr);
        if(statemode==STATEMODE_EXIT)break;
    }
    ExpandEnvironmentStrings(logO_dir,log_dir,BUFLEN);
    LocalFree(argv);
    if(statemode==STATEMODE_EXIT)return;

    // Left panel
    panel_loadsettings(panels,filters);
}

void settings_save()
{
    if(flags&FLAG_PRESERVECFG)return;
    if(!canWrite(L"sdi.cfg"))
    {
        log_err("ERROR in settings_save(): Write-protected,'sdi.cfg'\n");
        return;
    }
    FILE *f=_wfopen(L"sdi.cfg",L"wt");
    if(!f)return;
    fwprintf(f,L"\"-drp_dir:%s\"\n\"-index_dir:%s\"\n\"-output_dir:%s\"\n"
              "\"-data_dir:%s\"\n\"-log_dir:%s\"\n\n"
              "\"-finish_cmd:%s\"\n\"-finishrb_cmd:%s\"\n\"-finish_upd_cmd:%s\"\n\n"
              "\"-lang:%s\"\n\"-theme:%s\"\n-hintdelay:%d\n-filters:%d\n\n"
              "-port:%d\n-downlimit:%d\n-uplimit:%d\n-connections:%d\n\n",
            drp_dir,index_dir,output_dir,
            data_dir,logO_dir,
            finish,finish_rb,finish_upd,
            curlang,curtheme,hintdelay,filters,
            Updater.torrentport,Updater.downlimit,Updater.uplimit,Updater.connections);

    if(license)fwprintf(f,L"-license ");
    if(expertmode)fwprintf(f,L"-expertmode ");
    if(flags&FLAG_SHOWCONSOLE)fwprintf(f,L"-showconsole ");
    if(flags&FLAG_NORESTOREPOINT)fwprintf(f,L"-norestorepnt ");
    if(flags&FLAG_NOVIRUSALERTS)fwprintf(f,L"-novirusalerts ");

    if(flags&FLAG_SHOWDRPNAMES1)fwprintf(f,L"-showdrpnames1 ");
    if(flags&FLAG_SHOWDRPNAMES2)fwprintf(f,L"-showdrpnames2 ");
    if(flags&FLAG_OLDSTYLE)fwprintf(f,L"-oldstyle ");

    if(flags&FLAG_CHECKUPDATES)fwprintf(f,L"-checkupdates ");
    if(flags&FLAG_ONLYUPDATES)fwprintf(f,L"-onlyupdates ");
    fclose(f);
}

int settings_load(const wchar_t *filename)
{
    wchar_t buf[BUFLEN];

    if(!loadCFGFile(filename,buf))return 0;
    settings_parse(buf,0);
    return 1;
}
//}

//{  Main
int WINAPI WinMain(HINSTANCE hInst,HINSTANCE hinst,LPSTR pStr,int nCmd)
{
    UNREFERENCED_PARAMETER(hinst);
    UNREFERENCED_PARAMETER(pStr);
    ghInst=hInst;

    time_total=GetTickCount();

    // Hide the console window as soon as possible
    #ifndef CONSOLE_MODE
    DWORD dwProcessId;
    GetWindowThreadProcessId(GetConsoleWindow(),&dwProcessId);
    if(GetCurrentProcessId()!=dwProcessId)hideconsole=SW_SHOWNOACTIVATE;
    ShowWindow(GetConsoleWindow(),hideconsole);
    #endif

    // Determine number of CPU cores
    SYSTEM_INFO siSysInfo;
    GetSystemInfo(&siSysInfo);
    num_cores=siSysInfo.dwNumberOfProcessors;

    // 7-zip
    registerall();

    // Check if the mouse present
    if(!GetSystemMetrics(SM_MOUSEPRESENT))kbpanel=KB_FIELD;

    // Runtime error handlers
    start_exception_hadnlers();
    HMODULE backtrace=nullptr;
    backtrace=LoadLibraryA("backtrace.dll");
    #ifdef _DEBUG
    if(!backtrace)signal(SIGSEGV,SignalHandler);
    #else
    signal(SIGSEGV,SignalHandler);
    #endif

    // Load settings
    init_CLIParam();
    if(isCfgSwithExist(GetCommandLineW(),CLIParam.SaveInstalledFileName))
        settings_load(CLIParam.SaveInstalledFileName);
    else
    if(!settings_load(L"sdi.cfg"))
        settings_load(L"tools\\SDI\\settings.cfg");
    settings_parse(GetCommandLineW(),1);
    RUN_CLI(CLIParam);

    // Reset paths for GUI-less version of the app
    #ifdef CONSOLE_MODE
    flags|=FLAG_NOGUI;
    license=1;
    wcscpy(drp_dir,log_dir);
    wcscpy(index_dir,log_dir);
    wcscpy(output_dir,log_dir);
    #endif

    // Close the app if the work is done
    if(statemode==STATEMODE_EXIT)
    {
        if(backtrace)FreeLibrary(backtrace);
        ShowWindow(GetConsoleWindow(),SW_SHOW);
        return ret_global;
    }

    // Bring back the console window
    #ifndef CONSOLE_MODE
    ShowWindow(GetConsoleWindow(),(expertmode&&flags&FLAG_SHOWCONSOLE)?SW_SHOWNOACTIVATE:hideconsole);
    #endif

    // Start logging
    ExpandEnvironmentStrings(logO_dir,log_dir,BUFLEN);
    log_start(log_dir);
    if(log_verbose&LOG_VERBOSE_ARGS)
    {
        log_con("Settings\n");
        log_con("  drp_dir='%S'\n",drp_dir);
        log_con("  index_dir='%S'\n",index_dir);
        log_con("  output_dir='%S'\n",output_dir);
        log_con("  data_dir='%S'\n",data_dir);
        log_con("  log_dir='%S'\n",log_dir);
        log_con("  extractdir='%S'\n",extractdir);
        log_con("  lang=%S\n",curlang);
        log_con("  theme=%S\n",curtheme);
        log_con("  expertmode=%d\n",expertmode);
        log_con("  filters=%d\n",filters);
        log_con("  autoinstall=%d\n",flags&FLAG_AUTOINSTALL?1:0);
        log_con("  autoclose=%d\n",flags&FLAG_AUTOCLOSE?1:0);
        log_con("  failsafe=%d\n",flags&FLAG_FAILSAFE?1:0);
        log_con("  delextrainfs=%d\n",flags&FLAG_DELEXTRAINFS?1:0);
        log_con("  checkupdates=%d\n",flags&FLAG_CHECKUPDATES?1:0);
        log_con("  norestorepnt=%d\n",flags&FLAG_NORESTOREPOINT?1:0);
        log_con("  disableinstall=%d\n",flags&FLAG_DISABLEINSTALL?1:0);
        log_con("\n");
        if(statemode==STATEMODE_EMUL)log_con("Virtual system system config '%S'\n",state_file);
        if(virtual_arch_type)log_con("Virtual Windows version: %d-bit\n",virtual_arch_type);
        if(virtual_os_version)log_con("Virtual Windows version: %d.%d\n",virtual_os_version/10,virtual_os_version%10);
        log_con("\n");
    }

    #ifdef BENCH_MODE
    benchmark();
    #endif

    // Make dirs
    mkdir_r(drp_dir);
    mkdir_r(index_dir);
    mkdir_r(output_dir);

    // Load text
    vLang.init1(language,STR_NM,IDR_LANG);
    vTheme.init1(theme,THEME_NM,IDR_THEME);
    vLang.load(0);

    // Allocate resources
    Bundle bundle[2];
    bundle[0].bundle_init();
    bundle[1].bundle_init();
    manager_v[0].init(bundle[bundle_display].getMatcher());
    manager_v[1].init(bundle[bundle_display].getMatcher());
    deviceupdate_event=CreateEvent(nullptr,0,0,nullptr);

    // Start device/driver scan
    bundle[bundle_display].bundle_prep();
    invalidate(INVALIDATE_DEVICES|INVALIDATE_SYSINFO|INVALIDATE_INDEXES|INVALIDATE_MANAGER);
    HANDLE thr=(HANDLE)_beginthreadex(nullptr,0,&Bundle::thread_loadall,&bundle[0],0,nullptr);

    // Check updates
    #ifdef USE_TORRENT
    Updater.createThreads();
    #endif

    // Start folder monitors
    Filemon *mon_drp=Filemon::start(drp_dir,FILE_NOTIFY_CHANGE_LAST_WRITE|FILE_NOTIFY_CHANGE_FILE_NAME,1,drp_callback);
    virusmonitor_start();
    viruscheck(L"",0,0);

    // MAIN GUI LOOP
    gui(nCmd);

    // Wait till the device scan thread is finished
    deviceupdate_exitflag=1;
    SetEvent(deviceupdate_event);
    WaitForSingleObject(thr,INFINITE);
    CloseHandle_log(thr,L"WinMain",L"thr");
    CloseHandle_log(deviceupdate_event,L"WinMain",L"event");

    // Stop libtorrent
    #ifdef USE_TORRENT
    Updater.destroyThreads();
    #endif

    // Free allocated resources
    int i;
    for(i=0;i<BOX_NUM;i++)box[i].release();
    for(i=0;i<ICON_NUM;i++)icon[i].release();

    // Save settings
    #ifndef CONSOLE_MODE
    settings_save();
    #endif

    // Stop folder monitors
    if(mon_drp)mon_drp->stop();
    virusmonitor_stop();

    // Bring the console window back
    ShowWindow(GetConsoleWindow(),SW_SHOWNOACTIVATE);

    // Stop runtime error handlers
    #ifdef NDEBUG
    signal(SIGSEGV,SIG_DFL);
    #endif
    if(backtrace)FreeLibrary(backtrace);

    // Stop logging
    //time_total=GetTickCount()-time_total;
    log_times();
    log_stop();

    // Exit
    return ret_global;
}

void gui(int nCmd)
{
    if((flags&FLAG_NOGUI)||flags&FLAG_AUTOINSTALL)return;

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
        log_err("ERROR in gui(): failed to register '%S' class\n",wcx.lpszClassName);
        return;
    }

    // Register classPopup
    wcx.lpfnWndProc=PopupProcedure;
    wcx.lpszClassName=classPopup;
    wcx.hIcon=nullptr;
    if(!RegisterClassEx(&wcx))
    {
        log_err("ERROR in gui(): failed to register '%S' class\n",wcx.lpszClassName);
        UnregisterClass_log(classMain,ghInst,L"gui",L"classMain");
        return;
    }

    // Register classField
    wcx.lpfnWndProc=WindowGraphProcedure;
    wcx.lpszClassName=classField;
    if(!RegisterClassEx(&wcx))
    {
        log_err("ERROR in gui(): failed to register '%S' class\n",wcx.lpszClassName);
        UnregisterClass_log(classMain,ghInst,L"gui",L"classMain");
        UnregisterClass_log(classPopup,ghInst,L"gui",L"classPopup");
        return;
    }

    // license dialog
    if(!license)
        DialogBox(ghInst,MAKEINTRESOURCE(IDD_DIALOG1),0,(DLGPROC)LicenseProcedure);

    // Enable updates notifications
    if(license==2)
    {
        /*int f;
        f=lang_enum(hLang,L"langs",manager_g->matcher->state->locale);
        log_con("lang %d\n",f);
        lang_set(f);*/

        //if(MessageBox(0,STR(STR_UPD_DIALOG_MSG),STR(STR_UPD_DIALOG_TITLE),MB_YESNO|MB_ICONQUESTION)==IDYES)
        {
            flags|=FLAG_CHECKUPDATES;
            #ifdef USE_TORRENT
            Updater.createThreads();
            Updater.checkUpdates();
            #endif
        }
    }

    if(license)
    {
        hMain=CreateWindowEx(WS_EX_LAYERED,
                            classMain,
                            APPTITLE,
                            WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,
                            CW_USEDEFAULT,CW_USEDEFAULT,D(MAINWND_WX),D(MAINWND_WY),
                            nullptr,nullptr,ghInst,nullptr);
        if(!hMain)
        {
            log_err("ERROR in gui(): failed to create '%S' window\n",classMain);
            return;
        }

        //time_test=GetTickCount()-time_total;log_times();
        ShowWindow(hMain,flags&FLAG_NOGUI?SW_HIDE:nCmd);

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

    UnregisterClass_log(classMain,ghInst,L"gui",L"classMain");
    UnregisterClass_log(classPopup,ghInst,L"gui",L"classPopup");
    UnregisterClass_log(classField,ghInst,L"gui",L"classField");
}
//}

//{ Bundle
unsigned int __stdcall Bundle::thread_scandevices(void *arg)
{
    State *state=(State *)arg;

    if((invaidate_set&INVALIDATE_DEVICES)==0)return 0;

    if(statemode==STATEMODE_REAL)state->scanDevices();
    if(statemode==STATEMODE_EMUL)state->load(state_file);

    return 0;
}

unsigned int __stdcall Bundle::thread_loadindexes(void *arg)
{
    Collection *collection=(Collection *)arg;

    if(invaidate_set&INVALIDATE_INDEXES)collection->updatedir();
    return 0;
}

unsigned int __stdcall Bundle::thread_getsysinfo(void *arg)
{
    State *state=(State *)arg;

    if(statemode==STATEMODE_REAL&&invaidate_set&INVALIDATE_SYSINFO)
        state->getsysinfo_slow();

    state->isnotebook_a();
    state->genmarker();

    return 0;
}

unsigned int __stdcall Bundle::thread_loadall(void *arg)
{
    Bundle *bundle=(Bundle *)arg;

    InitializeCriticalSection(&sync);
    do
    {
        // Wait for an update request
        WaitForSingleObject(deviceupdate_event,INFINITE);
        if(deviceupdate_exitflag)break;
        bundle[bundle_shadow].bundle_init();
            /*static long long prmem;
            log_con("Total mem:%ld KB(%ld)\n",nvwa::total_mem_alloc/1024,nvwa::total_mem_alloc-prmem);
            prmem=nvwa::total_mem_alloc;*/

        // Update bundle
        log_con("*** START *** %d,%d\n",bundle_display,bundle_shadow);
        bundle[bundle_shadow].bundle_init();
        bundle[bundle_shadow].bundle_prep();
        bundle[bundle_shadow].bundle_load(&bundle[bundle_display]);

        // Check if the state has been udated during scanning
        int cancel_update=0;
        if(!(flags&FLAG_NOGUI))
        if(WaitForSingleObject(deviceupdate_event,0)==WAIT_OBJECT_0)cancel_update=1;

        if(cancel_update)
        {
            log_con("*** CANCEL ***\n\n");
            SetEvent(deviceupdate_event);
        }
        else
        {
            log_con("*** FINISH primary ***\n\n");
            invaidate_set&=~(INVALIDATE_DEVICES|INVALIDATE_INDEXES|INVALIDATE_SYSINFO);

            if((flags&FLAG_NOGUI)&&(flags&FLAG_AUTOINSTALL)==0)
            {
                // NOGUI mode
                manager_g->matcher=&bundle[bundle_shadow].matcher;
                manager_g->populate();
                manager_g->filter(filters);
            }
            else // GUI mode
            {
                while(hMain==nullptr);
                SendMessage(hMain,WM_BUNDLEREADY,(WPARAM)&bundle[bundle_shadow],(LPARAM)&bundle[bundle_display]);
            }

            // Save indexes, write info, etc
            bundle[bundle_shadow].bundle_lowprioirity();
            log_con("*** FINISH secondary ***\n\n");

            // Swap display and shadow bundle
            bundle_display^=1;
            bundle_shadow^=1;
        }
    }while(!deviceupdate_exitflag);

    DeleteCriticalSection(&sync);
    return 0;
}

void Bundle::bundle_init()
{
    state.init();
    collection.init(drp_dir,index_dir,output_dir);
    matcher.init(&state,&collection);
}

void Bundle::bundle_prep()
{
    state.getsysinfo_fast();
}
void Bundle::bundle_load(Bundle *pbundle)
{
    HANDLE thandle[3];

    time_test=GetTickCount();

    // Copy data from shadow if it's not updated
    if((invaidate_set&INVALIDATE_DEVICES)==0)
    {
        state=pbundle->state;time_devicescan=0;
        if(invaidate_set&INVALIDATE_SYSINFO)state.getsysinfo_fast();
    }
    if((invaidate_set&INVALIDATE_SYSINFO)==0)state.getsysinfo_slow(&pbundle->state);
    if((invaidate_set&INVALIDATE_INDEXES)==0){collection=pbundle->collection;time_indexes=0;}


    thandle[0]=(HANDLE)_beginthreadex(nullptr,0,&thread_scandevices,&state,0,nullptr);
    thandle[1]=(HANDLE)_beginthreadex(nullptr,0,&thread_loadindexes,&collection,0,nullptr);
    thandle[2]=(HANDLE)_beginthreadex(nullptr,0,&thread_getsysinfo,&state,0,nullptr);
    WaitForMultipleObjects(3,thandle,1,INFINITE);
    CloseHandle_log(thandle[0],L"bundle_load",L"0");
    CloseHandle_log(thandle[1],L"bundle_load",L"1");
    CloseHandle_log(thandle[2],L"bundle_load",L"2");

    /*if((invaidate_set&INVALIDATE_DEVICES)==0)
    {
        state=pbundle->state;time_devicescan=0;}*/

    matcher.getState()->textas.shrink();
    matcher.populate();
    time_test=GetTickCount()-time_test;
}

void Bundle::bundle_lowprioirity()
{
    wchar_t filename[BUFLEN];
    if(!time_startup)time_startup=GetTickCount()-time_total;
    log_times();

    redrawmainwnd();

    collection.printstats();
    state.print();
    matcher.print();
    manager_g->print_hr();

    #ifdef USE_TORRENT
    if(flags&FLAG_CHECKUPDATES&&!time_chkupdate)Updater.checkUpdates();
    #endif

    collection.save();
    gen_timestamp();
    wsprintf(filename,L"%s\\%sstate.snp",log_dir,timestamp);
    state.save(filename);

    if(flags&COLLECTION_PRINT_INDEX)
    {
        log_con("Saving humanreadable indexes...");
        collection.print_index_hr();
        flags&=~COLLECTION_PRINT_INDEX;
        log_con("DONE\n");
    }
}
//}

//{ Subroutes
void CALLBACK drp_callback(const wchar_t *szFile,DWORD action,LPARAM lParam)
{
    UNREFERENCED_PARAMETER(action);
    UNREFERENCED_PARAMETER(lParam);

    if(StrStrIW(szFile,L".7z")&&Updater.isPaused())invalidate(INVALIDATE_INDEXES);
}

void lang_refresh()
{
    if(!hMain||!hField)
    {
        log_err("ERROR in lang_refresh(): hMain is %d, hField is %d\n",hMain,hField);
        return;
    }

    rtl=language[STR_RTL].val;
    setMirroring(hMain);
    setMirroring(hLang);
    setMirroring(hTheme);
    setMirroring(hField);
    setMirroring(hPopup);

    RECT rect;
    GetWindowRect(hMain,&rect);
    MoveWindow(hMain,rect.left,rect.top,D(MAINWND_WX),D(MAINWND_WY)+1,1);
    MoveWindow(hMain,rect.left,rect.top,D(MAINWND_WX),D(MAINWND_WY),1);
    InvalidateRect(hPopup,nullptr,0);
}

void theme_refresh()
{
    // Set font
    if(hFont&&!DeleteObject(hFont))
        log_err("ERROR in manager_setfont(): failed DeleteObject\n");

    hFont=CreateFont(-D(FONT_SIZE),0,0,0,FW_DONTCARE,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,
                CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,VARIABLE_PITCH,D_STR(FONT_NAME));
    if(!hFont)
        log_err("ERROR in manager_setfont(): failed CreateFont\n");
    SendMessage(hTheme,WM_SETFONT,(WPARAM)hFont,MAKELPARAM(FALSE,0));
    SendMessage(hLang,WM_SETFONT,(WPARAM)hFont,MAKELPARAM(FALSE,0));

    if(!hMain||!hField)
    {
        log_err("ERROR in theme_refresh(): hMain is %d, hField is %d\n",hMain,hField);
        return;
    }

    // Resize window
    RECT rect;
    GetWindowRect(hMain,&rect);
    MoveWindow(hMain,rect.left,rect.top,D(MAINWND_WX),D(MAINWND_WY)+1,1);
    MoveWindow(hMain,rect.left,rect.top,D(MAINWND_WX),D(MAINWND_WY),1);
}

void snapshot()
{
    OPENFILENAME ofn;
    memset(&ofn,0,sizeof(OPENFILENAME));
    ofn.lStructSize=sizeof(OPENFILENAME);
    ofn.hwndOwner  =hMain;
    ofn.lpstrFilter=STR(STR_OPENSNAPSHOT);
    ofn.nMaxFile   =BUFLEN;
    ofn.lpstrDefExt=L"snp";
    ofn.lpstrFile  =state_file;
    ofn.Flags      =OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;

    if(GetOpenFileName(&ofn))
    {
        statemode=STATEMODE_EMUL;
        invalidate(INVALIDATE_DEVICES|INVALIDATE_SYSINFO|INVALIDATE_MANAGER);
    }
}

void extractto()
{
    wchar_t dir[BUFLEN];

    BROWSEINFO lpbi;
    memset(&lpbi,0,sizeof(BROWSEINFO));
    lpbi.hwndOwner=hMain;
    lpbi.pszDisplayName=dir;
    lpbi.lpszTitle=STR(STR_EXTRACTFOLDER);
    lpbi.ulFlags=BIF_NEWDIALOGSTYLE|BIF_EDITBOX;

    LPITEMIDLIST list=SHBrowseForFolder(&lpbi);
    if(list)
    {
        SHGetPathFromIDList(list,dir);

        int argc;
        wchar_t buf[BUFLEN];
        wchar_t **argv=CommandLineToArgvW(GetCommandLineW(),&argc);
        wsprintf(buf,L"%s\\drv.exe",dir);
        if(!CopyFile(argv[0],buf,0))
            log_err("ERROR in extractto(): failed CopyFile(%S,%S)\n",argv[0],buf);
        LocalFree(argv);

        wcscat(dir,L"\\drivers");
        wcscpy(extractdir,dir);
        manager_g->install(OPENFOLDER);
    }
}

void selectDrpDir()
{
    BROWSEINFO lpbi;
    memset(&lpbi,0,sizeof(BROWSEINFO));
    lpbi.hwndOwner=hMain;
    lpbi.pszDisplayName=drpext_dir;
    lpbi.lpszTitle=STR(STR_EXTRACTFOLDER);
    lpbi.ulFlags=BIF_NEWDIALOGSTYLE|BIF_EDITBOX;

    LPITEMIDLIST list=SHBrowseForFolder(&lpbi);
    if(list)
    {
        SHGetPathFromIDList(list,drpext_dir);
        //int len=wcslen(drpext_dir);
        //drpext_dir[len]=0;
        invalidate(INVALIDATE_INDEXES|INVALIDATE_MANAGER);
    }
}

void invalidate(int v)
{
    invaidate_set|=v;
    SetEvent(deviceupdate_event);
}
//}

//{ Scrollbar
void setscrollrange(int y)
{
    if(!hField)
    {
        log_err("ERROR in setscrollrange(): hField is 0\n");
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

int getscrollpos()
{
    if(!hField)
    {
        log_err("ERROR in getscrollpos(): hField is 0\n");
        return 0;
    }

    SCROLLINFO si;
    si.cbSize=sizeof(si);
    si.fMask=SIF_POS;
    si.nPos=0;
    GetScrollInfo(hField,SB_VERT,&si);
    return si.nPos;
}

void setscrollpos(int pos)
{
    if(!hField)
    {
        log_err("ERROR in setscrollpos(): hField is 0\n");
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
void get_resource(int id,void **data,int *size)
{
    HRSRC myResource=FindResource(nullptr,MAKEINTRESOURCE(id),(wchar_t *)RESFILE);
    if(!myResource)
    {
        log_err("ERROR in get_resource(): failed FindResource(%d)\n",id);
        *size=0;
        *data=nullptr;
        return;
    }
    *size=SizeofResource(nullptr,myResource);
    *data=LoadResource(nullptr,myResource);
}

void mkdir_r(const wchar_t *path)
{
    if(path[1]==L':'&&path[2]==0)return;
    if(!canWrite(path))
    {
        log_err("ERROR in mkdir_r(): Write-protected,'%S'\n",path);
        return;
    }

    wchar_t buf[BUFLEN];
    wcscpy(buf,path);
    wchar_t *p=buf;
    while((p=wcschr(p,L'\\')))
    {
        *p=0;
        if(_wmkdir(buf)<0&&errno!=EEXIST&&lstrlenW(buf)>2)
            log_err("ERROR in mkdir_r(): failed _wmkdir(%S,%d)\n",buf,errno);
        *p=L'\\';
        p++;
    }
    if(_wmkdir(buf)<0&&errno!=EEXIST&&lstrlenW(buf)>2)
        log_err("ERROR in mkdir_r(): failed _wmkdir(%S,%d)\n",buf,errno);
}

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
    MapWindowPoints(nullptr,hWnd,(LPPOINT)&rc,2);
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
    if(GetTickCount()-t>20&&log_verbose&LOG_VERBOSE_LAGCOUNTER)
        log_con("GUI lag in %S[%X]: %ld\n",str,uMsg,GetTickCount()-t);
}

void redrawfield()
{
    if(flags&FLAG_NOGUI)return;
    if(!hField)
    {
        log_err("ERROR in redrawfield(): hField is 0\n");
        return;
    }
    InvalidateRect(hField,nullptr,0);
}

void redrawmainwnd()
{
    if(flags&FLAG_NOGUI)return;
    if(!hMain)
    {
        log_err("ERROR in redrawmainwnd(): hMain is 0\n");
        return;
    }
    InvalidateRect(hMain,nullptr,0);
}

void tabadvance(int v)
{
    while(1)
    {
        kbpanel+=v;
        if(!kbpanel)kbpanel=KB_PANEL_CHK;
        if(kbpanel>KB_PANEL_CHK)kbpanel=KB_FIELD;

        if(kbpanel==KB_PANEL_CHK&&!D(PANEL12_WY))continue;
        if(!expertmode&&kbpanel>=KB_ACTIONS&&kbpanel<=KB_PANEL3)continue;
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
void arrowsAdvance(int v)
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

    for(int i=0;i<NUM_PANELS;i++)panels[i].keybAdvance(v);
    redrawmainwnd();
}
//}

//{ GUI
LRESULT CALLBACK WndProcCommon(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);

    RECT rect;
    short x,y;

    x=LOWORD(lParam);
    y=HIWORD(lParam);
    switch(uMsg)
    {
        case WM_MOUSELEAVE:
            ShowWindow(hPopup,SW_HIDE);
            InvalidateRect(hwnd,nullptr,0);
            break;

        case WM_MOUSEHOVER:
            InvalidateRect(hPopup,nullptr,0);
            ShowWindow(hPopup,floating_type==FLOATING_NONE?SW_HIDE:SW_SHOWNOACTIVATE);
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

LRESULT CALLBACK WndProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    RECT rect;
    short x,y;

    int i;
    int j,f;
    int wp;
    long long timer=GetTickCount();

    x=LOWORD(lParam);
    y=HIWORD(lParam);

    if(WndProcCommon(hwnd,uMsg,wParam,lParam))
    switch(uMsg)
    {
        case WM_CREATE:
            // Canvas
            canvasMain=new Canvas;
            hMain=hwnd;

            // Field
            hField=CreateWindowMF(classField,nullptr,hwnd,nullptr,WS_VSCROLL);

            // Popup
            hPopup=CreateWindowEx(WS_EX_LAYERED|WS_EX_NOACTIVATE|WS_EX_TOPMOST|WS_EX_TRANSPARENT,
                classPopup,L"",WS_POPUP,
                0,0,0,0,hwnd,(HMENU)nullptr,ghInst,nullptr);

            // Lang
            hLang=CreateWindowMF(WC_COMBOBOX,L"",hwnd,(HMENU)ID_LANG,CBS_DROPDOWNLIST|CBS_HASSTRINGS|WS_OVERLAPPED|WS_VSCROLL);
            PostMessage(hwnd,WM_UPDATELANG,0,0);

            // Theme
            hTheme=CreateWindowMF(WC_COMBOBOX,L"",hwnd,(HMENU)ID_THEME,CBS_DROPDOWNLIST|CBS_HASSTRINGS|WS_OVERLAPPED|WS_VSCROLL);
            PostMessage(hwnd,WM_UPDATETHEME,1,0);

            // Misc
            vault_startmonitors();
            DragAcceptFiles(hwnd,1);

            manager_g->populate();
            manager_g->filter(filters);
            manager_g->setpos();
            break;

        case WM_CLOSE:
            if(installmode==MODE_NONE||(flags&FLAG_AUTOCLOSE))
                DestroyWindow(hwnd);
            else if(MessageBox(hMain,STR(STR_INST_QUIT_MSG),STR(STR_INST_QUIT_TITLE),MB_YESNO|MB_ICONQUESTION)==IDYES)
                installmode=MODE_STOPPING;
            break;

        case WM_DESTROY:
            if(!DeleteObject(hFont))
                log_err("ERROR in manager_free(): failed DeleteObject\n");
            if(mon_lang)mon_lang->stop();
            if(mon_theme)mon_theme->stop();
            delete canvasMain;
            PostQuitMessage(0);
            break;

        case WM_UPDATELANG:
            SendMessage(hLang,CB_RESETCONTENT,0,0);
            lang_enum(hLang,L"langs",manager_g->matcher->getState()->getLocale());
            f=SendMessage(hLang,CB_FINDSTRINGEXACT,-1,(LPARAM)curlang);
            if(f==CB_ERR)f=SendMessage(hLang,CB_GETCOUNT,0,0)-1;
            lang_set(f);
            SendMessage(hLang,CB_SETCURSEL,f,0);
            lang_refresh();
            break;

        case WM_UPDATETHEME:
            SendMessage(hTheme,CB_RESETCONTENT,0,0);
            theme_enum(hTheme,L"themes");
            f=SendMessage(hTheme,CB_FINDSTRINGEXACT,-1,(LPARAM)curtheme);
            if(f==CB_ERR)f=vTheme.pickTheme();
            theme_set(f);
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
                Bundle *bb=(Bundle *)wParam;
                Manager *manager_prev=manager_g;
                log_con("{Sync");
                EnterCriticalSection(&sync);
                log_con("...\n");
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
                        wcscpy(drpext_dir,lpszFile);
                        invalidate(INVALIDATE_INDEXES|INVALIDATE_MANAGER);
                    }
                    else if(StrStrI(lpszFile,L".snp"))
                    {
                        wcscpy(state_file,lpszFile);
                        statemode=STATEMODE_EMUL;
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
                    wpos->cx=rect.right-20;
                    wpos->cy=rect.bottom-20;
                    wpos->x=10;
                    wpos->y=10;
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
            if(ctrl_down&&wParam==L'A')PostMessage(hMain,WM_COMMAND,ID_SELECT_ALL,0);
            if(ctrl_down&&wParam==L'N')PostMessage(hMain,WM_COMMAND,ID_SELECT_NONE,0);
            if(ctrl_down&&wParam==L'I')PostMessage(hMain,WM_COMMAND,ID_INSTALL,0);
            if(ctrl_down&&wParam==L'P')
            {
                panels[11].flipChecked(2);
                PostMessage(hMain,WM_COMMAND,ID_RESTPNT,0);
                redrawmainwnd();
            }
            if(ctrl_down&&wParam==L'R')
            {
                panels[11].flipChecked(3);
                PostMessage(hMain,WM_COMMAND,ID_REBOOT,0);
                redrawmainwnd();
            }
            if(wParam==VK_F5&&ctrl_down)
                invalidate(INVALIDATE_SYSINFO|INVALIDATE_MANAGER);else
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
                Autoclicker.wndclicker(2);
                MessageBox(hMain,L"Windows data recorded into the log.",L"Message",0);
            }
            if(wParam==VK_F8)
            {
                switch(flags&(FLAG_SHOWDRPNAMES1|FLAG_SHOWDRPNAMES2))
                {
                    case FLAG_SHOWDRPNAMES1:
                        flags^=FLAG_SHOWDRPNAMES1;
                        flags^=FLAG_SHOWDRPNAMES2;
                        break;

                    case FLAG_SHOWDRPNAMES2:
                        flags^=FLAG_SHOWDRPNAMES2;
                        break;

                    case 0:
                        flags^=FLAG_SHOWDRPNAMES1;
                        break;

                    default:
                        break;
                }
                manager_g->filter(filters);
                manager_g->setpos();
                redrawfield();
            }
            break;

        case WM_SYSKEYDOWN:
            if(wParam==VK_MENU)break;
            return DefWindowProc(hwnd,uMsg,wParam,lParam);

        case WM_DEVICECHANGE:
            if(installmode==MODE_INSTALLING)break;
            log_con("WM_DEVICECHANGE(%x,%x)\n",wParam,lParam);
            if(lParam<2)
                instflag|=RESTOREPOS;
            else
                instflag&=~RESTOREPOS;

            invalidate(INVALIDATE_DEVICES);
            break;

        case WM_SIZE:
            SetLayeredWindowAttributes(hMain,0,D(MAINWND_TRANSPARENCY),LWA_ALPHA);
            SetLayeredWindowAttributes(hPopup,0,D(POPUP_TRANSPARENCY),LWA_ALPHA);

            GetWindowRect(hwnd,&rect);
            main1x_c=x;
            main1y_c=y;

            i=D(PNLITEM_OFSX)+D(PANEL_LIST_OFSX);
            j=D(PANEL_LIST_OFSX)?0:1;
            f=D(PANEL_LIST_OFSX)?4:0;
            MoveWindow(hField,Xm(D(DRVLIST_OFSX),D(DRVLIST_WX)),Ym(D(DRVLIST_OFSY)),XM(D(DRVLIST_WX),D(DRVLIST_OFSX)),YM(D(DRVLIST_WY),D(DRVLIST_OFSY)),TRUE);

            panels[2].moveWindow(hLang,i,j,f);
            j=D(PANEL_LIST_OFSX)?1:3;
            panels[2].moveWindow(hTheme,i,j,f);
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

            drawbox(canvasMain->getDC(),0,0,rect.right+1,rect.bottom+1,BOX_MAINWND);
            SelectObject(canvasMain->getDC(),hFont);
            panels[7].draw(canvasMain->getDC());// draw revision
            for(i=0;i<NUM_PANELS;i++)if(i!=7)
            {
                panels[i].draw(canvasMain->getDC());
            }
            canvasMain->end();
            break;

        case WM_ERASEBKGND:
            return 1;

        case WM_MOUSEMOVE:
            GetClientRect(hwnd,&rect);
            i=panels_hitscan(x,y,&j);
            if(j==0||j==7||j==12)SetCursor(LoadCursor(nullptr,IDC_HAND));

            if(i>0)
            {
                if((i==1&&j==7)||(j==12))
                    drawpopup(-1,FLOATING_ABOUT,x,y,hwnd);
                else
                    drawpopup(panels[j].getStr(i)+1,i>0&&i<4&&j==0?FLOATING_SYSINFO:FLOATING_TOOLTIP,x,y,hwnd);
            }
            else
                drawpopup(-1,FLOATING_NONE,x,y,hwnd);

            if(panel_lasti!=i+j*256)
            {
                if(j>=0)panels[j].draw_inv();
                panels[panel_lasti/256].draw_inv();
            }
            if(j>=0)panel_lasti=i+j*256;else panel_lasti=0;
            break;

        case WM_LBUTTONUP:
            if(!mouseclick)break;

            i=panels_hitscan(x,y,&j);
            if(i<0)break;

            if(j==7||j==12)
                run_command(L"open",L"http://snappy-driver-installer.sourceforge.net",SW_SHOWNORMAL,0);
            else if(i<4&&j==0)
                run_command(L"devmgmt.msc",nullptr,SW_SHOW,0);
            else
                panels[j].click(i);
            break;

        case WM_RBUTTONUP:
            i=panels_hitscan(x,y,&j);
            if(i<0)break;
            if(i==2&&j==11)
            {
                floating_itembar=SLOT_RESTORE_POINT;
                manager_g->contextmenu(x-Xm(D(DRVLIST_OFSX),D(DRVLIST_WX)),y-Ym(D(DRVLIST_OFSY)));
            }
            break;

        case WM_RBUTTONDOWN:
            i=panels_hitscan(x,y,&j);
            if(i>=0&&i<4&&j==0)manager_g->matcher->getState()->contextmenu2(x,y);
            break;

        case WM_MOUSEWHEEL:
            i=GET_WHEEL_DELTA_WPARAM(wParam);
            if(space_down)
            {
                horiz_sh-=i/5;
                if(horiz_sh>0)horiz_sh=0;
                InvalidateRect(hPopup,nullptr,0);
            }
            else
                SendMessage(hField,WM_VSCROLL,MAKELONG(i>0?SB_LINEUP:SB_LINEDOWN,0),0);
            break;

        case WM_COMMAND:
            wp=LOWORD(wParam);
            switch(wp)
            {
                case ID_SCHEDULE:
                    manager_g->toggle(floating_itembar);
                    redrawfield();
                    break;

                case ID_SHOWALT:
                    if(floating_itembar==SLOT_RESTORE_POINT)
                    {
                        run_command(L"cmd",L"/c %windir%\\Sysnative\\rstrui.exe",SW_HIDE,0);
                        run_command(L"cmd",L"/c %windir%\\system32\\Restore\\rstrui.exe",SW_HIDE,0);
                    }
                    else
                    {
                        manager_g->expand(floating_itembar);
                    }
                    break;

                case ID_OPENINF:
                case ID_LOCATEINF:
                    manager_g->getINFpath(wp);
                    break;

                case ID_DEVICEMNG:
                    run_command(L"devmgmt.msc",nullptr,SW_SHOW,0);
                    break;

                case ID_EMU_32:
                    virtual_arch_type=32;
                    invalidate(INVALIDATE_SYSINFO|INVALIDATE_MANAGER);
                    break;

                case ID_EMU_64:
                    virtual_arch_type=64;
                    invalidate(INVALIDATE_SYSINFO|INVALIDATE_MANAGER);
                    break;

                case ID_DIS_INSTALL:
                    flags^=FLAG_DISABLEINSTALL;
                    break;

                case ID_DIS_RESTPNT:
                    flags^=FLAG_NORESTOREPOINT;
                    manager_g->set_rstpnt(flags&FLAG_NORESTOREPOINT?1:0);
                    break;

                default:
                    break;
            }
            if(wp>=ID_WIN_2000&&wp<=ID_WIN_10)
            {
                virtual_os_version=windows_ver[wp-ID_WIN_2000];
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
                    run_command(L"iexplore.exe",buf2,SW_SHOW,0);

                }
                else
                {
                    const wchar_t *str=manager_g->getHWIDby(id);
                    int len=wcslen(str)*2+2;
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
                    i=SendMessage((HWND)lParam,CB_GETCURSEL,0,0);
                    SendMessage((HWND)lParam,CB_GETLBTEXT,i,(LPARAM)curlang);
                    lang_set(i);
                    lang_refresh();
                }

                if(wp==ID_THEME)
                {
                    i=SendMessage((HWND)lParam,CB_GETCURSEL,0,0);
                    SendMessage((HWND)lParam,CB_GETLBTEXT,i,(LPARAM)curtheme);
                    theme_set(i);
                    theme_refresh();
                }
            }

            if(HIWORD(wParam)==BN_CLICKED)
            switch(wp)
            {
                case ID_INSTALL:
                    if(installmode==MODE_NONE)
                    {
                        if((flags&FLAG_EXTRACTONLY)==0)
                        wsprintf(extractdir,L"%s\\SDI",manager_g->matcher->getState()->textas.get(manager_g->matcher->getState()->getTemp()));
                        manager_g->install(INSTALLDRIVERS);
                    }
                    break;

                case ID_SELECT_NONE:
                    manager_g->selectnone();
                    redrawmainwnd();
                    redrawfield();
                    break;

                case ID_SELECT_ALL:
                    manager_g->selectall();
                    redrawmainwnd();
                    redrawfield();
                    break;

                case ID_OPENLOGS:
                    ShellExecute(hwnd,L"explore",log_dir,nullptr,nullptr,SW_SHOW);
                    break;

                case ID_SNAPSHOT:
                    snapshot();
                    break;

                case ID_EXTRACT:
                    extractto();
                    break;

                case ID_DRVDIR:
                    selectDrpDir();
                    break;

                case ID_SHOW_MISSING:
                case ID_SHOW_NEWER:
                case ID_SHOW_CURRENT:
                case ID_SHOW_OLD:
                case ID_SHOW_BETTER:
                case ID_SHOW_WORSE_RANK:
                case ID_SHOW_NF_MISSING:
                case ID_SHOW_NF_UNKNOWN:
                case ID_SHOW_NF_STANDARD:
                case ID_SHOW_ONE:
                case ID_SHOW_DUP:
                case ID_SHOW_INVALID:
                    filters=0;
                    for(i=0;i<NUM_PANELS;i++)filters+=panels[i].calcFilters();
                    manager_g->filter(filters);
                    manager_g->setpos();
                    break;

                case ID_RESTPNT:
                    manager_g->set_rstpnt(panels[11].isChecked(2));
                    break;

                case ID_REBOOT:
                    if(panels[11].isChecked(3))
                        flags|=FLAG_AUTOINSTALL;
                    else
                        flags&=~FLAG_AUTOINSTALL;
                    break;

                default:
                    break;
            }
            break;

        default:
            i=DefWindowProc(hwnd,uMsg,wParam,lParam);
            checktimer(L"MainD",timer,uMsg);
            return i;
    }
    checktimer(L"Main",timer,uMsg);
    return 0;
}

LRESULT CALLBACK WindowGraphProcedure(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
    SCROLLINFO si;
    RECT rect;
    short x,y;
    long long timer=GetTickCount();
    int i;

    x=LOWORD(lParam);
    y=HIWORD(lParam);
    if(WndProcCommon(hwnd,message,wParam,lParam))
    switch(message)
    {
        case WM_CREATE:
            canvasField=new Canvas;
            break;

        case WM_PAINT:
            y=getscrollpos();

            GetClientRect(hwnd,&rect);
            canvasField->begin(hwnd,rect.right,rect.bottom);

            BitBlt(canvasField->getDC(),0,0,rect.right,rect.bottom,canvasMain->getDC(),Xm(D(DRVLIST_OFSX),D(DRVLIST_WX)),Ym(D(DRVLIST_OFSY)),SRCCOPY);
            manager_g->draw(canvasField->getDC(),y);

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
            manager_g->hitscan(x,y,&floating_itembar,&i);
            if(floating_itembar==SLOT_SNAPSHOT)
            {
                statemode=STATEMODE_REAL;
                invalidate(INVALIDATE_DEVICES|INVALIDATE_SYSINFO|INVALIDATE_MANAGER);
            }
            if(floating_itembar==SLOT_DPRDIR)
            {
                *drpext_dir=0;
                invalidate(INVALIDATE_INDEXES|INVALIDATE_MANAGER);
            }
            if(floating_itembar==SLOT_EXTRACTING)
            {
                if(installmode==MODE_INSTALLING)
                    installmode=MODE_STOPPING;
                else if(installmode==MODE_NONE)
                    manager_g->clear();
            }
            if(floating_itembar==SLOT_DOWNLOAD)
            {
                #ifdef USE_TORRENT
                UpdateDialog.openDialog();
                #endif
                break;
            }

            if(floating_itembar>=0&&(i==1||i==0||i==3))
            {
                manager_g->toggle(floating_itembar);
                if(wParam&MK_SHIFT&&installmode==MODE_NONE)
                {
                    if((flags&FLAG_EXTRACTONLY)==0)
                    wsprintf(extractdir,L"%s\\SDI",manager_g->matcher->getState()->textas.get(manager_g->matcher->getState()->getTemp()));
                    manager_g->install(INSTALLDRIVERS);
                }
                redrawfield();
            }
            if(floating_itembar>=0&&i==2)
            {
                manager_g->expand(floating_itembar);
            }
            break;

        case WM_RBUTTONDOWN:
            manager_g->hitscan(x,y,&floating_itembar,&i);
            if(floating_itembar>=0&&(i==0||i==3))
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
                if(ctrl_down||expertmode)type=FLOATING_CMPDRIVER;

                manager_g->hitscan(x,y,&itembar_i,&i);
                if(i==0&&itembar_i>=RES_SLOTS&&(ctrl_down||space_down||expertmode))
                    drawpopup(itembar_i,type,x,y,hField);
                else if(itembar_i==SLOT_VIRUS_AUTORUN)
                    drawpopup(STR_VIRUS_AUTORUN_H,FLOATING_TOOLTIP,x,y,hField);
                else if(itembar_i==SLOT_VIRUS_RECYCLER)
                    drawpopup(STR_VIRUS_RECYCLER_H,FLOATING_TOOLTIP,x,y,hField);
                else if(itembar_i==SLOT_VIRUS_HIDDEN)
                    drawpopup(STR_VIRUS_HIDDEN_H,FLOATING_TOOLTIP,x,y,hField);
                else if(itembar_i==SLOT_EXTRACTING&&installmode)
                    drawpopup(instflag&INSTALLDRIVERS?STR_HINT_STOPINST:STR_HINT_STOPEXTR,FLOATING_TOOLTIP,x,y,hField);
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
            i=DefWindowProc(hwnd,message,wParam,lParam);
            checktimer(L"ListD",timer,message);
            return i;
    }
    checktimer(L"List",timer,message);
    return 0;
}

LRESULT CALLBACK PopupProcedure(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
    HDC hdcMem;
    RECT rect;
    WINDOWPOS *wp;

    switch(message)
    {
        case WM_WINDOWPOSCHANGING:
            if(floating_type!=FLOATING_TOOLTIP)break;

            wp=(WINDOWPOS*)lParam;
            hdcMem=GetDC(hwnd);
            GetClientRect(hwnd,&rect);
            rect.right=D(POPUP_WX);
            rect.bottom=floating_y;

            SelectObject(hdcMem,hFont);
            DrawText(hdcMem,STR(floating_itembar),-1,&rect,DT_WORDBREAK|DT_CALCRECT);

            AdjustWindowRectEx(&rect,WS_POPUPWINDOW|WS_VISIBLE,0,0);
            popup_resize(rect.right-rect.left+D(POPUP_OFSX)*2,rect.bottom-rect.top+D(POPUP_OFSY)*2);
            wp->cx=rect.right+D(POPUP_OFSX)*2;
            wp->cy=rect.bottom+D(POPUP_OFSY)*2;
            ReleaseDC(hwnd,hdcMem);
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

            drawbox(canvasPopup->getDC(),0,0,rect.right,rect.bottom,BOX_POPUP);
            switch(floating_type)
            {
                case FLOATING_SYSINFO:
                    SelectObject(canvasPopup->getDC(),hFont);
                    manager_g->matcher->getState()->popup_sysinfo(canvasPopup->getDC());
                    break;

                case FLOATING_TOOLTIP:
                    rect.left+=D(POPUP_OFSX);
                    rect.top+=D(POPUP_OFSY);
                    rect.right-=D(POPUP_OFSX);
                    rect.bottom-=D(POPUP_OFSY);
                    SelectObject(canvasPopup->getDC(),hFont);
                    SetTextColor(canvasPopup->getDC(),D(POPUP_TEXT_COLOR));
                    DrawText(canvasPopup->getDC(),STR(floating_itembar),-1,&rect,DT_WORDBREAK);
                    break;

                case FLOATING_CMPDRIVER:
                    SelectObject(canvasPopup->getDC(),hFont);
                    manager_g->popup_drivercmp(manager_g,canvasPopup->getDC(),rect,floating_itembar);
                    break;

                case FLOATING_DRIVERLST:
                    SelectObject(canvasPopup->getDC(),hFont);
                    manager_g->popup_driverlist(canvasPopup->getDC(),rect,floating_itembar);
                    break;

                case FLOATING_ABOUT:
                    SelectObject(canvasPopup->getDC(),hFont);
                    popup_about(canvasPopup->getDC());
                    break;

                case FLOATING_DOWNLOAD:
                    SelectObject(canvasPopup->getDC(),hFont);
                    #ifdef USE_TORRENT
                    Updater.showPopup(canvasPopup->getDC());
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
                    license=2;
                    EndDialog(hwnd,IDOK);
                    return TRUE;

                case IDCANCEL:
                    license=0;
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
