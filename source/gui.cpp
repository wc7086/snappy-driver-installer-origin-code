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
#include "settings.h"
#include "gui.h"
#include "logging.h"
#include "matcher.h"
#include "common.h"
#include "manager.h"
#include "theme.h"

#include <windows.h>

#include "system.h"
#include "enum.h"
#include "main.h"
#include "draw.h"

//{ Global vars
WidgetComposite *wPanels=nullptr;
//}

class autorun
{
public:
    autorun()
    {
        wPanel *p;
        wPanels=new WidgetComposite;

        // Revision
        p=new wPanel{1,BOX_PANEL8};
        p->Add(new wTextRev);
        wPanels->Add(p);

        // SysInfo
        p=new wPanel{3,BOX_PANEL1};
        p->Add(new wTextSys1);
        p->Add(new wTextSys2);
        p->Add(new wTextSys3);
        wPanels->Add(p);

        // Install
        p=new wPanel{3,BOX_PANEL2};
        wPanels->Add(p);

        // Theme/lang
        p=new wPanel{5,BOX_PANEL3};
        p->Add(new wText    {STR_LANG});
        p->Add(new wLang);
        p->Add(new wText    {STR_THEME});
        p->Add(new wTheme);
        p->Add(new wCheckbox{STR_EXPERT,            new ExpertmodeCheckboxCommand});
        wPanels->Add(p);

        // Actions
        p=new wPanel{4,BOX_PANEL4};
        p->Add(new wButton  {STR_OPENLOGS,          new OpenLogsCommand});
        p->Add(new wButton  {STR_SNAPSHOT,          new SnapshotCommand});
        p->Add(new wButton  {STR_EXTRACT,           new ExtractCommand});
        p->Add(new wButton  {STR_DRVDIR,            new DrvDirCommand});
        wPanels->Add(p);

        // Filters (found)
        p=new wPanel{7,BOX_PANEL5,true};
        p->Add(new wText    {STR_SHOW_FOUND});
        p->Add(new wCheckbox{STR_SHOW_MISSING,      new FiltersCommand{ID_SHOW_MISSING}});
        p->Add(new wCheckbox{STR_SHOW_NEWER,        new FiltersCommand{ID_SHOW_NEWER}});
        p->Add(new wCheckbox{STR_SHOW_CURRENT,      new FiltersCommand{ID_SHOW_CURRENT}});
        p->Add(new wCheckbox{STR_SHOW_OLD,          new FiltersCommand{ID_SHOW_OLD}});
        p->Add(new wCheckbox{STR_SHOW_BETTER,       new FiltersCommand{ID_SHOW_BETTER}});
        p->Add(new wCheckbox{STR_SHOW_WORSE_RANK,   new FiltersCommand{ID_SHOW_WORSE_RANK}});
        wPanels->Add(p);

        // Filters (not found)
        p=new wPanel{4,BOX_PANEL6,true};
        p->Add(new wText    {STR_SHOW_NOTFOUND});
        p->Add(new wCheckbox{STR_SHOW_NF_MISSING,   new FiltersCommand{ID_SHOW_NF_MISSING}});
        p->Add(new wCheckbox{STR_SHOW_NF_UNKNOWN,   new FiltersCommand{ID_SHOW_NF_UNKNOWN}});
        p->Add(new wCheckbox{STR_SHOW_NF_STANDARD,  new FiltersCommand{ID_SHOW_NF_STANDARD}});
        wPanels->Add(p);

        // Filters (special)
        p=new wPanel{3,BOX_PANEL7,true};
        p->Add(new wCheckbox{STR_SHOW_ONE,          new FiltersCommand{ID_SHOW_ONE}});
        p->Add(new wCheckbox{STR_SHOW_DUP,          new FiltersCommand{ID_SHOW_DUP}});
        p->Add(new wCheckbox{STR_SHOW_INVALID,      new FiltersCommand{ID_SHOW_INVALID}});
        wPanels->Add(p);

        // Install button
        p=new wPanel{1,BOX_PANEL9};
        p->Add(new wButtonInst{STR_INSTALL,         new InstallCommand});
        wPanels->Add(p);

        // Select all button
        p=new wPanel{1,BOX_PANEL10};
        p->Add(new wButton  {STR_SELECT_ALL,        new SelectAllCommand});
        wPanels->Add(p);

        // Select none button
        p=new wPanel{1,BOX_PANEL11};
        p->Add(new wButton  {STR_SELECT_NONE,       new SelectNoneCommand});
        wPanels->Add(p);

        // Options
        p=new wPanel{3,BOX_PANEL12};
        p->Add(new wText    {STR_OPTIONS});
        p->Add(new wCheckbox{STR_RESTOREPOINT,      new RestPointCheckboxCommand});
        p->Add(new wCheckbox{STR_REBOOT,            new RebootCheckboxCommand});
        wPanels->Add(p);

        // Logo
        p=new wLogo{1,BOX_PANEL13};
        p->Add(new wText    {0});
        wPanels->Add(p);
    }
};
autorun obj;
void drawnew(Canvas &canvas)
{
    wPanels->draw(canvas);
}

void FiltersCommand::UpdateCheckbox(bool *checked)
{
    *checked=(Settings.filters&(1<<action_id))?true:false;
}

void ExpertmodeCheckboxCommand::UpdateCheckbox(bool *checked)
{
    *checked=Settings.expertmode!=0;
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
        widgets[i]->arrange();
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

void wLang::arrange()
{
    MainWindow.hLang->Move(x1,y1-5,wx,360);
}

void wTheme::arrange()
{
    MainWindow.hTheme->Move(x1,y1-5,wx,360);
}

void wPanel::draw(Canvas &canvas)
{
    if(!wy)return;
    if(!Settings.expertmode&&isAdvanced)return;

    // Draw panel
    canvas.DrawWidget(x1,y1,x1+wx,y1+wy,boxi+(isSelected&&flags?1:0));

    // Draw childs
    ClipRegion rgn;
    rgn.setRegion(x1,y1,x1+wx,y1+wy);
    canvas.SetClipRegion(rgn);
    for(int i=0;i<num;i++)widgets[i]->draw(canvas);
    canvas.ClearClipRegion();
}

void wText::draw(Canvas &canvas)
{
    canvas.SetTextColor(D(isSelected?CHKBOX_TEXT_COLOR_H:CHKBOX_TEXT_COLOR));
    canvas.DrawTextXY(mirw(x1,0,wx),y1+0,STR(str_id));
    //canvas.drawrect(x1,y1,x1+wx,y1+wy,0xFF000000,0xFF,1,0);
}

void wTextRev::draw(Canvas &canvas)
{
    Version v{atoi(SVN_REV_D),atoi(SVN_REV_M),SVN_REV_Y};
    wchar_t buf[BUFLEN];

    wsprintf(buf,L"%s (",TEXT(SVN_REV2));
    v.str_date(buf+wcslen(buf));
    wcscat(buf,L")");if(rtl)wcscat(buf,L"\u200E");
    canvas.SetTextColor(D(CHKBOX_TEXT_COLOR));
    canvas.DrawTextXY(mirw(x1,0,wx),y1,buf);
}

void wCheckbox::draw(Canvas &canvas)
{
    if(isSelected&&MainWindow.kbpanel)
    {
        //canvas.drawbox(x+ofsx,y,x+XP()-ofsx,y+ofsy+wy,BOX_KBHLT);
        //isSelected=false;
    }

    command->UpdateCheckbox(&checked);

    canvas.DrawCheckbox(mirw(x1,0,wx-D(CHKBOX_SIZE)-2),y1,D(CHKBOX_SIZE)-2,D(CHKBOX_SIZE)-2,checked,isSelected);
    canvas.SetTextColor(D(isSelected?CHKBOX_TEXT_COLOR_H:CHKBOX_TEXT_COLOR));
    canvas.DrawTextXY(mirw(x1,D(CHKBOX_TEXT_OFSX),wx),y1,STR(str_id));
}

void wButton::draw(Canvas &canvas)
{
    if(!flags)canvas.DrawWidget(x1,y1,x1+wx,y1+wy-1,isSelected?BOX_BUTTON_H:BOX_BUTTON);

    canvas.SetTextColor(D(CHKBOX_TEXT_COLOR));
    canvas.DrawTextXY(mirw(x1,wy/2,wx),y1+(wy-D(FONT_SIZE)-2)/2,STR(str_id));
    //canvas.drawrect(x1,y1,x1+wx,y1+wy,0xFF000000,0xFF,1,0);
}

void wButtonInst::draw(Canvas &canvas)
{
    if(!flags)canvas.DrawWidget(x1,y1,x1+wx,y1+wy-1,isSelected?BOX_BUTTON_H:BOX_BUTTON);

    wchar_t buf[BUFLEN];
    canvas.SetTextColor(D(CHKBOX_TEXT_COLOR));
    wsprintf(buf,L"%s (%d)",STR(str_id),manager_g->countItems());
    int nwy=D(PANEL9_OFSX)==D(PANEL10_OFSX)?D(PANEL10_WY):wy;
    canvas.DrawTextXY(mirw(x1,nwy/2,wx),y1+(wy-D(FONT_SIZE)-2)/2,buf);
}

void wTextSys1::draw(Canvas &canvas)
{
    canvas.SetTextColor(D(CHKBOX_TEXT_COLOR));
    canvas.DrawTextXY(x1+10+SYSINFO_COL0,y1,STR(STR_SHOW_SYSINFO));
    canvas.DrawTextXY(x1+10+SYSINFO_COL1,y1,STR(STR_SYSINF_MOTHERBOARD));
    canvas.DrawTextXY(x1+10+SYSINFO_COL2,y1,STR(STR_SYSINF_ENVIRONMENT));
}

void wTextSys2::draw(Canvas &canvas)
{
    State *state=manager_g->matcher->getState();
    wchar_t buf[BUFLEN];

    wsprintf(buf,L"%s (%d-bit)",state->get_winverstr(),state->getArchitecture()?64:32);
    if(rtl)wcscat(buf,L"\u200E");
    canvas.DrawTextXY(x1+10+SYSINFO_COL0,y1,buf);
    canvas.DrawTextXY(x1+10+SYSINFO_COL1,y1,state->getProduct());
    canvas.DrawTextXY(x1+10+SYSINFO_COL2,y1,STR(STR_SYSINF_WINDIR));
    canvas.DrawTextXY(x1+10+SYSINFO_COL3,y1,state->textas.getw(state->getWindir()));
}

void wTextSys3::draw(Canvas &canvas)
{
    State *state=manager_g->matcher->getState();
    wchar_t buf[BUFLEN];

    wsprintf(buf,L"%s",(wx<10+SYSINFO_COL1)?state->getProduct():state->get_szCSDVersion());
    if(rtl)wcscat(buf,L"\u200E");
    canvas.DrawTextXY(x1+10+SYSINFO_COL0,y1,buf);
    wsprintf(buf,L"%s: %s",STR(STR_SYSINF_TYPE),STR(state->isLaptop?STR_SYSINF_LAPTOP:STR_SYSINF_DESKTOP));
    canvas.DrawTextXY(x1+10+SYSINFO_COL1,y1,buf);
    canvas.DrawTextXY(x1+10+SYSINFO_COL2,y1,STR(STR_SYSINF_TEMP));
    canvas.DrawTextXY(x1+10+SYSINFO_COL3,y1,state->textas.getw(state->getTemp()));
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
        Popup.drawpopup(-1,FLOATING_NONE,x,y,MainWindow.hMain);
}

void HoverVisiter::VisitWidget(Widget *a)
{
    a->hitscan(x,y);
    if(a->isSelected&&a->str_id)
    {
        Popup.drawpopup(a->str_id+1,FLOATING_TOOLTIP,x,y,MainWindow.hMain);
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
        Popup.drawpopup(-1,FLOATING_ABOUT,x,y,MainWindow.hMain);
        popup_active=true;
    }
}

void HoverVisiter::VisitwTextRev(wTextRev *a)
{
    a->hitscan(x,y);
    if(a->isSelected)
    {
        SetCursor(LoadCursor(nullptr,IDC_HAND));
        Popup.drawpopup(-1,FLOATING_ABOUT,x,y,MainWindow.hMain);
        popup_active=true;
    }
}

void HoverVisiter::VisitwTextSys1(wTextSys1 *a)
{
    a->hitscan(x,y);
    if(a->isSelected)
    {
        SetCursor(LoadCursor(nullptr,IDC_HAND));
        Popup.drawpopup(a->str_id,FLOATING_SYSINFO,x,y,MainWindow.hMain);
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
    if(a->isSelected||action_id==a->command->GetActionID())
    {
        if(right)
        {
            a->command->RightClick(x,y);
        }
        else
            if(act==CHECKBOX::GET)
                value=a->checked;
            else
            {
                switch(act)
                {
                    case CHECKBOX::SET:
                        if(a->checked)return;
                        a->checked=true;
                        break;

                    case CHECKBOX::CLEAR:
                        if(!a->checked)return;
                        a->checked=false;
                        break;

                    case CHECKBOX::TOGGLE:
                        a->checked^=1;
                        break;

                    case CHECKBOX::GET:
                    default:
                        break;
                }
                a->invalidate();
                a->command->LeftClick(a->checked);
            }
    }

    // Calc filters
    if(a->checked)sum+=a->command->GetBitfieldState();
}

void ExpertmodeCheckboxCommand::LeftClick(bool checked)
{
    Settings.expertmode=checked;

    if(Settings.expertmode||MainWindow.ctrl_down?SW_SHOWNOACTIVATE:MainWindow.hideconsole)
        Console->Show();
    else
        Console->Hide();

    InvalidateRect(MainWindow.hMain,nullptr,0);
}

void RestPointCheckboxCommand::LeftClick(bool checked)
{
    manager_g->set_rstpnt(checked);
}

void RebootCheckboxCommand::LeftClick(bool checked)
{
    if(checked)
        Settings.flags|=FLAG_AUTOINSTALL;
    else
        Settings.flags&=~FLAG_AUTOINSTALL;
}

void ClickVisiter::VisitwButton(wButton *a)
{
    a->hitscan(x,y);
    if(a->isSelected&&!right)a->command->LeftClick();
}

void ClickVisiter::VisitwLogo(wLogo *a)
{
    a->hitscan(x,y);
    if(a->isSelected&&!right)
        System.run_command(L"open",L"http://snappy-driver-installer.sourceforge.net",SW_SHOWNORMAL,0);
}

void ClickVisiter::VisitwTextRev(wTextRev *a)
{
    a->hitscan(x,y);
    if(a->isSelected&&!right)
        System.run_command(L"open",L"http://snappy-driver-installer.sourceforge.net",SW_SHOWNORMAL,0);
}

void ClickVisiter::VisitwTextSys1(wTextSys1 *a)
{
    a->hitscan(x,y);
    if(a->isSelected)
    {
        if(right)
            manager_g->matcher->getState()->contextmenu2(x,y);
        else
            System.run_command(L"devmgmt.msc",nullptr,SW_SHOW,0);
    }
}
//}

//{ Text
textdata_t::textdata_t(Canvas &canvas_,int xofs):
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

void textdata_vert::shift_r(){maxsz+=(int)POPUP_SYSINFO_OFS;}
void textdata_vert::shift_l(){maxsz-=(int)POPUP_SYSINFO_OFS;}

void textdata_t::TextOut_CM(int x1,int y1,const wchar_t *str,int color,int *maxsz1,int mode1)
{
    int ss=pcanvas->GetTextExtent(str);
    if(ss>*maxsz1)*maxsz1=ss;

    if(!mode1)return;
    pcanvas->SetTextColor(color);
    pcanvas->DrawTextXY(x1,y1,str);
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
    _vsnwprintf(buffer,BUFLEN,format,args);
    TextOut_CM(x,y,str,col,&maxsz,1);
    TextOut_CM((int)(x+POPUP_SYSINFO_OFS),y,buffer,col,&maxsz,1);
    y+=wy;
    va_end(args);
}
//}

int mirw(int x,int ofs,int w)
{
    UNREFERENCED_PARAMETER(w);
    return x+ofs;
}
int Xm(int x,int o)
{
    UNREFERENCED_PARAMETER(o);
    return x>=0?x:(MainWindow.main1x_c+x);
}
int Ym(int y){return y>=0?y:(MainWindow.main1y_c+y);}
int XM(int w,int x){return w>=0?w:(w+MainWindow.main1x_c-x);}
int YM(int y,int o){return y>=0?y:(MainWindow.main1y_c+y-o);}

int Xg(int x,int o)
{
    UNREFERENCED_PARAMETER(o);
    return x>=0?x:(MainWindow.mainx_c+x);
}
int Yg(int y){return y>=0?y:(MainWindow.mainy_c+y);}
int XG(int x,int o){return x>=0?x:(MainWindow.mainx_c+x-o);}
int YG(int y,int o){return y>=0?y:(MainWindow.mainy_c+y-o);}

bool isRebootDesired()
{
    ClickVisiter cv{ID_REBOOT,CHECKBOX::GET};
    wPanels->Accept(cv);
    return cv.GetValue()!=0;
}
