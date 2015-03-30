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

#define NUM_PANELS          13
#define PAN_ENT          18

//{ Structs
typedef struct _panelitem_t
{
    int type;
    int str_id;
    int action_id;
    int checked;
}panelitem_t;

class Image
{
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

private:
    HBITMAP bitmap;
    HGDIOBJ oldbitmap;
    HDC ldc;
    int sx,sy,hasalpha;
    int iscopy;

    void readFromFile(WCHAR *filename);
    void readFromRes(int id);
    void createBitmap(BYTE *data,int sz);

public:

    Image():bitmap(0),oldbitmap(0),ldc(0),sx(0),sy(0),hasalpha(0),iscopy(0){}
    void release();
    void makecopy(Image &t);
    void load(int i);
    bool isLoaded(){return ldc;}
    void draw(HDC dc,int x1,int y1,int x2,int y2,int anchor,int fill);
};

class Canvas
{
private:
    int x,y;
    HDC localDC;
    HDC hdcMem;
    HBITMAP bitmap,oldbitmap;
    PAINTSTRUCT ps;
    HWND hwnd;
    HRGN clipping;

public:
    void init();
    void free();
    HDC getDC(){return hdcMem;}
    void begin(HWND hwnd,int x,int y);
    void end();
};

class Panel
{
public:
    panelitem_t *items;

private:
    int index,indofs;

    int Xp();
    int Yp();
    int XP();
    int YP();

public:
    Panel(panelitem_t *itemsA,int indexA):items(itemsA),index(indexA),indofs((indexA+1)*PAN_ENT){}
    bool isChecked(int i){return items[i].checked;}
    void setChecked(int i,int val){items[i].checked=val;}
    void setfilters();
    int  hitscan(int x,int y);
    void draw_inv();
    virtual void draw(HDC hdc);
    void moveWindow(HWND hwnd,int i,int j,int f);
};
//}

//{ Global vars
extern Image box[BOX_NUM];
extern Image icon[ICON_NUM];

extern panelitem_t panel3[];
extern panelitem_t panel3_w[];
extern Panel panels[NUM_PANELS];
//}

int Xm(int x);
int Ym(int y);
int XM(int x,int o);
int YM(int y,int o);

int Xg(int x);
int Yg(int y);
int XG(int x,int o);
int YG(int y,int o);

int  panels_hitscan(int hx,int hy,int *ii);
void panel_setfilters(Panel *panel);

// Draw
void drawrectsel(HDC hdc,int x1,int y1,int x2,int y2,int color2,int w);
void box_draw(HDC hdc,int x1,int y1,int x2,int y2,int i);
void drawcheckbox(HDC hdc,int x,int y,int wx,int wy,int checked,int active);
void drawrect(HDC hdc,int x1,int y1,int x2,int y2,int color1,int color2,int w,int r);
void drawrevision(HDC hdcMem,int y);
void drawpopup(int itembar,int type,int x,int y,HWND hwnd);
