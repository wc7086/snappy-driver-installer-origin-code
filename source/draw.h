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

#ifndef DRAW_H
#define DRAW_H

#define NUM_PANELS  13
#define PAN_ENT     18

#include "themelist.h"

// Declarations
class Image;
class Panel;
class Panelitem;
class ClipRegionImp;

// Global vars
extern int rtl;
extern Image box[BOX_NUM];
extern Image icon[ICON_NUM];

extern Panelitem panel3[];
extern Panelitem panel3_w[];
extern Panel panels[NUM_PANELS];

// Image
class Image
{
    HBITMAP bitmap;
    HGDIOBJ oldbitmap;
    HDC ldc;
    int sx,sy,hasalpha;
    int iscopy;

private:
    void loadFromFile(wchar_t *filename);
    void loadFromRes(int id);
    void createBitmap(BYTE *data,int sz);

public:
    enum align
    {
        RIGHT   = 1,
        BOTTOM  = 2,
        HCENTER = 4,
        VCENTER = 8,
    };
    enum fillmode
    {
        HTILE   =  1,
        VTILE   =  2,
        HSTR    =  4,
        VSTR    =  8,
        ASPECT  = 16,
    };

    Image():bitmap(nullptr),oldbitmap(nullptr),ldc(nullptr),sx(0),sy(0),hasalpha(0),iscopy(0){}
    void release();
    void makecopy(Image &t);
    void load(int i);
    bool isLoaded()const{return ldc;}
    void draw(HDC dc,int x1,int y1,int x2,int y2,int anchor,int fill);
};

// ClipRegion
class ClipRegion
{
    ClipRegion(const ClipRegion&)=delete;
    void operator=(const ClipRegion&)=delete;

private:
    ClipRegionImp *imp;

public:
    ClipRegion();
    ClipRegion(int x1,int y1,int x2,int y2);
    ClipRegion(int x1,int y1,int x2,int y2,int r);
    void setRegion(int x1,int y1,int x2,int y2);
    ~ClipRegion();

    friend class Canvas;
};

// Font
class Font
{
    Font(const Font&)=delete;
    void operator=(const Font&)=delete;

private:
    HFONT hFont;
public:
    Font():hFont(nullptr){}
    ~Font();
    void SetFont(const wchar_t *name,int size,int bold=false);
    HFONT get(){return hFont;}

    friend class Canvas;
};

// Canvas
class Canvas
{
    int x,y;
    HDC localDC;
    HDC hdcMem;
    HBITMAP bitmap,oldbitmap;
    PAINTSTRUCT ps;
    HWND hwnd;
    HRGN clipping;

public:
    Canvas();
    ~Canvas();
    Canvas(const Canvas &)=delete;
    Canvas &operator=(const Canvas &)=delete;

    void drawrect(int x1,int y1,int x2,int y2,int color1,int color2,int w,int r);
    void drawbox(int x1,int y1,int x2,int y2,int i);
    void drawcheckbox(int x,int y,int wx,int wy,int checked,int active);
    void drawrevision(int y);
    void drawconnection(int x,int pos,int ofsy,int curpos);
    void TextOutH(int x,int y,LPCTSTR buf);
    void drawIcon(int x1,int y1,const char *guid_driverpack,const GUID *guid_device);
    void drawTextRect(LPCTSTR bufw,RECT *rect){DrawText(hdcMem,bufw,-1,rect,DT_WORDBREAK);}
    void drawLine(int x1,int y1,int x2,int y2);
    void drawRect(int x1,int y1,int x2,int y2,int color);
    void drawImage(Image &image,int x1,int y1,int wx,int wy,int flags1,int flags2){image.draw(hdcMem,x1,y1,wx,wy,flags1,flags2);}

    HDC getDC(){return hdcMem;}
    void setTextColor(int color);
    void setFont(Font *font);

    void setClipRegion(ClipRegion &clip);
    void clearClipRegion();
    void calcRect(const wchar_t *str,RECT *rect);

    void begin(HWND hwnd,int x,int y);
    void end();
};

// Panel
enum panel_type
{
    TYPE_GROUP         = 1,
    TYPE_TEXT          = 2,
    TYPE_CHECKBOX      = 3,
    TYPE_BUTTON        = 4,
    TYPE_GROUP_BREAK   = 5,
};

struct Panelitem
{
    int type;
    int str_id;
    int action_id;
    int checked;
};

class Widget
{
protected:
    int x1,y1,wx,wy;
    bool isSelected=false;
    int str_id;

public:
    Widget *parent;
    int flags;
    bool invalidated=true;

    Widget(int str_id_):str_id(str_id_){}
    virtual ~Widget(){}
    //virtual void click(int x,int y);
    virtual void draw(Canvas &){}
    virtual void arrange(){}
    virtual void hover(int x,int y);
    virtual void setboundbox(int v1,int v2,int v3,int v4){x1=v1;y1=v2;wx=v3;wy=v4;}
};

class WidgetComposite:public Widget
{
protected:
    int num=0;
    Widget *widgets[20];

public:
    virtual void Add(Widget *w)
    {
        widgets[num]=w;
        widgets[num]->parent=this;
        num++;
    }
    WidgetComposite():Widget(0){}
    ~WidgetComposite()
    {
        for(int i=0;i<num;i++)delete widgets[i];
    }
    void draw(Canvas &canvas)
    {
        for(int i=0;i<num;i++)widgets[i]->draw(canvas);
    }
    void hover(int x,int y)
    {
        Widget::hover(x,y);
        for(int i=0;i<num;i++)widgets[i]->hover(x,y);
    }
    void arrange()
    {
        for(int i=0;i<num;i++)widgets[i]->arrange();
    }
};

class wCanvas:public WidgetComposite
{
};

class wPanel:public WidgetComposite
{
    int sz,indofs,index;

public:
    wPanel(int sz_,int ofs):sz(sz_),indofs((ofs+1)*PAN_ENT),index(ofs){}
    void arrange();
    void draw(Canvas &canvas);
};

class wText:public Widget
{
public:
    void hover(int x,int y){Widget::hover(x,y);isSelected=0;}
    void draw(Canvas &canvas);
    wText(int str_id_):Widget(str_id_){}
};

class wLogo:public wPanel
{
public:
    void hover(int x,int y);
    wLogo(int sz_,int ofs):wPanel{sz_,ofs}{}
};

class wTextRev:public wText
{
public:
    void hover(int x,int y);
    void draw(Canvas &canvas);
    wTextRev():wText(0){}
};

class wTextSys1:public wText
{
public:
    void draw(Canvas &canvas);
    void hover(int x,int y);
    wTextSys1():wText(0){}
};

class wTextSys2:public wTextSys1
{
public:
    void draw(Canvas &canvas);
};

class wTextSys3:public wTextSys1
{
public:
    void draw(Canvas &canvas);
};

class wCheckbox:public Widget
{
    int action_id;

public:
    void draw(Canvas &canvas);
    wCheckbox(int str_id_,int action_id_):Widget(str_id_),action_id(action_id_){}
};

class wButton:public Widget
{
protected:
    int action_id;

public:
    void draw(Canvas &canvas);
    wButton(int str_id_,int action_id_):Widget(str_id_),action_id(action_id_){}
};

class wButtonInst:public wButton
{
public:
    void draw(Canvas &canvas);
    wButtonInst(int str_id_,int action_id_):wButton(str_id_,action_id_){}
};
extern WidgetComposite *wPanels;

class Panel
{
    Panelitem *items;
    int index,indofs;

    int Xp();
    int Yp();
    int XP();
    int YP();

public:
    Panel(Panelitem *itemsA,int indexA):items(itemsA),index(indexA),indofs((indexA+1)*PAN_ENT){}
    bool isChecked(int i)const{return items[i].checked;}
    void setChecked(int i,int val){items[i].checked=val;}
    void flipChecked(int i){items[i].checked^=1;}
    int getStr(int i){return items[i].str_id;}

    int calcFilters();
    void setFilters(int filters);
    int  hitscan(int x,int y);
    void keybAdvance(int v);
    void draw_inv();
    void draw(Canvas &canvas);
    void moveWindow(HWND hwnd,int i,int j,int f);
    void click(int i);

    friend class MainWindow_t;
};

// Combobox
class Combobox
{
    HWND handle;

public:
    /*Combobox(HWND hwnd,int id)
    {
        handle=CreateWindowMF(WC_COMBOBOX,L"",hwnd,(HMENU)id,CBS_DROPDOWNLIST|CBS_HASSTRINGS|WS_OVERLAPPED|WS_VSCROLL);
    }*/
    void reset()
    {
        SendMessage(handle,CB_RESETCONTENT,0,0);
    }
    int finditem(const wchar_t *str)
    {
        return SendMessage(handle,CB_FINDSTRINGEXACT,-1,(LPARAM)str);
    }

};

// Text
class textdata_t
{
protected:
    HDC hdcMem;
    Canvas *pcanvas;

    int ofsx;
    int wy;
    int maxsz;

public:
    int col;
    int x;
    int y;

protected:
    void TextOut_CM(int x,int y,const wchar_t *str,int color,int *maxsz,int mode);

public:
    textdata_t(Canvas &canvas,int ofsx=0);

    void TextOutF(int col,const wchar_t *format,...);
    void TextOutF(const wchar_t *format,...);
    void ret();
    void nl();
    void ret_ofs(int a);
    int getX(){return x;}
    int getY(){return y;}
};

class textdata_horiz_t:public textdata_t
{
    int i;
    int *limits;
    int mode;

public:
    textdata_horiz_t(Canvas &canvas,int ofsx1,int *lim,int mode1):textdata_t(canvas,ofsx1),i(0),limits(lim),mode(mode1){}
    void limitskip();
    void TextOutP(const wchar_t *format,...);
};

class textdata_vert:public textdata_t
{

public:
    textdata_vert(Canvas &canvas,int ofsx1=0):textdata_t(canvas,ofsx1)
    {

    }
    void shift_r();
    void shift_l();
    int getMaxsz(){return maxsz;}
    void TextOutSF(const wchar_t *str,const wchar_t *format,...);
};

// Popup
void popup_resize(int x,int y);
void popup_about(Canvas &canvas);
void format_size(wchar_t *buf,long long val,int isspeed);
void format_time(wchar_t *buf,long long val);

// Misc functions
int mirw(int x,int ofs,int w);

int Xm(int x,int o);
int Ym(int y);
int XM(int x,int o);
int YM(int y,int o);

int Xg(int x,int o);
int Yg(int y);
int XG(int x,int o);
int YG(int y,int o);

int  panels_hitscan(int hx,int hy,int *ii);
void panel_loadsettings(Panel *panel,int filters);

void drawpopup(int itembar,int type,int x,int y,HWND hwnd);
HICON CreateMirroredIcon(HICON hiconOrg);
void ShowProgressInTaskbar(HWND hwnd,bool show,long long complited=0,long long total=0);
void loadGUID(GUID *g,const char *s);
void drawnew(Canvas &canvas);

#endif
