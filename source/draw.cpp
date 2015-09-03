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
int rtl=0;
Image box[BOX_NUM];
Image icon[ICON_NUM];

Panelitem panel1[]=
{
    {TYPE_GROUP,0,3,0},
    {TYPE_TEXT,STR_SHOW_SYSINFO,0,0},
    {TYPE_TEXT,0,0,0},
    {TYPE_TEXT,0,0,0},
};

Panelitem panel2[]=
{
    {TYPE_GROUP,0,3,0},
    {0,STR_INSTALL,               ID_INSTALL,0},
    {0,STR_SELECT_ALL,            ID_SELECT_ALL,0},
    {0,STR_SELECT_NONE,           ID_SELECT_NONE,0},
};

Panelitem panel3[]=
{
    {TYPE_GROUP,KB_EXPERT,5,0},
    {TYPE_TEXT,STR_LANG,0,0},
    {TYPE_TEXT,0,0,0},
    {TYPE_TEXT,STR_THEME,0,0},
    {TYPE_TEXT,0,0,0},
    {TYPE_CHECKBOX,STR_EXPERT,              ID_EXPERT_MODE,0},
};

Panelitem panel3_w[]=
{
    {TYPE_GROUP,0,3,0},
    {TYPE_TEXT,STR_LANG,0,0},
    {TYPE_TEXT,STR_THEME,0,0},
    {TYPE_CHECKBOX,STR_EXPERT,              ID_EXPERT_MODE,0},
};

Panelitem panel4[]=
{
    {TYPE_GROUP_BREAK,KB_ACTIONS,4,0},
    {TYPE_BUTTON,STR_OPENLOGS,              ID_OPENLOGS,0},
    {TYPE_BUTTON,STR_SNAPSHOT,              ID_SNAPSHOT,0},
    {TYPE_BUTTON,STR_EXTRACT,               ID_EXTRACT,0},
    {TYPE_BUTTON,STR_DRVDIR,                ID_DRVDIR,0},
};

Panelitem panel5[]=
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

Panelitem panel6[]=
{
    {TYPE_GROUP_BREAK,KB_PANEL2,4,0},
    {TYPE_TEXT,STR_SHOW_NOTFOUND,0,0},
    {TYPE_CHECKBOX, STR_SHOW_NF_MISSING,    ID_SHOW_NF_MISSING,0},
    {TYPE_CHECKBOX, STR_SHOW_NF_UNKNOWN,    ID_SHOW_NF_UNKNOWN,0},
    {TYPE_CHECKBOX, STR_SHOW_NF_STANDARD,   ID_SHOW_NF_STANDARD,0},
};

Panelitem panel7[]=
{
    {TYPE_GROUP_BREAK,KB_PANEL3,3,0},
    {TYPE_CHECKBOX, STR_SHOW_ONE,           ID_SHOW_ONE,0},
    {TYPE_CHECKBOX, STR_SHOW_DUP,           ID_SHOW_DUP,0},
    {TYPE_CHECKBOX, STR_SHOW_INVALID,       ID_SHOW_INVALID,0},

};

Panelitem panel8[]=
{
    {TYPE_GROUP,0,1,0},
    {TYPE_TEXT,0,0,0},
};

Panelitem panel9[]=
{

    {TYPE_GROUP,KB_INSTALL,1,0},
    {TYPE_BUTTON,STR_INSTALL,               ID_INSTALL,0},
};

Panelitem panel10[]=
{
    {TYPE_GROUP,KB_INSTALL,1,0},
    {TYPE_BUTTON,STR_SELECT_ALL,            ID_SELECT_ALL,0},
};

Panelitem panel11[]=
{
    {TYPE_GROUP,KB_INSTALL,1,0},
    {TYPE_BUTTON,STR_SELECT_NONE,           ID_SELECT_NONE,0},
};

Panelitem panel12[]=
{
    {TYPE_GROUP,KB_PANEL_CHK,3,0},
    {TYPE_TEXT,STR_OPTIONS,0,0},
    {TYPE_CHECKBOX,STR_RESTOREPOINT,        ID_RESTPNT,0},
    {TYPE_CHECKBOX,STR_REBOOT,              ID_REBOOT,0},
};

Panelitem panel13[]=
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
    wchar_t *filename=D_STR(i);

    release();

    if(wcsstr(filename,L"RES_"))
        loadFromRes(_wtoi_my(filename+4));
    else
        loadFromFile(filename);
}

void Image::release()
{
    if(bitmap&&!iscopy)
    {
        SelectObject(ldc,oldbitmap);
        int r=DeleteDC(ldc);
            if(!r)log_err("ERROR in box_init(): failed DeleteDC\n");
        r=DeleteObject(bitmap);
            if(!r)log_err("ERROR in box_init(): failed DeleteObject\n");
    }
    bitmap=nullptr;
    ldc=nullptr;
    iscopy=0;
}

void Image::loadFromFile(wchar_t *filename)
{
    wchar_t buf[BUFLEN];
    FILE *f;
    int sz;

    if(!filename||!*filename)return;
    wsprintf(buf,L"%s\\themes\\%s",Settings.data_dir,filename);
    f=_wfopen(buf,L"rb");
    if(!f)
    {
        log_err("ERROR in image_loadFile(): file '%S' not found\n",buf);
        return;
    }
    fseek(f,0,SEEK_END);
    sz=ftell(f);
    fseek(f,0,SEEK_SET);
    std::unique_ptr<BYTE[]> imgbuf(new BYTE[sz]);

    sz=fread(imgbuf.get(),1,sz,f);
    if(!sz)
    {
        log_err("ERROR in image_loadFile(): cannnot read from file '%S'\n",buf);
        return;
    }
    fclose(f);
    createBitmap(imgbuf.get(),sz);
}

void Image::loadFromRes(int id)
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
    BYTE *big;
    hasalpha=sx=sy=0;
    ldc=nullptr;
#ifdef CONSOLE_MODE
    UNREFERENCED_PARAMETER(data)
    UNREFERENCED_PARAMETER(sz)
    return;
#else
    int ret=WebPGetInfo(data,sz,&sx,&sy);
    if(!ret)
    {
        log_err("ERROR in image_load(): failed WebPGetInfo(%d)\n",ret);
        return;
    }
    big=WebPDecodeBGRA(data,sz,&sx,&sy);
    if(!big)
    {
        log_err("ERROR in image_load(): failed WebPDecodeBGRA\n");
        return;
    }
#endif
    BITMAPINFO bmi;
    ZeroMemory(&bmi,sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth=sx;
    bmi.bmiHeader.biHeight=-sy;
    bmi.bmiHeader.biPlanes=1;
    bmi.bmiHeader.biBitCount=32;
    bmi.bmiHeader.biCompression=BI_RGB;
    bmi.bmiHeader.biSizeImage=sx*sy*4;

    BYTE *bits;
    ldc=CreateCompatibleDC(nullptr);
    bitmap=CreateDIBSection(ldc,&bmi,DIB_RGB_COLORS,(void **)&bits,nullptr,0);

    BYTE *p2=big;
    for(int i=0;i<sy*sx;i++)
    {
        BYTE B,G,R,A;
        B=*p2++;
        G=*p2++;
        R=*p2++;
        A=*p2++;
        double dA=A/255.;
        if(A!=255)hasalpha=1;

        *bits++=(BYTE)(B*dA);
        *bits++=(BYTE)(G*dA);
        *bits++=(BYTE)(R*dA);
        *bits++=A;
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

    SetLayout(ldc,rtl?LAYOUT_RTL:0);

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

//{ Canvas
Canvas::Canvas()
{
    hdcMem=CreateCompatibleDC(nullptr);
    if(!hdcMem)log_err("ERROR in canvas_init(): failed CreateCompatibleDC\n");
    int r=SetBkMode(hdcMem,TRANSPARENT);
    if(!r)log_err("ERROR in canvas_init(): failed SetBkMode\n");
    bitmap=nullptr;
    x=0;
    y=0;
}

Canvas::~Canvas()
{
    if(hdcMem)
    {
        int r=DeleteDC(hdcMem);
        if(!r)log_err("ERROR in canvas_free(): failed DeleteDC\n");
        hdcMem=nullptr;
    }

    if(bitmap)
    {
        //r=(int)SelectObject(hdcMem,oldbitmap);
        //if(!r)log_err("ERROR in canvas_free(): failed SelectObject\n");
        int r=DeleteObject(bitmap);
        if(!r)log_err("ERROR in canvas_free(): failed DeleteObject\n");
        bitmap=nullptr;
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
    SetLayout(hdcMem,rtl?LAYOUT_RTL:0);
}

void Canvas::end()
{
    int r;

    r=BitBlt(localDC,
            ps.rcPaint.left,ps.rcPaint.top,ps.rcPaint.right,ps.rcPaint.bottom,
            hdcMem,
            ps.rcPaint.left,ps.rcPaint.top,
            SRCCOPY);
    SelectClipRgn(hdcMem,nullptr);
    if(!r)log_err("ERROR in canvas_end(): failed BitBlt\n");
    r=DeleteObject(clipping);
    if(!r)log_err("ERROR in canvas_end(): failed DeleteObject\n");
    EndPaint(hwnd,&ps);
}
//}

//{ Panel
int Panel::Xp(){return Xm(D(PANEL_OFSX+indofs),D(PANEL_WX+indofs));}
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
            if(kbitem[kbpanel]>2)kbitem[kbpanel]=0;
            if(index-8!=kbitem[kbpanel])return -1;
            return index-8==kbitem[kbpanel];
        }
        if(kbitem[kbpanel]>items[0].action_id)kbitem[kbpanel]=0;
        while(items[kbitem[kbpanel]].type!=TYPE_CHECKBOX&&
              items[kbitem[kbpanel]].type!=TYPE_BUTTON)kbitem[kbpanel]++;

        return kbitem[kbpanel];
    }

    if(!wy)return -1;
    hx-=Xp()+D(PNLITEM_OFSX);
    hy-=Yp()+D(PNLITEM_OFSY);

    if(!Settings.expertmode&&items[0].type==TYPE_GROUP_BREAK)return -2;
    if(hx<0||hy<0||hx>XP()-D(PNLITEM_OFSX)*2)return -3;
    if(hy/wy>=items[0].action_id)return -4;
    int r=hy/wy+1;
    if(r>=0&&!items[r].type)return -1;
    return r;
}

void Panel::keybAdvance(int v)
{
    if(kbpanel&&items[0].str_id==kbpanel)
    {
        kbitem[kbpanel]+=v;

        while(kbitem[kbpanel]>=0&&kbitem[kbpanel]<=items[0].action_id&&
              items[kbitem[kbpanel]].type!=TYPE_CHECKBOX&&
              items[kbitem[kbpanel]].type!=TYPE_BUTTON)
              kbitem[kbpanel]+=v;

        if(kbitem[kbpanel]>items[0].action_id)kbitem[kbpanel]=0;
        if(kbitem[kbpanel]<0)kbitem[kbpanel]=items[0].action_id;
    }

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

    if(rtl)
        InvalidateRect(hMain,nullptr,0);
    else
        InvalidateRect(hMain,&rect,0);
}

int Panel::calcFilters()
{
    int sum=0;

    for(int j=0;j<items[0].action_id+1;j++)
        if(items[j].type==TYPE_CHECKBOX&&
            items[j].checked&&
            items[j].action_id!=ID_EXPERT_MODE)
            sum+=1<<items[j].action_id;
    return sum;
}

void Panel::setFilters(int filters_)
{
    for(int i=0;i<items[0].action_id+1;i++)
        if(items[i].action_id>=ID_SHOW_MISSING&&items[i].action_id<=ID_SHOW_INVALID)
            items[i].checked=filters_&(1<<items[i].action_id)?1:0;
}

void Panel::moveWindow(HWND hwnd,int i,int j,int f)
{
    MoveWindow(hwnd,Xp()+i,Yp()+j*D(PNLITEM_WY)-2+f,XP()-i-D(PNLITEM_OFSX),190*2,0);
}

void Panel::click(int i)
{
    if(items[i].type==TYPE_CHECKBOX||TYPE_BUTTON)
    {
        flipChecked(i);
        if(items[i].action_id==ID_EXPERT_MODE)
        {
            Settings.expertmode=isChecked(i);
            ShowWindow(GetConsoleWindow(),Settings.expertmode&&ctrl_down?SW_SHOWNOACTIVATE:hideconsole);
        }
        else
            PostMessage(hMain,WM_COMMAND,items[i].action_id+(BN_CLICKED<<16),0);

        InvalidateRect(hMain,nullptr,0);
    }
}

void Panel::draw(HDC hdc)
{
    wchar_t buf[BUFLEN];
    POINT p;
    HRGN rgn=nullptr;
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

    State *state=manager_g->matcher->getState();
    for(i=0;i<items[0].action_id+1;i++)
    {
        bool isSelected=i==cur_i;

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
            wsprintf(buf,L"%s (%d-bit)",state->get_winverstr(),state->getArchitecture()?64:32);
            if(rtl)wcscat(buf,L"\u200E");
            TextOutH(hdc,x+ofsx+10+SYSINFO_COL0,y+ofsy,buf);
            TextOutH(hdc,x+ofsx+10+SYSINFO_COL1,y+ofsy,state->getProduct());
            TextOutH(hdc,x+ofsx+10+SYSINFO_COL2,y+ofsy,STR(STR_SYSINF_WINDIR));
            TextOutH(hdc,x+ofsx+10+SYSINFO_COL3,y+ofsy,state->textas.getw(state->getWindir()));
        }

        // System Info (3rd line)
        if(i==3&&index==0)
        {
            wsprintf(buf,L"%s",(XP()<10+SYSINFO_COL1)?state->getProduct():state->get_szCSDVersion());
            if(rtl)wcscat(buf,L"\u200E");
            TextOutH(hdc,x+ofsx+10+SYSINFO_COL0,y+ofsy,buf);
            wsprintf(buf,L"%s: %s",STR(STR_SYSINF_TYPE),STR(state->isLaptop?STR_SYSINF_LAPTOP:STR_SYSINF_DESKTOP));
            TextOutH(hdc,x+ofsx+10+SYSINFO_COL1,y+ofsy,buf);
            TextOutH(hdc,x+ofsx+10+SYSINFO_COL2,y+ofsy,STR(STR_SYSINF_TEMP));
            TextOutH(hdc,x+ofsx+10+SYSINFO_COL3,y+ofsy,state->textas.getw(state->getTemp()));
        }
        if(items[i].type==TYPE_GROUP_BREAK&&!Settings.expertmode)break;
        switch(items[i].type)
        {
            case TYPE_CHECKBOX:
                if(isSelected&&kbpanel)
                {
                    drawbox(hdc,x+ofsx,y,x+XP()-ofsx,y+ofsy+wy,BOX_KBHLT);
                    isSelected=false;
                }
                drawcheckbox(hdc,mirw(x,ofsx,XP()-D(CHKBOX_SIZE)-2),y+ofsy,D(CHKBOX_SIZE)-2,D(CHKBOX_SIZE)-2,items[i].checked,isSelected);
                SetTextColor(hdc,D(isSelected?CHKBOX_TEXT_COLOR_H:CHKBOX_TEXT_COLOR));
                TextOutH(hdc,mirw(x,D(CHKBOX_TEXT_OFSX)+ofsx,XP()-ofsx*2),y+ofsy,STR(items[i].str_id));
                //if(i==cur_i&&kbpanel)drawrectsel(hdc,x+ofsx,y+ofsy,x+XP()-ofsx,y+ofsy+wy,0xff00,1);
                y+=D(PNLITEM_WY);
                SetTextAlign(hdc,TA_LEFT);
                break;

            case TYPE_BUTTON:
                if(index>=8&&index<=10&&D(PANEL_OUTLINE_WIDTH+indofs)<0)
                    drawbox(hdc,x+ofsx,y+ofsy,x+XP()-ofsx,y+ofsy+wy,isSelected?BOX_PANEL_H+index*2+2:BOX_PANEL+index*2+2);
                else
                    drawbox(hdc,x+ofsx,y+ofsy,x+XP()-ofsx,y+ofsy+wy-1,isSelected?BOX_BUTTON_H:BOX_BUTTON);

                SetTextColor(hdc,D(CHKBOX_TEXT_COLOR));

                if(i==1&&index==8) // Install button
                {
                    wsprintf(buf,L"%s (%d)",STR(items[i].str_id),manager_g->countItems());
                    int nwy=D(PANEL9_OFSX)==D(PANEL10_OFSX)?D(PANEL10_WY):wy;
                    TextOutH(hdc,mirw(x,ofsx+nwy/2,XP()),y+ofsy+(wy-D(FONT_SIZE)-2)/2,buf);
                }
                else
                    TextOutH(hdc,mirw(x,ofsx+wy/2,XP()),y+ofsy+(wy-D(FONT_SIZE)-2)/2,STR(items[i].str_id));

                y+=D(PNLITEM_WY);
                SetTextAlign(hdc,TA_LEFT);
                break;

            case TYPE_TEXT:
                if(i==1&&index==7) // Revision number
                {
                    Version v{atoi(SVN_REV_D),atoi(SVN_REV_M),SVN_REV_Y};

                    /*v.d=atoi(SVN_REV_D);
                    v.m=atoi(SVN_REV_M);
                    v.y=SVN_REV_Y;*/

                    wsprintf(buf,L"%s (",TEXT(SVN_REV2));
                    v.str_date(buf+wcslen(buf));
                    wcscat(buf,L")");if(rtl)wcscat(buf,L"\u200E");
                    SetTextColor(hdc,D(CHKBOX_TEXT_COLOR));
                    TextOutH(hdc,mirw(x,ofsx,XP()),y+ofsy,buf);
                }
                SetTextColor(hdc,D(isSelected&&i>11?CHKBOX_TEXT_COLOR_H:CHKBOX_TEXT_COLOR));
                TextOutH(hdc,mirw(x,ofsx,XP()),y+ofsy,STR(items[i].str_id));
                y+=D(PNLITEM_WY);
                break;

            case TYPE_GROUP_BREAK:
            case TYPE_GROUP:
                if(index>=8&&index<=10)break;
                if(i)y+=D(PNLITEM_WY);
                drawbox(hdc,x,y,x+XP(),y+(wy)*items[i].action_id+ofsy*2,
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
        SelectClipRgn(hdc,nullptr);
        DeleteObject(rgn);
    }
}
//}

//{ Text
void TextOut_CM(HDC hdcMem,int x,int y,const wchar_t *str,int color,int *maxsz,int mode)
{
    SIZE ss;
    GetTextExtentPoint32(hdcMem,str,wcslen(str),&ss);
    if(ss.cx>*maxsz)*maxsz=ss.cx;

    if(!mode)return;
    SetTextColor(hdcMem,color);
    TextOutH(hdcMem,x,y,str);
    //SetTextColor(hdcMem,0);
}

void TextOutP(textdata_t *td,const wchar_t *format,...)
{
    wchar_t buffer[BUFLEN];
    va_list args;
    va_start(args,format);
    _vsnwprintf(buffer,BUFLEN,format,args);

    TextOut_CM(td->hdcMem,td->x,td->y,buffer,td->col,&td->limits[td->i],td->mode);
    td->x+=td->limits[td->i];
    td->i++;
    va_end(args);
}

void TextOutF(textdata_t *td,int col,const wchar_t *format,...)
{
    wchar_t buffer[BUFLEN];
    va_list args;
    va_start(args,format);
    _vsnwprintf(buffer,BUFLEN,format,args);

    TextOut_CM(td->hdcMem,td->x,td->y,buffer,col,&td->maxsz,1);
    td->y+=td->wy;
    va_end(args);
}

void TextOutSF(textdata_t *td,const wchar_t *str,const wchar_t *format,...)
{
    wchar_t buffer[BUFLEN];
    va_list args;
    va_start(args,format);
    _vsnwprintf (buffer,BUFLEN,format,args);
    TextOut_CM(td->hdcMem,td->x,td->y,str,td->col,&td->maxsz,1);
    TextOut_CM(td->hdcMem,td->x+POPUP_SYSINFO_OFS,td->y,buffer,td->col,&td->maxsz,1);
    td->y+=td->wy;
    va_end(args);
}
//}

//{Popup
void popup_resize(int x,int y)
{
    if(floating_x!=x||floating_y!=y)
    {
        POINT p1;

        floating_x=x;
        floating_y=y;
        GetCursorPos(&p1);
        SetCursorPos(p1.x+1,p1.y);
        SetCursorPos(p1.x,p1.y);
    }
}

void popup_about(HDC hdcMem)
{
    textdata_t td;
    RECT rect;
    int p0=D(POPUP_OFSX);

    td.col=D(POPUP_TEXT_COLOR);
    td.wy=D(POPUP_WY);
    td.y=D(POPUP_OFSY);
    td.hdcMem=hdcMem;
    td.maxsz=0;

    td.x=p0;
    TextOutF(&td,td.col,L"Snappy Driver Installer %s",STR(STR_ABOUT_VER));
    td.y+=td.wy;
    rect.left=td.x;
    rect.top=td.y;
    rect.right=D(POPUP_WX)-p0*2;
    rect.bottom=500;
    DrawText(hdcMem,STR(STR_ABOUT_LICENSE),-1,&rect,DT_WORDBREAK);
    td.y+=td.wy*3;
    TextOutF(&td,td.col,L"%s%s",STR(STR_ABOUT_DEV_TITLE),STR(STR_ABOUT_DEV_LIST));
    TextOutF(&td,td.col,L"%s%s",STR(STR_ABOUT_TESTERS_TITLE),STR(STR_ABOUT_TESTERS_LIST));
    td.y+=td.wy*(intptr_t)STR(STR_ABOUT_SIZE);

    popup_resize(D(POPUP_WX),td.y+D(POPUP_OFSY));
}

void format_size(wchar_t *buf,long long val,int isspeed)
{
#ifdef USE_TORRENT
    StrFormatByteSizeW(val,buf,BUFLEN);
#else
    buf[0]=0;
    UNREFERENCED_PARAMETER(val)
#endif
    if(isspeed)wcscat(buf,STR(STR_UPD_SEC));
}

void format_time(wchar_t *buf,long long val)
{
    long long days,hours,mins,secs;

    secs=val/1000;
    mins=secs/60;
    hours=mins/60;
    days=hours/24;

    secs%=60;
    mins%=60;
    hours%=24;

    wcscpy(buf,L"\x221E");
    if(secs) wsprintf(buf,L"%d %s",(int)secs,STR(STR_UPD_TSEC));
    if(mins) wsprintf(buf,L"%d %s %d %s",(int)mins,STR(STR_UPD_TMIN),(int)secs,STR(STR_UPD_TSEC));
    if(hours)wsprintf(buf,L"%d %s %d %s",(int)hours,STR(STR_UPD_THOUR),(int)mins,STR(STR_UPD_TMIN));
    if(days) wsprintf(buf,L"%d %s %d %s",(int)days,STR(STR_UPD_TDAY),(int)hours,STR(STR_UPD_THOUR));
}
//}

//{ Draw
int mirw(int x,int ofs,int w)
{
    UNREFERENCED_PARAMETER(w)
    return x+ofs;
}
void TextOutH(HDC hdc,int x,int y,LPCTSTR buf)
{
    TextOut(hdc,x,y,buf,wcslen(buf));
}
int Xm(int x,int o)
{
    UNREFERENCED_PARAMETER(o)
    return x>=0?x:(main1x_c+x);
}
int Ym(int y){return y>=0?y:(main1y_c+y);}
int XM(int w,int x){return w>=0?w:(w+main1x_c-x);}
int YM(int y,int o){return y>=0?y:(main1y_c+y-o);}

int Xg(int x,int o)
{
    UNREFERENCED_PARAMETER(o)
    return x>=0?x:(mainx_c+x);
}
int Yg(int y){return y>=0?y:(mainy_c+y);}
int XG(int x,int o){return x>=0?x:(mainx_c+x-o);}
int YG(int y,int o){return y>=0?y:(mainy_c+y-o);}

int panels_hitscan(int hx,int hy,int *ii)
{
    int i;

    *ii=-1;
    for(i=0;i<NUM_PANELS;i++)
    {
        int r=panels[i].hitscan(hx,hy);
        if(r>=0)
        {
            *ii=i;
            return r;
        }
    }
    return -1;
}

void panel_loadsettings(Panel *panel,int filters_)
{
     // Expert mode
    panel3[5].checked=Settings.expertmode;
    panel3_w[3].checked=Settings.expertmode;

    for(int j=0;j<7;j++)panel[j].setFilters(filters_);
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

void drawbox(HDC hdc,int x1,int y1,int x2,int y2,int id)
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

    if(rtl)p.x+=floating_x;
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
        if(needupdate)InvalidateRect(hPopup,nullptr,0);

        TRACKMOUSEEVENT tme;
        tme.cbSize=sizeof(tme);
        tme.hwndTrack=hwnd;
        tme.dwFlags=TME_LEAVE|TME_HOVER;
        tme.dwHoverTime=(ctrl_down||space_down)?1:Settings.hintdelay;
        TrackMouseEvent(&tme);
    }
    if(type==FLOATING_NONE)ShowWindow(hPopup,SW_HIDE);
}

HICON CreateMirroredIcon(HICON hiconOrg)
{
    HDC hdcScreen,hdcBitmap,hdcMask=nullptr;
    HBITMAP hbm,hbmMask,hbmOld,hbmOldMask;
    BITMAP bm;
    ICONINFO ii;
    HICON hicon=nullptr;

    hdcBitmap=CreateCompatibleDC(nullptr);
    if(hdcBitmap)
    {
        hdcMask=CreateCompatibleDC(nullptr);
        if(hdcMask)
        {
            SetLayout(hdcBitmap,LAYOUT_RTL);
            SetLayout(hdcMask,LAYOUT_RTL);
        }
        else
        {
            DeleteDC(hdcBitmap);
            hdcBitmap=nullptr;
        }
    }
    hdcScreen=GetDC(nullptr);
    if(hdcScreen)
    {
        if(hdcBitmap&&hdcMask)
        {
            if(hiconOrg)
            {
                if(GetIconInfo(hiconOrg,&ii)&&GetObject(ii.hbmColor,sizeof(BITMAP),&bm))
                {
                    // Do the cleanup for the bitmaps.
                    DeleteObject(ii.hbmMask);
                    DeleteObject(ii.hbmColor);
                    ii.hbmMask=ii.hbmColor=nullptr;
                    hbm=CreateCompatibleBitmap(hdcScreen,bm.bmWidth,bm.bmHeight);
                    hbmMask=CreateBitmap(bm.bmWidth,bm.bmHeight,1,1,nullptr);
                    hbmOld=(HBITMAP)SelectObject(hdcBitmap,hbm);
                    hbmOldMask=(HBITMAP)SelectObject(hdcMask,hbmMask);
                    DrawIconEx(hdcBitmap,0,0,hiconOrg,bm.bmWidth,bm.bmHeight,0,nullptr,DI_IMAGE);
                    DrawIconEx(hdcMask,0,0,hiconOrg,bm.bmWidth,bm.bmHeight,0,nullptr,DI_MASK);
                    SelectObject(hdcBitmap,hbmOld);
                    SelectObject(hdcMask,hbmOldMask);

                    // Create the new mirrored icon and delete bitmaps
                    ii.hbmMask=hbmMask;
                    ii.hbmColor=hbm;
                    hicon=CreateIconIndirect(&ii);
                    DeleteObject(hbm);
                    DeleteObject(hbmMask);
                }
            }
        }
        ReleaseDC(nullptr,hdcScreen);
    }
    if(hdcBitmap)DeleteDC(hdcBitmap);
    if(hdcMask)DeleteDC(hdcMask);
    return hicon;
}
//}
