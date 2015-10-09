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

#include "com_header.h"
#include "guicon.h"
#include "matcher.h"
#include "settings.h"
#include "common.h"
#include "manager.h"

#include <windows.h>

// Depend on Win32API
#include "system.h"
#include "theme.h"    // todo: HWND
#include "enum.h"     // needs Version from indexing.h
#include "main.h"     // todo: lots of Win32
#include "draw.h"     // todo: lots of Win32

#include <setupapi.h>       // for SetupDiGetClassDescription()
#include <webp\decode.h>
#include <memory>

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

WidgetComposite *wPanels=nullptr;

class autorun
{
public:
    autorun()
    {
        wPanel *p;
        int ind=0;
        wPanels=new WidgetComposite;

        // SysInfo
        p=new wPanel{3,ind++};
        p->Add(new wTextSys1);
        p->Add(new wTextSys2);
        p->Add(new wTextSys3);
        wPanels->Add(p);

        // Install
        p=new wPanel{3,ind++};
        wPanels->Add(p);

        // Theme/lang
        p=new wPanel{5,ind++};
        p->Add(new wText    {STR_LANG});
        p->Add(new wText    {0});
        p->Add(new wText    {STR_THEME});
        p->Add(new wText    {0});
        p->Add(new wCheckbox{STR_EXPERT,            ID_EXPERT_MODE});
        wPanels->Add(p);

        // Theme/lang
        /*p=new wPanel{3,ind++};
        p->Add(new wText    {STR_LANG});
        p->Add(new wText    {STR_LANG});
        p->Add(new wCheckbox{STR_EXPERT,            ID_EXPERT_MODE});
        wPanels->Add(p);*/

        // Actions
        p=new wPanel{4,ind++};
        p->Add(new wButton  {STR_OPENLOGS,          ID_OPENLOGS});
        p->Add(new wButton  {STR_SNAPSHOT,          ID_SNAPSHOT});
        p->Add(new wButton  {STR_EXTRACT,           ID_EXTRACT});
        p->Add(new wButton  {STR_DRVDIR,            ID_DRVDIR});
        wPanels->Add(p);

        // Filters (found)
        p=new wPanel{7,ind++,true};
        p->Add(new wText    {STR_SHOW_FOUND});
        p->Add(new wCheckbox{STR_SHOW_MISSING,      ID_SHOW_MISSING});
        p->Add(new wCheckbox{STR_SHOW_NEWER,        ID_SHOW_NEWER});
        p->Add(new wCheckbox{STR_SHOW_CURRENT,      ID_SHOW_CURRENT});
        p->Add(new wCheckbox{STR_SHOW_OLD,          ID_SHOW_OLD});
        p->Add(new wCheckbox{STR_SHOW_BETTER,       ID_SHOW_BETTER});
        p->Add(new wCheckbox{STR_SHOW_WORSE_RANK,   ID_SHOW_WORSE_RANK});
        wPanels->Add(p);

        // Filters (not found)
        p=new wPanel{4,ind++,true};
        p->Add(new wText    {STR_SHOW_NOTFOUND});
        p->Add(new wCheckbox{STR_SHOW_NF_MISSING,   ID_SHOW_NF_MISSING});
        p->Add(new wCheckbox{STR_SHOW_NF_UNKNOWN,   ID_SHOW_NF_UNKNOWN});
        p->Add(new wCheckbox{STR_SHOW_NF_STANDARD,  ID_SHOW_NF_STANDARD});
        wPanels->Add(p);

        // Filters (special)
        p=new wPanel{3,ind++,true};
        p->Add(new wCheckbox{STR_SHOW_ONE,          ID_SHOW_ONE});
        p->Add(new wCheckbox{STR_SHOW_DUP,          ID_SHOW_DUP});
        p->Add(new wCheckbox{STR_SHOW_INVALID,      ID_SHOW_INVALID});
        wPanels->Add(p);

        // Revision
        p=new wPanel{1,ind++};
        p->Add(new wTextRev);
        wPanels->Add(p);

        // Install button
        p=new wPanel{1,ind++};
        p->Add(new wButtonInst{STR_INSTALL,         ID_INSTALL});
        wPanels->Add(p);

        // Select all button
        p=new wPanel{1,ind++};
        p->Add(new wButton  {STR_SELECT_ALL,        ID_SELECT_ALL});
        wPanels->Add(p);

        // Select none button
        p=new wPanel{1,ind++};
        p->Add(new wButton  {STR_SELECT_NONE,       ID_SELECT_NONE});
        wPanels->Add(p);

        // Options
        p=new wPanel{3,ind++};
        p->Add(new wText    {STR_OPTIONS});
        p->Add(new wCheckbox{STR_RESTOREPOINT,      ID_RESTPNT});
        p->Add(new wCheckbox{STR_REBOOT,            ID_REBOOT});
        wPanels->Add(p);

        // Logo
        p=new wLogo{1,ind++};
        p->Add(new wText    {0});
        wPanels->Add(p);
    }
};
autorun obj;
void drawnew(Canvas &canvas)
{
    wPanels->arrange();
    wPanels->draw(canvas);
}

void wPanel::arrange()
{
    int ofsx=D(PNLITEM_OFSX),ofsy=D(PNLITEM_OFSY);
    int wy1=D(PANEL_WY+indofs);

    x1=Xm(D(PANEL_OFSX+indofs),D(PANEL_WX+indofs));
    y1=Ym(D(PANEL_OFSY+indofs));
    wx=XM(D(PANEL_WX+indofs),D(PANEL_OFSX+indofs));
    wy=wy1*sz+ofsy*2;

    if(!wy1)wy=0;

    for(int i=0;i<num;i++)
    {
        widgets[i]->flags=D(PANEL_OUTLINE_WIDTH+indofs)<0?1:0;
        widgets[i]->setboundbox(x1+ofsx,y1+ofsy+i*D(PNLITEM_WY),wx-ofsx*2,wy1);
    }

    if(D(PANEL_OUTLINE_WIDTH+indofs)<0)
    {
        flags=1;
        x1+=ofsx;
        y1+=ofsy;
        wx-=ofsx*2;
        wy-=ofsy*2;
    }
    else
        flags=0;
}

void Widget::hitscan(int x,int y)
{
    bool newisSelected=(x>=x1&&x<x1+wx&&y>=y1&&y<y1+wy);
    if(newisSelected!=isSelected)
    {
        invalidate();
        isSelected=newisSelected;
    }
}

void Widget::invalidate()
{
    RECT rect;

    rect.left=x1;
    rect.top=y1;
    rect.right=x1+wx;
    rect.bottom=y1+wy;

    if(rtl)
        InvalidateRect(MainWindow.hMain,nullptr,0);
    else
        InvalidateRect(MainWindow.hMain,&rect,0);
}

void wPanel::draw(Canvas &canvas)
{
    if(!wy)return;
    if(!Settings.expertmode&&isAdvanced)return;

    // Draw panel
    canvas.drawbox(x1,y1,x1+wx,y1+wy,(isSelected&&flags)?BOX_PANEL_H+index*2+2:BOX_PANEL+index*2+2);

    // Draw childs
    ClipRegion rgn;
    rgn.setRegion(x1,y1,x1+wx,y1+wy);
    canvas.setClipRegion(rgn);
    for(int i=0;i<num;i++)widgets[i]->draw(canvas);
    canvas.clearClipRegion();
}

void wText::draw(Canvas &canvas)
{
    canvas.setTextColor(D(isSelected?CHKBOX_TEXT_COLOR_H:CHKBOX_TEXT_COLOR));
    canvas.TextOutH(mirw(x1,0,wx),y1+0,STR(str_id));
    //canvas.drawrect(x1,y1,x1+wx,y1+wy,0xFF000000,0xFF,1,0);
}

void wTextRev::draw(Canvas &canvas)
{
    Version v{atoi(SVN_REV_D),atoi(SVN_REV_M),SVN_REV_Y};
    wchar_t buf[BUFLEN];

    wsprintf(buf,L"%s (",TEXT(SVN_REV2));
    v.str_date(buf+wcslen(buf));
    wcscat(buf,L")");if(rtl)wcscat(buf,L"\u200E");
    canvas.setTextColor(D(CHKBOX_TEXT_COLOR));
    canvas.TextOutH(mirw(x1,0,wx),y1,buf);
}

void wCheckbox::draw(Canvas &canvas)
{
    if(isSelected&&MainWindow.kbpanel)
    {
        //canvas.drawbox(x+ofsx,y,x+XP()-ofsx,y+ofsy+wy,BOX_KBHLT);
        //isSelected=false;
    }

    switch(action_id)
    {
        case ID_EXPERT_MODE:
            checked=Settings.expertmode;
            break;
        case ID_RESTPNT:
            break;

        case ID_REBOOT:
            break;

        default:
            checked=(Settings.filters&(1<<action_id))?true:false;

    }

    canvas.drawcheckbox(mirw(x1,0,wx-D(CHKBOX_SIZE)-2),y1,D(CHKBOX_SIZE)-2,D(CHKBOX_SIZE)-2,checked,isSelected);
    canvas.setTextColor(D(isSelected?CHKBOX_TEXT_COLOR_H:CHKBOX_TEXT_COLOR));
    canvas.TextOutH(mirw(x1,D(CHKBOX_TEXT_OFSX),wx),y1,STR(str_id));
}

void wButton::draw(Canvas &canvas)
{
    if(!flags)canvas.drawbox(x1,y1,x1+wx,y1+wy-1,isSelected?BOX_BUTTON_H:BOX_BUTTON);

    canvas.setTextColor(D(CHKBOX_TEXT_COLOR));
    canvas.TextOutH(mirw(x1,wy/2,wx),y1+(wy-D(FONT_SIZE)-2)/2,STR(str_id));
    //canvas.drawrect(x1,y1,x1+wx,y1+wy,0xFF000000,0xFF,1,0);
}

void wButtonInst::draw(Canvas &canvas)
{
    if(!flags)canvas.drawbox(x1,y1,x1+wx,y1+wy-1,isSelected?BOX_BUTTON_H:BOX_BUTTON);

    wchar_t buf[BUFLEN];
    canvas.setTextColor(D(CHKBOX_TEXT_COLOR));
    wsprintf(buf,L"%s (%d)",STR(str_id),manager_g->countItems());
    int nwy=D(PANEL9_OFSX)==D(PANEL10_OFSX)?D(PANEL10_WY):wy;
    canvas.TextOutH(mirw(x1,nwy/2,wx),y1+(wy-D(FONT_SIZE)-2)/2,buf);
}

void wTextSys1::draw(Canvas &canvas)
{
    canvas.setTextColor(D(CHKBOX_TEXT_COLOR));
    canvas.TextOutH(x1+10+SYSINFO_COL0,y1,STR(STR_SHOW_SYSINFO));
    canvas.TextOutH(x1+10+SYSINFO_COL1,y1,STR(STR_SYSINF_MOTHERBOARD));
    canvas.TextOutH(x1+10+SYSINFO_COL2,y1,STR(STR_SYSINF_ENVIRONMENT));
}

void wTextSys2::draw(Canvas &canvas)
{
    State *state=manager_g->matcher->getState();
    wchar_t buf[BUFLEN];

    wsprintf(buf,L"%s (%d-bit)",state->get_winverstr(),state->getArchitecture()?64:32);
    if(rtl)wcscat(buf,L"\u200E");
    canvas.TextOutH(x1+10+SYSINFO_COL0,y1,buf);
    canvas.TextOutH(x1+10+SYSINFO_COL1,y1,state->getProduct());
    canvas.TextOutH(x1+10+SYSINFO_COL2,y1,STR(STR_SYSINF_WINDIR));
    canvas.TextOutH(x1+10+SYSINFO_COL3,y1,state->textas.getw(state->getWindir()));
}

void wTextSys3::draw(Canvas &canvas)
{
    State *state=manager_g->matcher->getState();
    wchar_t buf[BUFLEN];

    wsprintf(buf,L"%s",(wx<10+SYSINFO_COL1)?state->getProduct():state->get_szCSDVersion());
    if(rtl)wcscat(buf,L"\u200E");
    canvas.TextOutH(x1+10+SYSINFO_COL0,y1,buf);
    wsprintf(buf,L"%s: %s",STR(STR_SYSINF_TYPE),STR(state->isLaptop?STR_SYSINF_LAPTOP:STR_SYSINF_DESKTOP));
    canvas.TextOutH(x1+10+SYSINFO_COL1,y1,buf);
    canvas.TextOutH(x1+10+SYSINFO_COL2,y1,STR(STR_SYSINF_TEMP));
    canvas.TextOutH(x1+10+SYSINFO_COL3,y1,state->textas.getw(state->getTemp()));
}

//{ Accepters
void Widget::Accept(WidgetVisitor &visitor)
{
    visitor.VisitWidget(this);
}

void WidgetComposite::Accept(WidgetVisitor &visitor)
{
    visitor.VisitWidget(this);
    for(int i=0;i<num;i++)widgets[i]->Accept(visitor);
}

void wPanel::Accept(WidgetVisitor &visitor)
{
    if(!Settings.expertmode&&isAdvanced)return;

    visitor.VisitWidget(this);
    for(int i=0;i<num;i++)widgets[i]->Accept(visitor);
}

void wText::Accept(WidgetVisitor &visitor)
{
    visitor.VisitwText(this);
}

void wCheckbox::Accept(WidgetVisitor &visitor)
{
    visitor.VisitwCheckbox(this);
}

void wButton::Accept(WidgetVisitor &visitor)
{
    visitor.VisitwButton(this);
}

void wLogo::Accept(WidgetVisitor &visitor)
{
    visitor.VisitwLogo(this);
}

void wTextRev::Accept(WidgetVisitor &visitor)
{
    visitor.VisitwTextRev(this);
}

void wTextSys1::Accept(WidgetVisitor &visitor)
{
    visitor.VisitwTextSys1(this);
}
//}

//{ HoverVisiter
HoverVisiter::~HoverVisiter()
{
    if(popup_active==false)
        drawpopup(-1,FLOATING_NONE,x,y,MainWindow.hMain);
}

void HoverVisiter::VisitWidget(Widget *a)
{
    a->hitscan(x,y);
    if(a->isSelected&&a->str_id)
    {
        drawpopup(a->str_id+1,FLOATING_TOOLTIP,x,y,MainWindow.hMain);
        popup_active=true;
    }
}

void HoverVisiter::VisitWidgetComposite(WidgetComposite *a)
{
    a->hitscan(x,y);
}

void HoverVisiter::VisitwText(wText *a)
{
    VisitWidget(a);
    a->isSelected=0;
}

void HoverVisiter::VisitwLogo(wLogo *a)
{
    a->hitscan(x,y);
    if(a->isSelected)
    {
        SetCursor(LoadCursor(nullptr,IDC_HAND));
        drawpopup(-1,FLOATING_ABOUT,x,y,MainWindow.hMain);
        popup_active=true;
    }
}

void HoverVisiter::VisitwTextRev(wTextRev *a)
{
    a->hitscan(x,y);
    if(a->isSelected)
    {
        SetCursor(LoadCursor(nullptr,IDC_HAND));
        drawpopup(-1,FLOATING_ABOUT,x,y,MainWindow.hMain);
        popup_active=true;
    }
}

void HoverVisiter::VisitwTextSys1(wTextSys1 *a)
{
    a->hitscan(x,y);
    if(a->isSelected)
    {
        SetCursor(LoadCursor(nullptr,IDC_HAND));
        drawpopup(a->str_id,FLOATING_SYSINFO,x,y,MainWindow.hMain);
        popup_active=true;
    }
}
//}

//{ ClickVisitor
ClickVisiter::~ClickVisiter()
{
    if(Settings.filters!=sum&&Settings.expertmode)
    {
        Settings.filters=sum;
        manager_g->filter(Settings.filters);
        manager_g->setpos();
    }
}

void ClickVisiter::VisitwCheckbox(wCheckbox *a)
{
    a->hitscan(x,y);
    if(a->isSelected)
    {
        a->checked^=1;
        a->invalidate();
        InvalidateRect(MainWindow.hMain,nullptr,0);
        if(a->action_id==ID_EXPERT_MODE)
        {
            Settings.expertmode=a->checked;
            ShowWindow(GetConsoleWindow(),Settings.expertmode||MainWindow.ctrl_down?SW_SHOWNOACTIVATE:MainWindow.hideconsole);
            InvalidateRect(MainWindow.hMain,nullptr,0);
        }
    }
    if(a->checked)switch(a->action_id)
    {
        case ID_REBOOT:
        case ID_RESTPNT:
        case ID_EXPERT_MODE:
            break;

        default:
            sum+=1<<a->action_id;
    }
}

void ClickVisiter::VisitwButton(wButton *a)
{
    a->hitscan(x,y);
    if(a->isSelected)
    {
        PostMessage(MainWindow.hMain,WM_COMMAND,a->action_id+(BN_CLICKED<<16),0);
        InvalidateRect(MainWindow.hMain,nullptr,0);
    }
}

void ClickVisiter::VisitwLogo(wLogo *a)
{
    a->hitscan(x,y);
    if(a->isSelected)
        System.run_command(L"open",L"http://snappy-driver-installer.sourceforge.net",SW_SHOWNORMAL,0);
}

void ClickVisiter::VisitwTextRev(wTextRev *a)
{
    a->hitscan(x,y);
    if(a->isSelected)
        System.run_command(L"open",L"http://snappy-driver-installer.sourceforge.net",SW_SHOWNORMAL,0);
}

void ClickVisiter::VisitwTextSys1(wTextSys1 *a)
{
    a->hitscan(x,y);
    if(a->isSelected)
        System.run_command(L"devmgmt.msc",nullptr,SW_SHOW,0);
}
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
            if(!r)Log.print_err("ERROR in box_init(): failed DeleteDC\n");
        r=DeleteObject(bitmap);
            if(!r)Log.print_err("ERROR in box_init(): failed DeleteObject\n");
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
        Log.print_err("ERROR in image_loadFile(): file '%S' not found\n",buf);
        return;
    }
    fseek(f,0,SEEK_END);
    sz=ftell(f);
    fseek(f,0,SEEK_SET);
    std::unique_ptr<BYTE[]> imgbuf(new BYTE[sz]);

    sz=fread(imgbuf.get(),1,sz,f);
    if(!sz)
    {
        Log.print_err("ERROR in image_loadFile(): cannnot read from file '%S'\n",buf);
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
        Log.print_err("ERROR in image_loadRes(): failed get_resource\n");
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
        Log.print_err("ERROR in image_load(): failed WebPGetInfo(%d)\n",ret);
        return;
    }
    big=WebPDecodeBGRA(data,sz,&sx,&sy);
    if(!big)
    {
        Log.print_err("ERROR in image_load(): failed WebPDecodeBGRA\n");
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
//    Log.print_con("%dx%d:%d,%d\n",sx,sy,hasalpha,index);
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
void Canvas::drawconnection(int x1,int pos,int ofsy,int curpos)
{
    HPEN oldpen,newpen;

    newpen=CreatePen(PS_SOLID,D(DRVITEM_LINE_WIDTH),D(DRVITEM_LINE_COLOR));
    oldpen=(HPEN)SelectObject(hdcMem,newpen);
    MoveToEx(hdcMem,x1-D(DRVITEM_LINE_INTEND)/2,curpos-D(DRVITEM_DIST_Y0)+D(DRVITEM_WY)-ofsy,nullptr);
    LineTo(hdcMem,x1-D(DRVITEM_LINE_INTEND)/2,pos+D(DRVITEM_WY)/2);
    LineTo(hdcMem,x1,pos+D(DRVITEM_WY)/2);
    SelectObject(hdcMem,oldpen);
    DeleteObject(newpen);
}

Canvas::Canvas():
    x(0),
    y(0),
    localDC(nullptr),
    hdcMem(CreateCompatibleDC(nullptr)),
    bitmap(nullptr),
    oldbitmap(nullptr),
    //ps(nullptr),
    hwnd(nullptr),
    clipping(nullptr)
{
    if(!hdcMem)Log.print_err("ERROR in canvas_init(): failed CreateCompatibleDC\n");
    int r=SetBkMode(hdcMem,TRANSPARENT);
    if(!r)Log.print_err("ERROR in canvas_init(): failed SetBkMode\n");
}

Canvas::~Canvas()
{
    if(hdcMem)
    {
        int r=DeleteDC(hdcMem);
        if(!r)Log.print_err("ERROR in canvas_free(): failed DeleteDC\n");
        hdcMem=nullptr;
    }

    if(bitmap)
    {
        //r=(int)SelectObject(hdcMem,oldbitmap);
        //if(!r)Log.log_err("ERROR in canvas_free(): failed SelectObject\n");
        int r=DeleteObject(bitmap);
        if(!r)Log.print_err("ERROR in canvas_free(): failed DeleteObject\n");
        bitmap=nullptr;
    }
}

void Canvas::begin(HWND nhwnd,int nx,int ny)
{
    unsigned r32;

    hwnd=nhwnd;
    localDC=BeginPaint(hwnd,&ps);
    if(!localDC)Log.print_err("ERROR in canvas_begin(): failed BeginPaint\n");

    if(x!=nx||y!=ny)
    {
        x=nx;
        y=ny;
        if(bitmap)
        {
            HGDIOBJ r=SelectObject(hdcMem,oldbitmap);
            if(!r)Log.print_err("ERROR in canvas_begin(): failed SelectObject(oldbitmap)\n");
            r32=DeleteObject(bitmap);
            if(!r32)Log.print_err("ERROR in canvas_begin(): failed DeleteObject\n");
        }
        bitmap=CreateCompatibleBitmap(localDC,x,y);
        if(!bitmap)Log.print_err("ERROR in canvas_begin(): failed CreateCompatibleBitmap\n");
        oldbitmap=(HBITMAP)SelectObject(hdcMem,bitmap);
        if(!oldbitmap)Log.print_err("ERROR in canvas_begin(): failed SelectObject(bitmap)\n");
    }
    clipping=CreateRectRgnIndirect(&ps.rcPaint);
    if(!clipping)Log.print_err("ERROR in canvas_begin(): failed BeginPaint\n");
    SetStretchBltMode(hdcMem,HALFTONE);
    r32=SelectClipRgn(hdcMem,clipping);
    if(!r32)Log.print_err("ERROR in canvas_begin(): failed SelectClipRgn\n");
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
    if(!r)Log.print_err("ERROR in canvas_end(): failed BitBlt\n");
    r=DeleteObject(clipping);
    if(!r)Log.print_err("ERROR in canvas_end(): failed DeleteObject\n");
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

    if(MainWindow.kbpanel&&items[0].str_id==MainWindow.kbpanel)
    {
        if(MainWindow.kbpanel==KB_INSTALL)
        {
            if(MainWindow.kbitem[MainWindow.kbpanel]>2)MainWindow.kbitem[MainWindow.kbpanel]=0;
            if(index-8!=MainWindow.kbitem[MainWindow.kbpanel])return -1;
            return index-8==MainWindow.kbitem[MainWindow.kbpanel];
        }
        if(MainWindow.kbitem[MainWindow.kbpanel]>items[0].action_id)MainWindow.kbitem[MainWindow.kbpanel]=0;
        while(items[MainWindow.kbitem[MainWindow.kbpanel]].type!=TYPE_CHECKBOX&&
              items[MainWindow.kbitem[MainWindow.kbpanel]].type!=TYPE_BUTTON)MainWindow.kbitem[MainWindow.kbpanel]++;

        return MainWindow.kbitem[MainWindow.kbpanel];
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
    if(MainWindow.kbpanel&&items[0].str_id==MainWindow.kbpanel)
    {
        MainWindow.kbitem[MainWindow.kbpanel]+=v;

        while(MainWindow.kbitem[MainWindow.kbpanel]>=0&&MainWindow.kbitem[MainWindow.kbpanel]<=items[0].action_id&&
              items[MainWindow.kbitem[MainWindow.kbpanel]].type!=TYPE_CHECKBOX&&
              items[MainWindow.kbitem[MainWindow.kbpanel]].type!=TYPE_BUTTON)
              MainWindow.kbitem[MainWindow.kbpanel]+=v;

        if(MainWindow.kbitem[MainWindow.kbpanel]>items[0].action_id)MainWindow.kbitem[MainWindow.kbpanel]=0;
        if(MainWindow.kbitem[MainWindow.kbpanel]<0)MainWindow.kbitem[MainWindow.kbpanel]=items[0].action_id;
    }

}

/*void Panel::draw_inv()
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
        InvalidateRect(MainWindow.hMain,nullptr,0);
    else
        InvalidateRect(MainWindow.hMain,&rect,0);
}*/

/*int Panel::calcFilters()
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
            items[i].checked=(filters_&(1<<items[i].action_id))?1:0;
}*/

void Panel::moveWindow(HWND hwnd,int i,int j,int f)
{
    MoveWindow(hwnd,Xp()+i,Yp()+j*D(PNLITEM_WY)-2+f,XP()-i-D(PNLITEM_OFSX),190*2,0);
}

/*void Panel::click(int i)
{
    if(items[i].type==TYPE_CHECKBOX||items[i].type==TYPE_BUTTON)
    {
        flipChecked(i);
        if(items[i].action_id==ID_EXPERT_MODE)
        {
            Settings.expertmode=isChecked(i);
            ShowWindow(GetConsoleWindow(),Settings.expertmode&&MainWindow.ctrl_down?SW_SHOWNOACTIVATE:MainWindow.hideconsole);
        }
        else
            PostMessage(MainWindow.hMain,WM_COMMAND,items[i].action_id+(BN_CLICKED<<16),0);

        InvalidateRect(MainWindow.hMain,nullptr,0);
    }
}*/

/*void Panel::draw(Canvas &canvas)
{
    wchar_t buf[BUFLEN];
    POINT p;
    ClipRegion rgn;
    int cur_i;
    int i;
    int x=Xp(),y=Yp();
    int ofsx=D(PNLITEM_OFSX),ofsy=D(PNLITEM_OFSY);
    int wy=D(PANEL_WY+indofs);

    if(XP()<0)return;
    if(!D(PANEL_WY+indofs))return;

    GetCursorPos(&p);
    ScreenToClient(MainWindow.hMain,&p);
    cur_i=hitscan(p.x,p.y);

    for(i=0;i<items[0].action_id+1;i++)
    {
        bool isSelected=i==cur_i;

        State *state=manager_g->matcher->getState();
        canvas.setTextColor(D(CHKBOX_TEXT_COLOR));
        // System Info (1st line)
        if(i==1&&index==0)
        {
            canvas.TextOutH(x+ofsx+SYSINFO_COL1,y+ofsy,STR(STR_SYSINF_MOTHERBOARD));
            canvas.TextOutH(x+ofsx+SYSINFO_COL2,y+ofsy,STR(STR_SYSINF_ENVIRONMENT));
        }

        // System Info (2nd line)
        if(i==2&&index==0)
        {
            wsprintf(buf,L"%s (%d-bit)",state->get_winverstr(),state->getArchitecture()?64:32);
            if(rtl)wcscat(buf,L"\u200E");
            canvas.TextOutH(x+ofsx+10+SYSINFO_COL0,y+ofsy,buf);
            canvas.TextOutH(x+ofsx+10+SYSINFO_COL1,y+ofsy,state->getProduct());
            canvas.TextOutH(x+ofsx+10+SYSINFO_COL2,y+ofsy,STR(STR_SYSINF_WINDIR));
            canvas.TextOutH(x+ofsx+10+SYSINFO_COL3,y+ofsy,state->textas.getw(state->getWindir()));
        }

        // System Info (3rd line)
        if(i==3&&index==0)
        {
            wsprintf(buf,L"%s",(XP()<10+SYSINFO_COL1)?state->getProduct():state->get_szCSDVersion());
            if(rtl)wcscat(buf,L"\u200E");
            canvas.TextOutH(x+ofsx+10+SYSINFO_COL0,y+ofsy,buf);
            wsprintf(buf,L"%s: %s",STR(STR_SYSINF_TYPE),STR(state->isLaptop?STR_SYSINF_LAPTOP:STR_SYSINF_DESKTOP));
            canvas.TextOutH(x+ofsx+10+SYSINFO_COL1,y+ofsy,buf);
            canvas.TextOutH(x+ofsx+10+SYSINFO_COL2,y+ofsy,STR(STR_SYSINF_TEMP));
            canvas.TextOutH(x+ofsx+10+SYSINFO_COL3,y+ofsy,state->textas.getw(state->getTemp()));
        }
        if(items[i].type==TYPE_GROUP_BREAK&&!Settings.expertmode)break;
        switch(items[i].type)
        {
            case TYPE_CHECKBOX:
                if(isSelected&&MainWindow.kbpanel)
                {
                    canvas.drawbox(x+ofsx,y,x+XP()-ofsx,y+ofsy+wy,BOX_KBHLT);
                    isSelected=false;
                }
                canvas.drawcheckbox(mirw(x,ofsx,XP()-D(CHKBOX_SIZE)-2),y+ofsy,D(CHKBOX_SIZE)-2,D(CHKBOX_SIZE)-2,items[i].checked,isSelected);
                canvas.setTextColor(D(isSelected?CHKBOX_TEXT_COLOR_H:CHKBOX_TEXT_COLOR));
                canvas.TextOutH(mirw(x,D(CHKBOX_TEXT_OFSX)+ofsx,XP()-ofsx*2),y+ofsy,STR(items[i].str_id));
                //if(i==cur_i&&kbpanel)drawrectsel(hdc,x+ofsx,y+ofsy,x+XP()-ofsx,y+ofsy+wy,0xff00,1);
                y+=D(PNLITEM_WY);
                break;

            case TYPE_BUTTON:
                if(index>=8&&index<=10&&D(PANEL_OUTLINE_WIDTH+indofs)<0)
                    canvas.drawbox(x+ofsx,y+ofsy,x+XP()-ofsx,y+ofsy+wy,isSelected?BOX_PANEL_H+index*2+2:BOX_PANEL+index*2+2);
                else
                    canvas.drawbox(x+ofsx,y+ofsy,x+XP()-ofsx,y+ofsy+wy-1,isSelected?BOX_BUTTON_H:BOX_BUTTON);

                canvas.setTextColor(D(CHKBOX_TEXT_COLOR));

                if(i==1&&index==8) // Install button
                {
                    wsprintf(buf,L"%s (%d)",STR(items[i].str_id),manager_g->countItems());
                    int nwy=D(PANEL9_OFSX)==D(PANEL10_OFSX)?D(PANEL10_WY):wy;
                    canvas.TextOutH(mirw(x,ofsx+nwy/2,XP()),y+ofsy+(wy-D(FONT_SIZE)-2)/2,buf);
                }
                else
                    canvas.TextOutH(mirw(x,ofsx+wy/2,XP()),y+ofsy+(wy-D(FONT_SIZE)-2)/2,STR(items[i].str_id));

                y+=D(PNLITEM_WY);
                break;

            case TYPE_TEXT:
                if(i==1&&index==7) // Revision number
                {
                    Version v{atoi(SVN_REV_D),atoi(SVN_REV_M),SVN_REV_Y};


                    wsprintf(buf,L"%s (",TEXT(SVN_REV2));
                    v.str_date(buf+wcslen(buf));
                    wcscat(buf,L")");if(rtl)wcscat(buf,L"\u200E");
                    canvas.setTextColor(D(CHKBOX_TEXT_COLOR));
                    canvas.TextOutH(mirw(x,ofsx,XP()),y+ofsy,buf);
                }
                canvas.setTextColor(D(isSelected&&i>11?CHKBOX_TEXT_COLOR_H:CHKBOX_TEXT_COLOR));
                canvas.TextOutH(mirw(x,ofsx,XP()),y+ofsy,STR(items[i].str_id));
                y+=D(PNLITEM_WY);
                break;

            case TYPE_GROUP_BREAK:
            case TYPE_GROUP:
                if(index>=8&&index<=10)break;
                if(i)y+=D(PNLITEM_WY);
                canvas.drawbox(x,y,x+XP(),y+(wy)*items[i].action_id+ofsy*2,
                         BOX_PANEL+index*2+2);
                rgn.setRegion(x,y,x+XP(),y+(wy)*items[i].action_id+ofsy*2);
                canvas.setClipRegion(rgn);
                break;

            default:
                break;
        }

    }
    canvas.clearClipRegion();
}*/
//}

//{ Text
textdata_t::textdata_t(Canvas &canvas_,int xofs):
    hdcMem(canvas_.getDC()),
    pcanvas(&canvas_),
    ofsx(xofs),
    wy(D(POPUP_WY)),
    maxsz(0),
    col(D(POPUP_TEXT_COLOR)),
    x(D(POPUP_OFSX)+xofs),
    y(D(POPUP_OFSY))
{
}

void textdata_t::ret()
{
    x=D(POPUP_OFSX)+ofsx;
}

void textdata_t::ret_ofs(int a)
{
    x=D(POPUP_OFSX)+a;
}

void textdata_t::nl()
{
    y+=wy;
}

void textdata_horiz_t::limitskip()
{
    x+=limits[i++];
}

void textdata_vert::shift_r(){maxsz+=POPUP_SYSINFO_OFS;}
void textdata_vert::shift_l(){maxsz-=POPUP_SYSINFO_OFS;}

void textdata_t::TextOut_CM(int x1,int y1,const wchar_t *str,int color,int *maxsz1,int mode1)
{
    SIZE ss;
    GetTextExtentPoint32(hdcMem,str,wcslen(str),&ss);
    if(ss.cx>*maxsz1)*maxsz1=ss.cx;

    if(!mode1)return;
    pcanvas->setTextColor(color);
    pcanvas->TextOutH(x1,y1,str);
}

void textdata_horiz_t::TextOutP(const wchar_t *format,...)
{
    wchar_t buffer[BUFLEN];
    va_list args;
    va_start(args,format);
    _vsnwprintf(buffer,BUFLEN,format,args);

    TextOut_CM(x,y,buffer,col,&limits[i],mode);
    x+=limits[i];
    i++;
    va_end(args);
}

void textdata_t::TextOutF(int col1,const wchar_t *format,...)
{
    wchar_t buffer[BUFLEN];
    va_list args;
    va_start(args,format);
    _vsnwprintf(buffer,BUFLEN,format,args);

    TextOut_CM(x,y,buffer,col1,&maxsz,1);
    y+=wy;
    va_end(args);
}

void textdata_t::TextOutF(const wchar_t *format,...)
{
    wchar_t buffer[BUFLEN];
    va_list args;
    va_start(args,format);
    _vsnwprintf(buffer,BUFLEN,format,args);

    TextOut_CM(x,y,buffer,col,&maxsz,1);
    y+=wy;
    va_end(args);
}

void textdata_vert::TextOutSF(const wchar_t *str,const wchar_t *format,...)
{
    wchar_t buffer[BUFLEN];
    va_list args;
    va_start(args,format);
    _vsnwprintf (buffer,BUFLEN,format,args);
    TextOut_CM(x,y,str,col,&maxsz,1);
    TextOut_CM(x+POPUP_SYSINFO_OFS,y,buffer,col,&maxsz,1);
    y+=wy;
    va_end(args);
}
//}

//{Popup
void popup_resize(int x,int y)
{
    if(Popup.floating_x!=x||Popup.floating_y!=y)
    {
        POINT p1;

        Popup.floating_x=x;
        Popup.floating_y=y;
        GetCursorPos(&p1);
        SetCursorPos(p1.x+1,p1.y);
        SetCursorPos(p1.x,p1.y);
    }
}

void popup_about(Canvas &canvas)
{
    textdata_vert td(canvas);

    td.ret();
    td.TextOutF(L"Snappy Driver Installer %s",STR(STR_ABOUT_VER));
    td.nl();

    RECT rect;
    rect.left=td.getX();
    rect.top=td.getY();
    rect.right=D(POPUP_WX)-D(POPUP_OFSX)*2;
    rect.bottom=900;
    DrawText(canvas.getDC(),STR(STR_ABOUT_LICENSE),-1,&rect,DT_WORDBREAK|DT_CALCRECT);
    DrawText(canvas.getDC(),STR(STR_ABOUT_LICENSE),-1,&rect,DT_WORDBREAK);

    td.nl();
    td.nl();
    td.nl();
    td.TextOutF(L"%s%s",STR(STR_ABOUT_DEV_TITLE),STR(STR_ABOUT_DEV_LIST));
    td.TextOutF(L"%s%s",STR(STR_ABOUT_TESTERS_TITLE),STR(STR_ABOUT_TESTERS_LIST));

    popup_resize(D(POPUP_WX),rect.bottom+D(POPUP_OFSY));
}

void format_size(wchar_t *buf,long long val,int isspeed)
{
#ifdef USE_TORRENT
    StrFormatSize(val,buf,BUFLEN);
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
void Canvas::TextOutH(int x1,int y1,LPCTSTR buf)
{
    TextOut(hdcMem,x1,y1,buf,wcslen(buf));
}
int Xm(int x,int o)
{
    UNREFERENCED_PARAMETER(o)
    return x>=0?x:(MainWindow.main1x_c+x);
}
int Ym(int y){return y>=0?y:(MainWindow.main1y_c+y);}
int XM(int w,int x){return w>=0?w:(w+MainWindow.main1x_c-x);}
int YM(int y,int o){return y>=0?y:(MainWindow.main1y_c+y-o);}

int Xg(int x,int o)
{
    UNREFERENCED_PARAMETER(o)
    return x>=0?x:(MainWindow.mainx_c+x);
}
int Yg(int y){return y>=0?y:(MainWindow.mainy_c+y);}
int XG(int x,int o){return x>=0?x:(MainWindow.mainx_c+x-o);}
int YG(int y,int o){return y>=0?y:(MainWindow.mainy_c+y-o);}

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

//{ ClipRegion
class ClipRegionImp
{
    ClipRegionImp(const ClipRegionImp&)=delete;
    operator=(const ClipRegionImp&)=delete;

private:
    HRGN hrgn;
public:
    ClipRegionImp():hrgn(nullptr){}
    ClipRegionImp(int x1,int y1,int x2,int y2);
    ClipRegionImp(int x1,int y1,int x2,int y2,int r);
    ~ClipRegionImp();

    void setRegion(int x1,int y1,int x2,int y2);

    friend class Canvas;
};

ClipRegionImp::ClipRegionImp(int x1,int y1,int x2,int y2):
    hrgn(CreateRectRgn(x1,y1,x2,y2))
{
    if(!hrgn)Log.print_err("ERROR in ClipRegion(): failed CreateRectRgn\n");
}

ClipRegionImp::ClipRegionImp(int x1,int y1,int x2,int y2,int r):
    hrgn(CreateRoundRectRgn(x1,y1,x2,y2,r,r))
{
    if(!hrgn)Log.print_err("ERROR in ClipRegion(): failed CreateRoundRectRgn\n");
}

void ClipRegionImp::setRegion(int x1,int y1,int x2,int y2)
{
    if(hrgn)DeleteObject(hrgn);
    hrgn=CreateRectRgn(x1,y1,x2,y2);
    if(!hrgn)Log.print_err("ERROR in ClipRegion(): failed setRegion\n");
}

ClipRegionImp::~ClipRegionImp()
{
    if(hrgn)DeleteObject(hrgn);
}

ClipRegion::ClipRegion(int x1,int y1,int x2,int y2):imp(new ClipRegionImp(x1,y1,x2,y2)){}
ClipRegion::ClipRegion(int x1,int y1,int x2,int y2,int r):imp(new ClipRegionImp(x1,y1,x2,y2,r)){}
ClipRegion::ClipRegion():imp(new ClipRegionImp()){}
ClipRegion::~ClipRegion(){delete imp;}
void ClipRegion::setRegion(int x1,int y1,int x2,int y2){imp->setRegion(x1,y1,x2,y2);}
//}

void Font::SetFont(const wchar_t *name,int size,int bold)
{
    if(hFont&&!DeleteObject(hFont))
        Log.print_err("ERROR in setfont(): failed DeleteObject\n");

    hFont=CreateFont(-size,0,0,0,bold?FW_BOLD:FW_DONTCARE,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,
                CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,VARIABLE_PITCH,name);

    if(!hFont)Log.print_err("ERROR in setfont(): failed CreateFont\n");
}

Font::~Font()
{
    if(hFont&&!DeleteObject(hFont))
        Log.print_err("ERROR in manager_free(): failed DeleteObject\n");
}

void Canvas::setTextColor(int color)
{
    SetTextColor(hdcMem,color);
}

void Canvas::setFont(Font *font)
{
    SelectObject(hdcMem,font->hFont);
}

void Canvas::setClipRegion(ClipRegion &clip)
{
    SelectClipRgn(hdcMem,clip.imp->hrgn);
}

void Canvas::clearClipRegion()
{
    SelectClipRgn(hdcMem,nullptr);
}

void Canvas::calcRect(const wchar_t *str,RECT *rect)
{
    DrawText(hdcMem,str,-1,rect,DT_WORDBREAK|DT_CALCRECT);
}

void loadGUID(GUID *g,const char *s)
{
    char d[3];
    d[2]=0;
    g->Data1=strtol(s+1,nullptr,16);
    g->Data2=strtol(s+10,nullptr,16);
    g->Data3=strtol(s+15,nullptr,16);
    memcpy(d,s+15+5,2);g->Data4[0]=strtol(d,nullptr,16);
    memcpy(d,s+15+5+2,2);g->Data4[1]=strtol(d,nullptr,16);
    memcpy(d,s+15+6+4,2);g->Data4[2]=strtol(d,nullptr,16);
    memcpy(d,s+15+6+6,2);g->Data4[3]=strtol(d,nullptr,16);
    memcpy(d,s+15+6+8,2);g->Data4[4]=strtol(d,nullptr,16);
    memcpy(d,s+15+6+10,2);g->Data4[5]=strtol(d,nullptr,16);
    memcpy(d,s+15+6+12,2);g->Data4[6]=strtol(d,nullptr,16);
    memcpy(d,s+15+6+14,2);g->Data4[7]=strtol(d,nullptr,16);

    /*Log.print_con("%s\n",s);
    Log.print_con("{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}\n\n",g->Data1,g->Data2,g->Data3,
        (int)(g->Data4[0]),(int)(g->Data4[1]),
        (int)(g->Data4[2]),(int)(g->Data4[3]),(int)(g->Data4[4]),
        (int)(g->Data4[5]),(int)(g->Data4[6]),(int)(g->Data4[7]));*/
}

void Canvas::drawIcon(int x1,int y1,const char *guid_driverpack,const GUID *guid_device)
{
    HICON hIcon;
    bool ret=false;
    if(guid_driverpack)
    {
        GUID gd;
        loadGUID(&gd,guid_driverpack);
        ret=SetupDiLoadClassIcon(&gd,&hIcon,nullptr);
    }
    if(!ret)ret=SetupDiLoadClassIcon(guid_device,&hIcon,nullptr);
    if(ret)
    {
        if(rtl)
        {
            HICON miricon;
            miricon=CreateMirroredIcon(hIcon);
            DestroyIcon(hIcon);
            hIcon=miricon;
        }
        DrawIconEx(hdcMem,x1,y1,hIcon,D(ITEM_ICON_SIZE),D(ITEM_ICON_SIZE),0,nullptr,DI_NORMAL);
        DestroyIcon(hIcon);
    }
}

void Canvas::drawRect(int x1,int y1,int x2,int y2,int color)
{
    SelectObject(hdcMem,GetStockObject(DC_BRUSH));
    SelectObject(hdcMem,GetStockObject(DC_PEN));
    SetDCBrushColor(hdcMem,color);
    Rectangle(hdcMem,x1,y1,x2,y2);
}

void Canvas::drawLine(int x1,int y1,int x2,int y2)
{
    MoveToEx(hdcMem,x1,y1,nullptr);
    LineTo(hdcMem,x2,y2);
}

void Canvas::drawrect(int x1,int y1,int x2,int y2,int color1,int color2,int w,int rn)
{
    HPEN newpen,oldpen;
    HBRUSH newbrush,oldbrush;
    HGDIOBJ r;
    unsigned r32;

    //oldbrush=(HBRUSH)SelectObject(hdc,GetStockObject(color1&0xFF000000?NULL_BRUSH:DC_BRUSH));
    newbrush=CreateSolidBrush(color1);
    oldbrush=(HBRUSH)SelectObject(hdcMem,newbrush);
    if(color1&0xFF000000)(HBRUSH)SelectObject(hdcMem,GetStockObject(NULL_BRUSH));

    if(!oldbrush)Log.print_err("ERROR in drawrect(): failed SelectObject(GetStockObject)\n");
    r32=SetDCBrushColor(hdcMem,color1);
    if(r32==CLR_INVALID)Log.print_err("ERROR in drawrect(): failed SetDCBrushColor\n");

    newpen=CreatePen(w?PS_SOLID:PS_NULL,w,color2);
    if(!newpen)Log.print_err("ERROR in drawrect(): failed CreatePen\n");
    oldpen=(HPEN)SelectObject(hdcMem,newpen);
    if(!oldpen)Log.print_err("ERROR in drawrect(): failed SelectObject(newpen)\n");

    if(rn)
        RoundRect(hdcMem,x1,y1,x2,y2,rn,rn);
    else
        Rectangle(hdcMem,x1,y1,x2,y2);

    r=SelectObject(hdcMem,oldpen);
    if(!r)Log.print_err("ERROR in drawrect(): failed SelectObject(oldpen)\n");
    r=SelectObject(hdcMem,oldbrush);
    if(!r)Log.print_err("ERROR in drawrect(): failed SelectObject(oldbrush)\n");
    r32=DeleteObject(newpen);
    if(!r32)Log.print_err("ERROR in drawrect(): failed DeleteObject(newpen)\n");
    r32=DeleteObject(newbrush);
    if(!r32)Log.print_err("ERROR in drawrect(): failed DeleteObject(newbrush)\n");
}

void Canvas::drawbox(int x1,int y1,int x2,int y2,int id)
{
    if(id<0||id>=BOX_NUM)
    {
        Log.print_err("ERROR in box_draw(): invalid id=%d\n",id);
        return;
    }
    int i=boxindex[id];
    if(i<0||i>=THEME_NM)
    {
        Log.print_err("ERROR in box_draw(): invalid index=%d\n",i);
        return;
    }
    drawrect(x1,y1,x2,y2,D(i),D(i+1),D(i+2),D(i+3));
    box[id].draw(hdcMem,x1,y1,x2,y2,D(i+5),D(i+6));
}

void Canvas::drawcheckbox(int x1,int y1,int wx,int wy,int checked,int active)
{
    RECT rect;
    int i=4+(active?1:0)+(checked?2:0);

    rect.left=x1;
    rect.top=y1;
    rect.right=x1+wx;
    rect.bottom=y1+wy;

    if(icon[i].isLoaded())
        icon[i].draw(hdcMem,x1,y1,x1+wx,y1+wy,0,Image::HSTR|Image::VSTR);
    else
        DrawFrameControl(hdcMem,&rect,DFC_BUTTON,DFCS_BUTTONCHECK|(checked?DFCS_CHECKED:0));
}

void drawpopup(int itembar,int type,int x,int y,HWND hwnd)
{
    POINT p={x,y};
    HMONITOR hMonitor;
    MONITORINFO mi;
    int needupdate;

    if((type==FLOATING_CMPDRIVER||type==FLOATING_DRIVERLST)&&itembar<0)type=FLOATING_NONE;
    if(type==FLOATING_TOOLTIP&&(itembar<=1||!*STR(itembar)))type=FLOATING_NONE;

    if(rtl)p.x+=Popup.floating_x;
    ClientToScreen(hwnd,&p);
    needupdate=Popup.floating_itembar!=itembar||Popup.floating_type!=type;
    Popup.floating_itembar=itembar;
    Popup.floating_type=type;

    if(type!=FLOATING_NONE)
    {
        //if(type==FLOATING_ABOUT)p.y=p.y-floating_y-30;
        //if(type==FLOATING_CMPDRIVER||type==FLOATING_DRIVERLST)
        {
            hMonitor=MonitorFromPoint(p,MONITOR_DEFAULTTONEAREST);
            mi.cbSize=sizeof(MONITORINFO);
            GetMonitorInfo(hMonitor,&mi);

            mi.rcWork.right-=15;
            if(p.x+Popup.floating_x>mi.rcWork.right)p.x=mi.rcWork.right-Popup.floating_x;
            if(p.x<5)p.x=5;
            if(p.y+Popup.floating_y>mi.rcWork.bottom-20)p.y=p.y-Popup.floating_y-30;
            if(p.y<5)p.y=5;
        }

        MoveWindow(Popup.hPopup,p.x+10,p.y+20,Popup.floating_x,Popup.floating_y,1);
        if(needupdate)InvalidateRect(Popup.hPopup,nullptr,0);

        TRACKMOUSEEVENT tme;
        tme.cbSize=sizeof(tme);
        tme.hwndTrack=hwnd;
        tme.dwFlags=TME_LEAVE|TME_HOVER;
        tme.dwHoverTime=(MainWindow.ctrl_down||MainWindow.space_down)?1:Settings.hintdelay;
        TrackMouseEvent(&tme);
    }
    if(type==FLOATING_NONE)ShowWindow(Popup.hPopup,SW_HIDE);
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
