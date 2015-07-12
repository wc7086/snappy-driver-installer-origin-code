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
#include "device.h"

//{ Global vars
long long ar_total,ar_proceed;
int instflag;
int itembar_act;
int needreboot=0;
wchar_t extractdir[BUFLEN];

long long totalinstalltime,totalextracttime;
long long installtime,extracttime;

//{ Installer
void _7z_total(long long i)
{
    ar_proceed=0;
    ar_total=i;
}

int _7z_setcomplited(long long i)
{
    if(statemode==STATEMODE_EXIT)return S_OK;
    if(installmode==MODE_STOPPING)return E_ABORT;
    if(manager_g->items_list.empty())return S_OK;
    if(!manager_g->items_list[itembar_act].checked)return E_ABORT;

    ar_proceed=i;
    manager_g->items_list[itembar_act].updatecur();
    manager_g->updateoverall();
    redrawfield();
    return S_OK;
}

void driver_install(wchar_t *hwid,const wchar_t *inf,int *ret,int *needrb)
{
    wchar_t cmd[BUFLEN];
    wchar_t buf[BUFLEN];
    void *install64bin;
    HANDLE thr;
    int size;
    FILE *f;

    *ret=1;*needrb=1;
    wsprintf(cmd,L"%s\\install64.exe",extractdir);
    if(!PathFileExists(cmd))
    {
        mkdir_r(extractdir);
        log_con("Dir: (%S)\n",extractdir);
        f=_wfopen(cmd,L"wb");
        if(f)
            log_con("Created '%S'\n",cmd);
        else
            log_con("Failed to create '%S'\n",cmd);
        get_resource(IDR_INSTALL64,&install64bin,&size);
        fwrite(install64bin,1,size,f);
        fclose(f);
    }

    Autoclicker.setflag(1);
    log_save();
    thr=(HANDLE)_beginthreadex(nullptr,0,&Autoclicker_t::thread_clicker,nullptr,0,nullptr);
    {
        if(flags&FLAG_DISABLEINSTALL)
            Sleep(2000);
        else
            *ret=UpdateDriverForPlugAndPlayDevices(hMain,hwid,inf,INSTALLFLAG_FORCE,needrb);
    }

    if(!*ret)*ret=GetLastError();
    if((flags&FLAG_DISABLEINSTALL)==0)
    if((unsigned)*ret==0xE0000235||manager_g->matcher->getState()->getArchitecture())//ERROR_IN_WOW64
    {
        wsprintf(buf,L"\"%s\" \"%s\"",hwid,inf);
        wsprintf(cmd,L"%s\\install64.exe",extractdir);
        log_con("'%S %S'\n",cmd,buf);
        *ret=run_command(cmd,buf,SW_HIDE,1);
        if((*ret&0x7FFFFFFF)==1)
        {
            *needrb=*ret&0x80000000?1:0;
            *ret&=~0x80000000;
        }
    }
    Autoclicker.setflag(0);
    WaitForSingleObject(thr,INFINITE);
    CloseHandle_log(thr,L"driver_install",L"thr");
    if(*ret==1)SaveHWID(hwid);
}

void removeextrainfs(wchar_t *inf)
{
    wchar_t buf[BUFLEN];
    wchar_t *s=inf;
    HANDLE hFind;
    WIN32_FIND_DATA FindFileData;

    while(wcschr(s,L'\\'))s=wcschr(s,L'\\')+1;

    wcscpy(buf,inf);
    wcscpy(buf+(s-inf),L"*.inF");
    hFind=FindFirstFile(buf,&FindFileData);
    if(hFind!=INVALID_HANDLE_VALUE)
    do
    if(!(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
    {
        wcscpy(buf+(s-inf),FindFileData.cFileName);
        if(!StrStrIW(inf,FindFileData.cFileName))
            log_con("deleting %S (%d)\n",buf,DeleteFile(buf));
        else
            log_con("keeping  %S\n",buf);
    }
    while(FindNextFile(hFind,&FindFileData)!=0);
}

unsigned int __stdcall Manager::thread_install(void *arg)
{
    UNREFERENCED_PARAMETER(arg)

    itembar_t *itembar,*itembar1;
    wchar_t cmd[BUFLEN];
    wchar_t hwid[BUFLEN];
    wchar_t inf[BUFLEN];
    wchar_t buf[BUFLEN];
    unsigned i,j;
    RESTOREPOINTINFOW pRestorePtSpec;
    STATEMGRSTATUS pSMgrStatus;
    HINSTANCE hinstLib=nullptr;
    WINAPI5t_SRSetRestorePointW WIN5f_SRSetRestorePointW;
    int r=0;
    int failed=0,installed=0;

    EnterCriticalSection(&sync);

    // Prepare extract dir

    installmode=MODE_INSTALLING;
    manager_g->items_list[SLOT_EXTRACTING].install_status=
        instflag&INSTALLDRIVERS?STR_INST_INSTALLING:STR_EXTR_EXTRACTING;
    manager_g->items_list[SLOT_EXTRACTING].isactive=1;
    manager_g->setpos();
    if(panels[11].isChecked(3))flags|=FLAG_AUTOINSTALL;

    // Download driverpacks
#ifdef USE_TORRENT
    unsigned downdrivers=0;
    itembar=&manager_g->items_list[RES_SLOTS];
    for(i=RES_SLOTS;i<manager_g->items_list.size()&&installmode==MODE_INSTALLING;i++,itembar++)
        if(itembar->checked&&itembar->isactive&&itembar->hwidmatch&&itembar->hwidmatch->getdrp_packontorrent())
    {
        if(!Updater.isTorrentReady())
        {
            log_con("Waiting for torrent");
            for(j=0;j<200;j++)
            {
                log_con("*");
                if(Updater.isTorrentReady())
                {
                    log_con("DONE\n");
                    break;
                }
                Sleep(100);
            }
            if(!Updater.isTorrentReady())break;
        }
        UpdateDialog.setPriorities(itembar->hwidmatch->getdrp_packname(),1);
        downdrivers++;
    }
    if(downdrivers)
    {
        Updater.resumeDownloading();
        log_con("{{{{{{{{\n");
        while(installmode&&!Updater.isUpdateCompleted())
        {
            Sleep(500);
        }
        log_con("{}}}}}}}}}\n");
    }
    if(installmode==MODE_STOPPING)
    {
        itembar->install_status=STR_INST_STOPPING;
        manager_g->items_list[SLOT_EXTRACTING].install_status=STR_INST_STOPPING;
        manager_g->selectnone();
    }
#endif

    // Restore point
    if(manager_g->items_list[SLOT_RESTORE_POINT].checked)
    {
        hinstLib=LoadLibrary(L"SrClient.dll");
        WIN5f_SRSetRestorePointW=(WINAPI5t_SRSetRestorePointW)GetProcAddress(hinstLib,"SRSetRestorePointW");

        if(hinstLib&&WIN5f_SRSetRestorePointW)
        {
            manager_g->items_list[SLOT_RESTORE_POINT].percent=500;
            manager_g->items_list[SLOT_RESTORE_POINT].install_status=STR_REST_CREATING;
            itembar_act=SLOT_RESTORE_POINT;
            redrawfield();

            memset(&pRestorePtSpec,0,sizeof(RESTOREPOINTINFOW));
            pRestorePtSpec.dwEventType=BEGIN_SYSTEM_CHANGE;
            pRestorePtSpec.dwRestorePtType=DEVICE_DRIVER_INSTALL;
            wcscpy(pRestorePtSpec.szDescription,L"Installed drivers");
            r=1;
            LeaveCriticalSection(&sync);
            if(flags&FLAG_DISABLEINSTALL)
                Sleep(2000);
            else
                r=WIN5f_SRSetRestorePointW(&pRestorePtSpec,&pSMgrStatus);
            EnterCriticalSection(&sync);

            log_con("rt rest point{ %d(%d)\n",r,pSMgrStatus.nStatus);
            manager_g->items_list[SLOT_RESTORE_POINT].percent=1000;
            if(r)
            {
                manager_g->items_list[SLOT_RESTORE_POINT].install_status=STR_REST_CREATED;

            }else
            {
                manager_g->items_list[SLOT_RESTORE_POINT].install_status=STR_REST_FAILED;
                log_err("ERROR in thread_install: Failed to create restore point\n");
            }
        }
        else
        {
            manager_g->items_list[SLOT_RESTORE_POINT].install_status=STR_REST_FAILED;
            log_err("ERROR in thread_install: Failed to create restore point %d\n",hinstLib);
        }
        redrawfield();
        if(hinstLib)FreeLibrary(hinstLib);
        manager_g->set_rstpnt(0);
        manager_g->items_list[SLOT_RESTORE_POINT].percent=0;
    }
    totalextracttime=totalinstalltime=0;
    wsprintf(buf,L"%ws\\SetupAPI.dev.log",manager_g->matcher->getState()->textas.get(manager_g->matcher->getState()->getWindir()));
    _wremove(buf);
goaround:
    itembar=&manager_g->items_list[0];
    for(i=0;i<manager_g->items_list.size()&&installmode==MODE_INSTALLING;i++,itembar++)
        if(i>=RES_SLOTS&&itembar->checked&&itembar->isactive&&itembar->hwidmatch)
    {
        int unpacked=0;
        int limits[7];

        memset(limits,0,sizeof(limits));
        itembar_act=i;
        ar_proceed=0;
        Hwidmatch *hwidmatch=itembar->hwidmatch;
        log_con("Installing $%04d\n",i);
        hwidmatch->print_hr();
        wsprintf(cmd,L"%s\\%S",extractdir,hwidmatch->getdrp_infpath());

        manager_g->animstart=GetTickCount();
        offset_target=(itembar->curpos>>16);
        SetTimer(hMain,1,1000/60,nullptr);

        // Extract
        extracttime=GetTickCount();
        wsprintf(inf,L"%s\\%S%S",
                unpacked?hwidmatch->getdrp_packpath():extractdir,
                hwidmatch->getdrp_infpath(),
                hwidmatch->getdrp_infname());
        if(PathFileExists(inf))
        {
            log_con("Already unpacked(%S)\n",inf);
            _7z_total(100);
            _7z_setcomplited(100);
            redrawfield();
        }
        else
        if(wcsstr(hwidmatch->getdrp_packname(),L"unpacked.7z"))
        {
            log_con("Unpacked '%S'\n",hwidmatch->getdrp_packpath());
            unpacked=1;
            _7z_total(100);
            _7z_setcomplited(100);
            redrawfield();
        }
        else
        {
            wsprintf(cmd,L"app x -y \"%s\\%s\" -o\"%s\"",hwidmatch->getdrp_packpath(),hwidmatch->getdrp_packname(),
                    extractdir,
                    hwidmatch->getdrp_infpath());

            itembar1=itembar;
            for(j=i;j<manager_g->items_list.size();j++,itembar1++)
                if(itembar1->checked&&
                   !wcscmp(hwidmatch->getdrp_packpath(),itembar1->hwidmatch->getdrp_packpath())&&
                   !wcscmp(hwidmatch->getdrp_packname(),itembar1->hwidmatch->getdrp_packname()))
            {
                wsprintf(buf,L" \"%S\"",itembar1->hwidmatch->getdrp_infpath());
                if(!wcsstr(cmd,buf))wcscat(cmd,buf);
            }
            log_con("Extracting via '%S'\n",cmd);
            itembar->install_status=instflag&INSTALLDRIVERS?STR_INST_EXTRACT:STR_EXTR_EXTRACTING;
            redrawfield();
            int tries=0;
            do
            {
                if(!itembar->checked||installmode!=MODE_INSTALLING||tries>60)break;
                LeaveCriticalSection(&sync);
                r=Extract7z(cmd);
                EnterCriticalSection(&sync);
                itembar=&manager_g->items_list[itembar_act];
                if(r==2)
                {
                    log_con("Error, checking for driverpack availability...");
                    if(PathFileExists(hwidmatch->getdrp_packpath()))break;
                    log_con("Waiting for driverpacks to become available.");
                    do
                    {
                        log_con(".");
                        Sleep(1000);
                        tries++;
                        if(!itembar->checked||installmode!=MODE_INSTALLING||tries>60)break;
                    }while(!PathFileExists(hwidmatch->getdrp_packpath())&&!hwidmatch->getdrp_packontorrent());
                    log_con("OK\n");
                }
            }while(r&&!hwidmatch->getdrp_packontorrent());
            if(installmode==MODE_STOPPING)
            {
                manager_g->items_list[SLOT_EXTRACTING].install_status=STR_INST_STOPPING;
                itembar->install_status=STR_INST_STOPPING;
            }
            if(!itembar->checked)manager_g->items_list[SLOT_EXTRACTING].install_status=STR_INST_STOPPING;
            //itembar->percent=manager_g->items_list[SLOT_EMPTY].percent;
            hwidmatch=itembar->hwidmatch;
            totalextracttime+=extracttime=GetTickCount()-extracttime;
            log_con("Ret %d, %ld secs\n",r,extracttime/1000);
            if(r&&itembar->install_status!=STR_INST_STOPPING)
            {
                itembar->install_status=STR_EXTR_FAILED;
                itembar->val1=r;
                itembar->checked=0;
                log_err("ERROR: extraction failed\n");
            }
        }

        if(instflag&OPENFOLDER&&itembar->checked)
            itembar->install_status=STR_EXTR_OK;

        // Install driver
        if(instflag&INSTALLDRIVERS&&itembar->checked)
        {
            int needrb=0,ret=1;
            wsprintf(inf,L"%s\\%S%S",
                   unpacked?hwidmatch->getdrp_packpath():extractdir,
                   hwidmatch->getdrp_infpath(),
                   hwidmatch->getdrp_infname());
            wsprintf(hwid,L"%S",hwidmatch->getdrp_drvHWID());
            log_con("Install32 '%S','%S'\n",hwid,inf);
            itembar->install_status=STR_INST_INSTALL;
            redrawfield();

            installtime=GetTickCount();
            LeaveCriticalSection(&sync);
            if(installmode==MODE_INSTALLING)
                driver_install(hwid,inf,&ret,&needrb);
            else
                ret=1;
            EnterCriticalSection(&sync);
            totalinstalltime+=installtime=GetTickCount()-installtime;
            itembar=&manager_g->items_list[itembar_act];

            if(ret==1)installed++;else failed++;
            log_con("Ret %d(0x%X),%s,%ld secs\n\n",ret,ret,needrb?"rb":"norb",installtime/1000);
            if(installmode==MODE_STOPPING)
            {
                itembar->install_status=STR_INST_STOPPING;
                manager_g->items_list[SLOT_EXTRACTING].install_status=STR_INST_STOPPING;
                manager_g->selectnone();
            }
            else
            {
                if(ret==1)
                    itembar->install_status=needrb?STR_INST_REBOOT:STR_INST_OK;
                else
                {
                    manager_g->expand(i,EXPAND_MODE::EXPAND);
                    itembar->install_status=STR_INST_FAILED;
                    itembar->val1=ret;
                    log_err("ERROR: installation failed\n");
                }

                if(needrb)needreboot=1;
            }
        }
        if(!unpacked&&(flags&FLAG_DELEXTRAINFS))removeextrainfs(inf);
        if(instflag&INSTALLDRIVERS)itembar->percent=0;
        itembar->checked=0;
        redrawmainwnd();
    }
    if(installmode==MODE_INSTALLING)
    {
        itembar=&manager_g->items_list[RES_SLOTS];
        for(j=RES_SLOTS;j<manager_g->items_list.size();j++,itembar++)
            if(itembar->checked)
                goto goaround;
    }
    // Instalation competed by this point
    wsprintf(buf,L"%ws\\SetupAPI.dev.log",manager_g->matcher->getState()->textas.get(manager_g->matcher->getState()->getWindir()));
    wsprintf(cmd,L"%s\\%ssetupAPI.log",log_dir,timestamp);
    if(!(flags&FLAG_NOLOGFILE))CopyFile(buf,cmd,0);

    if(instflag&OPENFOLDER)
    {
        wchar_t *p=extractdir+wcslen(extractdir);
        while(*(--p)!='\\');
        *p=0;
        log_con("%S\n",extractdir);
        ShellExecute(nullptr,L"explore",extractdir,nullptr,nullptr,SW_SHOW);
        manager_g->items_list[SLOT_EXTRACTING].isactive=0;
        manager_g->clear();
        manager_g->setpos();
    }
    if(instflag&INSTALLDRIVERS&&(flags&FLAG_KEEPTEMPFILES)==0)
    {
        wsprintf(buf,L" /c rd /s /q \"%s\"",extractdir);
        run_command(L"cmd",buf,SW_HIDE,1);
    }

    manager_g->items_list[SLOT_EXTRACTING].percent=0;
    if(installmode==MODE_STOPPING)
    {
        flags&=~FLAG_AUTOINSTALL;
        installmode=MODE_NONE;
    }
    if(installmode==MODE_INSTALLING)
    {
        manager_g->items_list[SLOT_EXTRACTING].install_status=
            needreboot?STR_INST_COMPLITED_RB:STR_INST_COMPLITED;
        installmode=MODE_SCANNING;

        ShowProgressInTaskbar(hMain,TBPF_NOPROGRESS,0,0);
        FLASHWINFO fi;
        fi.cbSize=sizeof(FLASHWINFO);
        fi.hwnd=hMain;
        fi.dwFlags=FLASHW_ALL|FLASHW_TIMERNOFG;
        fi.uCount=1;
        fi.dwTimeout=0;
        FlashWindowEx(&fi);
    }
    itembar_act=0;
    log_con("Exctract: %ld secs\n",totalextracttime/1000);
    log_con("Install: %ld secs\n",totalinstalltime/1000);
    ret_global=installed+(failed<<16);
    if(needreboot)ret_global|=0x40<<24;
    LeaveCriticalSection(&sync);
    ShowProgressInTaskbar(hMain,TBPF_NOPROGRESS,0,0);
    invalidate(INVALIDATE_DEVICES);
    redrawmainwnd();

    return 0;
}
//}

//{ Autoclicker
const wnddata_t Autoclicker_t::clicktbl[NUM_CLICKDATA]=
{
    // Windows XP
    {
        396,-1,
        390,283,
#ifdef AUTOCLICKER_CONFIRM
        107,249,// continue
#else
        245,249,  // stop
#endif
        132,23
    },
    // Windows 7 and Windows 8.1 (normal)
    {
        500,270,
        500,270,
#ifdef AUTOCLICKER_CONFIRM
        47,139,  // continue
        448,87   // continue
#else
        47,67,     // stop
        448,72     // stop
#endif
    },
    // Windows 7 and Windows 8.1 (rare)
    {
        500,230,
        500,230,
#ifdef AUTOCLICKER_CONFIRM
        47,120,  // continue
        448,66   // continue
#else
        47,67,     // stop
        448,53     // stop
#endif
    },
    // Windows 7 and Windows 8.1 (rare)
    {
        500,244,
        500,244,
#ifdef AUTOCLICKER_CONFIRM
        47,126,  // continue
        448,74   // continue
#else
        47,67,     // stop
        448,59     // stop
#endif
    },
    {
        -1,212,
        -1,212,
#ifdef AUTOCLICKER_CONFIRM
        -1,118,   // continue
        94,23     // continue
#else
        -1,118,   // stop
        129,23    // stop
#endif
    }
};
volatile int Autoclicker_t::clicker_flag;
Autoclicker_t Autoclicker;

void Autoclicker_t::calcwnddata(wnddata_t *w,HWND hwnd)
{
    WINDOWINFO pwi,pwb;
    HWND parent=GetParent(hwnd);
    pwb.cbSize=pwi.cbSize=sizeof(WINDOWINFO);

    GetWindowInfo(parent,&pwi);
    w->wnd_wx=pwi.rcWindow.right-pwi.rcWindow.left;
    w->wnd_wy=pwi.rcWindow.bottom-pwi.rcWindow.top;
    w->cln_wx=pwi.rcClient.right-pwi.rcClient.left;
    w->cln_wy=pwi.rcClient.bottom-pwi.rcClient.top;

    GetWindowInfo(hwnd,&pwb);
    w->btn_x =pwb.rcClient.left-pwi.rcClient.left;
    w->btn_y =pwb.rcClient.top-pwi.rcClient.top;
    w->btn_wx=pwb.rcClient.right-pwb.rcClient.left;
    w->btn_wy=pwb.rcClient.bottom-pwb.rcClient.top;
}

int Autoclicker_t::cmpclickdata(int *a,int *b)
{
    int i;

    for(i=0;i<8;i++,a++,b++)
    if(*a!=*b&&*b!=-1)return 0;

    return 1;
}

BOOL CALLBACK Autoclicker_t::EnumWindowsProc(HWND hwnd,LPARAM lParam)
{
    wchar_t buf[BUFLEN];
    WINDOWINFO pwi;
    wnddata_t w;
    int i;

    pwi.cbSize=sizeof(WINDOWINFO);
    GetWindowInfo(hwnd,&pwi);

    if(lParam&2)
    {
        GetWindowText(hwnd,buf,BUFLEN);
        log_file("Window %06X,%06X '%S'\n",hwnd,GetParent(hwnd),buf);
        GetClassName(hwnd,buf,BUFLEN);
        log_file("Class: '%S'\n",buf);
        RealGetWindowClass(hwnd,buf,BUFLEN);
        log_file("RealClass: '%S'\n",buf);
        log_file("\n");
    }

    if((lParam&1)==1)
    {
        Autoclicker.calcwnddata(&w,hwnd);
        if(lParam&2)
        {
            log_file("* MainWindow (%d,%d) (%d,%d)\n",w.wnd_wx,w.wnd_wy,w.cln_wx,w.cln_wy);
            log_file("* Child (%d,%d,%d,%d)\n",w.btn_x,w.btn_y,w.btn_wx,w.btn_wy);
            log_file("\n");
        }

        if((lParam&2)==0)for(i=0;i<NUM_CLICKDATA;i++)
            if(Autoclicker.cmpclickdata((int *)&w,(int *)&clicktbl[i]))
        {
            SwitchToThisWindow(hwnd,0);
            GetWindowInfo(GetParent(hwnd),&pwi);
            SetCursorPos(pwi.rcClient.left+w.btn_x+w.btn_wx/2,pwi.rcClient.top+w.btn_y+w.btn_wy/2);
            Sleep(500);
            if(IsWindow(hwnd))
            {
                GetWindowInfo(hwnd,&pwi);
                Autoclicker.calcwnddata(&w,hwnd);

                if(Autoclicker.cmpclickdata((int *)&w,(int *)&clicktbl[i]))
                {
                    GetWindowInfo(GetParent(hwnd),&pwi);
                    SetCursorPos(pwi.rcClient.left+w.btn_x+w.btn_wx/2,pwi.rcClient.top+w.btn_y+w.btn_wy/2);
                    int x=w.btn_x+w.btn_wx/2;
                    int y=w.btn_y+w.btn_wy/2;
                    int pos=(y<<16)|x;
                    SendMessage(GetParent(hwnd),WM_LBUTTONDOWN,0,pos);
                    SendMessage(GetParent(hwnd),WM_LBUTTONUP,  0,pos);
                    SetActiveWindow(hwnd);
                    SendMessage(hwnd,BM_CLICK,0,0);
                    log_con("Autoclicker fired\n");
                }
            }
        }
    }

    if((lParam&1)==0)
        EnumChildWindows(hwnd,EnumWindowsProc,lParam|1);

    return 1;
}

void Autoclicker_t::wndclicker(int mode)
{
    EnumChildWindows(GetDesktopWindow(),EnumWindowsProc,mode);
}

unsigned int __stdcall Autoclicker_t::thread_clicker(void *arg)
{
    UNREFERENCED_PARAMETER(arg);

    while(clicker_flag)
    {
        EnumChildWindows(GetDesktopWindow(),EnumWindowsProc,0);
        Sleep(100);
    }
    return 0;
}
//}
