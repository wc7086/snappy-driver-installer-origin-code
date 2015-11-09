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

// Declarations
class Image;
class ClipRegionImp;
class ImageImp;

// Global vars
extern int rtl;

//{ Image
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

    virtual ~Image(){}
    virtual void Load(int strid)=0;
    virtual void MakeCopy(ImageImp &t)=0;
};
//}

//{ ImageStorange
class ImageStorange
{
public:
    virtual ~ImageStorange(){}
    virtual Image *GetImage(int n)=0;
    virtual void LoadAll()=0;
};
ImageStorange *CreateImageStorange(int n,const int *ind,int add_=0);
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
    void SetFont(const wchar_t *name,int size,bool bold=false);
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

private:
    HICON CreateMirroredIcon(HICON hiconOrg);
    void loadGUID(GUID *g,const char *s);

public:
    Canvas();
    ~Canvas();

    void CopyCanvas(Canvas *source,int x1,int y1);
    void begin(HWND hwnd,int x,int y,bool mirror=true);
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

// Popup
//void popup_resize(int x,int y);
void popup_about(Canvas &canvas);
void format_size(wchar_t *buf,long long val,int isspeed);
void format_time(wchar_t *buf,long long val);

//{ Combobox
class Combobox
{
    HWND handle;

public:
    Combobox(HWND hwnd,int id);
    void Clear();
    void AddItem(const wchar_t *str);
    int FindItem(const wchar_t *str);
    int GetNumItems();
    void SetCurSel(int i);
    void Focus();
    void SetFont(Font *font);
    void Move(int x1,int y1,int wx,int wy);
    void SetMirroring();
};
//}

#endif
