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

// Declarations
class TorrentStatus_t;
class UpdateDialog_t;
class Updater_t;

// Global variables
extern TorrentStatus_t TorrentStatus;
extern UpdateDialog_t UpdateDialog;
extern Updater_t Updater;

// TorrentStatus
class TorrentStatus_t
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

    friend class Updater_t;
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
    static int CALLBACK CompareFunc(LPARAM lParam1,LPARAM lParam2,LPARAM lParamSort);
    void ListView_SetItemTextUpdate(HWND hwnd,int iItem,int iSubItem,const wchar_t *str);
    void calctotalsize();
    void updateTexts();

    void setCheckboxes();
    void setPriorities();
    static LRESULT CALLBACK NewButtonProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
    static BOOL CALLBACK UpdateProcedure(HWND hwnd,UINT Message,WPARAM wParam,LPARAM lParam);

public:
    int  populate(int flags);
    void setPriorities(const wchar_t *name,int pri);
    void openDialog();
    void clearList();
};

// Updater
class Updater_t
{
    static HANDLE downloadmangar_event;
    static HANDLE thandle_download;

    static int downloadmangar_exitflag;
    static int finishedupdating;
    static int finisheddownloading;

    int averageSpeed;
    long long torrenttime;

private:
    void updateTorrentStatus();
    void removeOldDriverpacks(const wchar_t *ptr);
    void moveNewFiles();

public:
    static int torrentport,downlimit,uplimit,connections;

    void checkUpdates();
    void showProgress(wchar_t *buf);
    void showPopup(HDC hdcMem);
    void createThreads();
    void destroyThreads();

    void downloadTorrent();
    void resumeDownloading();
    static unsigned int __stdcall thread_download(void *arg);

    bool isTorrentReady();
    bool isPaused(){return TorrentStatus.sessionpaused;}
    bool isUpdateCompleted(){return finishedupdating;}
};
