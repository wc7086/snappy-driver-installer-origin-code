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

class Canvas;

// Filters
enum GUI_ID
{
    ID_SHOW_MISSING     =1,
    ID_SHOW_NEWER       =2,
    ID_SHOW_CURRENT     =3,
    ID_SHOW_OLD         =4,
    ID_SHOW_BETTER      =5,
    ID_SHOW_WORSE_RANK  =6,

    ID_SHOW_NF_MISSING  =7,
    ID_SHOW_NF_UNKNOWN  =8,
    ID_SHOW_NF_STANDARD= 9,

    ID_SHOW_ONE        =10,
    ID_SHOW_DUP        =11,
    ID_SHOW_INVALID    =12,

    ID_INSTALL         =13,
    ID_SELECT_ALL      =14,
    ID_SELECT_NONE     =15,
    ID_EXPERT_MODE     =16,

    ID_LANG            =17,
    ID_THEME           =18,

    ID_OPENLOGS        =19,
    ID_SNAPSHOT        =20,
    ID_EXTRACT         =21,
    ID_DRVDIR          =22,

    ID_SCHEDULE        =23,
    ID_SHOWALT         =24,
    ID_OPENINF         =25,
    ID_LOCATEINF       =26,

    ID_EMU_32          =27,
    ID_EMU_64          =28,
    ID_DEVICEMNG       =29,
    ID_DIS_INSTALL     =30,
    ID_DIS_RESTPNT     =31,

    ID_WIN_2000        =32,
    ID_WIN_XP          =33,
    ID_WIN_VISTA       =34,
    ID_WIN_7           =35,
    ID_WIN_8           =36,
    ID_WIN_81          =37,
    ID_WIN_10          =38,

    ID_RESTPNT         =39,
    ID_REBOOT          =40,

    ID_URL0            =41,
    ID_URL1            =42,
    ID_URL2            =43,
    ID_URL3            =44,
    ID_URL4            =45,

    ID_HWID_CLIP      =100,
    ID_HWID_WEB       =200,
};

// fileter_show
enum fileter_show
{
    FILTER_SHOW_MISSING    = (1<<ID_SHOW_MISSING),
    FILTER_SHOW_NEWER      = (1<<ID_SHOW_NEWER),
    FILTER_SHOW_CURRENT    = (1<<ID_SHOW_CURRENT),
    FILTER_SHOW_OLD        = (1<<ID_SHOW_OLD),
    FILTER_SHOW_BETTER     = (1<<ID_SHOW_BETTER),
    FILTER_SHOW_WORSE_RANK = (1<<ID_SHOW_WORSE_RANK),

    FILTER_SHOW_NF_MISSING = (1<<ID_SHOW_NF_MISSING),
    FILTER_SHOW_NF_UNKNOWN = (1<<ID_SHOW_NF_UNKNOWN),
    FILTER_SHOW_NF_STANDARD= (1<<ID_SHOW_NF_STANDARD),

    FILTER_SHOW_ONE        = (1<<ID_SHOW_ONE),
    FILTER_SHOW_DUP        = (1<<ID_SHOW_DUP),
    FILTER_SHOW_INVALID    = (1<<ID_SHOW_INVALID),
};

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
    int val1,val2;
    int percent;

    int isactive;
    int checked;
    int first;
    int index;
    int rm;

    int intend;
    int oldpos,curpos,tagpos,accel;

public:
    itembar_t(Devicematch *devicematch,Hwidmatch *match,int groupindex,int rm,int first);
    itembar_t();
    void itembar_setpos(int *pos,int *cnt);
    void str_status(wchar_t *buf);
    void drawbutton(Canvas &canvas,int x,int pos,const wchar_t *str1,const wchar_t *str2);
    void updatecur();
    int  box_status();

    void contextmenu(int x,int y);
    void popup_drivercmp(Manager *manager,Canvas &canvas,RECT rect,int index);
};
int  itembar_cmp(itembar_t *a,itembar_t *b,Txt *ta,Txt *tb);

// Manager
class Manager
{
    std::vector<itembar_t> items_list;
    long animstart;

public:
    Matcher_interface *matcher;

private:
    int  calc_cutoff();
    int  isbehind(int pos,int ofs,int j);
    int  drawitem(Canvas &canvas,int index,int ofsy,int zone,int cutoff);

public:
    void init(Matcher_interface *matcher);
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
    void popup_driverlist(Canvas &canvas,RECT rect,unsigned i);
    int  manager_drplive(wchar_t *s);
    void set_rstpnt(int checked);
    void itembar_settext(int i,const wchar_t *txt1,int percent);
    void itembar_settext(int i,int act,const wchar_t *txt1=nullptr,int val1v=0,int val2v=1,int percent=-1);
    void itembar_setactive(int i,int val);
    void popup_drivercmp(Manager *manager,Canvas &canvas,RECT rect,int index);
    void contextmenu(int x,int y);
    const wchar_t *getHWIDby(int id);
    static unsigned int __stdcall thread_install(void *arg);

    friend int _7z_setcomplited(long long i); // TODO: friend
};
