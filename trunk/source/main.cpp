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
#define _wcsicmp StrCmpIW

// Manager
manager_t manager_v[2];
manager_t *manager_g=&manager_v[0];
int manager_active=0;
int bundle_display=1;
int bundle_shadow=0;
int volatile installmode=MODE_NONE;
CRITICAL_SECTION sync;

// Window
HINSTANCE ghInst;
MSG msg;
HFONT hFont=0;
Canvas canvasMain;
Canvas canvasField;
Canvas canvasPopup;
const WCHAR classMain[]= L"classMain";
const WCHAR classField[]=L"classField";
const WCHAR classPopup[]=L"classPopup";
HWND hMain=0;
HWND hField=0;
HWND hPopup=0;
HWND hLang=0;
HWND hTheme=0;

// Window helpers
int panel_lasti=0;
int field_lasti,field_lastz;
int main1x_c,main1y_c;
int mainx_c,mainy_c;
int mainx_w,mainy_w;
int mousex=-1,mousey=-1,mousedown=0,mouseclick=0;
int cntd=0;
int hideconsole=SW_HIDE;
unsigned offset_target=0;

int kbpanel=KB_NONE;
int kbitem[]={0,0,0,0,0,0,0,0,0,0,0};
int ctrl_down=0;
int space_down=0;
int shift_down=0;
int floating_type=0;
int floating_itembar=-1;
int floating_x=1,floating_y=1;
int horiz_sh=0;
int scrollvisible=0;

int ret_global=0;
volatile int deviceupdate_exitflag=0;
FILE *snplist=0;
HANDLE deviceupdate_event;

// Settings
WCHAR drp_dir   [BUFLEN]=L"drivers";
WCHAR drpext_dir[BUFLEN]=L"";
WCHAR index_dir [BUFLEN]=L"indexes\\SDI";
WCHAR output_dir[BUFLEN]=L"indexes\\SDI\\txt";
WCHAR data_dir  [BUFLEN]=L"tools\\SDI";
WCHAR logO_dir  [BUFLEN]=L"logs";
WCHAR log_dir   [BUFLEN];

WCHAR state_file[BUFLEN]=L"untitled.snp";
WCHAR finish    [BUFLEN]=L"";
WCHAR finish_upd[BUFLEN]=L"";
WCHAR finish_rb [BUFLEN]=L"";
WCHAR HWIDs     [BUFLEN]=L"";

//int flags=COLLECTION_USE_LZMA;
int flags=0;
int statemode=0;
int expertmode=0;
int license=0;
WCHAR curlang [BUFLEN]=L"";
WCHAR curtheme[BUFLEN]=L"(default)";
int hintdelay=500;
int filters=
    (1<<ID_SHOW_MISSING)+
    (1<<ID_SHOW_NEWER)+
    (1<<ID_SHOW_BETTER)+
    (1<<ID_SHOW_NF_MISSING)+
    (1<<ID_SHOW_ONE);
int virtual_os_version=0;
int virtual_arch_type=0;

#define NUM_OS 8
const WCHAR *windows_name[NUM_OS]=
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

void panel_setfilters(panel_t *panel)
{
    int i,j;

    for(j=0;j<7;j++)
    for(i=0;i<panel[j].items[0].action_id+1;i++)
        if(panel[j].items[i].action_id>=ID_SHOW_MISSING&&panel[j].items[i].action_id<=ID_SHOW_INVALID)
            panel[j].items[i].checked=filters&(1<<panel[j].items[i].action_id)?1:0;
}

//{ Main
int main2(int argc, char* argv[]);
void str_unicode2ansi(char *a)
{
    WCHAR *u=(WCHAR *)a;
    while((*a++=*u++));
}
void str_unicode2ansi(const WCHAR *s,char *d)
{
    while((*d++=*s++));
}
int _wtoi_my(const WCHAR *str)
{
    int val;
    swscanf(str,L"%d",&val);
    return val;
}

void settings_parse(const WCHAR *str,int ind)
{
    WCHAR buf[BUFLEN];
    WCHAR **argv,*pr;
    int argc;
    int i;

    log_con("Args:[%S]\n",str);
    argv=CommandLineToArgvW(str,&argc);
    for(i=ind;i<argc;i++)
    {
        pr=argv[i];
        if (pr[0] == '/') pr[0]='-';
        if( wcsstr(pr,L"-drp_dir:"))     wcscpy(drp_dir,pr+9);else
        if( wcsstr(pr,L"-index_dir:"))   wcscpy(index_dir,pr+11);else
        if( wcsstr(pr,L"-output_dir:"))  wcscpy(output_dir,pr+12);else
        if( wcsstr(pr,L"-data_dir:"))    wcscpy(data_dir,pr+10);else
        if( wcsstr(pr,L"-log_dir:"))     {wcscpy(logO_dir,pr+9);
        ExpandEnvironmentStrings(logO_dir,log_dir,BUFLEN);
        }else
        if( wcsstr(pr,L"-finish_cmd:"))  wcscpy(finish,pr+12);else
        if( wcsstr(pr,L"-finishrb_cmd:"))wcscpy(finish_rb,pr+14);else
        if( wcsstr(pr,L"-finish_upd_cmd:"))wcscpy(finish_upd,pr+16);else
        if( wcsstr(pr,L"-lang:"))        wcscpy(curlang,pr+6);else
        if( wcsstr(pr,L"-theme:"))       wcscpy(curtheme,pr+7);else
        if(!wcscmp(pr,L"-expertmode"))   expertmode=1;else
        if( wcsstr(pr,L"-hintdelay:"))   hintdelay=_wtoi_my(pr+11);else
        if( wcsstr(pr,L"-port:"))        torrentport=_wtoi_my(pr+6);else
        if( wcsstr(pr,L"-downlimit:"))   downlimit=_wtoi_my(pr+11);else
        if( wcsstr(pr,L"-uplimit:"))     uplimit=_wtoi_my(pr+9);else
        if( wcsstr(pr,L"-filters:"))     filters=_wtoi_my(pr+9);else
        if(!wcscmp(pr,L"-license"))      license=1;else
        if(!wcscmp(pr,L"-norestorepnt")) flags|=FLAG_NORESTOREPOINT;else
        if(!wcscmp(pr,L"-showdrpnames1"))flags|=FLAG_SHOWDRPNAMES1;else
        if(!wcscmp(pr,L"-showdrpnames2"))flags|=FLAG_SHOWDRPNAMES2;else
        if(!wcscmp(pr,L"-oldstyle"))     flags|=FLAG_OLDSTYLE;else
        if(!wcscmp(pr,L"-preservecfg"))  flags|=FLAG_PRESERVECFG;else
        if(!wcscmp(pr,L"-showconsole"))  flags|=FLAG_SHOWCONSOLE;else
        if(!wcscmp(pr,L"-checkupdates")) flags|=FLAG_CHECKUPDATES;else
        if(!wcscmp(pr,L"-onlyupdates"))  flags|=FLAG_ONLYUPDATES;else
        if(!wcscmp(pr,L"-7z"))
        {
            WCHAR cmd[BUFLEN];
            wsprintf(cmd,L"7za.exe %s",wcsstr(GetCommandLineW(),L"-7z")+4);
            log_con("Executing '%S'\n",cmd);
            registerall();
            statemode=STATEMODE_EXIT;
            ret_global=Extract7z(cmd);
            log_con("Ret: %d\n",ret_global);
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
            log_con("Install '%S' '%s'\n",argv[i+1],argv[i+2]);
            GetEnvironmentVariable(L"TEMP",buf,BUFLEN);
            wsprintf(extractdir,L"%s\\SDI",buf);
            installmode=MODE_INSTALLING;
            driver_install(argv[i+1],argv[i+2],&ret_global,&needreboot);
            log_con("Ret: %X,%d\n",ret_global,needreboot);
            if(needreboot)ret_global|=0x80000000;
            wsprintf(buf,L" /c rd /s /q \"%s\"",extractdir);
            RunSilent(L"cmd",buf,SW_HIDE,1);
            statemode=STATEMODE_EXIT;
            break;
        }
        else
        if( wcsstr(pr,L"-hwid:"))        wcscpy(HWIDs,pr+6);else
        if(!wcscmp(pr,L"-filtersp"))     flags|=FLAG_FILTERSP;else
        if(!wcscmp(pr,L"-reindex"))      flags|=COLLECTION_FORCE_REINDEXING;else
        if(!wcscmp(pr,L"-index_hr"))     flags|=COLLECTION_PRINT_INDEX;else
        if(!wcscmp(pr,L"-nogui"))        flags|=FLAG_NOGUI|FLAG_AUTOCLOSE;else
        if(!wcscmp(pr,L"-autoinstall"))  flags|=FLAG_AUTOINSTALL;else
        if(!wcscmp(pr,L"-autoclose"))    flags|=FLAG_AUTOCLOSE;else
        if(!wcscmp(pr,L"-autoupdate"))   flags|=FLAG_AUTOUPDATE;else
        if(!wcscmp(pr,L"-nologfile"))    flags|=FLAG_NOLOGFILE;else
        if(!wcscmp(pr,L"-nosnapshot"))   flags|=FLAG_NOSNAPSHOT;else
        if(!wcscmp(pr,L"-nostamp"))      flags|=FLAG_NOSTAMP;else
        if( wcsstr(pr,L"-extractdir:"))  {flags|=FLAG_EXTRACTONLY;wcscpy(extractdir,pr+12);}else
        if(!wcscmp(pr,L"-keepunpackedindex"))flags|=FLAG_KEEPUNPACKINDEX;else
        if(!wcscmp(pr,L"-keeptempfiles"))flags|=FLAG_KEEPTEMPFILES;else
        if(!wcscmp(pr,L"-disableinstall"))flags|=FLAG_DISABLEINSTALL;else
        if(!wcscmp(pr,L"-failsafe"))     flags|=FLAG_FAILSAFE;else
        if(!wcscmp(pr,L"-delextrainfs")) flags|=FLAG_DELEXTRAINFS;else
        if( wcsstr(pr,L"-verbose:"))     log_verbose=_wtoi_my(pr+9);else
        if( wcsstr(pr,L"-snplist:"))     snplist=_wfopen(pr+9,L"rt");else
        if( wcsstr(pr,L"-ls:"))          {wcscpy(state_file,pr+4);statemode=STATEMODE_LOAD;}else
        if(!wcscmp(pr,L"-a:32"))         virtual_arch_type=32;else
        if(!wcscmp(pr,L"-a:64"))         virtual_arch_type=64;else
        if( wcsstr(pr,L"-v:"))           virtual_os_version=_wtoi_my(pr+3);else
        if(StrCmpIW(pr,SAVE_INSTALLED_ID_DEF)==0)
            Parse_save_installed_id_swith(pr);else
        if(wcsstr(pr,L"-?"))CLIParam.ShowHelp=TRUE;else
        if(StrCmpIW(pr,HWIDINSTALLED_DEF)==0)
            Parse_HWID_installed_swith(pr); else
        if(StrCmpIW(pr,GFG_DEF)==0)  continue;
        else
            log_err("Unknown argument '%S'\n",pr);
        if(statemode==STATEMODE_EXIT)break;
    }
    LocalFree(argv);
    if(statemode==STATEMODE_EXIT)return;
    // Expert mode
    panel3[5].checked=expertmode;
    panel3_w[3].checked=expertmode;

    // Left panel
    panel_setfilters(panels);
}

void settings_save()
{
    FILE *f;

    if(flags&FLAG_PRESERVECFG)return;
    if(!canWrite(L"sdi.cfg"))
    {
        log_err("ERROR in settings_save(): Write-protected,'sdi.cfg'\n");
        return;
    }
    f=_wfopen(L"sdi.cfg",L"wt");
    if(!f)return;
    fwprintf(f,L"\"-drp_dir:%s\"\n\"-index_dir:%s\"\n\"-output_dir:%s\"\n"
              "\"-data_dir:%s\"\n\"-log_dir:%s\"\n\n"
              "\"-finish_cmd:%s\"\n\"-finishrb_cmd:%s\"\n"
              "\"-lang:%s\"\n\"-theme:%s\"\n\n"
              "-hintdelay:%d\n-port:%d\n-downlimit:%d\n-uplimit:%d\n-filters:%d\n\n",
            drp_dir,index_dir,output_dir,
            data_dir,logO_dir,
            finish,finish_rb,
            curlang,curtheme,
            hintdelay,
            torrentport,downlimit,uplimit,
            filters);

    if(license)fwprintf(f,L"-license ");
    if(expertmode)fwprintf(f,L"-expertmode ");
    if(flags&FLAG_NORESTOREPOINT)fwprintf(f,L"-norestorepnt ");
    if(flags&FLAG_SHOWDRPNAMES1)fwprintf(f,L"-showdrpnames1 ");
    if(flags&FLAG_SHOWDRPNAMES2)fwprintf(f,L"-showdrpnames2 ");
    if(flags&FLAG_OLDSTYLE)fwprintf(f,L"-oldstyle ");
    if(flags&FLAG_SHOWCONSOLE)fwprintf(f,L"-showconsole ");
    if(flags&FLAG_CHECKUPDATES)fwprintf(f,L"-checkupdates ");
    if(flags&FLAG_ONLYUPDATES)fwprintf(f,L"-onlyupdates ");
    fclose(f);
}

int  settings_load(const WCHAR *filename)
{
    WCHAR buf[BUFLEN];

    if(!LoadCFGFile(filename,buf))return 0;
    settings_parse(buf,0);
    return 1;
}

void SignalHandler(int signum)
{
    log_err("!!! Crashed %d!!!\n",signum);
    log_save();
    log_stop();
}

void CALLBACK drp_callback(const WCHAR *szFile,DWORD action,LPARAM lParam)
{
    UNREFERENCED_PARAMETER(szFile);
    UNREFERENCED_PARAMETER(action);
    UNREFERENCED_PARAMETER(lParam);

    //if(StrStrIW(szFile,L".7z")||StrStrIW(szFile,L".inf"))SetEvent(event);
}

void checkupdates()
{
#ifdef USE_TORRENT
    if(downloadmangar_event)return;

    torrentstatus.sessionpaused=1;
    if(flags&FLAG_CHECKUPDATES)
    {
        downloadmangar_event=CreateEvent(0,1,0,0);
        thandle_download=(HANDLE)_beginthreadex(0,0,&thread_download,0,0,0);
    }
#endif
}

int WINAPI WinMain(HINSTANCE hInst,HINSTANCE hinst,LPSTR pStr,int nCmd)
{
    UNREFERENCED_PARAMETER(hinst);
    UNREFERENCED_PARAMETER(pStr);

    bundle_t bundle[2];
    monitor_t mon_drp;
    HANDLE thr;
    HMODULE backtrace=0;
    DWORD dwProcessId;
    time_startup=time_total=GetTickCount();
    ghInst=hInst;

// Hide the console window as soon as possible
    GetWindowThreadProcessId(GetConsoleWindow(),&dwProcessId);
    if(GetCurrentProcessId()!=dwProcessId)hideconsole=SW_SHOWNOACTIVATE;
    ShowWindow(GetConsoleWindow(),hideconsole);

// Runtime error handlers
    start_exception_hadnlers();
    #ifdef _DEBUG
    backtrace=LoadLibraryA("backtrace.dll");
    #else
    signal(SIGSEGV,SignalHandler);
    #endif

// Load settings
    init_CLIParam();
    if (isCfgSwithExist(GetCommandLineW(),CLIParam.SaveInstalledFileName))
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
        if(*state_file&&statemode)log_con("Virtual system system config '%S'\n",state_file);
        if(virtual_arch_type)log_con("Virtual Windows version: %d-bit\n",virtual_arch_type);
        if(virtual_os_version)log_con("Virtual Windows version: %d.%d\n",virtual_os_version/10,virtual_os_version%10);
        log_con("\n");
    }

// Make dirs
    mkdir_r(drp_dir);
    mkdir_r(index_dir);
    mkdir_r(output_dir);

// Load text
    vault_init();
    vault_loadfromres(&vLang,IDR_LANG);

// Allocate resources
    bundle_init(&bundle[0]);
    bundle_init(&bundle[1]);
    manager_v[0].manager_init(&bundle[bundle_display].matcher);
    manager_v[1].manager_init(&bundle[bundle_display].matcher);
    deviceupdate_event=CreateEvent(0,0,0,0);

// Start device/driver scan
    bundle_prep(&bundle[bundle_display]);
    thr=(HANDLE)_beginthreadex(0,0,&thread_loadall,&bundle[0],0,0);

// Check updates
    checkupdates();

// Start folder monitors
    mon_drp=monitor_start(drp_dir,FILE_NOTIFY_CHANGE_LAST_WRITE|FILE_NOTIFY_CHANGE_FILE_NAME,1,drp_callback);
    virusmonitor_start();
    viruscheck(L"",0,0);

// MAIN GUI LOOP
    if(!(flags&FLAG_NOGUI)||flags&FLAG_AUTOINSTALL)gui(nCmd);

// Wait till the device scan thread is finished
    deviceupdate_exitflag=1;
    SetEvent(deviceupdate_event);
    WaitForSingleObject(thr,INFINITE);
    CloseHandle_log(thr,L"WinMain",L"thr");
    CloseHandle_log(deviceupdate_event,L"WinMain",L"event");

// Stop libtorrent
    #ifdef USE_TORRENT
    if(flags&FLAG_CHECKUPDATES)
    {
        downloadmangar_exitflag=1;
        SetEvent(downloadmangar_event);
        WaitForSingleObject(thandle_download,INFINITE);
        CloseHandle_log(thandle_download,L"thandle_download",L"thr");
        CloseHandle_log(downloadmangar_event,L"downloadmangar_event",L"event");
    }
    update_stop();
    #endif

// Free allocated resources
    bundle_free(&bundle[0]);
    bundle_free(&bundle[1]);
    vault_free();
    manager_v[0].manager_free();
    manager_v[1].manager_free();

// Save settings
    #ifndef CONSOLE_MODE
    settings_save();
    #endif

// Stop folder monitors
    if(mon_drp)monitor_stop(mon_drp);
    virusmonitor_stop();

// Bring the console window back
    ShowWindow(GetConsoleWindow(),SW_SHOWNOACTIVATE);
    #ifdef CONSOLE_MODE
    //MessageBox(0,L"В папке logs отчет создан!",L"Сообщение",0);
    #endif

// Stop runtime error handlers
    #ifdef NDEBUG
    signal(SIGSEGV,SIG_DFL);
    #endif
    if(backtrace)FreeLibrary(backtrace);

// Stop logging
    time_total=GetTickCount()-time_total;
    log_times();
    log_stop();

// Exit
    return ret_global;
}
//}

//{ Threads
unsigned int __stdcall thread_scandevices(void *arg)
{
    bundle_t *bundle=(bundle_t *)arg;
    State *state=&bundle->state;
    //log_con("{thread_scandevices\n");
    if(statemode==0)
        state->scanDevices();else
    if(statemode==STATEMODE_LOAD)
        state->load(state_file);
    //log_con("}thread_scandevices\n");
    return 0;
}

unsigned int __stdcall thread_loadindexes(void *arg)
{
    bundle_t *bundle=(bundle_t *)arg;
    Collection *collection=&bundle->collection;

    //log_con("{thread_loadindexes\n");
    if(manager_g->items_list[SLOT_EMPTY].curpos==1)*drpext_dir=0;
    collection->driverpack_dir=*drpext_dir?drpext_dir:drp_dir;
    //printf("'%S'\n",collection->driverpack_dir);
    collection->load();
    //log_con("}thread_loadindexes\n");
    return 0;
}

unsigned int __stdcall thread_getsysinfo(void *arg)
{
    bundle_t *bundle=(bundle_t *)arg;
    State *state=&bundle->state;

    //log_con("{thread_getsysinfo\n");
    if(statemode!=STATEMODE_LOAD)state->getsysinfo_slow();
    //log_con("}thread_getsysinfo\n");
    return 0;
}

unsigned int __stdcall thread_loadall(void *arg)
{
    bundle_t *bundle=(bundle_t *)arg;

    InitializeCriticalSection(&sync);
    do
    {
        int cancel_update=0;
        //long long t=GetTickCount();

        log_con("*** START *** %d,%d\n",bundle_display,bundle_shadow);
        bundle_prep(&bundle[bundle_shadow]);
        bundle_load(&bundle[bundle_shadow]);

        if(!(flags&FLAG_NOGUI))
        if(WaitForSingleObject(deviceupdate_event,0)==WAIT_OBJECT_0)cancel_update=1;

        if(!cancel_update)
        {
            if((flags&FLAG_NOGUI||hMain==0)&&(flags&FLAG_AUTOINSTALL)==0)
            {
                manager_g->matcher=&bundle[bundle_shadow].matcher;
                manager_g->manager_populate();
                manager_g->manager_filter(filters);
            }
            else
                SendMessage(hMain,WM_BUNDLEREADY,(WPARAM)&bundle[bundle_shadow],(LPARAM)&bundle[bundle_display]);
        }

        log_con("{2Sync\n");
        EnterCriticalSection(&sync);
        log_con("*");
        bundle_lowprioirity(&bundle[bundle_shadow]);

        if(cancel_update)
            log_con("*** CANCEL ***\n\n");
        else
        {
            log_con("*** FINISH ***\n\n");
            bundle_display^=1;
            bundle_shadow^=1;
        }
        //printf("%d\n",)
        bundle_free(&bundle[bundle_shadow]);
        bundle_init(&bundle[bundle_shadow]);
        if(cancel_update)SetEvent(deviceupdate_event);
        log_con("}2Sync\n");
        LeaveCriticalSection(&sync);
        WaitForSingleObject(deviceupdate_event,INFINITE);
        if(snplist)
        {
            fgetws(state_file,BUFLEN,snplist);
            log_con("SNP: '%S'\n",state_file);
            deviceupdate_exitflag=feof(snplist);
        }
        //printf("%ld\n",GetTickCount()-t);
    }while(!deviceupdate_exitflag);
    DeleteCriticalSection(&sync);
    return 0;
}
//}

//{ Bundle
void bundle_init(bundle_t *bundle)
{
    bundle->state.init();
    bundle->collection.init(drp_dir,index_dir,output_dir,flags);
    bundle->matcher.init(&bundle->state,&bundle->collection);
}

void bundle_prep(bundle_t *bundle)
{
    bundle->state.getsysinfo_fast();
}

void bundle_free(bundle_t *bundle)
{
    bundle->collection.release();
    bundle->matcher.release();
    bundle->state.release();
}

void bundle_load(bundle_t *bundle)
{
    HANDLE thandle[3];

    thandle[0]=(HANDLE)_beginthreadex(0,0,&thread_scandevices,bundle,0,0);
    thandle[1]=(HANDLE)_beginthreadex(0,0,&thread_loadindexes,bundle,0,0);
    thandle[2]=(HANDLE)_beginthreadex(0,0,&thread_getsysinfo,bundle,0,0);
    WaitForMultipleObjects(3,thandle,1,INFINITE);
    CloseHandle_log(thandle[0],L"bundle_load",L"0");
    CloseHandle_log(thandle[1],L"bundle_load",L"1");
    CloseHandle_log(thandle[2],L"bundle_load",L"2");

    bundle->state.isnotebook_a();
    bundle->matcher.populate();
    bundle->matcher.sort();
}

void bundle_lowprioirity(bundle_t *bundle)
{
    WCHAR filename[BUFLEN];
    time_startup=GetTickCount()-time_startup;

    redrawmainwnd();

    bundle->collection.printstates();
    //collection_finddrp(&bundle->collection,L"");
    bundle->state.print();
    bundle->matcher.print();
    manager_g->manager_print_hr();

#ifdef USE_TORRENT
    if(flags&FLAG_CHECKUPDATES&&!time_chkupdate&&canWrite(L"update"))
    {
        log_con("Event 1\n");
        SetEvent(downloadmangar_event);
    }
#endif
    bundle->collection.save();
    gen_timestamp();
    wsprintf(filename,L"%s\\%sstate.snp",log_dir,timestamp);
    bundle->state.save(filename);

    if(flags&COLLECTION_PRINT_INDEX)
    {
        log_con("Saving humanreadable indexes...");
        bundle->collection.print();
        flags&=~COLLECTION_PRINT_INDEX;
        log_con("DONE\n");
    }
}
//}

//{ Windows
HWND CreateWindowM(const WCHAR *type,const WCHAR *name,HWND hwnd,HMENU id)
{
    return CreateWindow(type,name,WS_CHILD|WS_VISIBLE,
                        0,0,0,0,hwnd,id,ghInst,NULL);
}

HWND CreateWindowMF(const WCHAR *type,const WCHAR *name,HWND hwnd,HMENU id,DWORD f)
{
    return CreateWindow(type,name,WS_CHILD|WS_VISIBLE|f,
                        0,0,0,0,hwnd,id,ghInst,NULL);
}

void setfont()
{
    if(hFont&&!DeleteObject(hFont))
        log_err("ERROR in manager_setfont(): failed DeleteObject\n");

    hFont=CreateFont(-D(FONT_SIZE),0,0,0,FW_DONTCARE,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,
                CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,VARIABLE_PITCH,(WCHAR *)D(FONT_NAME));
    if(!hFont)
        log_err("ERROR in manager_setfont(): failed CreateFont\n");
}

void redrawfield()
{
    if(!hField)
    {
        log_err("ERROR in redrawfield(): hField is 0\n");
        return;
    }
    InvalidateRect(hField,NULL,TRUE);
    //UpdateWindow(hField);
}

void redrawmainwnd()
{
    if(!hMain)
    {
        log_err("ERROR in redrawfield(): hField is 0\n");
        return;
    }
    //log_con("Redraw main %d\n",cntd++);
    InvalidateRect(hMain,NULL,TRUE);
    //UpdateWindow(hMain);
}

void lang_refresh()
{
    if(!hMain||!hField)
    {
        log_err("ERROR in lang_refresh(): hMain is %d, hField is %d\n",hMain,hField);
        return;
    }
    redrawmainwnd();
    redrawfield();
    InvalidateRect(hPopup,0,1);
#ifdef USE_TORRENT
    upddlg_updatelang();
#endif

    POINT p;
    GetCursorPos(&p);
    SetCursorPos(p.x+1,p.y);
    SetCursorPos(p.x,p.y);
}

void theme_refresh()
{
    RECT rect;

    setfont();
    SendMessage(hTheme,WM_SETFONT,(WPARAM)hFont,MAKELPARAM(FALSE,0));
    SendMessage(hLang,WM_SETFONT,(WPARAM)hFont,MAKELPARAM(FALSE,0));

    if(!hMain||!hField)
    {
        log_err("ERROR in theme_refresh(): hMain is 0\n");
        return;
    }
    panels[2].items=D(PANEL_LIST_OFSX)?panel3_w:panel3;
    GetWindowRect(hMain,&rect);
    MoveWindow(hMain,rect.left,rect.top,D(MAINWND_WX),D(MAINWND_WY)+1,1);
    MoveWindow(hMain,rect.left,rect.top,D(MAINWND_WX),D(MAINWND_WY),1);
}

void setscrollrange(int y)
{
    SCROLLINFO si;
    RECT rect;

    if(!hField)
    {
        log_err("ERROR in setscrollrange(): hField is 0\n");
        return;
    }
    GetClientRect(hField,&rect);

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
    SCROLLINFO si;

    if(!hField)
    {
        log_err("ERROR in getscrollpos(): hField is 0\n");
        return 0;
    }
    si.cbSize=sizeof(si);
    si.fMask=SIF_POS;
    si.nPos=0;
    GetScrollInfo(hField,SB_VERT,&si);
    return si.nPos;
}

void setscrollpos(int pos)
{
    SCROLLINFO si;

    if(!hField)
    {
        log_err("ERROR in setscrollpos(): hField is 0\n");
        return;
    }
    si.cbSize=sizeof(si);
    si.fMask=SIF_POS;
    si.nPos=pos;
    SetScrollInfo(hField,SB_VERT,&si,TRUE);
}
//}

//{ Helpers
void get_resource(int id,void **data,int *size)
{
    HRSRC myResource=FindResource(NULL,MAKEINTRESOURCE(id),(WCHAR *)RESFILE);
    if(!myResource)
    {
        log_err("ERROR in get_resource(): failed FindResource(%d)\n",id);
        *size=0;
        *data=0;
        return;
    }
    *size=SizeofResource(NULL,myResource);
    *data=LoadResource(NULL,myResource);
}

const WCHAR *get_winverstr(manager_t *manager1)
{
    int i;
    int ver=manager1->matcher->state->platform.dwMinorVersion;
    ver+=10*manager1->matcher->state->platform.dwMajorVersion;

    if(ver==64)ver=100;
    if(ver==52)
    {
        if(manager1->matcher->state->architecture)
            ver=51;
        else
            return L"Windows Server 2003";
    }
    for(i=0;i<NUM_OS;i++)if(windows_ver[i]==ver)return windows_name[i];
    return windows_name[NUM_OS-1];
}

void mkdir_r(const WCHAR *path)
{
    WCHAR buf[BUFLEN];
    WCHAR *p;

    if(path[1]==L':'&&path[2]==0)return;
    if(!canWrite(path))
    {
        log_err("ERROR in mkdir_r(): Write-protected,'%S'\n",path);
        return;
    }
    wcscpy(buf,path);
    p=buf;
    while((p=wcschr(p,L'\\')))
    {
        *p=0;
        if(_wmkdir(buf)<0&&errno!=EEXIST)
            log_err("ERROR in mkdir_r(): failed _wmkdir(%S,%d)\n",buf,errno);
        *p=L'\\';
        p++;
    }
    if(_wmkdir(buf)<0&&errno!=EEXIST)
        log_err("ERROR in mkdir_r(): failed _wmkdir(%S,%d)\n",buf,errno);
}
//}

//{ GUI
void tabadvance(int v)
{
    while(1)
    {
        kbpanel+=v;
        if(!kbpanel)kbpanel=KB_PANEL_CHK;
        if(kbpanel>KB_PANEL_CHK)kbpanel=KB_FIELD;

        if(!expertmode&&kbpanel>=KB_ACTIONS&&kbpanel<=KB_PANEL3)continue;
        //if(kbpanel==KB_PANEL_CHK&&!YP(&panels[11]))continue;
        break;
    }
    //log_con("Tab %d,%d\n",kbpanel,YP(&panels[11]));
    if(kbpanel==KB_LANG)SetFocus(hLang);else
    if(kbpanel==KB_THEME)SetFocus(hTheme);else
        SetFocus(hMain);
    redrawfield();
    redrawmainwnd();
}

void gui(int nCmd)
{
    int done=0;
    WNDCLASSEX wcx;
    memset(&wcx,0,sizeof(WNDCLASSEX));
    wcx.cbSize=         sizeof(WNDCLASSEX);
    wcx.lpfnWndProc=    WndProc;
    wcx.hInstance=      ghInst;
    wcx.hIcon=          LoadIcon(ghInst,MAKEINTRESOURCE(IDI_ICON1));
    wcx.hCursor=        (HCURSOR)LoadCursor(0,IDC_ARROW);
    wcx.lpszClassName=  classMain;
    wcx.hbrBackground=  (HBRUSH)(COLOR_WINDOW+1);
    if(!RegisterClassEx(&wcx))
    {
        log_err("ERROR in gui(): failed to register '%S' class\n",wcx.lpszClassName);
        return;
    }

    wcx.lpfnWndProc=PopupProcedure;
    wcx.lpszClassName=classPopup;
    wcx.hIcon=0;
    if(!RegisterClassEx(&wcx))
    {
        log_err("ERROR in gui(): failed to register '%S' class\n",wcx.lpszClassName);
        UnregisterClass_log(classMain,ghInst,L"gui",L"classMain");
        return;
    }

    wcx.lpfnWndProc=WindowGraphProcedure;
    wcx.lpszClassName=classField;
    if(!RegisterClassEx(&wcx))
    {
        log_err("ERROR in gui(): failed to register '%S' class\n",wcx.lpszClassName);
        UnregisterClass_log(classMain,ghInst,L"gui",L"classMain");
        UnregisterClass_log(classPopup,ghInst,L"gui",L"classPopup");
        return;
    }

    if(!license)
        DialogBox(ghInst,MAKEINTRESOURCE(IDD_DIALOG1),0,(DLGPROC)LicenseProcedure);

    if(license==2)
    {
        int f;
        f=lang_enum(hLang,L"langs",manager_g->matcher->state->locale);
        log_con("lang %d\n",f);
        lang_set(f);

        //if(MessageBox(0,STR(STR_UPD_DIALOG_MSG),STR(STR_UPD_DIALOG_TITLE),MB_YESNO|MB_ICONQUESTION)==IDYES)
        {
            flags|=FLAG_CHECKUPDATES;
            checkupdates();
#ifdef USE_TORRENT
            if(canWrite(L"update"))SetEvent(downloadmangar_event);
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
                            0,0,ghInst,0);
        if(!hMain)
        {
            log_err("ERROR in gui(): failed to create '%S' window\n",classMain);
            return;
        }

        ShowWindow(hMain,flags&FLAG_NOGUI?SW_HIDE:nCmd);

        while(!done)
        {
            while(WAIT_IO_COMPLETION==MsgWaitForMultipleObjectsEx(0,0,INFINITE,QS_ALLINPUT,MWMO_ALERTABLE));

            while(PeekMessage(&msg,0,0,0,PM_REMOVE))
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
                    if(msg.wParam==VK_SPACE)
                    {
                        SendMessage(hMain,WM_LBUTTONDOWN,0,0);
                        SendMessage(hMain,WM_LBUTTONUP,0,0);
                    }
                    if(msg.wParam==VK_TAB&&shift_down)
                    {
                        tabadvance(-1);
                    }
                    if(msg.wParam==VK_TAB&&!shift_down)
                    {
                        tabadvance(1);
                    }
                    if(msg.wParam==VK_UP)
                    {
                        if(kbitem[kbpanel]>0)kbitem[kbpanel]--;
                        redrawmainwnd();
                        redrawfield();
                    }
                    if(msg.wParam==VK_DOWN)
                    {
                        kbitem[kbpanel]++;
                        redrawmainwnd();
                        redrawfield();
                    }
                    /*if(msg.wParam==VK_LEFT)
                    {
                        if(kbitem[kbpanel]>0)kbitem[kbpanel]--;
                        redrawmainwnd();
                        redrawfield();
                    }
                    if(msg.wParam==VK_RIGHT)
                    {
                        kbitem[kbpanel]++;
                        redrawmainwnd();
                        redrawfield();
                    }*/
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
            InvalidateRect(hwnd,0,0);//common
            break;

        case WM_MOUSEHOVER:
            InvalidateRect(hPopup,0,1);
            ShowWindow(hPopup,floating_type==FLOATING_NONE?SW_HIDE:SW_SHOWNOACTIVATE);
            break;

        case WM_ACTIVATE:
            InvalidateRect(hwnd,0,0);//common
            break;

        case WM_MOUSEMOVE:
            if(mousedown==1||mousedown==2)
            {
                GetWindowRect(hMain,&rect);
                if(mousedown==2||abs(mousex-x)>2||abs(mousey-y)>2)
                {
                    mousedown=2;
                    MoveWindow(hMain,rect.left+x-mousex,rect.top+y-mousey,mainx_w,mainy_w,1);
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
                mousedown=1;
                SetCapture(hwnd);
            }
            else
                mousedown=3;

            break;

        case WM_CANCELMODE:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
            mousex=-1;
            mousey=-1;
            SetCursor(LoadCursor(0,IDC_ARROW));
            ReleaseCapture();
            if(mousedown==2)
            {
                mousedown=0;
                mouseclick=0;
                break;
            }
            mouseclick=mousedown&&uMsg==WM_LBUTTONUP?1:0;
            mousedown=0;
            return 1;

        default:
            return 1;
    }
    return 0;
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
        statemode=STATEMODE_LOAD;
        PostMessage(hMain,WM_DEVICECHANGE,7,2);
    }
}

void extractto()
{
    BROWSEINFO lpbi;
    WCHAR dir[BUFLEN];
    WCHAR buf[BUFLEN];
    LPITEMIDLIST list;
    WCHAR **argv;
    int argc;

    memset(&lpbi,0,sizeof(BROWSEINFO));
    lpbi.hwndOwner=hMain;
    lpbi.pszDisplayName=dir;
    lpbi.lpszTitle=STR(STR_EXTRACTFOLDER);
    lpbi.ulFlags=BIF_NEWDIALOGSTYLE|BIF_EDITBOX;


    list=SHBrowseForFolder(&lpbi);
    if(list)
    {
        SHGetPathFromIDList(list,dir);

        argv=CommandLineToArgvW(GetCommandLineW(),&argc);
        //printf("'%S',%d\n",argv[0],argc);
        wsprintf(buf,L"%s\\drv.exe",dir);
        if(!CopyFile(argv[0],buf,0))
            log_err("ERROR in extractto(): failed CopyFile(%S,%S)\n",argv[0],buf);
        LocalFree(argv);

        wcscat(dir,L"\\drivers");
        wcscpy(extractdir,dir);
        manager_install(OPENFOLDER);
    }
}

void set_rstpnt(int checked)
{
    manager_g->items_list[SLOT_RESTORE_POINT].checked=panels[11].items[2].checked=checked;
    //if(D(PANEL12_WY))manager_g->items_list[SLOT_RESTORE_POINT].isactive=checked;
    manager_g->manager_setpos();
    redrawfield();
}

void drvdir()
{
    BROWSEINFO lpbi;
    LPITEMIDLIST list;

    memset(&lpbi,0,sizeof(BROWSEINFO));
    lpbi.hwndOwner=hMain;
    lpbi.pszDisplayName=drpext_dir;
    lpbi.lpszTitle=STR(STR_EXTRACTFOLDER);
    lpbi.ulFlags=BIF_NEWDIALOGSTYLE|BIF_EDITBOX;

    list=SHBrowseForFolder(&lpbi);
    if(list)
    {
        SHGetPathFromIDList(list,drpext_dir);
        int len=wcslen(drpext_dir);
        drpext_dir[len]=0;
//        printf("'%S',%d\n",drpext_dir,len);
        PostMessage(hMain,WM_DEVICECHANGE,7,2);
    }
}

const WCHAR *getHWIDby(int id,int num)
{
    devicematch_t *devicematch_f=manager_g->items_list[id].devicematch;
    WCHAR *p;
    char *t=manager_g->matcher->state->text;
    int i=0;

    if(devicematch_f->device->HardwareID)
    {
        p=(WCHAR *)(t+devicematch_f->device->HardwareID);
        while(*p)
        {
            if(i==num)return p;
            p+=lstrlen(p)+1;
            i++;
        }
    }
    if(devicematch_f->device->CompatibleIDs)
    {
        p=(WCHAR *)(t+devicematch_f->device->CompatibleIDs);
        while(*p)
        {
            if(i==num)return p;
            p+=lstrlen(p)+1;
            i++;
        }
    }
    return L"";
}

void escapeAmpUrl(WCHAR *buf,WCHAR *source)
{
    WCHAR *p1=buf,*p2=source;

    while(*p2)
    {
        *p1=*p2;
        if(*p1==L'&')
        {
            *p1++=L'%';
            *p1++=L'2';
            *p1=L'6';
        }
        if(*p1==L'\\')
        {
            *p1++=L'%';
            *p1++=L'5';
            *p1=L'C';
        }
        p1++;p2++;
    }
    *p1=0;
}

void checktimer(const WCHAR *str,long long t,int uMsg)
{
    if(GetTickCount()-t>20&&log_verbose&LOG_VERBOSE_LAGCOUNTER)
        log_con("GUI lag in %S[%X]: %ld\n",str,uMsg,GetTickCount()-t);
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
            canvasMain.init();
            hMain=hwnd;

            // Field
            hField=CreateWindowMF(classField,NULL,hwnd,0,WS_VSCROLL);

            // Popup
            hPopup=CreateWindowEx(WS_EX_LAYERED|WS_EX_NOACTIVATE|WS_EX_TOPMOST|WS_EX_TRANSPARENT,
                classPopup,L"",WS_POPUP,
                0,0,0,0,hwnd,(HMENU)0,ghInst,0);

            // Lang
            hLang=CreateWindowMF(WC_COMBOBOX,L"",hwnd,(HMENU)ID_LANG,CBS_DROPDOWNLIST|CBS_HASSTRINGS|WS_OVERLAPPED|WS_VSCROLL);
            SendMessage(hwnd,WM_UPDATELANG,0,0);

            // Theme
            hTheme=CreateWindowMF(WC_COMBOBOX,L"",hwnd,(HMENU)ID_THEME,CBS_DROPDOWNLIST|CBS_HASSTRINGS|WS_OVERLAPPED|WS_VSCROLL);
            SendMessage(hwnd,WM_UPDATETHEME,0,0);

            // Misc
            vault_startmonitors();
            DragAcceptFiles(hwnd,1);

            manager_g->manager_populate();
            manager_g->manager_filter(filters);
            manager_g->manager_setpos();

            GetWindowRect(GetDesktopWindow(),&rect);
            rect.left=(rect.right-D(MAINWND_WX))/2;
            rect.top=(rect.bottom-D(MAINWND_WY))/2;
            MoveWindow(hwnd,rect.left,rect.top,D(MAINWND_WX),D(MAINWND_WY),1);
            break;

        case WM_CLOSE:
            if(installmode==MODE_NONE||(flags&FLAG_AUTOCLOSE))
                DestroyWindow(hwnd);
            else if(MessageBox(0,STR(STR_INST_QUIT_MSG),STR(STR_INST_QUIT_TITLE),MB_YESNO|MB_ICONQUESTION)==IDYES)
                installmode=MODE_STOPPING;
            break;

        case WM_DESTROY:
            if(!DeleteObject(hFont))
                log_err("ERROR in manager_free(): failed DeleteObject\n");
            vault_stopmonitors();
            canvasMain.free();
            PostQuitMessage(0);
            break;

        case WM_UPDATELANG:
            SendMessage(hLang,CB_RESETCONTENT,0,0);
            lang_enum(hLang,L"langs",manager_g->matcher->state->locale);
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
            if(f==CB_ERR)
            {
                theme_set(f);
                panels[2].items=D(PANEL_LIST_OFSX)?panel3_w:panel3;
                j=SendMessage(hTheme,CB_GETCOUNT,0,0);
                for(i=0;i<j;i++)
                    if(StrStrI(vTheme.namelist[i],(WCHAR *)D(THEME_NAME))&&
                       StrStrI(vTheme.namelist[i],L"big")==0){f=i;break;}
            }else
                theme_set(f);
            panels[2].items=D(PANEL_LIST_OFSX)?panel3_w:panel3;
            SendMessage(hTheme,CB_SETCURSEL,f,0);
            theme_refresh();
            break;

        case WM_BUNDLEREADY:
            {
                bundle_t *bb=(bundle_t *)wParam;
                manager_t *manager_prev=manager_g;

                log_con("{Sync");
                EnterCriticalSection(&sync);
                log_con("...\n");
                manager_active++;
                manager_active&=1;
                manager_g=&manager_v[manager_active];

                manager_g->matcher=&bb->matcher;
                memcpy(&manager_g->items_list.front(),&manager_prev->items_list.front(),sizeof(itembar_t)*RES_SLOTS);
                manager_g->manager_populate();
                manager_g->manager_filter(filters);
                manager_g->items_list[SLOT_SNAPSHOT].isactive=statemode==STATEMODE_LOAD?1:0;
                manager_g->items_list[SLOT_DPRDIR].isactive=*drpext_dir?1:0;
                manager_g->manager_restorepos(manager_prev);
                viruscheck(L"",0,0);
                manager_g->manager_setpos();
                log_con("}Sync\n");
                LeaveCriticalSection(&sync);

#ifdef USE_TORRENT
                upddlg_populatelist(0,0);
#endif
                //log_con("Mode in WM_BUNDLEREADY: %d\n",installmode);
                if(flags&FLAG_AUTOINSTALL)
                {
                    int cnt=0;
                    if(installmode==MODE_SCANNING)
                    {
                        if(!panels[11].items[3].checked)manager_g->manager_selectall();
                        itembar_t *itembar=&manager_g->items_list[RES_SLOTS];
                        for(i=RES_SLOTS;(unsigned)i<manager_g->items_list.size();i++,itembar++)
                            if(itembar->checked)
                        {
                            cnt++;
                        }

                        if(!cnt)flags&=~FLAG_AUTOINSTALL;
                        log_con("Autoinstall rescan: %d found\n",cnt);
                    }

                    if(installmode==MODE_NONE||(installmode==MODE_SCANNING&&cnt))
                    {
                        if(!panels[11].items[3].checked)manager_g->manager_selectall();
                        if((flags&FLAG_EXTRACTONLY)==0)
                        wsprintf(extractdir,L"%s\\SDI",manager_g->matcher->state->text+manager_g->matcher->state->temp);
                        manager_install(INSTALLDRIVERS);
                    }
                    else
                    {
                        WCHAR buf[BUFLEN];

                        installmode=MODE_NONE;
                        if(panels[11].items[3].checked)
                            wcscpy(buf,L" /c Shutdown.exe -r -t 3");
                        else
                            wsprintf(buf,L" /c %s",needreboot?finish_rb:finish);

                        if(*(needreboot?finish_rb:finish)||panels[11].items[3].checked)
                            RunSilent(L"cmd",buf,SW_HIDE,0);

                        if(flags&FLAG_AUTOCLOSE)PostMessage(hMain,WM_CLOSE,0,0);
                    }
                }
                else
                    if(installmode==MODE_SCANNING)installmode=MODE_NONE;
            }
            break;

        case WM_DROPFILES:
        {
            WCHAR lpszFile[MAX_PATH]={0};
            UINT uFile=0;
            HDROP hDrop=(HDROP)wParam;

            uFile=DragQueryFile(hDrop,0xFFFFFFFF,NULL,0);
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
                    PostMessage(hMain,WM_DEVICECHANGE,7,2);
                }
                else if(StrStrI(lpszFile,L".snp"))
                {
                    wcscpy(state_file,lpszFile);
                    statemode=STATEMODE_LOAD;
                    PostMessage(hMain,WM_DEVICECHANGE,7,2);
                }
                //else
                //    MessageBox(NULL,lpszFile,NULL,MB_ICONINFORMATION);
            }
            DragFinish(hDrop);
            break;
        }

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
                panels[11].items[2].checked^=1;
                PostMessage(hMain,WM_COMMAND,ID_RESTPNT,0);
                redrawmainwnd();
            }
            if(ctrl_down&&wParam==L'R')
            {
                panels[11].items[3].checked^=1;
                PostMessage(hMain,WM_COMMAND,ID_REBOOT,0);
                redrawmainwnd();
            }
            if(wParam==VK_F5)
                PostMessage(hwnd,WM_DEVICECHANGE,7,2);
            if(wParam==VK_F6&&ctrl_down)
            {
                manager_g->manager_testitembars();
                manager_g->manager_setpos();
                redrawfield();
            }
            if(wParam==VK_F7)
            {
                wndclicker(2);
                MessageBox(0,L"Windows data recorded into the log.",L"Message",0);
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
                //flags^=FLAG_SHOWDRPNAMES1;
                manager_g->manager_filter(filters);
                manager_g->manager_setpos();
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

            SetEvent(deviceupdate_event);
            break;

        case WM_SIZE:
            SetLayeredWindowAttributes(hMain,0,D(MAINWND_TRANSPARENCY),LWA_ALPHA);
            SetLayeredWindowAttributes(hPopup,0,D(POPUP_TRANSPARENCY),LWA_ALPHA);

            GetWindowRect(hwnd,&rect);
            main1x_c=x;
            main1y_c=y;
            mainx_w=rect.right-rect.left;
            mainy_w=rect.bottom-rect.top;

            i=D(PNLITEM_OFSX)+D(PANEL_LIST_OFSX);
            j=D(PANEL_LIST_OFSX)?0:1;
            f=D(PANEL_LIST_OFSX)?4:0;
            MoveWindow(hField,Xm(D(DRVLIST_OFSX)),Ym(D(DRVLIST_OFSY)),XM(D(DRVLIST_WX),D(DRVLIST_OFSX)),YM(D(DRVLIST_WY),D(DRVLIST_OFSY)),TRUE);

            panels[2].moveWindow(hLang,i,j,f);
            j=D(PANEL_LIST_OFSX)?1:3;
            panels[2].moveWindow(hTheme,i,j,f);
/*            MoveWindow(hLang, Xp(&panels[2])+i,Yp(&panels[2])+j*D(PNLITEM_WY)-2+f,XP(&panels[2])-i-D(PNLITEM_OFSX),190*2,0);
            j=D(PANEL_LIST_OFSX)?1:3;
            MoveWindow(hTheme,Xp(&panels[2])+i,Yp(&panels[2])+j*D(PNLITEM_WY)-2+f,XP(&panels[2])-i-D(PNLITEM_OFSX),190*2,0);*/
            manager_g->manager_setpos();

            redrawmainwnd();
            break;

        case WM_TIMER:
            if(manager_g->manager_animate())
                redrawfield();
            else
                KillTimer(hwnd,1);
            break;

        case WM_PAINT:
            GetClientRect(hwnd,&rect);
            canvasMain.begin(hwnd,rect.right,rect.bottom);

            box_draw(canvasMain.getDC(),0,0,rect.right+1,rect.bottom+1,BOX_MAINWND);
            SelectObject(canvasMain.getDC(),hFont);
            panels[7].panel_draw(canvasMain.getDC());// draw revision
            for(i=0;i<NUM_PANELS;i++)if(i!=7)
            {
                panels[i].panel_draw(canvasMain.getDC());
            }
            canvasMain.end();
            break;

        case WM_ERASEBKGND:
            return 1;

        case WM_MOUSEMOVE:
            GetClientRect(hwnd,&rect);
            i=panels_hitscan(x,y,&j);
            if(j==0||j==7||j==12)SetCursor(LoadCursor(0,IDC_HAND));
            //log_con("%d,%d\n",j,i);

            if(i>0)
            {
                if((i==1&&j==7)||(j==12))
                    drawpopup(-1,FLOATING_ABOUT,x,y,hwnd);
                else
                    drawpopup(panels[j].items[i].str_id+1,i>0&&i<4&&j==0?FLOATING_SYSINFO:FLOATING_TOOLTIP,x,y,hwnd);
            }
            else
                drawpopup(-1,FLOATING_NONE,x,y,hwnd);

            if(panel_lasti!=i+j*256)
            {
                if(j>=0)panels[j].panel_draw_inv();
                panels[panel_lasti/256].panel_draw_inv();
            }
            if(j>=0)panel_lasti=i+j*256;else panel_lasti=0;
            break;

        case WM_LBUTTONUP:
            if(!mouseclick)break;
            i=panels_hitscan(x,y,&j);
            if(i<0)break;
            if(i<4&&j==0)
            {
                RunSilent(L"devmgmt.msc",0,SW_SHOW,0);
            }
            else
            //log_con("%d,%d\n",j,i);
            if(panels[j].items[i].type==TYPE_CHECKBOX||TYPE_BUTTON)
            {
                panels[j].items[i].checked^=1;
                if(panels[j].items[i].action_id==ID_EXPERT_MODE)
                {
                    expertmode=panels[j].items[i].checked;
                    ShowWindow(GetConsoleWindow(),expertmode&&ctrl_down?SW_SHOWNOACTIVATE:hideconsole);
                }
                else
                    PostMessage(hwnd,WM_COMMAND,panels[j].items[i].action_id+(BN_CLICKED<<16),0);

                InvalidateRect(hwnd,NULL,TRUE);
            }
            if(j==7||j==12)RunSilent(L"open",L"http://snappy-driver-installer.sourceforge.net",SW_SHOWNORMAL,0);
            break;

        case WM_RBUTTONUP:
            i=panels_hitscan(x,y,&j);
            if(i<0)break;
            if(i==2&&j==11)
            {
                floating_itembar=SLOT_RESTORE_POINT;
                contextmenu(x-Xm(D(DRVLIST_OFSX)),y-Ym(D(DRVLIST_OFSY)));
            }
            break;

        case WM_RBUTTONDOWN:
            i=panels_hitscan(x,y,&j);
            if(i>=0&&i<4&&j==0)contextmenu2(x,y);
            break;

        case WM_MOUSEWHEEL:
            i=GET_WHEEL_DELTA_WPARAM(wParam);
            if(space_down)
            {
                horiz_sh-=i/5;
                if(horiz_sh>0)horiz_sh=0;
                InvalidateRect(hPopup,0,0);
            }
            else
                SendMessage(hField,WM_VSCROLL,MAKELONG(i>0?SB_LINEUP:SB_LINEDOWN,0),0);
            break;

        case WM_COMMAND:
            wp=LOWORD(wParam);
//            printf("com:%d,%d,%d\n",wp,HIWORD(wParam),BN_CLICKED);

            switch(wp)
            {
                case ID_SCHEDULE:
                    manager_g->manager_toggle(floating_itembar);
                    redrawfield();
                    break;

                case ID_SHOWALT:
                    if(floating_itembar==SLOT_RESTORE_POINT)
                    {
                        RunSilent(L"cmd",L"/c %windir%\\Sysnative\\rstrui.exe",SW_HIDE,0);
                        RunSilent(L"cmd",L"/c %windir%\\system32\\Restore\\rstrui.exe",SW_HIDE,0);
                    }
                    else
                    {
                        manager_g->manager_expand(floating_itembar);
                    }
                    break;

                case ID_OPENINF:
                case ID_LOCATEINF:
                    {
                        WCHAR buf[BUFLEN];

                        devicematch_t *devicematch_f=manager_g->items_list[floating_itembar].devicematch;
                        Driver *cur_driver=&manager_g->matcher->state->Drivers_list[devicematch_f->device->driver_index];
                        wsprintf(buf,L"%s%s%s",
                                (wp==ID_LOCATEINF)?L"/select,":L"",
                               manager_g->matcher->state->text+manager_g->matcher->state->windir,
                               manager_g->matcher->state->text+cur_driver->InfPath);

                        if(wp==ID_OPENINF)
                            RunSilent(buf,L"",SW_SHOW,0);
                        else
                            RunSilent(L"explorer.exe",buf,SW_SHOW,0);
                    }
                    break;

                case ID_DEVICEMNG:
                    RunSilent(L"devmgmt.msc",0,SW_SHOW,0);
                    break;

                case ID_EMU_32:
                    virtual_arch_type=32;
                    PostMessage(hMain,WM_DEVICECHANGE,7,2);
                    break;

                case ID_EMU_64:
                    virtual_arch_type=64;
                    PostMessage(hMain,WM_DEVICECHANGE,7,2);
                    break;

                case ID_DIS_INSTALL:
                    flags^=FLAG_DISABLEINSTALL;
                    break;

                case ID_DIS_RESTPNT:
                    flags^=FLAG_NORESTOREPOINT;
                    manager_g->items_list[SLOT_RESTORE_POINT].isactive=(flags&FLAG_NORESTOREPOINT)==0;
                    manager_g->manager_setpos();
                    break;

                default:
                    break;
            }
            /*if(wp>=ID_URL0&&wp<=ID_URL4)
            {
                RunSilent(L"open",menu3url[wp-ID_URL0],SW_SHOWNORMAL,0);
            }*/
            if(wp>=ID_WIN_2000&&wp<=ID_WIN_10)
            {
                virtual_os_version=windows_ver[wp-ID_WIN_2000];
                PostMessage(hMain,WM_DEVICECHANGE,7,2);
            }
            if(wp>=ID_HWID_CLIP&&wp<=ID_HWID_WEB+100)
            {
                int id=wp%100;
                //printf("%S\n",getHWIDby(floating_itembar,id));

                if(wp>=ID_HWID_WEB)
                {
                    WCHAR buf[BUFLEN];
                    WCHAR buf2[BUFLEN];
                    wsprintf(buf,L"https://www.google.com/#q=%s",getHWIDby(floating_itembar,id));
                    wsprintf(buf,L"http://catalog.update.microsoft.com/v7/site/search.aspx?q=%s",getHWIDby(floating_itembar,id));
                    escapeAmpUrl(buf2,buf);
                    RunSilent(L"iexplore.exe",buf2,SW_SHOW,0);

                }
                else
                {
                    int len=wcslen(getHWIDby(floating_itembar,id))*2+2;
                    HGLOBAL hMem=GlobalAlloc(GMEM_MOVEABLE,len);
                    memcpy(GlobalLock(hMem),getHWIDby(floating_itembar,id),len);
                    GlobalUnlock(hMem);
                    OpenClipboard(0);
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
                        wsprintf(extractdir,L"%s\\SDI",manager_g->matcher->state->text+manager_g->matcher->state->temp);
                        manager_install(INSTALLDRIVERS);
                    }
                    break;

                case ID_SELECT_NONE:
                    manager_g->manager_selectnone();
                    redrawmainwnd();
                    redrawfield();
                    break;

                case ID_SELECT_ALL:
                    manager_g->manager_selectall();
                    redrawmainwnd();
                    redrawfield();
                    break;

                case ID_OPENLOGS:
                    ShellExecute(hwnd,L"explore",log_dir,0,0,SW_SHOW);
                    break;

                case ID_SNAPSHOT:
                    snapshot();
                    break;

                case ID_EXTRACT:
                    extractto();
                    break;

                case ID_DRVDIR:
                    drvdir();
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
                    for(i=0;i<NUM_PANELS;i++)
                    for(j=0;j<panels[i].items[0].action_id+1;j++)
                        if(panels[i].items[j].type==TYPE_CHECKBOX&&
                           panels[i].items[j].checked&&
                           panels[i].items[j].action_id!=ID_EXPERT_MODE)
                            filters+=1<<panels[i].items[j].action_id;

                    manager_g->manager_filter(filters);
                    manager_g->manager_setpos();
                    //manager_print(manager_g);
                    break;

                case ID_RESTPNT:
                    set_rstpnt(panels[11].items[2].checked);
                    break;

                case ID_REBOOT:
                    if(panels[11].items[3].checked)
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

void escapeAmp(WCHAR *buf,WCHAR *source)
{
    WCHAR *p1=buf,*p2=source;

    while(*p2)
    {
        *p1=*p2;
        if(*p1==L'&')*(++p1)=L'&';
        p1++;p2++;
    }
    *p1=0;
}

void contextmenu2(int x,int y)
{
    int i;
    RECT rect;
    OSVERSIONINFOEX *platform=&manager_g->matcher->state->platform;
    HMENU
        hPopupMenu=CreatePopupMenu(),
        hSub1=CreatePopupMenu();
    int ver=platform->dwMinorVersion+10*platform->dwMajorVersion;
    int arch=manager_g->matcher->state->architecture;

    for(i=0;i<NUM_OS-1;i++)
    {
        InsertMenu(hSub1,i,MF_BYPOSITION|MF_STRING|(ver==windows_ver[i]?MF_CHECKED:0),
                   ID_WIN_2000+i,windows_name[i]);
    }

    i=0;
    InsertMenu(hPopupMenu,i++,MF_BYPOSITION|MF_STRING|MF_POPUP,(UINT_PTR)hSub1,STR(STR_SYS_WINVER));
    InsertMenu(hPopupMenu,i++,MF_BYPOSITION|MF_STRING|(arch==0?MF_CHECKED:0),ID_EMU_32,STR(STR_SYS_32));
    InsertMenu(hPopupMenu,i++,MF_BYPOSITION|MF_STRING|(arch==1?MF_CHECKED:0),ID_EMU_64,STR(STR_SYS_64));
    InsertMenu(hPopupMenu,i++,MF_BYPOSITION|MF_SEPARATOR,0,0);
    InsertMenu(hPopupMenu,i++,MF_BYPOSITION|MF_STRING,ID_DEVICEMNG,STR(STR_SYS_DEVICEMNG));
    InsertMenu(hPopupMenu,i++,MF_BYPOSITION|MF_SEPARATOR,0,0);
    InsertMenu(hPopupMenu,i++,MF_BYPOSITION|MF_STRING|(flags&FLAG_DISABLEINSTALL?MF_CHECKED:0),ID_DIS_INSTALL,STR(STR_SYS_DISINSTALL));
    InsertMenu(hPopupMenu,i++,MF_BYPOSITION|MF_STRING|(flags&FLAG_NORESTOREPOINT?MF_CHECKED:0),ID_DIS_RESTPNT,STR(STR_SYS_DISRESTPNT));
    SetForegroundWindow(hMain);
    GetWindowRect(hMain,&rect);
    TrackPopupMenu(hPopupMenu,TPM_LEFTALIGN,rect.left+x,rect.top+y,0,hMain,NULL);
}

void contextmenu(int x,int y)
{
    int i;
    RECT rect;
    HMENU
        hPopupMenu=CreatePopupMenu(),
        hSub1=CreatePopupMenu(),
        hSub2=CreatePopupMenu();

    itembar_t *itembar=&manager_g->items_list[floating_itembar];
    int flags1=itembar->checked?MF_CHECKED:0;
    int flags2=itembar->isactive&2?MF_CHECKED:0;
    int flagssubmenu=0;

    if(!itembar->hwidmatch)flags1|=MF_GRAYED;
    if(manager_g->groupsize(itembar->index)<2)flags2|=MF_GRAYED;

    if(floating_itembar==SLOT_RESTORE_POINT)
    {
        i=0;
        InsertMenu(hPopupMenu,i++,MF_BYPOSITION|MF_STRING|flags1,ID_SCHEDULE, STR(STR_REST_SCHEDULE));
        InsertMenu(hPopupMenu,i++,MF_BYPOSITION|MF_STRING,       ID_SHOWALT,  STR(STR_REST_ROLLBACK));
        SetForegroundWindow(hMain);
        GetWindowRect(hField,&rect);
        TrackPopupMenu(hPopupMenu,TPM_LEFTALIGN,rect.left+x,rect.top+y,0,hMain,NULL);
        return;
    }
    if(floating_itembar<RES_SLOTS)return;
    //printf("itembar %d\n",floating_itembar);

    devicematch_t *devicematch_f=manager_g->items_list[floating_itembar].devicematch;
    Driver *cur_driver=0;
    WCHAR *p;
    char *t=manager_g->matcher->state->text;
    if(devicematch_f->device->driver_index>=0)cur_driver=&manager_g->matcher->state->Drivers_list[devicematch_f->device->driver_index];
    int flags3=cur_driver?0:MF_GRAYED;
    WCHAR buf[512];

    i=0;
    if(devicematch_f->device->HardwareID)
    {
        p=(WCHAR *)(t+devicematch_f->device->HardwareID);
        while(*p)
        {
            escapeAmp(buf,p);
            InsertMenu(hSub1,i,MF_BYPOSITION|MF_STRING,ID_HWID_WEB+i,buf);
            InsertMenu(hSub2,i,MF_BYPOSITION|MF_STRING,ID_HWID_CLIP+i,buf);
            p+=lstrlen(p)+1;
            i++;
        }
    }
    if(devicematch_f->device->CompatibleIDs)
    {
        p=(WCHAR *)(t+devicematch_f->device->CompatibleIDs);
        while(*p)
        {
            escapeAmp(buf,p);
            InsertMenu(hSub1,i,MF_BYPOSITION|MF_STRING,ID_HWID_WEB+i,buf);
            InsertMenu(hSub2,i,MF_BYPOSITION|MF_STRING,ID_HWID_CLIP+i,buf);
            p+=lstrlen(p)+1;
            i++;
        }
    }
    if(!i)flagssubmenu=MF_GRAYED;

    i=0;
    InsertMenu(hPopupMenu,i++,MF_BYPOSITION|MF_STRING|flags1,ID_SCHEDULE, STR(STR_CONT_INSTALL));
    InsertMenu(hPopupMenu,i++,MF_BYPOSITION|MF_STRING|flags2,ID_SHOWALT,  STR(STR_CONT_SHOWALT));
    InsertMenu(hPopupMenu,i++,MF_BYPOSITION|MF_SEPARATOR,0,0);
    InsertMenu(hPopupMenu,i++,MF_BYPOSITION|MF_STRING|MF_POPUP|flagssubmenu,(UINT_PTR)hSub1,STR(STR_CONT_HWID_SEARCH));
    InsertMenu(hPopupMenu,i++,MF_BYPOSITION|MF_STRING|MF_POPUP|flagssubmenu,(UINT_PTR)hSub2,STR(STR_CONT_HWID_CLIP));
    InsertMenu(hPopupMenu,i++,MF_BYPOSITION|MF_SEPARATOR,0,0);
    InsertMenu(hPopupMenu,i++,MF_BYPOSITION|MF_STRING|flags3,ID_OPENINF,  STR(STR_CONT_OPENINF));
    InsertMenu(hPopupMenu,i++,MF_BYPOSITION|MF_STRING|flags3,ID_LOCATEINF,STR(STR_CONT_LOCATEINF));
    SetForegroundWindow(hMain);
    GetWindowRect(hField,&rect);
    TrackPopupMenu(hPopupMenu,TPM_LEFTALIGN,rect.left+x,rect.top+y,0,hMain,NULL);
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
            canvasField.init();
            break;

        case WM_PAINT:
            y=getscrollpos();

            GetClientRect(hwnd,&rect);
            canvasField.begin(hwnd,rect.right,rect.bottom);

            BitBlt(canvasField.getDC(),0,0,rect.right,rect.bottom,canvasMain.getDC(),Xm(D(DRVLIST_OFSX)),Ym(D(DRVLIST_OFSY)),SRCCOPY);
            manager_g->manager_draw(canvasField.getDC(),y);

            canvasField.end();
            break;

        case WM_DESTROY:
            canvasField.free();
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
            manager_g->manager_hitscan(x,y,&floating_itembar,&i);
            if(floating_itembar==SLOT_SNAPSHOT)
            {
                statemode=0;
                PostMessage(hMain,WM_DEVICECHANGE,7,2);
            }
            if(floating_itembar==SLOT_DPRDIR)
            {
                *drpext_dir=0;
                PostMessage(hMain,WM_DEVICECHANGE,7,2);
            }
            if(floating_itembar==SLOT_EXTRACTING)
            {
                if(installmode==MODE_INSTALLING)
                    installmode=MODE_STOPPING;
                else if(installmode==MODE_NONE)
                    manager_g->manager_clear();
            }
            if(floating_itembar==SLOT_DOWNLOAD)
            {
#ifdef USE_TORRENT
                DialogBox(ghInst,MAKEINTRESOURCE(IDD_DIALOG2),hwnd,(DLGPROC)UpdateProcedure);
                break;
#endif
            }

            if(floating_itembar>=0&&(i==1||i==0||i==3))
            {
                manager_g->manager_toggle(floating_itembar);
                if(wParam&MK_SHIFT&&installmode==MODE_NONE)
                {
                    if((flags&FLAG_EXTRACTONLY)==0)
                    wsprintf(extractdir,L"%s\\SDI",manager_g->matcher->state->text+manager_g->matcher->state->temp);
                    manager_install(INSTALLDRIVERS);
                }
                redrawfield();
            }
            if(floating_itembar>=0&&i==2)
            {
                manager_g->manager_expand(floating_itembar);
            }
            break;

        case WM_RBUTTONDOWN:
            manager_g->manager_hitscan(x,y,&floating_itembar,&i);
            if(floating_itembar>=0&&(i==0||i==3))
                contextmenu(x,y);
            break;

        case WM_MBUTTONDOWN:
            mousedown=3;
            mousex=x;
            mousey=y;
            SetCursor(LoadCursor(0,IDC_SIZEALL));
            SetCapture(hwnd);
            break;

        case WM_MOUSEMOVE:
            si.cbSize=sizeof(si);
            if(mousedown==3)
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

                if(space_down)type=FLOATING_DRIVERLST;else
                if(ctrl_down||expertmode)type=FLOATING_CMPDRIVER;

                manager_g->manager_hitscan(x,y,&itembar_i,&i);
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
            canvasPopup.init();
            break;

        case WM_DESTROY:
            canvasPopup.free();
            break;

        case WM_PAINT:
            GetClientRect(hwnd,&rect);
            canvasPopup.begin(hwnd,rect.right,rect.bottom);

            //log_con("Redraw Popup %d\n",cntd++);
            box_draw(canvasPopup.getDC(),0,0,rect.right,rect.bottom,BOX_POPUP);
            switch(floating_type)
            {
                case FLOATING_SYSINFO:
                    SelectObject(canvasPopup.getDC(),hFont);
                    popup_sysinfo(manager_g,canvasPopup.getDC());
                    break;

                case FLOATING_TOOLTIP:
                    rect.left+=D(POPUP_OFSX);
                    rect.top+=D(POPUP_OFSY);
                    rect.right-=D(POPUP_OFSX);
                    rect.bottom-=D(POPUP_OFSY);
                    SelectObject(canvasPopup.getDC(),hFont);
                    SetTextColor(canvasPopup.getDC(),D(POPUP_TEXT_COLOR));
                    DrawText(canvasPopup.getDC(),STR(floating_itembar),-1,&rect,DT_WORDBREAK);
                    break;

                case FLOATING_CMPDRIVER:
                    SelectObject(canvasPopup.getDC(),hFont);
                    popup_drivercmp(manager_g,canvasPopup.getDC(),rect,floating_itembar);
                    break;

                case FLOATING_DRIVERLST:
                    SelectObject(canvasPopup.getDC(),hFont);
                    popup_driverlist(manager_g,canvasPopup.getDC(),rect,floating_itembar);
                    break;

                case FLOATING_ABOUT:
                    SelectObject(canvasPopup.getDC(),hFont);
                    popup_about(canvasPopup.getDC());
                    break;

                case FLOATING_DOWNLOAD:
                    SelectObject(canvasPopup.getDC(),hFont);
                    popup_download(canvasPopup.getDC());
                    break;

                default:
                    break;
            }

            canvasPopup.end();
            break;

        case WM_ERASEBKGND:
            return 1;

        default:
            return DefWindowProc(hwnd,message,wParam,lParam);
    }
    return 0;
}

void GetRelativeCtrlRect(HWND hWnd,RECT *rc)
{
   GetWindowRect(hWnd,rc);
   ScreenToClient(GetParent(hWnd),(LPPOINT)&((LPPOINT)rc)[0]);
   ScreenToClient(GetParent(hWnd),(LPPOINT)&((LPPOINT)rc)[1]);
   rc->right-=rc->left;
   rc->bottom-=rc->top;
}

BOOL CALLBACK LicenseProcedure(HWND hwnd,UINT Message,WPARAM wParam,LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    HWND hEditBox;
    RECT rect;
    LPCSTR s;int sz;

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

        case WM_WINDOWPOSCHANGED :
            {
                WINDOWPOS *wpos=(WINDOWPOS*)lParam;

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
            }
            return TRUE;

        default:
            break;
    }
    return FALSE;
}
//}
