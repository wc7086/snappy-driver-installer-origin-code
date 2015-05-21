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

// Global vars
extern long long ar_total,ar_proceed;
extern int instflag;
extern int itembar_act;
extern int needreboot;
extern long long totalinstalltime,totalextracttime;

// Installer
#define INSTALLDRIVERS      1
#define OPENFOLDER          2
#define RESTOREPOS          4

int showpercent(int a);
void driver_install(wchar_t *hwid,const wchar_t *inf,int *ret,int *needrb);
void removeextrainfs(wchar_t *inf);
void _7z_total(long long i);
int _7z_setcomplited(long long i);

// Autoclicker
#define NUM_CLICKDATA 5
#define AUTOCLICKER_CONFIRM

struct wnddata_t
{
    // Main wnd
    int wnd_wx,wnd_wy;
    int cln_wx,cln_wy;

    // Install button
    int btn_x, btn_y;
    int btn_wx,btn_wy;
};

void calcwnddata(wnddata_t *w,HWND hwnd);
int cmpclickdata(int *a,int *b);
BOOL CALLBACK EnumWindowsProc(HWND hwnd,LPARAM lParam);
void wndclicker(int mode);
unsigned int __stdcall thread_clicker(void *arg);
