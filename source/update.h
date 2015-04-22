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

struct torrent_status_t
{
    long long downloaded,downloadsize;
    long long uploaded;
    int elapsed,remaining;

    const wchar_t *status;
    wchar_t error[BUFLEN];
    int uploadspeed,downloadspeed;
    int seedstotal,seedsconnected;
    int peerstotal,peersconnected;
    int wasted,wastedhashfailes;

    int sessionpaused,torrentpaused;
};

extern volatile int downloadmangar_exitflag;
extern int torrentport;
extern int downlimit,uplimit;

// Dialog
class UpdateDialog_t
{
private:
    int  getnewver(const char *ptr);
    int  getcurver(const char *ptr);

    void ListView_SetItemTextUpdate(HWND hwnd,int iItem,int iSubItem,wchar_t *str);
    static LRESULT CALLBACK NewButtonProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
    static BOOL CALLBACK UpdateProcedure(HWND hwnd,UINT Message,WPARAM wParam,LPARAM lParam);
    static int CALLBACK CompareFunc(LPARAM lParam1,LPARAM lParam2,LPARAM lParamSort);

public:
    void upddlg_updatelang();
    void upddlg_setcheckboxes(HWND hList);
    void upddlg_setpriorities(HWND hList);
    void upddlg_setpriorities_driverpack(const wchar_t *name,int pri);
    void upddlg_calctotalsize(HWND hList);
    int  upddlg_populatelist(HWND hList,int flags);

    void open_dialog();
};
extern UpdateDialog_t UpdateDialog;

// Update
class Updater_t
{
private:
    static HANDLE downloadmangar_event;
    static HANDLE thandle_download;

static int finisheddownloading;
static int finishedupdating;
int averageSpeed;

private:
    void update_movefiles();

public:
    static torrent_status_t torrentstatus;

    int istorrentready();
    void update_start();
    void update_stop();
    void update_resume();
    void update_getstatus(torrent_status_t *t);
    void delolddrp(const char *ptr);
    static unsigned int __stdcall thread_download(void *arg);
    void checkupdates();
    void shutdown();
    void start_torrent();
    bool isPaused(){return torrentstatus.sessionpaused;}
    bool isReady(){return finishedupdating;}
};
extern Updater_t Updater;

