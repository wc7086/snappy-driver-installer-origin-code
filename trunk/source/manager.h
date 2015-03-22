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
typedef struct _status_t
{
    int filter,status;
}status_t;


typedef struct _itembar_t
{
    devicematch_t *devicematch;
    hwidmatch_t *hwidmatch;

    WCHAR txt1[1024];
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
}itembar_t;

typedef struct _manager_t
{
    matcher_t *matcher;

    itembar_t *items_list;
    heap_t items_handle;

    long animstart;
}manager_t;

typedef struct _textdata_t
{
    HDC hdcMem;
    int x;
    int y;
    int wy;
    int maxsz;
    int col;
    int i;
    int *limits;
    int mode;
}textdata_t;

//{ Global vars
extern WCHAR extractdir[BUFLEN];
//}

// Manager
void manager_init(manager_t *manager,matcher_t *matcher);
void manager_free(manager_t *manager);
void manager_populate(manager_t *manager);
void manager_filter(manager_t *manager,int options);
void manager_print_tbl(manager_t *manager);
void manager_print_hr(manager_t *manager);
void manager_sorta(matcher_t *m,int *v);
int  manager_drplive(WCHAR *s);

// User interaction
void manager_hitscan(manager_t *manager,int x,int y, int *i,int *zone);
void manager_install(int flags);
void manager_clear(manager_t *manager);
void manager_testitembars(manager_t *manager);
void manager_toggle(manager_t *manager,int index);
void manager_expand(manager_t *manager,int index);
void manager_selectnone(manager_t *manager);
void manager_selectall(manager_t *manager);

// Helpers
int groupsize(manager_t *manager,int index);
void itembar_init(itembar_t *item,devicematch_t *devicematch,hwidmatch_t *match,int groupindex,int rm,int first);
void itembar_settext(manager_t *manager,int i,const WCHAR *txt1,int percent);
void itembar_setpos(itembar_t *itembar,int *pos,int *cnt);
int  itembar_cmp(itembar_t *a,itembar_t *b,CHAR *ta,CHAR *tb);
int  isdrivervalid(hwidmatch_t *hwidmatch);
void str_status(WCHAR *buf,itembar_t *itembar);
int  box_status(int index);
void str_date(version_t *v,WCHAR *buf);
const WCHAR *str_version(version_t *ver);

// Driver list
void manager_setpos(manager_t *manager);
int  manager_animate(manager_t *manager);
void drawbutton(HDC hdc,int x,int pos,int index,const WCHAR *str1,const WCHAR *str2);
int  manager_drawitem(manager_t *manager,HDC hdc,int index,int ofsy,int zone,int cutoff);
int  isbehind(manager_t *manager,int pos,int ofs,int j);
int  calc_cutoff(manager_t *manager);
void manager_draw(manager_t *manager,HDC hdc,int ofsy);
void manager_restorepos(manager_t *manager,manager_t *manager_prev);

// Draw
void TextOut_CM(HDC hdcMem,int x,int y,const WCHAR *str,int color,int *maxsz,int mode);
void TextOutP(textdata_t *td,const WCHAR *format,...);
void TextOutF(textdata_t *td,int col,const WCHAR *format,...);
void TextOutSF(textdata_t *td,const WCHAR *str,const WCHAR *format,...);

// Popup
void format_size(WCHAR *buf,long long val,int isspeed);
void format_time(WCHAR *buf,long long val);
void popup_resize(int x,int y);
void popup_driverline(hwidmatch_t *hwidmatch,int *limits,HDC hdcMem,int ln,int mode,int index);
void popup_driverlist(manager_t *manager,HDC hdcMem,RECT rect,int i);
int  pickcat(hwidmatch_t *hwidmatch,State *state);
int  isvalidcat(hwidmatch_t *hwidmatch,State *state);
void popup_drivercmp(manager_t *manager,HDC hdcMem,RECT rect,int i);
void popup_about(HDC hdcMem);
void popup_sysinfo(manager_t *manager,HDC hdcMem);
void popup_download(HDC hdcMem);
