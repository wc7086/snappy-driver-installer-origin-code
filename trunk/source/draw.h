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

typedef struct _panel_t
{
    panelitem_t *items;
    int index;
}panel_t;

//}

//{ Global vars
extern Image box[BOX_NUM];
extern Image icon[ICON_NUM];

extern panelitem_t panel3[];
extern panelitem_t panel3_w[];
extern panel_t panels[NUM_PANELS];
//}


int Xp(panel_t *p);
int Yp(panel_t *p);
int XP(panel_t *p);
int YP(panel_t *p);

int Xm(int x);
int Ym(int y);
int XM(int x,int o);
int YM(int y,int o);

int Xg(int x);
int Yg(int y);
int XG(int x,int o);
int YG(int y,int o);

// Draw
void drawrectsel(HDC hdc,int x1,int y1,int x2,int y2,int color2,int w);
void box_draw(HDC hdc,int x1,int y1,int x2,int y2,int i);
void drawcheckbox(HDC hdc,int x,int y,int wx,int wy,int checked,int active);
void drawrect(HDC hdc,int x1,int y1,int x2,int y2,int color1,int color2,int w,int r);
void drawrevision(HDC hdcMem,int y);
void drawpopup(int itembar,int type,int x,int y,HWND hwnd);

// Panel
void panel_setfilters(panel_t *panel);
int  panels_hitscan(int hx,int hy,int *ii);
int  panel_hitscan(panel_t *panel,int x,int y);
void panel_draw_inv(panel_t *panel);
void panel_draw(HDC hdc,panel_t *panel);

