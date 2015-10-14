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

#include "themelist.h"

// Declarations
class Image;
class ClipRegionImp;

// Global vars
extern int rtl;

//{ Image
class Image
{
    HBITMAP bitmap=nullptr;
    HGDIOBJ oldbitmap=nullptr;
    HDC ldc=nullptr;
    int sx=0,sy=0,hasalpha=0;
    int iscopy=0;

private:
    void LoadFromFile(wchar_t *filename);
    void LoadFromRes(int id);
    void CreateMyBitmap(BYTE *data,size_t sz);
    void Draw(HDC dc,int x1,int y1,int x2,int y2,int anchor,int fill);
    void Release();
    bool IsLoaded()const;
    friend class Canvas;

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

    ~Image(){Release();}
    void Load(int strid);
    void MakeCopy(Image &t);
};
//}

//{ ClipRegion
class ClipRegion
{
    ClipRegion(const ClipRegion&)=delete;
    void operator=(const ClipRegion&)=delete;

private:
    ClipRegionImp *imp;
    friend class Canvas;

public:
    ClipRegion();
    ClipRegion(int x1,int y1,int x2,int y2);
    ClipRegion(int x1,int y1,int x2,int y2,int r);
    void setRegion(int x1,int y1,int x2,int y2);
    ~ClipRegion();
};
//}


//{ Font
class Font
{
    Font(const Font&)=delete;
    void operator=(const Font&)=delete;

private:
    HFONT hFont;
    friend class Canvas;
    friend class Combobox;

public:
    Font():hFont(nullptr){}
    ~Font();
    void SetFont(const wchar_t *name,int size,int bold=false);
    HFONT get(){return hFont;}
};
//}

//{ Canvas
class Canvas
{
    Canvas(const Canvas &)=delete;
    Canvas &operator=(const Canvas &)=delete;

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

    void CopyCanvas(Canvas *source,int x1,int y1);
    void begin(HWND hwnd,int x,int y);
    void end();

    void SetClipRegion(ClipRegion &clip);
    void ClearClipRegion();

    void DrawEmptyRect(int x1,int y1,int x2,int y2,int color);
    void DrawFilledRect(int x1,int y1,int x2,int y2,int color1,int color2,int w,int r);
    void DrawWidget(int x1,int y1,int x2,int y2,int i);
    void DrawImage(Image &image,int x1,int y1,int wx,int wy,int flags1,int flags2);
    void DrawLine(int x1,int y1,int x2,int y2);
    void DrawCheckbox(int x,int y,int wx,int wy,int checked,int active);
    void DrawConnection(int x,int pos,int ofsy,int curpos);
    void DrawIcon(int x1,int y1,const char *guid_driverpack,const GUID *guid_device);

    void SetTextColor(int color);
    void SetFont(Font *font);
    void DrawTextXY(int x,int y,const wchar_t *buf);
    void DrawTextRect(const wchar_t *bufw,RECT *rect);
    void CalcBoundingBox(const wchar_t *str,RECT *rect);
    int  GetTextExtent(const wchar_t *str);
};
//}

//{ ### Text ###
class textdata_t
{
protected:
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
//}

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

void drawpopup(int itembar,int type,int x,int y,HWND hwnd);
HICON CreateMirroredIcon(HICON hiconOrg);
void ShowProgressInTaskbar(HWND hwnd,bool show,long long complited=0,long long total=0);
void loadGUID(GUID *g,const char *s);
void drawnew(Canvas &canvas);
bool isRebootDesired();

//{ Combobox
class Combobox
{
    HWND handle;

public:
    Combobox(HWND hwnd,int id);
    HWND gethandle(){return handle;}
    void Clear()
    {
        SendMessage(handle,CB_RESETCONTENT,0,0);
    }
    void AddItem(const wchar_t *str)
    {
        SendMessage(handle,CB_ADDSTRING,0,(LPARAM)str);
    }
    int FindItem(const wchar_t *str)
    {
        return SendMessage(handle,CB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)str);
    }
    int GetNumItems()
    {
        return SendMessage(handle,CB_GETCOUNT,0,0);
    }
    void SetCurSel(int i)
    {
        SendMessage(handle,CB_SETCURSEL,i,0);
    }

    void Focus()
    {
        SetFocus(handle);
    }
    void SetFont(Font *font)
    {
        SendMessage(handle,WM_SETFONT,(WPARAM)font->hFont,MAKELPARAM(FALSE,0));
    }
    void Move(int x1,int y1,int wx,int wy)
    {
        MoveWindow(handle,x1,y1,wx,wy,false);
    }
    void SetMirroring();
};
//}

#endif
