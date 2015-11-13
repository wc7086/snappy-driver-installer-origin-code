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

#ifdef USE_TORRENT
#include "common.h"
#include "settings.h"
#include "logging.h"
#include "matcher.h"
#include "update.h"
#include "manager.h"
#include "theme.h"
#include "gui.h"
#include "system.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4244)
#pragma warning(disable:4245)
#pragma warning(disable:4267)
#pragma warning(disable:4512)
#endif

#include "libtorrent/config.hpp"
#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/alert_types.hpp"
#include "libtorrent/session.hpp"

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <windows.h>
#include <shobjidl.h>
#ifdef _MSC_VER
#include <process.h>
#endif

#include "main.h"
#include "draw.h"

#define TORRENT_URL "http://snappy-driver-installer.sourceforge.net/SDI_Update.torrent"
#define SMOOTHING_FACTOR 0.005

using namespace libtorrent;

// TorrentStatus
class TorrentStatus_t
{
    long long downloaded,downloadsize;
    long long uploaded;
    __int64 elapsed,remaining;

    int status_strid;
    wchar_t error[BUFLEN];
    int uploadspeed,downloadspeed;
    int seedstotal,seedsconnected;
    int peerstotal,peersconnected;
    int wasted,wastedhashfailes;

    bool sessionpaused,torrentpaused;

    friend class UpdaterImp;
};

// UpdateDialog
class UpdateDialog_t
{
    static const int cxn[];
    static WNDPROC wpOrigButtonProc;
    static int bMouseInWindow;
    static HWND hUpdate;
    int totalsize;
    static HWND hListg;

private:
    int  getnewver(const char *ptr);
    int  getcurver(const char *ptr);
    static LPARAM CALLBACK CompareFunc(LPARAM lParam1,LPARAM lParam2,LPARAM lParamSort);
    void ListView_SetItemTextUpdate(HWND hwnd,int iItem,int iSubItem,const wchar_t *str);
    void calctotalsize();
    void updateTexts();

    void setCheckboxes();
    void setPriorities();
    static LRESULT CALLBACK NewButtonProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
    static BOOL CALLBACK UpdateProcedure(HWND hwnd,UINT Message,WPARAM wParam,LPARAM lParam);

public:
    int  populate(int flags,bool clearlist=false);
    void setFilePriority(const wchar_t *name,int pri);
    void openDialog();
};

// Updater
class UpdaterImp:public Updater_t
{
    static Event *downloadmangar_event;
    static ThreadAbs *thandle_download;

    static int downloadmangar_exitflag;
    static bool finishedupdating;
    static bool finisheddownloading;

    int averageSpeed=0;
    long long torrenttime=0;

private:
    void downloadTorrent();
    void updateTorrentStatus();
    void removeOldDriverpacks(const wchar_t *ptr);
    void moveNewFiles();
    static unsigned int __stdcall thread_download(void *arg);

public:
    UpdaterImp();
    ~UpdaterImp();

    void ShowProgress(wchar_t *buf);
    void ShowPopup(Canvas &canvas);

    void checkUpdates();
    void resumeDownloading();
    void pause();

    bool isTorrentReady();
    bool isPaused();
    bool isUpdateCompleted();

    int  Populate(int flags);
    void SetFilePriority(const wchar_t *name,int pri);
    void OpenDialog();
};
Updater_t *CreateUpdater(){return new UpdaterImp;}

//{ Global variables
session *hSession=nullptr;
torrent_handle hTorrent;
session_settings settings;
dht_settings dht;

UpdateDialog_t UpdateDialog;
Updater_t *Updater;
TorrentStatus_t TorrentStatus;

enum DOWNLOAD_STATUS
{
    DOWNLOAD_STATUS_WAITING,
    DOWNLOAD_STATUS_DOWLOADING_TORRENT,
    DOWNLOAD_STATUS_TORRENT_GOT,
    DOWNLOAD_STATUS_DOWLOADING_DATA,
    DOWNLOAD_STATUS_FINISHED_DOWNLOADING,
    DOWNLOAD_STATUS_PAUSING,
    DOWNLOAD_STATUS_STOPPING,
};

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
int Updater_t::connections=0;
int UpdaterImp::downloadmangar_exitflag;
bool UpdaterImp::finishedupdating;
bool UpdaterImp::finisheddownloading;
Event *UpdaterImp::downloadmangar_event=nullptr;
ThreadAbs *UpdaterImp::thandle_download=nullptr;
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
    WStringShort bffw;

    bffw.sprintf(L"%S",ptr);
    wchar_t *s=bffw.GetV();
    while(*s)
    {
        if(*s=='_'&&s[1]>='0'&&s[1]<='9')
        {
            *s=0;
            s=const_cast<wchar_t *>(manager_g->matcher->finddrp(bffw.Get()));
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

static bool yes1(libtorrent::torrent_status const&){return true;}

LPARAM CALLBACK UpdateDialog_t::CompareFunc(LPARAM lParam1,LPARAM lParam2,LPARAM lParamSort)
{
	UNREFERENCED_PARAMETER(lParamSort);
    return lParam1-lParam2;
}

void UpdateDialog_t::ListView_SetItemTextUpdate(HWND hwnd,int iItem,int iSubItem,const wchar_t *str)
{
    wchar_t buf[BUFLEN];

    *buf=0;
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
    WStringShort buf;
    buf.sprintf(STR(STR_UPD_TOTALSIZE),totalsize);
    SetWindowText(GetDlgItem(hUpdate,IDTOTALSIZE),buf.Get());

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
    if(Updater->isPaused())return;

    // The app and indexes
    int baseChecked=0,indexesChecked=0;
    for(int i=0;i<Updater->numfiles;i++)
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
        if(item.lParam>=0)val=hTorrent.file_priority((int)item.lParam);

        ListView_SetCheckState(hListg,i,val);
    }
}

void UpdateDialog_t::setPriorities()
{
    // Clear priorities for driverpacks
    for(int i=0;i<Updater->numfiles;i++)
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
        if(item.lParam>= 0)hTorrent.file_priority(static_cast<int>(item.lParam),val);
    }

    // Set priorities for the app and indexes
    for(int i=0;i<Updater->numfiles;i++)
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
            Popup->drawpopup(STR_UPD_BTN_THISPC_H,FLOATING_TOOLTIP,x,y,hWnd);
            ShowWindow(Popup->hPopup,SW_SHOWNOACTIVATE);
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
            Popup->drawpopup(0,FLOATING_NONE,0,0,hWnd);
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
            setMirroring(hwnd);
            ListView_SetExtendedListViewStyle(hListg,LVS_EX_CHECKBOXES|LVS_EX_FULLROWSELECT);
            lvc.mask=LVCF_FMT|LVCF_WIDTH|LVCF_SUBITEM|LVCF_TEXT;
            lvc.pszText=const_cast<wchar_t *>(L"");
            for(i=0;i<6;i++)
            {
                lvc.cx=cxn[i];
                lvc.iSubItem=i;
                lvc.fmt=i?LVCFMT_RIGHT:LVCFMT_LEFT;
                ListView_InsertColumn(hListg,i,&lvc);
            }

            hUpdate=hwnd;
            UpdateDialog.populate(0,true);
            UpdateDialog.updateTexts();
            UpdateDialog.setCheckboxes();
            if(Settings.flags&FLAG_ONLYUPDATES)SendMessage(chk,BM_SETCHECK,BST_CHECKED,0);

            wpOrigButtonProc=(WNDPROC)SetWindowLongPtr(thispcbut,GWLP_WNDPROC,(LONG_PTR)NewButtonProc);
            SetTimer(hwnd,1,2000,nullptr);

            if(Settings.flags&FLAG_AUTOUPDATE)SendMessage(hwnd,WM_COMMAND,IDCHECKALL,0);
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
            //Log.print_con(".");
            break;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDOK:
                    hUpdate=nullptr;
                    UpdateDialog.setPriorities();
                    Settings.flags&=~FLAG_ONLYUPDATES;
                    if(SendMessage(chk,BM_GETCHECK,0,0))Settings.flags|=FLAG_ONLYUPDATES;
                    Updater->resumeDownloading();
                    EndDialog(hwnd,IDOK);
                    return TRUE;

                case IDACCEPT:
                    UpdateDialog.setPriorities();
                    Settings.flags&=~FLAG_ONLYUPDATES;
                    if(SendMessage(chk,BM_GETCHECK,0,0))Settings.flags|=FLAG_ONLYUPDATES;
                    Updater->resumeDownloading();
                    return TRUE;

                case IDCANCEL:
                    hUpdate=nullptr;
                    EndDialog(hwnd,IDCANCEL);
                    return TRUE;

                case IDONLYUPDATE:
                    Settings.flags&=~FLAG_ONLYUPDATES;
                    if(SendMessage(chk,BM_GETCHECK,0,0))Settings.flags|=FLAG_ONLYUPDATES;
                    UpdateDialog.populate(0,true);
                    break;

                case IDCHECKALL:
                case IDUNCHECKALL:
                    for(i=0;i<ListView_GetItemCount(hListg);i++)
                        ListView_SetCheckState(hListg,i,LOWORD(wParam)==IDCHECKALL?1:0);
                    if(Settings.flags&FLAG_AUTOUPDATE)
                    {
                        Settings.flags&=~FLAG_AUTOUPDATE;
                        SendMessage(hwnd,WM_COMMAND,IDOK,0);
                    }
                    return TRUE;

                case IDCHECKTHISPC:
                    for(i=0;i<ListView_GetItemCount(hListg);i++)
                    {
                        *buf=0;
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

int UpdateDialog_t::populate(int update,bool clearlist)
{
    wchar_t buf[BUFLEN];
    int ret=0;

    // Read torrent info
    boost::intrusive_ptr<torrent_info const> ti;
    std::vector<size_type> file_progress;
    ti=hTorrent.torrent_file();
    Updater->numfiles=0;
    if(!ti)return 0;
    hTorrent.file_progress(file_progress);
    Updater->numfiles=ti->num_files();

    // Calculate size and progress for the app and indexes
    int missingindexes=0;
    int newver=0;
    __int64 basesize=0,basedownloaded=0;
    __int64 indexsize=0,indexdownloaded=0;
    for(int i=0;i<Updater->numfiles;i++)
    {
        file_entry fe=ti->file_at(i);
        const char *filenamefull=strchr(fe.path.c_str(),'\\')+1;

        if(StrStrIA(filenamefull,"indexes\\"))
        {
            indexsize+=fe.size;
            indexdownloaded+=file_progress[i];
            wsprintf(buf,L"%S",filenamefull);
            *wcsstr(buf,L"DP_")=L'_';
            strsub(buf,L"indexes\\SDI",Settings.index_dir);
            if(!System.FileExists(buf))
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
    if(clearlist)ListView_DeleteAllItems(hListg);

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
        wsprintf(buf,L"%d %s",(int)(basesize/1024/1024),STR(STR_UPD_MB));
        ListView_SetItemTextUpdate(hListg,row,1,buf);
        wsprintf(buf,L"%d%%",(int)(basedownloaded*100/basesize));
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
        wsprintf(buf,L"%d %s",(int)(indexsize/1024/1024),STR(STR_UPD_MB));
        ListView_SetItemTextUpdate(hListg,row,1,buf);
        wsprintf(buf,L"%d%%",(int)(indexdownloaded*100/indexsize));
        ListView_SetItemTextUpdate(hListg,row,2,buf);
        ListView_SetItemTextUpdate(hListg,row,5,STR(STR_UPD_YES));
        row++;
    }

    // Add driverpacks to the list
    for(int i=0;i<Updater->numfiles;i++)
    {
        file_entry fe=ti->file_at(i);
        const char *filenamefull=strchr(fe.path.c_str(),'\\')+1;
        const char *filename=strchr(filenamefull,'\\')+1;
        if(!filename)filename=filenamefull;

        if(StrStrIA(filenamefull,".7z"))
        {
            int oldver;

            wsprintf(buf,L"%S",filename);
            lvI.pszText=buf;
            int sz=(int)(fe.size/1024/1024);
            if(!sz)sz=1;

            newver=getnewver(filenamefull);
            oldver=getcurver(filename);

            if(Settings.flags&FLAG_ONLYUPDATES)
                {if(newver>oldver&&oldver)ret++;else continue;}
            else
                if(newver>oldver)ret++;

            if(newver>oldver&&hListg)
            {
                lvI.lParam=i;
                if(!update)row=ListView_InsertItem(hListg,&lvI);
                wsprintf(buf,L"%d %s",sz,STR(STR_UPD_MB));
                ListView_SetItemTextUpdate(hListg,row,1,buf);
                wsprintf(buf,L"%d%%",(int)(file_progress[i]*100/ti->file_at(i).size));
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

void UpdateDialog_t::setFilePriority(const wchar_t *name,int pri)
{
    char buf[BUFLEN];
    wsprintfA(buf,"%S",name);

    for(int i=0;i<Updater->numfiles;i++)
    if(StrStrIA(hTorrent.torrent_file()->file_at(i).path.c_str(),buf))
    {
        hTorrent.file_priority(i,pri);
        Log.print_con("Req(%S,%d)\n",name,pri);
    }
}

void UpdateDialog_t::openDialog()
{
    DialogBox(ghInst,MAKEINTRESOURCE(IDD_DIALOG2),MainWindow.hMain,(DLGPROC)UpdateProcedure);
}
//}

//{ Updater
void UpdaterImp::updateTorrentStatus()
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
    t->status_strid=STR_TR_ST0+st.state;
    if(hSession->is_paused())t->status_strid=STR_TR_ST4;
    finisheddownloading=st.is_finished;

    wcscpy(t->error,L"");

    t->uploadspeed=st.upload_payload_rate;
    t->downloadspeed=st.download_payload_rate;

    t->seedstotal=st.list_seeds;
    t->peerstotal=st.list_peers;
    t->seedsconnected=st.num_seeds;
    t->peersconnected=st.num_peers;

    t->wasted=(int)st.total_redundant_bytes;
    t->wastedhashfailes=(int)st.total_failed_bytes;

    if(torrenttime)t->elapsed=GetTickCount()-torrenttime;
    if(t->downloadspeed)
    {
        averageSpeed=static_cast<int>(SMOOTHING_FACTOR*t->downloadspeed+(1-SMOOTHING_FACTOR)*averageSpeed);
        if(averageSpeed)t->remaining=(t->downloadsize-t->downloaded)/averageSpeed*1000;
    }

    t->sessionpaused=hSession->is_paused();
    t->torrentpaused=st.paused;

    if(TorrentStatus.downloadsize)
        manager_g->itembar_settext(SLOT_DOWNLOAD,1,nullptr,-1,-1,TorrentStatus.downloaded*1000/TorrentStatus.downloadsize);
    MainWindow.redrawfield();
}

void UpdaterImp::removeOldDriverpacks(const wchar_t *ptr)
{
    WStringShort bffw;
    bffw.append(ptr);
    wchar_t *s=bffw.GetV();
    while(*s)
    {
        if(*s=='_'&&s[1]>='0'&&s[1]<='9')
        {
            *s=0;
            s=const_cast<wchar_t *>(manager_g->matcher->finddrp(bffw.Get()));
            if(!s)return;
            WStringShort buf;
            buf.sprintf(L"%ws\\%s",Settings.drp_dir,s);
            Log.print_con("Old file: %S\n",buf.Get());
            _wremove(buf.Get());
            return;
        }
        s++;
    }
}

void UpdaterImp::moveNewFiles()
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
        WStringShort buf;
        buf.sprintf(L"/c del %ws\\_*.bin",Settings.index_dir);
        System.run_command(L"cmd",buf.Get(),SW_HIDE,1);
    }

    for(i=0;i<numfiles;i++)
    if(hTorrent.file_priority(i))
    {
        file_entry fe=ti->file_at(i);
        const char *filenamefull=strchr(fe.path.c_str(),'\\')+1;

        // Skip autorun.inf and del_old_driverpacks.bat
        if(StrStrIA(filenamefull,"autorun.inf")||StrStrIA(filenamefull,".bat"))continue;

        // Determine destination dirs
        WStringShort filenamefull_src;
        wchar_t filenamefull_dst[BUFLEN];
        filenamefull_src.sprintf(L"update\\%S",fe.path.c_str());
        wsprintf(filenamefull_dst,L"%S",filenamefull);
        strsub(filenamefull_dst,L"indexes\\SDI",Settings.index_dir);
        strsub(filenamefull_dst,L"drivers",Settings.drp_dir);
        strsub(filenamefull_dst,L"tools\\SDI",Settings.data_dir);

        // Delete old driverpacks
        if(StrStrIA(filenamefull,"drivers\\"))removeOldDriverpacks(filenamefull_dst+8);

        // Prepare online indexes
        wchar_t *p=filenamefull_dst;
        while(wcschr(p,L'\\'))p=wcschr(p,L'\\')+1;
        if(p&&StrStrIW(filenamefull_src.Get(),L"indexes\\SDI\\"))*p=L'_';

        // Create dirs for the file
        WStringShort dirs;
        dirs.append(filenamefull_dst);
        p=dirs.GetV();
        while(wcschr(p,L'\\'))p=wcschr(p,L'\\')+1;
        if(p[-1]==L'\\')
        {
            *--p=0;
            mkdir_r(dirs.Get());
        }

        // Move file
        Log.print_con("New file: %S\n",filenamefull_dst);
        if(!MoveFileEx(filenamefull_src.Get(),filenamefull_dst,MOVEFILE_REPLACE_EXISTING))
            Log.print_syserr(GetLastError(),L"MoveFileEx()");
    }
    System.run_command(L"cmd",L" /c rd /s /q update",SW_HIDE,1);
}

void UpdaterImp::checkUpdates()
{
    if(System.canWrite(L"update"))
    {
        downloadmangar_exitflag=DOWNLOAD_STATUS_DOWLOADING_TORRENT;
        downloadmangar_event->raise();
    }
}

void UpdaterImp::ShowProgress(wchar_t *buf)
{
    wchar_t num1[64],num2[64];

    format_size(num1,TorrentStatus.downloaded,0);
    format_size(num2,TorrentStatus.downloadsize,0);

    wsprintf(buf,STR(STR_UPD_PROGRES),num1,num2,
                (TorrentStatus.downloadsize)?TorrentStatus.downloaded*100/TorrentStatus.downloadsize:0);
}

void UpdaterImp::ShowPopup(Canvas &canvas)
{
    textdata_vert td(canvas);
    TorrentStatus_t t;
    int p0=D_X(POPUP_OFSX),p1=D_X(POPUP_OFSX)+10;
    long long per=0;
    wchar_t num1[BUFLEN],num2[BUFLEN];

    td.y=D_X(POPUP_OFSY);

    //update_getstatus(&t);
    t=TorrentStatus;

    format_size(num1,t.downloaded,0);
    format_size(num2,t.downloadsize,0);
    if(t.downloadsize)per=t.downloaded*100/t.downloadsize;
    td.TextOutSF(STR(STR_DWN_DOWNLOADED),STR(STR_DWN_DOWNLOADED_F),num1,num2,per);
    format_size(num1,t.uploaded,0);
    td.TextOutSF(STR(STR_DWN_UPLOADED),num1);
    format_time(num1,t.elapsed);
    td.TextOutSF(STR(STR_DWN_ELAPSED),num1);
    format_time(num1,t.remaining);
    td.TextOutSF(STR(STR_DWN_REMAINING),num1);

    td.nl();
    if(t.status_strid)
        td.TextOutSF(STR(STR_DWN_STATUS),L"%s",STR(t.status_strid));
    if(*t.error)
    {
        td.col=D_C(POPUP_CMP_INVALID_COLOR);
        td.TextOutSF(STR(STR_DWN_ERROR),L"%s",t.error);
        td.col=D_C(POPUP_TEXT_COLOR);
    }
    format_size(num1,t.downloadspeed,1);
    td.TextOutSF(STR(STR_DWN_DOWNLOADSPEED),num1);
    format_size(num1,t.uploadspeed,1);
    td.TextOutSF(STR(STR_DWN_UPLOADSPEED),num1);

    td.nl();
    td.TextOutSF(STR(STR_DWN_SEEDS),STR(STR_DWN_SEEDS_F),t.seedsconnected,t.seedstotal);
    td.TextOutSF(STR(STR_DWN_PEERS),STR(STR_DWN_SEEDS_F),t.peersconnected,t.peerstotal);
    format_size(num1,t.wasted,0);
    format_size(num2,t.wastedhashfailes,0);
    td.TextOutSF(STR(STR_DWN_WASTED),STR(STR_DWN_WASTED_F),num1,num2);

//    TextOutSF(&td,L"Paused",L"%d,%d",t.sessionpaused,t.torrentpaused);
    Popup->popup_resize((int)(td.getMaxsz()+POPUP_SYSINFO_OFS+p0+p1),td.y+D_X(POPUP_OFSY));
}

UpdaterImp::UpdaterImp()
{
    TorrentStatus.sessionpaused=1;
    downloadmangar_exitflag=DOWNLOAD_STATUS_WAITING;

    downloadmangar_event=CreateEventWr(true);
    thandle_download=CreateThread();
    thandle_download->start(&thread_download,nullptr);
}

UpdaterImp::~UpdaterImp()
{
    if(thandle_download)
    {
        downloadmangar_exitflag=DOWNLOAD_STATUS_STOPPING;
        downloadmangar_event->raise();
        thandle_download->join();
        delete thandle_download;
        delete downloadmangar_event;
    }
}

void UpdaterImp::downloadTorrent()
{
    error_code ec;
    int i;
    add_torrent_params params;

    Timers.start(time_chkupdate);
    hSession=new session();

    hSession->start_lsd();
    hSession->start_upnp();
    hSession->start_natpmp();

    // Connecting
    hSession->listen_on(std::make_pair(torrentport,torrentport),ec);
    if(ec)Log.print_err("ERROR: failed to open listen socket: %s\n",ec.message().c_str());
    Log.print_con("Listen port: %d (%s)\nDownload limit: %dKb\nUpload limit: %dKb\n",
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
    if(ec)Log.print_err("ERROR: failed to add torrent: %s\n",ec.message().c_str());

    // Pause and set speed limits
    hSession->pause();
    hTorrent.set_download_limit(downlimit*1024);
    hTorrent.set_upload_limit(uplimit*1024);
    if(connections)hTorrent.set_max_connections(connections);
    hTorrent.resume();

    // Download torrent
    Log.print_con("Waiting for torrent");
    for(i=0;i<200;i++)
    {
        Log.print_con(".");
        if(hTorrent.torrent_file())
        {
            downloadmangar_exitflag=DOWNLOAD_STATUS_TORRENT_GOT;
            Log.print_con("DONE\n");
            break;
        }
        if(downloadmangar_exitflag!=DOWNLOAD_STATUS_DOWLOADING_TORRENT)break;
        Sleep(100);
    }
    if(!hTorrent.torrent_file())
    {
        downloadmangar_exitflag=DOWNLOAD_STATUS_STOPPING;
        Log.print_con("FAILED\n");
        return;
    }

    // Pupulate list
    i=UpdateDialog.populate(0);
    Log.print_con("Latest version: R%d\nUpdated driverpacks available: %d\n",i>>8,i&0xFF);
    for(int j=0;j<numfiles;j++)hTorrent.file_priority(j,0);
    Timers.stop(time_chkupdate);
}

void UpdaterImp::resumeDownloading()
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
        Log.print_con("torrent_resume\n");
        downloadmangar_exitflag=DOWNLOAD_STATUS_DOWLOADING_DATA;
        downloadmangar_event->raise();
    }
    hSession->resume();
    finisheddownloading=0;
    finishedupdating=0;
    torrenttime=GetTickCount();
}

unsigned int __stdcall UpdaterImp::thread_download(void *arg)
{
	UNREFERENCED_PARAMETER(arg);

    // Wait till is allowed to download the torrent
    Log.print_con("{thread_download\n");
    downloadmangar_event->wait();
    if(downloadmangar_exitflag!=DOWNLOAD_STATUS_DOWLOADING_TORRENT)
    {
        Log.print_con("}thread_download(never started)\n");
        return 0;
    }

    // Download torrent
    UpdaterImp *Updater1=dynamic_cast<UpdaterImp *>(Updater);
    Updater1->downloadTorrent();
    if(downloadmangar_exitflag!=DOWNLOAD_STATUS_TORRENT_GOT)
    {
        Log.print_con("}thread_download(failed to download torrent)\n");
        return 0;
    }

    Updater1->updateTorrentStatus();
    downloadmangar_event->reset();

    while(downloadmangar_exitflag!=DOWNLOAD_STATUS_STOPPING)
    {
        // Wait till is allowed to download driverpacks
        if(Settings.flags&FLAG_AUTOUPDATE&&System.canWrite(L"update"))
            UpdateDialog.openDialog();
        else
            downloadmangar_event->wait();

        if(downloadmangar_exitflag==DOWNLOAD_STATUS_STOPPING)break;

        // Downloading loop
        Log.print_con("{torrent_start\n");
        while(downloadmangar_exitflag==DOWNLOAD_STATUS_DOWLOADING_DATA&&hSession)
        {
            Sleep(500);

            // Show progress
            Updater1->updateTorrentStatus();
            MainWindow.ShowProgressInTaskbar(true,TorrentStatus.downloaded,TorrentStatus.downloadsize);
            InvalidateRect(Popup->hPopup,nullptr,0);

            // Send libtorrent messages to log
            std::unique_ptr<alert> holder;
            holder=hSession->pop_alert();
            while(holder.get())
            {
                Log.print_con("Torrent: %s | %s\n",holder.get()->what(),holder.get()->message().c_str());
                holder=hSession->pop_alert();
            }

            if(finisheddownloading)
            {
                Log.print_con("Torrent: finished\n");
                hSession->pause();

                // Flash cache
                Log.print_con("Torrent: flushing cache...");
                hTorrent.flush_cache();
                while(1)
                {
                    alert const* a=hSession->wait_for_alert(seconds(60*2));
                    if(!a)
                    {
                        Log.print_con("time out\n");
                        break;
                    }
                    std::unique_ptr<alert> holder2=hSession->pop_alert();
                    if(alert_cast<cache_flushed_alert>(a))
                    {
                        Log.print_con("done\n");
                        break;
                    }
                }

                // Move files
                Updater1->moveNewFiles();
                hTorrent.force_recheck();

                // Update list
                UpdateDialog.populate(0,true);

                // Execute user cmd
                if(*Settings.finish_upd)
                {
                    WStringShort buf;
                    buf.sprintf(L" /c %s",Settings.finish_upd);
                    System.run_command(L"cmd",buf.Get(),SW_HIDE,0);
                }
                if(Settings.flags&FLAG_AUTOCLOSE)PostMessage(MainWindow.hMain,WM_CLOSE,0,0);

                // Flash in taskbar
                MainWindow.ShowProgressInTaskbar(false);
                FLASHWINFO fi;
                fi.cbSize=sizeof(FLASHWINFO);
                fi.hwnd=MainWindow.hMain;
                fi.dwFlags=FLASHW_ALL|FLASHW_TIMERNOFG;
                fi.uCount=1;
                fi.dwTimeout=0;
                FlashWindowEx(&fi);
                downloadmangar_exitflag=DOWNLOAD_STATUS_FINISHED_DOWNLOADING;
                invalidate(INVALIDATE_INDEXES|INVALIDATE_MANAGER);
            }
        }
        // Download is completed
        finishedupdating=1;
        hSession->pause();
        Updater1->updateTorrentStatus();
        monitor_pause=0;
        Log.print_con("}torrent_stop\n");
        downloadmangar_event->reset();
    }

    if(hSession)
    {
        Log.print_con("Closing torrent session...");
        delete hSession;
        Log.print_con("DONE\n");
        hSession=nullptr;
    }
    Log.print_con("}thread_download\n");
    return 0;
}

void UpdaterImp::pause(){downloadmangar_exitflag=DOWNLOAD_STATUS_PAUSING;}

bool UpdaterImp::isTorrentReady(){return hTorrent.torrent_file()!=nullptr;}
bool UpdaterImp::isPaused(){return TorrentStatus.sessionpaused;}
bool UpdaterImp::isUpdateCompleted(){return finishedupdating;}

int  UpdaterImp::Populate(int flags){return UpdateDialog.populate(flags);}
void UpdaterImp::SetFilePriority(const wchar_t *name,int pri){UpdateDialog.setFilePriority(name,pri);}
void UpdaterImp::OpenDialog(){UpdateDialog.openDialog();}
//}
#else

#include "update.h"

Updater_t *Updater;
int Updater_t::torrentport=50171;
int Updater_t::downlimit=0;
int Updater_t::uplimit=0;
int Updater_t::connections=0;
#endif
