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

#ifdef USE_TORRENT

#define BOOST_ALL_NO_LIB
#define BOOST_ASIO_ENABLE_CANCELIO
#define BOOST_ASIO_HASH_MAP_BUCKETS 1021
#define BOOST_ASIO_SEPARATE_COMPILATION
#define BOOST_EXCEPTION_DISABLE
#define BOOST_MULTI_INDEX_DISABLE_SERIALIZATION
#define BOOST_SYSTEM_STATIC_LINK 1
#define TORRENT_DISABLE_GEO_IP
#define TORRENT_NO_DEPRECATE
#define TORRENT_PRODUCTION_ASSERTS 1
#define TORRENT_RELEASE_ASSERTS 1
#define TORRENT_USE_I2P 0
#define TORRENT_USE_ICONV 0
#define TORRENT_USE_IPV6 0
#define TORRENT_USE_TOMMATH

#define IPV6_TCLASS 30
#include "libtorrent/config.hpp"
#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/alert_types.hpp"
#include "libtorrent/session.hpp"

#define TORRENT_URL "http://snappy-driver-installer.sourceforge.net/SDI_Update.torrent"
#define SMOOTHING_FACTOR 0.005

#define _WIN32_IE 0x0501
#include "main.h"

using namespace libtorrent;

//{ Global variables
session *hSession=nullptr;
torrent_handle hTorrent;
session_settings settings;
dht_settings dht;

int numfiles;

UpdateDialog_t UpdateDialog;
Updater_t Updater;
TorrentStatus_t TorrentStatus;

// UpdateDialog (static)
const int UpdateDialog_t::cxn[]={199,60,44,70,70,90};
HWND UpdateDialog_t::hListg;
HWND UpdateDialog_t::hUpdate=nullptr;
WNDPROC UpdateDialog_t::wpOrigButtonProc;
int UpdateDialog_t::bMouseInWindow=0;

// Updater (static)
int Updater_t::torrentport=50171;
int Updater_t::downlimit=0;
int Updater_t::uplimit=0;
int Updater_t::downloadmangar_exitflag;
int Updater_t::finishedupdating;
int Updater_t::finisheddownloading;
HANDLE Updater_t::downloadmangar_event=nullptr;
HANDLE Updater_t::thandle_download=nullptr;
//}


//{ UpdateDialog
int UpdateDialog_t::getnewver(const char *s)
{
    while(*s)
    {
        if(*s=='_'&&s[1]>='0'&&s[1]<='9')
            return atoi(s+1);

        s++;
    }
    return 0;
}

int UpdateDialog_t::getcurver(const char *ptr)
{
    wchar_t bffw[BUFLEN];
    wchar_t *s=bffw;

    wsprintf(bffw,L"%S",ptr);
    while(*s)
    {
        if(*s=='_'&&s[1]>='0'&&s[1]<='9')
        {
            *s=0;
            s=manager_g->matcher->finddrp(bffw);
            if(!s)return 0;
            while(*s)
            {
                if(*s==L'_'&&s[1]>=L'0'&&s[1]<=L'9')
                    return _wtoi_my(s+1);

                s++;
            }
            return 0;
        }
        s++;
    }
    return 0;
}

static int yes1(libtorrent::torrent_status const&){return true;}

int CALLBACK UpdateDialog_t::CompareFunc(LPARAM lParam1,LPARAM lParam2,LPARAM lParamSort)
{
    UNREFERENCED_PARAMETER(lParamSort)
    return lParam1-lParam2;
}

void UpdateDialog_t::ListView_SetItemTextUpdate(HWND hwnd,int iItem,int iSubItem,const wchar_t *str)
{
    wchar_t buf[BUFLEN];

    ListView_GetItemText(hwnd,iItem,iSubItem,buf,BUFLEN);
    if(wcscmp(str,buf)!=0)
        ListView_SetItemText(hwnd,iItem,iSubItem,const_cast<wchar_t *>(str));
}

void UpdateDialog_t::calctotalsize()
{
    totalsize=0;
    for(int i=0;i<ListView_GetItemCount(hListg);i++)
    if(ListView_GetCheckState(hListg,i))
    {
        wchar_t buf[BUFLEN];
        ListView_GetItemText(hListg,i,1,buf,32);
        totalsize+=_wtoi_my(buf);
    }
}

void UpdateDialog_t::updateTexts()
{
    if(!hUpdate)return;

    // Buttons
    SetWindowText(hUpdate,STR(STR_UPD_TITLE));
    SetWindowText(GetDlgItem(hUpdate,IDONLYUPDATE),STR(STR_UPD_ONLYUPDATES));
    SetWindowText(GetDlgItem(hUpdate,IDCHECKALL),STR(STR_UPD_BTN_ALL));
    SetWindowText(GetDlgItem(hUpdate,IDUNCHECKALL),STR(STR_UPD_BTN_NONE));
    SetWindowText(GetDlgItem(hUpdate,IDCHECKTHISPC),STR(STR_UPD_BTN_THISPC));
    SetWindowText(GetDlgItem(hUpdate,IDOK),STR(STR_UPD_BTN_OK));
    SetWindowText(GetDlgItem(hUpdate,IDCANCEL),STR(STR_UPD_BTN_CANCEL));
    SetWindowText(GetDlgItem(hUpdate,IDACCEPT),STR(STR_UPD_BTN_ACCEPT));

    // Total size
    wchar_t buf[BUFLEN];
    wsprintf(buf,STR(STR_UPD_TOTALSIZE),totalsize);
    SetWindowText(GetDlgItem(hUpdate,IDTOTALSIZE),buf);

    // Column headers
    LVCOLUMN lvc;
    lvc.mask=LVCF_TEXT;
    for(int i=0;i<6;i++)
    {
        lvc.pszText=const_cast<wchar_t *>(STR(STR_UPD_COL_NAME+i));
        ListView_SetColumn(GetDlgItem(hUpdate,IDLIST),i,&lvc);
    }
}

void UpdateDialog_t::setCheckboxes()
{
    if(Updater.isPaused())return;

    // The app and indexes
    int baseChecked=0,indexesChecked=0;
    for(int i=0;i<numfiles;i++)
    if(hTorrent.file_priority(i)==2)
    {
        if(StrStrIA(hTorrent.torrent_file()->file_at(i).path.c_str(),"indexes\\"))
            indexesChecked=1;
        else
            baseChecked=1;
    }

    // Driverpacks
    for(int i=0;i<ListView_GetItemCount(hListg);i++)
    {
        LVITEM item;
        item.mask=LVIF_PARAM;
        item.iItem=i;
        ListView_GetItem(hListg,&item);
        int val=0;

        if(item.lParam==-2)val=baseChecked;
        if(item.lParam==-1)val=indexesChecked;
        if(item.lParam>=0)val=hTorrent.file_priority(item.lParam);

        ListView_SetCheckState(hListg,i,val);
    }
}

void UpdateDialog_t::setPriorities()
{
    // Clear priorities for driverpacks
    for(int i=0;i<numfiles;i++)
    if(StrStrIA(hTorrent.torrent_file()->file_at(i).path.c_str(),"drivers\\"))
        hTorrent.file_priority(i,0);

    // Set priorities for driverpacks
    int base_pri=0,indexes_pri=0;
    for(int i=0;i<ListView_GetItemCount(hListg);i++)
    {
        LVITEM item;
        item.mask=LVIF_PARAM;
        item.iItem=i;
        ListView_GetItem(hListg,&item);
        int val=ListView_GetCheckState(hListg,i);

        if(item.lParam==-2)base_pri=val?2:0;
        if(item.lParam==-1)indexes_pri=val?2:0;
        if(item.lParam>= 0)hTorrent.file_priority(item.lParam,val);
    }

    // Set priorities for the app and indexes
    for(int i=0;i<numfiles;i++)
    if(!StrStrIA(hTorrent.torrent_file()->file_at(i).path.c_str(),"drivers\\"))
        hTorrent.file_priority(i,StrStrIA(hTorrent.torrent_file()->file_at(i).path.c_str(),"indexes\\")?indexes_pri:base_pri);
}

LRESULT CALLBACK UpdateDialog_t::NewButtonProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    short x,y;

    x=LOWORD(lParam);
    y=HIWORD(lParam);

    switch(uMsg)
    {
        case WM_MOUSEMOVE:
            drawpopup(STR_UPD_BTN_THISPC_H,FLOATING_TOOLTIP,x,y,hWnd);
            ShowWindow(hPopup,SW_SHOWNOACTIVATE);
            if(!bMouseInWindow)
            {
                bMouseInWindow=1;
                TRACKMOUSEEVENT tme;
                tme.cbSize=sizeof(tme);
                tme.dwFlags=TME_LEAVE;
                tme.hwndTrack=hWnd;
                TrackMouseEvent(&tme);
            }
            break;

        case WM_MOUSELEAVE:
            bMouseInWindow=0;
            drawpopup(-1,FLOATING_NONE,0,0,hWnd);
            break;

        default:
            return CallWindowProc(wpOrigButtonProc,hWnd,uMsg,wParam,lParam);
    }
    return true;
}

BOOL CALLBACK UpdateDialog_t::UpdateProcedure(HWND hwnd,UINT Message,WPARAM wParam,LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    LVCOLUMN lvc;
    HWND thispcbut,chk;
    wchar_t buf[32];
    int i;

    hListg=GetDlgItem(hwnd,IDLIST);
    thispcbut=GetDlgItem(hwnd,IDCHECKTHISPC);
    chk=GetDlgItem(hwnd,IDONLYUPDATE);

    switch(Message)
    {
        case WM_INITDIALOG:
            ListView_SetExtendedListViewStyle(hListg,LVS_EX_CHECKBOXES|LVS_EX_FULLROWSELECT);

            lvc.mask=LVCF_FMT|LVCF_WIDTH|LVCF_SUBITEM|LVCF_TEXT;
            lvc.pszText=(wchar_t *)L"";
            for(i=0;i<6;i++)
            {
                lvc.cx=cxn[i];
                lvc.iSubItem=i;
                lvc.fmt=i?LVCFMT_RIGHT:LVCFMT_LEFT;
                ListView_InsertColumn(hListg,i,&lvc);
            }

            hUpdate=hwnd;
            UpdateDialog.populate(0);
            UpdateDialog.updateTexts();
            UpdateDialog.setCheckboxes();
            if(flags&FLAG_ONLYUPDATES)SendMessage(chk,BM_SETCHECK,BST_CHECKED,0);

            wpOrigButtonProc=(WNDPROC)SetWindowLongPtr(thispcbut,GWLP_WNDPROC,(LONG_PTR)NewButtonProc);
            SetTimer(hwnd,1,2000,nullptr);

            if(flags&FLAG_AUTOUPDATE)SendMessage(hwnd,WM_COMMAND,IDCHECKALL,0);
            return TRUE;

        case WM_NOTIFY:
            if(((LPNMHDR)lParam)->code==LVN_ITEMCHANGED)
            {
                UpdateDialog.calctotalsize();
                UpdateDialog.updateTexts();
                return TRUE;
            }
            break;

        case WM_DESTROY:
            SetWindowLongPtr(thispcbut,GWLP_WNDPROC,(LONG_PTR)wpOrigButtonProc);
            hListg=nullptr;
            break;

        case WM_TIMER:
            if(hSession&&hSession->is_paused()==0)UpdateDialog.populate(1);
            //log_con(".");
            break;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDOK:
                    hUpdate=nullptr;
                    UpdateDialog.setPriorities();
                    flags&=~FLAG_ONLYUPDATES;
                    if(SendMessage(chk,BM_GETCHECK,0,0))flags|=FLAG_ONLYUPDATES;
                    Updater.resumeDownloading();
                    EndDialog(hwnd,IDOK);
                    return TRUE;

                case IDACCEPT:
                    UpdateDialog.setPriorities();
                    flags&=~FLAG_ONLYUPDATES;
                    if(SendMessage(chk,BM_GETCHECK,0,0))flags|=FLAG_ONLYUPDATES;
                    Updater.resumeDownloading();
                    return TRUE;

                case IDCANCEL:
                    hUpdate=nullptr;
                    EndDialog(hwnd,IDCANCEL);
                    return TRUE;

                case IDCHECKALL:
                case IDUNCHECKALL:
                    for(i=0;i<ListView_GetItemCount(hListg);i++)
                        ListView_SetCheckState(hListg,i,LOWORD(wParam)==IDCHECKALL?1:0);
                    if(flags&FLAG_AUTOUPDATE)
                    {
                        flags&=~FLAG_AUTOUPDATE;
                        SendMessage(hwnd,WM_COMMAND,IDOK,0);
                    }
                    return TRUE;

                case IDCHECKTHISPC:
                    for(i=0;i<ListView_GetItemCount(hListg);i++)
                    {
                        ListView_GetItemText(hListg,i,5,buf,32);
                        ListView_SetCheckState(hListg,i,StrStrIW(buf,STR(STR_UPD_YES))?1:0);
                    }
                    return TRUE;

                default:
                    break;
            }
            break;

        default:
            break;
    }
    return FALSE;
}

int UpdateDialog_t::populate(int update)
{
    wchar_t buf[BUFLEN];
    int ret=0;

    // Read torrent info
    boost::intrusive_ptr<torrent_info const> ti;
    std::vector<size_type> file_progress;
    ti=hTorrent.torrent_file();
    numfiles=0;
    if(!ti)return 0;
    hTorrent.file_progress(file_progress);
    numfiles=ti->num_files();

    // Calculate size and progress for the app and indexes
    int missingindexes=0;
    int newver=0;
    int basesize=0,basedownloaded=0;
    int indexsize=0,indexdownloaded=0;
    for(int i=0;i<numfiles;i++)
    {
        file_entry fe=ti->file_at(i);
        const char *filenamefull=StrChrA(fe.path.c_str(),'\\')+1;

        if(StrStrIA(filenamefull,"indexes\\"))
        {
            indexsize+=fe.size;
            indexdownloaded+=file_progress[i];
            wsprintf(buf,L"%S",filenamefull);
            *wcsstr(buf,L"DP_")=L'_';
            strsub(buf,L"indexes\\SDI",index_dir);
            if(!PathFileExists(buf))
                missingindexes=1;
        }
        else if(!StrStrIA(filenamefull,"drivers\\"))
        {
            basesize+=fe.size;
            basedownloaded+=file_progress[i];
            if(StrStrIA(filenamefull,"sdi_R"))
                newver=atol(StrStrIA(filenamefull,"sdi_R")+5);
        }
    }

    // Disable redrawing of the list
    if(hListg)SendMessage(hListg,WM_SETREDRAW,0,0);

    // Setup LVITEM
    LVITEM lvI;
    lvI.mask      =LVIF_TEXT|LVIF_STATE|LVIF_PARAM;
    lvI.stateMask =0;
    lvI.iSubItem  =0;
    lvI.state     =0;
    lvI.iItem     =0;

    // Add the app to the list
    lvI.lParam    =-2;
    //newver=300;
    int row=0;
    if(newver>SVN_REV)ret+=newver<<8;
    if(newver>SVN_REV&&hListg)
    {
        lvI.pszText=const_cast<wchar_t *>(STR(STR_UPD_APP));
        if(!update)row=ListView_InsertItem(hListg,&lvI);
        wsprintf(buf,L"%d %s",basesize/1024/1024,STR(STR_UPD_MB));
        ListView_SetItemTextUpdate(hListg,row,1,buf);
        wsprintf(buf,L"%d%%",basedownloaded*100/basesize);
        ListView_SetItemTextUpdate(hListg,row,2,buf);
        wsprintf(buf,L" SDI_R%d",newver);
        ListView_SetItemTextUpdate(hListg,row,3,buf);
        wsprintf(buf,L" SDI_R%d",SVN_REV);
        ListView_SetItemTextUpdate(hListg,row,4,buf);
        ListView_SetItemTextUpdate(hListg,row,5,STR(STR_UPD_YES));
        row++;
    }

    // Add indexes to the list
    lvI.lParam    =-1;
    if(missingindexes&&hListg)
    {
        lvI.pszText=const_cast<wchar_t *>(STR(STR_UPD_INDEXES));
        if(!update)row=ListView_InsertItem(hListg,&lvI);
        wsprintf(buf,L"%d %s",indexsize/1024/1024,STR(STR_UPD_MB));
        ListView_SetItemTextUpdate(hListg,row,1,buf);
        wsprintf(buf,L"%d%%",indexdownloaded*100/indexsize);
        ListView_SetItemTextUpdate(hListg,row,2,buf);
        ListView_SetItemTextUpdate(hListg,row,5,STR(STR_UPD_YES));
        row++;
    }

    // Add driverpacks to the list
    for(int i=0;i<numfiles;i++)
    {
        file_entry fe=ti->file_at(i);
        const char *filenamefull=StrChrA(fe.path.c_str(),'\\')+1;
        const char *filename=StrChrA(filenamefull,'\\')+1;
        if(!filename)filename=filenamefull;

        if(StrStrIA(filenamefull,".7z"))
        {
            int oldver;

            wsprintf(buf,L"%S",filename);
            lvI.pszText=buf;
            int sz=fe.size/1024/1024;
            if(!sz)sz=1;

            newver=getnewver(filenamefull);
            oldver=getcurver(filename);

            if(flags&FLAG_ONLYUPDATES)
                {if(newver>oldver&&oldver)ret++;}
            else
                if(newver>oldver)ret++;

            if(newver>oldver&&hListg)
            {
                lvI.lParam=i;
                if(!update)row=ListView_InsertItem(hListg,&lvI);
                wsprintf(buf,L"%d %s",sz,STR(STR_UPD_MB));
                ListView_SetItemTextUpdate(hListg,row,1,buf);
                wsprintf(buf,L"%d%%",file_progress[i]*100/ti->file_at(i).size);
                ListView_SetItemTextUpdate(hListg,row,2,buf);
                wsprintf(buf,L"%d",newver);
                ListView_SetItemTextUpdate(hListg,row,3,buf);
                wsprintf(buf,L"%d",oldver);
                if(!oldver)wsprintf(buf,L"%ws",STR(STR_UPD_MISSING));
                ListView_SetItemTextUpdate(hListg,row,4,buf);
                wsprintf(buf,L"%S",filename);
                wsprintf(buf,L"%ws",STR(STR_UPD_YES+manager_g->manager_drplive(buf)));
                ListView_SetItemTextUpdate(hListg,row,5,buf);
                row++;
            }
        }
    }

    // Enable redrawing of the list
    if(hListg)
    {
        ListView_SortItems(hListg,CompareFunc,0);
        SendMessage(hListg,WM_SETREDRAW,1,0);
    }

    if(update)return ret;

    if(ret)manager_g->itembar_settext(SLOT_NODRIVERS,0);
    manager_g->itembar_settext(SLOT_DOWNLOAD,ret?1:0,nullptr,ret,0,0);
    return ret;
}

void UpdateDialog_t::setPriorities(const wchar_t *name,int pri)
{
    char buf[BUFLEN];
    wsprintfA(buf,"%S",name);

    for(int i=0;i<numfiles;i++)
    if(StrStrIA(hTorrent.torrent_file()->file_at(i).path.c_str(),buf))
    {
        hTorrent.file_priority(i,pri);
        log_con("Req(%S,%d)\n",name,pri);
    }
}

void UpdateDialog_t::openDialog()
{
    DialogBox(ghInst,MAKEINTRESOURCE(IDD_DIALOG2),hMain,(DLGPROC)UpdateProcedure);
}

void UpdateDialog_t::clearList()
{
    ListView_DeleteAllItems(hListg);
}
//}

//{ Updater
void Updater_t::updateTorrentStatus()
{
    std::vector<torrent_status> temp;
    TorrentStatus_t *t=&TorrentStatus;

    memset(t,0,sizeof(TorrentStatus_t));
    if(hSession==nullptr)
    {
        wcscpy(t->error,STR(STR_DWN_ERRSES));
        return;
    }
    hSession->get_torrent_status(&temp,&yes1,0);

    if(temp.empty())
    {
        wcscpy(t->error,STR(STR_DWN_ERRTOR));
        return;
    }
    torrent_status& st=temp[0];

    t->downloaded=st.total_wanted_done;
    t->downloadsize=st.total_wanted;
    t->uploaded=st.total_payload_upload;

    t->elapsed=13;
    t->status=STR(STR_TR_ST0+(int)st.state);
    if(hSession->is_paused())t->status=STR(STR_TR_ST4);
    finisheddownloading=st.is_finished;

    wcscpy(t->error,L"");

    t->uploadspeed=st.upload_payload_rate;
    t->downloadspeed=st.download_payload_rate;

    t->seedstotal=st.list_seeds;
    t->peerstotal=st.list_peers;
    t->seedsconnected=st.num_seeds;
    t->peersconnected=st.num_peers;

    t->wasted=st.total_redundant_bytes;
    t->wastedhashfailes=st.total_failed_bytes;

    if(torrenttime)t->elapsed=GetTickCount()-torrenttime;
    if(t->downloadspeed)
    {
        averageSpeed=SMOOTHING_FACTOR*t->downloadspeed+(1-SMOOTHING_FACTOR)*averageSpeed;
        if(averageSpeed)t->remaining=(t->downloadsize-t->downloaded)/averageSpeed*1000;
    }

    t->sessionpaused=hSession->is_paused();
    t->torrentpaused=st.paused;

    if(TorrentStatus.downloadsize)
        manager_g->itembar_settext(SLOT_DOWNLOAD,1,nullptr,-1,-1,TorrentStatus.downloaded*1000/TorrentStatus.downloadsize);
    redrawfield();
}

void Updater_t::removeOldDriverpacks(const wchar_t *ptr)
{
    wchar_t bffw[BUFLEN];
    wchar_t *s=bffw;

    wcscpy(bffw,ptr);
    while(*s)
    {
        if(*s=='_'&&s[1]>='0'&&s[1]<='9')
        {
            *s=0;
            s=manager_g->matcher->finddrp(bffw);
            if(!s)return;
            wchar_t buf[BUFLEN];
            wsprintf(buf,L"%ws\\%s",drp_dir,s);
            log_con("Old file: %S\n",buf);
            _wremove(buf);
            return;
        }
        s++;
    }
}

void Updater_t::moveNewFiles()
{
    int i;
    boost::intrusive_ptr<torrent_info const> ti;

    ti=hTorrent.torrent_file();
    monitor_pause=1;

    // Delete old online idexes if new are downloaded
    for(i=0;i<numfiles;i++)
        if(hTorrent.file_priority(i)&&
           StrStrIA(ti->file_at(i).path.c_str(),"indexes\\SDI"))
            break;
    if(i!=numfiles)
    {
        wchar_t buf [BUFLEN];
        wsprintf(buf,L"/c del %ws\\_*.bin",index_dir);
        run_command(L"cmd",buf,SW_HIDE,1);
    }

    for(i=0;i<numfiles;i++)
    if(hTorrent.file_priority(i))
    {
        file_entry fe=ti->file_at(i);
        const char *filenamefull=StrChrA(fe.path.c_str(),'\\')+1;

        // Skip autorun.inf and del_old_driverpacks.bat
        if(StrStrIA(filenamefull,"autorun.inf")||StrStrIA(filenamefull,".bat"))continue;

        // Determine destination dirs
        wchar_t filenamefull_src[BUFLEN];
        wchar_t filenamefull_dst[BUFLEN];
        wsprintf(filenamefull_src,L"update\\%S",fe.path.c_str());
        wsprintf(filenamefull_dst,L"%S",filenamefull);
        strsub(filenamefull_dst,L"indexes\\SDI",index_dir);
        strsub(filenamefull_dst,L"drivers",drp_dir);
        strsub(filenamefull_dst,L"tools\\SDI",data_dir);

        // Delete old driverpacks
        if(StrStrIA(filenamefull,"drivers\\"))removeOldDriverpacks(filenamefull_dst+8);

        // Prepare online indexes
        wchar_t *p=filenamefull_dst;
        while(wcschr(p,L'\\'))p=wcschr(p,L'\\')+1;
        if(p&&StrStrIW(filenamefull_src,L"indexes\\SDI\\"))*p=L'_';

        // Create dirs for the file
        wchar_t dirs[BUFLEN];
        wcscpy(dirs,filenamefull_dst);
        p=dirs;
        while(wcschr(p,L'\\'))p=wcschr(p,L'\\')+1;
        if(p[-1]==L'\\')
        {
            *--p=0;
            mkdir_r(dirs);
        }

        // Move file
        log_con("New file: %S\n",filenamefull_dst);
        if(!MoveFileEx(filenamefull_src,filenamefull_dst,MOVEFILE_REPLACE_EXISTING))
            print_error(GetLastError(),L"MoveFileEx()");
    }
    run_command(L"cmd",L" /c rd /s /q update",SW_HIDE,1);
}

void Updater_t::checkUpdates()
{
    if(canWrite(L"update"))SetEvent(downloadmangar_event);
}

void Updater_t::showProgress(wchar_t *buf)
{
    wchar_t num1[64],num2[64];

    format_size(num1,TorrentStatus.downloaded,0);
    format_size(num2,TorrentStatus.downloadsize,0);

    wsprintf(buf,STR(STR_UPD_PROGRES),num1,num2,
                (TorrentStatus.downloadsize)?TorrentStatus.downloaded*100/TorrentStatus.downloadsize:0);
}

void Updater_t::showPopup(HDC hdcMem)
{
    textdata_t td;
    TorrentStatus_t t;
    int p0=D(POPUP_OFSX),p1=D(POPUP_OFSX)+10;
    int per=0;
    wchar_t num1[BUFLEN],num2[BUFLEN];


    td.col=D(POPUP_TEXT_COLOR);
    td.y=D(POPUP_OFSY);
    td.wy=D(POPUP_WY);
    td.hdcMem=hdcMem;
    td.maxsz=0;
    td.x=p0;

    //update_getstatus(&t);
    t=TorrentStatus;

    format_size(num1,t.downloaded,0);
    format_size(num2,t.downloadsize,0);
    if(t.downloadsize)per=t.downloaded*100/t.downloadsize;
    TextOutSF(&td,STR(STR_DWN_DOWNLOADED),STR(STR_DWN_DOWNLOADED_F),num1,num2,per);
    format_size(num1,t.uploaded,0);
    TextOutSF(&td,STR(STR_DWN_UPLOADED),num1);
    format_time(num1,t.elapsed);
    TextOutSF(&td,STR(STR_DWN_ELAPSED),num1);
    format_time(num1,t.remaining);
    TextOutSF(&td,STR(STR_DWN_REMAINING),num1);

    td.y+=td.wy;
    if(t.status)
        TextOutSF(&td,STR(STR_DWN_STATUS),L"%s",t.status);
    if(*t.error)
    {
        td.col=D(POPUP_CMP_INVALID_COLOR);
        TextOutSF(&td,STR(STR_DWN_ERROR),L"%s",t.error);
        td.col=D(POPUP_TEXT_COLOR);
    }
    format_size(num1,t.downloadspeed,1);
    TextOutSF(&td,STR(STR_DWN_DOWNLOADSPEED),num1);
    format_size(num1,t.uploadspeed,1);
    TextOutSF(&td,STR(STR_DWN_UPLOADSPEED),num1);

    td.y+=td.wy;
    TextOutSF(&td,STR(STR_DWN_SEEDS),STR(STR_DWN_SEEDS_F),t.seedsconnected,t.seedstotal);
    TextOutSF(&td,STR(STR_DWN_PEERS),STR(STR_DWN_SEEDS_F),t.peersconnected,t.peerstotal);
    format_size(num1,t.wasted,0);
    format_size(num2,t.wastedhashfailes,0);
    TextOutSF(&td,STR(STR_DWN_WASTED),STR(STR_DWN_WASTED_F),num1,num2);

//    TextOutSF(&td,L"Paused",L"%d,%d",t.sessionpaused,t.torrentpaused);
    popup_resize((td.maxsz+POPUP_SYSINFO_OFS+p0+p1),td.y+D(POPUP_OFSY));
}

void Updater_t::createThreads()
{
    if(thandle_download)return;

    TorrentStatus.sessionpaused=1;
    downloadmangar_exitflag=0;
    if(flags&FLAG_CHECKUPDATES)
    {
        downloadmangar_event=CreateEvent(nullptr,1,0,nullptr);
        thandle_download=(HANDLE)_beginthreadex(nullptr,0,&thread_download,nullptr,0,nullptr);
    }
}

void Updater_t::destroyThreads()
{
    if(thandle_download)
    {
        log_con("Stopping torrent thread...");
        downloadmangar_exitflag=1;
        SetEvent(downloadmangar_event);
        WaitForSingleObject(thandle_download,INFINITE);
        CloseHandle_log(thandle_download,L"thandle_download",L"thr");
        CloseHandle_log(downloadmangar_event,L"downloadmangar_event",L"event");
        log_con("DONE\n");
    }

    if(hSession)
    {
        log_con("Closing torrent session...");
        delete hSession;
        log_con("DONE\n");
    }
}

void Updater_t::downloadTorrent()
{
    error_code ec;
    int i;
    add_torrent_params params;

    time_chkupdate=GetTickCount();
    hSession=new session();

    hSession->start_lsd();
    hSession->start_upnp();
    hSession->start_natpmp();

    // Connecting
    hSession->listen_on(std::make_pair(torrentport,torrentport),ec);
    if(ec)log_err("ERROR: failed to open listen socket: %s\n",ec.message().c_str());
    log_con("Listen port: %d (%s)\nDownload limit: %dKb\nUpload limit: %dKb\n",
            hSession->listen_port(),hSession->is_listening()?"connected":"disconnected",
            downlimit,uplimit);

    // Session settings
    dht.privacy_lookups=true;
    hSession->set_dht_settings(dht);
    settings.use_dht_as_fallback=false;
    hSession->add_dht_router(std::make_pair(std::string("router.bittorrent.com"),6881));
    hSession->add_dht_router(std::make_pair(std::string("router.utorrent.com"),6881));
    hSession->add_dht_router(std::make_pair(std::string("router.bitcomet.com"),6881));
    hSession->start_dht();
    hSession->set_alert_mask(
        alert::error_notification|
        alert::tracker_notification|
        alert::ip_block_notification|
        alert::dht_notification|
        alert::performance_warning|
        alert::storage_notification);

    // Settings
    settings.user_agent="Snappy Driver Installer/" SVN_REV2;
    settings.always_send_user_agent=true;
    settings.anonymous_mode=false;
    settings.choking_algorithm=session_settings::auto_expand_choker;
    settings.disk_cache_algorithm=session_settings::avoid_readback;
    settings.volatile_read_cache=false;
    hSession->set_settings(settings);

    // Setup path and URL
    params.save_path="update";
    params.url=TORRENT_URL;
    params.flags=add_torrent_params::flag_paused;
    hTorrent=hSession->add_torrent(params,ec);
    if(ec)log_err("ERROR: failed to add torrent: %s\n",ec.message().c_str());

    // Pause and set speed limits
    hSession->pause();
    hTorrent.set_download_limit(downlimit*1024);
    hTorrent.set_upload_limit(uplimit*1024);
    hTorrent.resume();

    // Download torrent
    log_con("Waiting for torrent");
    for(i=0;i<100;i++)
    {
        log_con(".");
        if(hTorrent.torrent_file())
        {
            log_con("DONE\n");
            break;
        }
        if(downloadmangar_exitflag)break;
        Sleep(100);
    }
    log_con(hTorrent.torrent_file()?"":"FAILED\n");

    // Pupulate list
    i=UpdateDialog.populate(0);
    log_con("Latest version: R%d\nUpdated driverpacks available: %d\n",i>>8,i&0xFF);
    for(i=0;i<numfiles;i++)hTorrent.file_priority(i,0);
    time_chkupdate=GetTickCount()-time_chkupdate;
}

void Updater_t::resumeDownloading()
{
    if(!hSession||!hTorrent.torrent_file())
    {
        finisheddownloading=1;
        finishedupdating=1;
        return;
    }
    if(hSession->is_paused())
    {
        hTorrent.force_recheck();
        log_con("torrent_resume\n");
        SetEvent(downloadmangar_event);
    }
    hSession->resume();
    finisheddownloading=0;
    finishedupdating=0;
    torrenttime=GetTickCount();
}

unsigned int __stdcall Updater_t::thread_download(void *arg)
{
    UNREFERENCED_PARAMETER(arg)

    // Wait till is allowed to download the torrent
    log_con("{thread_download\n");
    WaitForSingleObject(downloadmangar_event,INFINITE);
    if(downloadmangar_exitflag)return 0;

    // Download torrent
    Updater.downloadTorrent();
    Updater.updateTorrentStatus();
    ResetEvent(downloadmangar_event);

    while(!downloadmangar_exitflag)
    {
        // Wait till is allowed to download driverpacks
        if(flags&FLAG_AUTOUPDATE&&canWrite(L"update"))
            UpdateDialog.openDialog();
        else
            WaitForSingleObject(downloadmangar_event,INFINITE);
        if(downloadmangar_exitflag)break;

        // Downloading loop
        log_con("{torrent_start\n");
        while(!downloadmangar_exitflag&&hSession)
        {
            Sleep(500);

            // Show progress
            Updater.updateTorrentStatus();
            ShowProgressInTaskbar(hMain,TBPF_NORMAL,TorrentStatus.downloaded,TorrentStatus.downloadsize);
            InvalidateRect(hPopup,nullptr,0);

            // Send libtorrent messages to log
            std::unique_ptr<alert> holder;
            holder=hSession->pop_alert();
            while(holder.get())
            {
                log_con("Torrent: %s | %s\n",holder.get()->what(),holder.get()->message().c_str());
                holder=hSession->pop_alert();
            }

            if(finisheddownloading)
            {
                log_con("Torrent: finished\n");
                hSession->pause();

                // Flash cache
                log_con("Torrent: flushing cache...");
                hTorrent.flush_cache();
                while(1)
                {
                    alert const* a=hSession->wait_for_alert(seconds(60*2));
                    if(!a)
                    {
                        log_con("time out\n");
                        break;
                    }
                    std::unique_ptr<alert> holder2=hSession->pop_alert();
                    if(alert_cast<cache_flushed_alert>(a))
                    {
                        log_con("done\n");
                        break;
                    }
                }

                // Move files
                Updater.moveNewFiles();
                hTorrent.force_recheck();

                // Update list
                UpdateDialog.clearList();
                UpdateDialog.populate(0);

                // Execute user cmd
                if(*finish_upd)
                {
                    wchar_t buf[BUFLEN];
                    wsprintf(buf,L" /c %s",finish_upd);
                    run_command(L"cmd",buf,SW_HIDE,0);
                }
                if(flags&FLAG_AUTOCLOSE)PostMessage(hMain,WM_CLOSE,0,0);

                // Flash in taskbar
                ShowProgressInTaskbar(hMain,TBPF_NOPROGRESS,0,0);
                FLASHWINFO fi;
                fi.cbSize=sizeof(FLASHWINFO);
                fi.hwnd=hMain;
                fi.dwFlags=FLASHW_ALL|FLASHW_TIMERNOFG;
                fi.uCount=1;
                fi.dwTimeout=0;
                FlashWindowEx(&fi);
                break;
            }
        }
        // Download is completed
        finishedupdating=1;
        hSession->pause();
        Updater.updateTorrentStatus();
        monitor_pause=0;
        invaidate(INVALIDATE_INDEXES);
        log_con("}torrent_stop\n");
        ResetEvent(downloadmangar_event);
    }
    log_con("}thread_download\n");
    return 0;
}

bool Updater_t::isTorrentReady(){return hTorrent.torrent_file()!=nullptr;}
//}
#else

#include "main.h"
Updater_t Updater;
int Updater_t::torrentport=50171;
int Updater_t::downlimit=0;
int Updater_t::uplimit=0;
#endif
