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

#include "main.h"

//{ Global vars
Image box[BOX_NUM];
Image icon[ICON_NUM];

panelitem_t panel1[]=
{
    {TYPE_GROUP,0,3,0},
    {TYPE_TEXT,STR_SHOW_SYSINFO,0,0},
    {TYPE_TEXT,0,0,0},
    {TYPE_TEXT,0,0,0},
};

panelitem_t panel2[]=
{
    {TYPE_GROUP,0,3,0},
    {0,STR_INSTALL,               ID_INSTALL,0},
    {0,STR_SELECT_ALL,            ID_SELECT_ALL,0},
    {0,STR_SELECT_NONE,           ID_SELECT_NONE,0},
};

panelitem_t panel3[]=
{
    {TYPE_GROUP,KB_EXPERT,5,0},
    {TYPE_TEXT,STR_LANG,0,0},
    {TYPE_TEXT,0,0,0},
    {TYPE_TEXT,STR_THEME,0,0},
    {TYPE_TEXT,0,0,0},
    {TYPE_CHECKBOX,STR_EXPERT,              ID_EXPERT_MODE,0},
};

panelitem_t panel3_w[]=
{
    {TYPE_GROUP,0,3,0},
    {TYPE_TEXT,STR_LANG,0,0},
    {TYPE_TEXT,STR_THEME,0,0},
    {TYPE_CHECKBOX,STR_EXPERT,              ID_EXPERT_MODE,0},
};

panelitem_t panel4[]=
{
    {TYPE_GROUP_BREAK,KB_ACTIONS,4,0},
    {TYPE_BUTTON,STR_OPENLOGS,              ID_OPENLOGS,0},
    {TYPE_BUTTON,STR_SNAPSHOT,              ID_SNAPSHOT,0},
    {TYPE_BUTTON,STR_EXTRACT,               ID_EXTRACT,0},
    {TYPE_BUTTON,STR_DRVDIR,                ID_DRVDIR,0},
};

panelitem_t panel5[]=
{
    {TYPE_GROUP_BREAK,KB_PANEL1,7,0},
    {TYPE_TEXT,STR_SHOW_FOUND,0,0},
    {TYPE_CHECKBOX, STR_SHOW_MISSING,       ID_SHOW_MISSING,0},
    {TYPE_CHECKBOX, STR_SHOW_NEWER,         ID_SHOW_NEWER,0},
    {TYPE_CHECKBOX, STR_SHOW_CURRENT,       ID_SHOW_CURRENT,0},
    {TYPE_CHECKBOX, STR_SHOW_OLD,           ID_SHOW_OLD,0},
    {TYPE_CHECKBOX, STR_SHOW_BETTER,        ID_SHOW_BETTER,0},
    {TYPE_CHECKBOX, STR_SHOW_WORSE_RANK,    ID_SHOW_WORSE_RANK,0},
};

panelitem_t panel6[]=
{
    {TYPE_GROUP_BREAK,KB_PANEL2,4,0},
    {TYPE_TEXT,STR_SHOW_NOTFOUND,0,0},
    {TYPE_CHECKBOX, STR_SHOW_NF_MISSING,    ID_SHOW_NF_MISSING,0},
    {TYPE_CHECKBOX, STR_SHOW_NF_UNKNOWN,    ID_SHOW_NF_UNKNOWN,0},
    {TYPE_CHECKBOX, STR_SHOW_NF_STANDARD,   ID_SHOW_NF_STANDARD,0},
};

panelitem_t panel7[]=
{
    {TYPE_GROUP_BREAK,KB_PANEL3,3,0},
    {TYPE_CHECKBOX, STR_SHOW_ONE,           ID_SHOW_ONE,0},
    {TYPE_CHECKBOX, STR_SHOW_DUP,           ID_SHOW_DUP,0},
    {TYPE_CHECKBOX, STR_SHOW_INVALID,       ID_SHOW_INVALID,0},

};

panelitem_t panel8[]=
{
    {TYPE_GROUP,0,1,0},
    {TYPE_TEXT,0,0,0},
};

panelitem_t panel9[]=
{

    {TYPE_GROUP,KB_INSTALL,1,0},
    {TYPE_BUTTON,STR_INSTALL,               ID_INSTALL,0},
};

panelitem_t panel10[]=
{
    {TYPE_GROUP,KB_INSTALL,1,0},
    {TYPE_BUTTON,STR_SELECT_ALL,            ID_SELECT_ALL,0},
};

panelitem_t panel11[]=
{
    {TYPE_GROUP,KB_INSTALL,1,0},
    {TYPE_BUTTON,STR_SELECT_NONE,           ID_SELECT_NONE,0},
};

panelitem_t panel12[]=
{
    {TYPE_GROUP,KB_PANEL_CHK,3,0},
    {TYPE_TEXT,STR_OPTIONS,0,0},
    {TYPE_CHECKBOX,STR_RESTOREPOINT,        ID_RESTPNT,0},
    {TYPE_CHECKBOX,STR_REBOOT,              ID_REBOOT,0},
};

panelitem_t panel13[]=
{
    {TYPE_GROUP,0,1,0},
    {TYPE_TEXT,0,0,0},
};

Panel panels[NUM_PANELS]=
{
    {panel1,  0},
    {panel2,  1},
    {panel3,  2},
    {panel4,  3},
    {panel5,  4},
    {panel6,  5},
    {panel7,  6},
    {panel8,  7},
    {panel9,  8},
    {panel10, 9},
    {panel11,10},
    {panel12,11},
    {panel13,12},
};
//}

//{ Image
void Image::makecopy(Image &t)
{
    release();
    bitmap=t.bitmap;
    ldc=t.ldc;
    sx=t.sx;
    sy=t.sy;
    hasalpha=t.hasalpha;
    iscopy=1;
}

void Image::load(int i)
{
    WCHAR *filename=(WCHAR *)D(i);

    release();

    if(wcsstr(filename,L"RES_"))
        readFromRes(_wtoi_my(filename+4));
    else
        readFromFile(filename);
}

void Image::release()
{
    int r;

    if(bitmap&&!iscopy)
    {
        SelectObject(ldc,oldbitmap);
        r=DeleteDC(ldc);
            if(!r)log_err("ERROR in box_init(): failed DeleteDC\n");
        r=DeleteObject(bitmap);
            if(!r)log_err("ERROR in box_init(): failed DeleteObject\n");
    }
    bitmap=0;
    ldc=0;
    iscopy=0;
}

void Image::readFromFile(WCHAR *filename)
{
    WCHAR buf[BUFLEN];
    FILE *f;
    int sz;
    BYTE *imgbuf;

    if(!filename||!*filename)return;
    wsprintf(buf,L"%s\\themes\\%s",data_dir,filename);
    f=_wfopen(buf,L"rb");
    if(!f)
    {
        log_err("ERROR in image_loadFile(): file '%S' not found\n",buf);
        return;
    }
    fseek(f,0,SEEK_END);
    sz=ftell(f);
    fseek(f,0,SEEK_SET);
    imgbuf=new BYTE[sz];

    sz=fread(imgbuf,1,sz,f);
    if(!sz)
    {
        log_err("ERROR in image_loadFile(): cannnot read from file '%S'\n",buf);
        delete[] imgbuf;
        return;
    }
    fclose(f);
    createBitmap(imgbuf,sz);
    delete[] imgbuf;
}

void Image::readFromRes(int id)
{
    int sz;
    HGLOBAL myResourceData;

    get_resource(id,&myResourceData,&sz);
    if(!sz)
    {
        log_err("ERROR in image_loadRes(): failed get_resource\n");
        return;
    }
    createBitmap((BYTE *)myResourceData,sz);
}

void Image::createBitmap(BYTE *data,int sz)
{
    BITMAPINFO bmi;
    BYTE *bits;
    BYTE *big;
    BYTE *p1,*p2;
    int ret;
    int i;

    hasalpha=sx=sy=0;
    ldc=0;
#ifdef CONSOLE_MODE
    return;
#else
    ret=WebPGetInfo((PBYTE)data,sz,&sx,&sy);
    if(!ret)
    {
        log_err("ERROR in image_load(): failed WebPGetInfo\n");
        return;
    }
    big=WebPDecodeBGRA((PBYTE)data,sz,&sx,&sy);
    if(!big)
    {
        log_err("ERROR in image_load(): failed WebPDecodeBGRA\n");
        return;
    }
#endif
    ZeroMemory(&bmi,sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth=sx;
    bmi.bmiHeader.biHeight=-sy;
    bmi.bmiHeader.biPlanes=1;
    bmi.bmiHeader.biBitCount=32;
    bmi.bmiHeader.biCompression=BI_RGB;
    bmi.bmiHeader.biSizeImage=sx*sy*4;

    ldc=CreateCompatibleDC(0);
    bitmap=CreateDIBSection(ldc,&bmi,DIB_RGB_COLORS,(void **)&bits,0,0);

    p1=bits;p2=big;
    for(i=0;i<sx*sy;i++)
    {
        BYTE B,G,R,A;
        B=*p2++;
        G=*p2++;
        R=*p2++;
        A=*p2++;
        double dA=A/255.;
        if(A!=255)hasalpha=1;

        *p1++=(BYTE)(B*dA);
        *p1++=(BYTE)(G*dA);
        *p1++=(BYTE)(R*dA);
        *p1++=A;
    }
    SelectObject(ldc,bitmap);
    free(big);
//    log_con("%dx%d:%d,%d\n",sx,sy,hasalpha,index);
}

void Image::draw(HDC dc,int x1,int y1,int x2,int y2,int anchor,int fill)
{
    BLENDFUNCTION blend={AC_SRC_OVER,0,255,AC_SRC_ALPHA};
    int xi,yi,wx,wy,wx1,wy1,wx2,wy2;

    if(!sx)return;

    wx=(fill&HSTR)?x2-x1:sx;
    wy=(fill&VSTR)?y2-y1:sy;
    if(fill&ASPECT)
    {
        if(fill&HSTR)wy=sy*((double)wx/sx);
        if(fill&VSTR)wx=sx*((double)wy/sy);
    }

    for(xi=0;xi<x2;xi+=wx)
    {
        for(yi=0;yi<y2;yi+=wy)
        {
            int x=x1+xi,y=y1+yi;
            if(anchor&RIGHT)  x=x2-xi-wx;
            if(anchor&BOTTOM) y=y2-yi-wy;
            if(anchor&HCENTER)x=(x2-x1-wx)/2;
            if(anchor&VCENTER)y=(y2-y1-wy)/2;

            wx1=(x+wx>x2)?x2-x:wx;
            wy1=(y+wy>y2)?y2-y:wy;
            wx2=(x+wx>x2)?wx1:sx;
            wy2=(y+wy>y2)?wy1:sy;

            if(hasalpha)
                AlphaBlend(dc,x,y,wx1,wy1,ldc,0,0,wx2,wy2,blend);
            else if(wx==wx2&&wy==wy2)
                BitBlt(dc,x,y,wx1,wy1,ldc,0,0,SRCCOPY);
            else
                StretchBlt(dc,x,y,wx1,wy1,ldc,0,0,wx2,wy2,SRCCOPY);

            if((fill&VTILE)==0)break;
        }
        if((fill&HTILE)==0)break;
    }
    //drawrect(dc,x1,y1,x2,y2,0xFF000000,0xFF00,1,0);
}
//}

//{ Draw
int Xm(int x){return x>=0?x:(main1x_c+x);}
int Ym(int y){return y>=0?y:(main1y_c+y);}
int XM(int x,int o){return x>=0?x:(main1x_c+x-o);}
int YM(int y,int o){return y>=0?y:(main1y_c+y-o);}

int Xg(int x){return x>=0?x:(mainx_c+x);}
int Yg(int y){return y>=0?y:(mainy_c+y);}
int XG(int x,int o){return x>=0?x:(mainx_c+x-o);}
int YG(int y,int o){return y>=0?y:(mainy_c+y-o);}

int panels_hitscan(int hx,int hy,int *ii)
{
    int i,r=-1;

    *ii=-1;
    for(i=0;i<NUM_PANELS;i++)
    {
        r=panels[i].hitscan(hx,hy);
        if(r>=0)
        {
            *ii=i;
            return r;
        }
    }
    return -1;
}

void panel_setfilters(Panel *panel)
{
    for(int j=0;j<7;j++)panel[j].setfilters();
}

void drawrect(HDC hdc,int x1,int y1,int x2,int y2,int color1,int color2,int w,int rn)
{
    HPEN newpen,oldpen;
    HBRUSH newbrush,oldbrush;
    HGDIOBJ r;
    unsigned r32;

    //oldbrush=(HBRUSH)SelectObject(hdc,GetStockObject(color1&0xFF000000?NULL_BRUSH:DC_BRUSH));
    newbrush=CreateSolidBrush(color1);
    oldbrush=(HBRUSH)SelectObject(hdc,newbrush);
    if(color1&0xFF000000)(HBRUSH)SelectObject(hdc,GetStockObject(NULL_BRUSH));

    if(!oldbrush)log_err("ERROR in drawrect(): failed SelectObject(GetStockObject)\n");
    r32=SetDCBrushColor(hdc,color1);
    if(r32==CLR_INVALID)log_err("ERROR in drawrect(): failed SetDCBrushColor\n");

    newpen=CreatePen(w?PS_SOLID:PS_NULL,w,color2);
    if(!newpen)log_err("ERROR in drawrect(): failed CreatePen\n");
    oldpen=(HPEN)SelectObject(hdc,newpen);
    if(!oldpen)log_err("ERROR in drawrect(): failed SelectObject(newpen)\n");

    if(rn)
        RoundRect(hdc,x1,y1,x2,y2,rn,rn);
    else
        Rectangle(hdc,x1,y1,x2,y2);

    r=SelectObject(hdc,oldpen);
    if(!r)log_err("ERROR in drawrect(): failed SelectObject(oldpen)\n");
    r=SelectObject(hdc,oldbrush);
    if(!r)log_err("ERROR in drawrect(): failed SelectObject(oldbrush)\n");
    r32=DeleteObject(newpen);
    if(!r32)log_err("ERROR in drawrect(): failed DeleteObject(newpen)\n");
    r32=DeleteObject(newbrush);
    if(!r32)log_err("ERROR in drawrect(): failed DeleteObject(newbrush)\n");
}

void drawrectsel(HDC hdc,int x1,int y1,int x2,int y2,int color2,int w)
{
    HPEN newpen,oldpen;
    HBRUSH oldbrush;
    HGDIOBJ r;
    x1-=2;
    y1-=2;
    x2+=2;
    y2-=2;

    oldbrush=(HBRUSH)SelectObject(hdc,GetStockObject(NULL_BRUSH));
    if(!oldbrush)log_err("ERROR in drawrectsel(): failed SelectObject(GetStockObject)\n");

    newpen=CreatePen(PS_DOT,w,color2);
    if(!newpen)log_err("ERROR in drawrectsel(): failed CreatePen\n");
    oldpen=(HPEN)SelectObject(hdc,newpen);
    if(!oldpen)log_err("ERROR in drawrectsel(): failed SelectObject(newpen)\n");

    Rectangle(hdc,x1,y1,x2,y2);

    r=SelectObject(hdc,oldpen);
    if(!r)log_err("ERROR in drawrectsel(): failed SelectObject(oldpen)\n");
    r=SelectObject(hdc,oldbrush);
    if(!r)log_err("ERROR in drawrectsel(): failed SelectObject(oldbrush)\n");
}

void box_draw(HDC hdc,int x1,int y1,int x2,int y2,int id)
{

    if(id<0||id>=BOX_NUM)
    {
        log_err("ERROR in box_draw(): invalid id=%d\n",id);
        return;
    }
    int i=boxindex[id];
    if(i<0||i>=THEME_NM)
    {
        log_err("ERROR in box_draw(): invalid index=%d\n",i);
        return;
    }
    drawrect(hdc,x1,y1,x2,y2,D(i),D(i+1),D(i+2),D(i+3));
    box[id].draw(hdc,x1,y1,x2,y2,D(i+5),D(i+6));
}

void drawcheckbox(HDC hdc,int x,int y,int wx,int wy,int checked,int active)
{
    RECT rect;
    int i=4+(active?1:0)+(checked?2:0);

    rect.left=x;
    rect.top=y;
    rect.right=x+wx;
    rect.bottom=y+wy;

    if(icon[i].isLoaded())
        icon[i].draw(hdc,x,y,x+wx,y+wy,0,Image::HSTR|Image::VSTR);
    else
        DrawFrameControl(hdc,&rect,DFC_BUTTON,DFCS_BUTTONCHECK|(checked?DFCS_CHECKED:0));
}

void drawpopup(int itembar,int type,int x,int y,HWND hwnd)
{
    POINT p={x,y};
    HMONITOR hMonitor;
    MONITORINFO mi;
    int needupdate;

    if((type==FLOATING_CMPDRIVER||type==FLOATING_DRIVERLST)&&itembar<0)type=FLOATING_NONE;
    if(type==FLOATING_TOOLTIP&&(itembar<=1||!*STR(itembar)))type=FLOATING_NONE;

    ClientToScreen(hwnd,&p);
    needupdate=floating_itembar!=itembar||floating_type!=type;
    floating_itembar=itembar;
    floating_type=type;

    if(type!=FLOATING_NONE)
    {
        //if(type==FLOATING_ABOUT)p.y=p.y-floating_y-30;
        //if(type==FLOATING_CMPDRIVER||type==FLOATING_DRIVERLST)
        {
            hMonitor=MonitorFromPoint(p,MONITOR_DEFAULTTONEAREST);
            mi.cbSize=sizeof(MONITORINFO);
            GetMonitorInfo(hMonitor,&mi);

            mi.rcWork.right-=15;
            if(p.x+floating_x>mi.rcWork.right)p.x=mi.rcWork.right-floating_x;
            if(p.x<5)p.x=5;
            if(p.y+floating_y>mi.rcWork.bottom-20)p.y=p.y-floating_y-30;
            if(p.y<5)p.y=5;
        }

        MoveWindow(hPopup,p.x+10,p.y+20,floating_x,floating_y,1);
        if(needupdate)InvalidateRect(hPopup,0,0);

        TRACKMOUSEEVENT tme;
        tme.cbSize=sizeof(tme);
        tme.hwndTrack=hwnd;
        tme.dwFlags=TME_LEAVE|TME_HOVER;
        tme.dwHoverTime=(ctrl_down||space_down)?1:hintdelay;
        TrackMouseEvent(&tme);
    }
    //ShowWindow(hPopup,type==FLOATING_NONE?SW_HIDE:SW_SHOWNOACTIVATE);
    if(type==FLOATING_NONE)ShowWindow(hPopup,SW_HIDE);
}
//}

//{ Canvas
void Canvas::init()
{
    int r;

    hdcMem=CreateCompatibleDC(0);
    if(!hdcMem)log_err("ERROR in canvas_init(): failed CreateCompatibleDC\n");
    r=SetBkMode(hdcMem,TRANSPARENT);
    if(!r)log_err("ERROR in canvas_init(): failed SetBkMode\n");
    bitmap=0;
    x=0;
    y=0;
}

void Canvas::free()
{
    int r;

    if(hdcMem)
    {
        r=DeleteDC(hdcMem);
        if(!r)log_err("ERROR in canvas_free(): failed DeleteDC\n");
        hdcMem=0;
    }

    if(bitmap)
    {
        //r=(int)SelectObject(hdcMem,oldbitmap);
        //if(!r)log_err("ERROR in canvas_free(): failed SelectObject\n");
        r=DeleteObject(bitmap);
        if(!r)log_err("ERROR in canvas_free(): failed DeleteObject\n");
        bitmap=0;
    }
}

void Canvas::begin(HWND nhwnd,int nx,int ny)
{
    HGDIOBJ r;
    unsigned r32;

    hwnd=nhwnd;
    localDC=BeginPaint(hwnd,&ps);
    if(!localDC)log_err("ERROR in canvas_begin(): failed BeginPaint\n");

    if(x!=nx||y!=ny)
    {
        x=nx;
        y=ny;
        if(bitmap)
        {
            r=SelectObject(hdcMem,oldbitmap);
            if(!r)log_err("ERROR in canvas_begin(): failed SelectObject(oldbitmap)\n");
            r32=DeleteObject(bitmap);
            if(!r32)log_err("ERROR in canvas_begin(): failed DeleteObject\n");
        }
        bitmap=CreateCompatibleBitmap(localDC,x,y);
        if(!bitmap)log_err("ERROR in canvas_begin(): failed CreateCompatibleBitmap\n");
        oldbitmap=(HBITMAP)SelectObject(hdcMem,bitmap);
        if(!oldbitmap)log_err("ERROR in canvas_begin(): failed SelectObject(bitmap)\n");
    }
    clipping=CreateRectRgnIndirect(&ps.rcPaint);
    if(!clipping)log_err("ERROR in canvas_begin(): failed BeginPaint\n");
    SetStretchBltMode(hdcMem,HALFTONE);
    r32=SelectClipRgn(hdcMem,clipping);
    if(!r32)log_err("ERROR in canvas_begin(): failed SelectClipRgn\n");
}

void Canvas::end()
{
    int r;

    r=BitBlt(localDC,
            ps.rcPaint.left,ps.rcPaint.top,ps.rcPaint.right,ps.rcPaint.bottom,
            hdcMem,
            ps.rcPaint.left,ps.rcPaint.top,
            SRCCOPY);
    SelectClipRgn(hdcMem,0);
    if(!r)log_err("ERROR in canvas_end(): failed BitBlt\n");
    r=DeleteObject(clipping);
    if(!r)log_err("ERROR in canvas_end(): failed DeleteObject\n");
    EndPaint(hwnd,&ps);
}
//}

//{ Panel
int Panel::Xp(){return Xm(D(PANEL_OFSX+indofs));}
int Panel::Yp(){return Ym(D(PANEL_OFSY+indofs));}
int Panel::XP(){return XM(D(PANEL_WX+indofs),D(PANEL_OFSX+indofs));}
int Panel::YP(){return YM(D(PANEL_WY+indofs),D(PANEL_OFSY+indofs));}

int Panel::hitscan(int hx,int hy)
{
    int wy=D(PANEL_WY+indofs);

    if(kbpanel&&items[0].str_id==kbpanel)
    {
        if(kbpanel==KB_INSTALL)
        {
            if(kbitem[kbpanel]>2)kbitem[kbpanel]=2;
            return index-8==kbitem[kbpanel];
        }
        if(kbitem[kbpanel]>items[0].action_id)kbitem[kbpanel]=items[0].action_id;
        while(items[kbitem[kbpanel]].type!=TYPE_CHECKBOX&&
              items[kbitem[kbpanel]].type!=TYPE_BUTTON)kbitem[kbpanel]++;

        return kbitem[kbpanel];
    }

    if(!wy)return -1;
    hx-=Xp()+D(PNLITEM_OFSX);
    hy-=Yp()+D(PNLITEM_OFSY);

    if(!expertmode&&items[0].type==TYPE_GROUP_BREAK)return -2;
    if(hx<0||hy<0||hx>XP()-D(PNLITEM_OFSX)*2)return -3;
    if(hy/wy>=items[0].action_id)return -4;
    int r=hy/wy+1;
    if(r>=0&&!items[r].type)return -1;
    return r;
}

void Panel::draw_inv()
{
    int x=Xp(),y=Yp();
    int wy=D(PANEL_WY+indofs);
    int ofsy=D(PNLITEM_OFSY);
    RECT rect;

    rect.left=x;
    rect.top=y;
    rect.right=x+XP();
    rect.bottom=y+(wy+1)*items[0].action_id+ofsy*2;
    InvalidateRect(hMain,&rect,0);
}

void Panel::setfilters()
{
    for(int i=0;i<items[0].action_id+1;i++)
        if(items[i].action_id>=ID_SHOW_MISSING&&items[i].action_id<=ID_SHOW_INVALID)
            items[i].checked=filters&(1<<items[i].action_id)?1:0;
}

void Panel::moveWindow(HWND hwnd,int i,int j,int f)
{
    MoveWindow(hwnd,Xp()+i,Yp()+j*D(PNLITEM_WY)-2+f,XP()-i-D(PNLITEM_OFSX),190*2,0);
}

static void TextOutH(HDC hdc,int x,int y,LPCTSTR buf){TextOut(hdc,x,y,buf,wcslen(buf));}

void Panel::draw(HDC hdc)
{
    WCHAR buf[BUFLEN];
    POINT p;
    HRGN rgn=0;
    int cur_i;
    int i;
    int x=Xp(),y=Yp();
    int ofsx=D(PNLITEM_OFSX),ofsy=D(PNLITEM_OFSY);
    int wy=D(PANEL_WY+indofs);

    if(XP()<0)return;
    if(!D(PANEL_WY+indofs))return;

    GetCursorPos(&p);
    ScreenToClient(hMain,&p);
    cur_i=hitscan(p.x,p.y);

    State *state=manager_g->matcher->state;
    for(i=0;i<items[0].action_id+1;i++)
    {
        SetTextColor(hdc,D(CHKBOX_TEXT_COLOR));

        // System Info (1st line)
        if(i==1&&index==0)
        {
            TextOutH(hdc,x+ofsx+SYSINFO_COL1,y+ofsy,STR(STR_SYSINF_MOTHERBOARD));
            TextOutH(hdc,x+ofsx+SYSINFO_COL2,y+ofsy,STR(STR_SYSINF_ENVIRONMENT));
        }

        // System Info (2nd line)
        if(i==2&&index==0)
        {
            wsprintf(buf,L"%s (%d-bit)",get_winverstr(manager_g),state->architecture?64:32);
            TextOutH(hdc,x+ofsx+10+SYSINFO_COL0,y+ofsy,buf);
            TextOutH(hdc,x+ofsx+10+SYSINFO_COL1,y+ofsy,state->getProduct());
            TextOutH(hdc,x+ofsx+10+SYSINFO_COL2,y+ofsy,STR(STR_SYSINF_WINDIR));
            TextOutH(hdc,x+ofsx+10+SYSINFO_COL3,y+ofsy,state->textas.getw(state->windir));
        }

        // System Info (3rd line)
        if(i==3&&index==0)
        {
            TextOutH(hdc,x+ofsx+10+SYSINFO_COL0,y+ofsy,(XP()<10+SYSINFO_COL1)?state->getProduct():state->platform.szCSDVersion);
            wsprintf(buf,L"%s: %s",STR(STR_SYSINF_TYPE),STR(state->isLaptop?STR_SYSINF_LAPTOP:STR_SYSINF_DESKTOP));
            TextOutH(hdc,x+ofsx+10+SYSINFO_COL1,y+ofsy,buf);
            TextOutH(hdc,x+ofsx+10+SYSINFO_COL2,y+ofsy,STR(STR_SYSINF_TEMP));
            TextOutH(hdc,x+ofsx+10+SYSINFO_COL3,y+ofsy,state->textas.getw(state->temp));
        }
        if(items[i].type==TYPE_GROUP_BREAK&&!expertmode)break;
        switch(items[i].type)
        {
            case TYPE_CHECKBOX:
                drawcheckbox(hdc,x+ofsx,y+ofsy,D(CHKBOX_SIZE)-2,D(CHKBOX_SIZE)-2,items[i].checked,i==cur_i);
                SetTextColor(hdc,D(i==cur_i?CHKBOX_TEXT_COLOR_H:CHKBOX_TEXT_COLOR));
                TextOut(hdc,x+D(CHKBOX_TEXT_OFSX)+ofsx,y+ofsy,STR(items[i].str_id),wcslen(STR(items[i].str_id)));
                if(i==cur_i&&kbpanel)drawrectsel(hdc,x+ofsx,y+ofsy,x+XP()-ofsx,y+ofsy+wy,0xff00,1);
                y+=D(PNLITEM_WY);
                break;

            case TYPE_BUTTON:
                if(index>=8&&index<=10&&D(PANEL_OUTLINE_WIDTH+indofs)<0)
                    box_draw(hdc,x+ofsx,y+ofsy,x+XP()-ofsx,y+ofsy+wy,i==cur_i?BOX_PANEL_H+index*2+2:BOX_PANEL+index*2+2);
                else
                    box_draw(hdc,x+ofsx,y+ofsy,x+XP()-ofsx,y+ofsy+wy-1,i==cur_i?BOX_BUTTON_H:BOX_BUTTON);

                SetTextColor(hdc,D(CHKBOX_TEXT_COLOR));

                if(i==1&&index==8) // Install button
                {
                    unsigned j,cnt=0;
                    itembar_t *itembar;

                    itembar=&manager_g->items_list[RES_SLOTS];
                    for(j=RES_SLOTS;j<manager_g->items_list.size();j++,itembar++)
                    if(itembar->checked)cnt++;

                    wsprintf(buf,L"%s (%d)",STR(items[i].str_id),cnt);
                    TextOut(hdc,x+ofsx+wy/2,y+ofsy+(wy-D(FONT_SIZE)-2)/2,buf,wcslen(buf));
                }
                else
                    TextOut(hdc,x+ofsx+wy/2,y+ofsy+(wy-D(FONT_SIZE)-2)/2,STR(items[i].str_id),wcslen(STR(items[i].str_id)));

                y+=D(PNLITEM_WY);
                break;

            case TYPE_TEXT:
                if(i==1&&index==7) // Revision number
                {
                    version_t v;

                    v.d=atoi(SVN_REV_D);
                    v.m=atoi(SVN_REV_M);
                    v.y=SVN_REV_Y;

                    wsprintf(buf,L"%s (",TEXT(SVN_REV2));
                    str_date(&v,buf+wcslen(buf));
                    wcscat(buf,L")");
                    SetTextColor(hdc,D(CHKBOX_TEXT_COLOR));
                    TextOut(hdc,x+ofsx,y+ofsy,buf,wcslen(buf));
                }
                SetTextColor(hdc,D(i==cur_i&&i>11?CHKBOX_TEXT_COLOR_H:CHKBOX_TEXT_COLOR));
                TextOut(hdc,x+ofsx,y+ofsy,STR(items[i].str_id),wcslen(STR(items[i].str_id)));
                y+=D(PNLITEM_WY);
                break;

            case TYPE_GROUP_BREAK:
            case TYPE_GROUP:
                if(index>=8&&index<=10)break;
                if(i)y+=D(PNLITEM_WY);
                box_draw(hdc,x,y,x+XP(),y+(wy)*items[i].action_id+ofsy*2,
                         BOX_PANEL+index*2+2);
                rgn=CreateRectRgn(x,y,x+XP(),y+(wy)*items[i].action_id+ofsy*2);
                SelectClipRgn(hdc,rgn);
                break;

            default:
                break;
        }

    }
    if(rgn)
    {
        SelectClipRgn(hdc,0);
        DeleteObject(rgn);
    }
}
//}
