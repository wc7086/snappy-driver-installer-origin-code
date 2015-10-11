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

#ifndef MANAGER_H
#define MANAGER_H

class Canvas;
class Txt;
class Devicematch;
class Hwidmatch;
class Matcher;

#include <vector>

// EXPAND_MODE
enum class EXPAND_MODE
{
    TOGGLE,
    EXPAND,
    COLLAPSE,
};

// Itembar slots
enum SLOTS
{
    SLOT_EMPTY         = 0,
    SLOT_VIRUS_AUTORUN = 1,
    SLOT_VIRUS_RECYCLER= 2,
    SLOT_VIRUS_HIDDEN  = 3,
    SLOT_NODRIVERS     = 4,
    SLOT_DOWNLOAD      = 5,
    SLOT_NOUPDATES     = 6,
    SLOT_DPRDIR        = 7,
    SLOT_SNAPSHOT      = 8,
    SLOT_INDEXING      = 9,
    SLOT_EXTRACTING    =10,
    SLOT_RESTORE_POINT =11,
    RES_SLOTS          =12,
};

#define NUM_STATUS 6
class Manager;
struct status_t
{
    int filter,status;
};

// itembar_t
class itembar_t
{
public:
    Devicematch *devicematch;
    Hwidmatch *hwidmatch;

    wchar_t txt1[1024];
    int install_status;
    __int64 val1,val2;
	__int64 percent;

    int isactive;
    int checked;
    int first;
    int index;
    int rm;

    int intend;
    int oldpos,curpos,tagpos,accel;

public:
    itembar_t(Devicematch *devicematch,Hwidmatch *match,int groupindex,int rm,int first);
    itembar_t(){}
    void itembar_setpos(int *pos,int *cnt);
    void str_status(wchar_t *buf);
    void drawbutton(Canvas &canvas,int x,int pos,const wchar_t *str1,const wchar_t *str2);
    void updatecur();
    int  box_status();

    void contextmenu(int x,int y);
    void popup_drivercmp(Manager *manager,Canvas &canvas,int wx,int wy,int index);
};
int  itembar_cmp(itembar_t *a,itembar_t *b,Txt *ta,Txt *tb);

// Manager
class Manager
{
    std::vector<itembar_t> items_list;
    long animstart;

public:
    Matcher *matcher;

private:
    int  calc_cutoff();
    int  isbehind(int pos,int ofs,int j);
    int  drawitem(Canvas &canvas,int index,int ofsy,int zone,int cutoff);

public:
    void init(Matcher *matcher);
    void populate();
    void filter(int options);
    void print_tbl();
    void print_hr();

// User interaction
    void hitscan(int x,int y, int *i,int *zone);
    void clear();
    void updateoverall();
    void install(int flags);
    void testitembars();
    void toggle(int index);
    void expand(int index,EXPAND_MODE f);
    void selectnone();
    void selectall();

// Driver list
    int countItems();
    int groupsize(int index);
    void setpos();
    int  animate();

    void getINFpath(int wp);
    void draw(Canvas &canvas,int ofsy);
    void restorepos1(Manager *manager_prev);
    void restorepos(Manager *manager_prev);
    void popup_driverlist(Canvas &canvas,int wx,int wy,unsigned i);
    int  manager_drplive(wchar_t *s);
    void set_rstpnt(int checked);
    void itembar_settext(int i,const wchar_t *txt1,int percent);
    void itembar_settext(int i,int act,const wchar_t *txt1=nullptr,__int64 val1v=0,__int64 val2v=1,__int64 percent=-1);
    void itembar_setactive(int i,int val);
    void popup_drivercmp(Manager *manager,Canvas &canvas,int wx,int wy,int index);
    void contextmenu(int x,int y);
    const wchar_t *getHWIDby(int id);
    static unsigned int __stdcall thread_install(void *arg);

    friend int _7z_setcomplited(long long i); // TODO: friend
};

#endif
