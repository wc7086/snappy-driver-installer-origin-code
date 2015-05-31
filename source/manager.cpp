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
const status_t statustnl[NUM_STATUS]=
{
    {FILTER_SHOW_CURRENT,   STATUS_CURRENT},
    {FILTER_SHOW_NEWER,     STATUS_NEW},
    {FILTER_SHOW_OLD,       STATUS_OLD},
    {FILTER_SHOW_WORSE_RANK,STATUS_WORSE},
    {FILTER_SHOW_BETTER,    STATUS_BETTER},
    {FILTER_SHOW_MISSING,   STATUS_MISSING},
};
//}

//{ Manager
void Manager::init(Matcher *matchera)
{
    matcher=matchera;
    items_list.clear();

    for(int i=0;i<RES_SLOTS;i++)
        items_list.push_back(itembar_t(nullptr,nullptr,i,0,1));
}

void Matcher::sorta(int *v)
{
    Devicematch *devicematch_i,*devicematch_j;
    Hwidmatch *hwidmatch_i,*hwidmatch_j;
    int i,j,num;

    num=devicematch_list.size();

    for(i=0;i<num;i++)v[i]=i;

    for(i=0;i<num;i++)
    {
        for(j=i+1;j<num;j++)
        {
            devicematch_i=&devicematch_list[v[i]];
            devicematch_j=&devicematch_list[v[j]];
            hwidmatch_i=(devicematch_i->num_matches)?&hwidmatch_list[devicematch_i->start_matches]:nullptr;
            hwidmatch_j=(devicematch_j->num_matches)?&hwidmatch_list[devicematch_j->start_matches]:nullptr;
            int ismi=devicematch_i->isMissing(state);
            int ismj=devicematch_j->isMissing(state);

            if(ismi<ismj)
            {
                int t;

                t=v[i];
                v[i]=v[j];
                v[j]=t;
            }
            else
            if(ismi==ismj)
            if((hwidmatch_i&&hwidmatch_j&&StrCmpW(hwidmatch_i->getdrp_packname(),hwidmatch_j->getdrp_packname())>0)
               ||
               (!hwidmatch_i&&hwidmatch_j))
            {
                int t;

                t=v[i];
                v[i]=v[j];
                v[j]=t;
            }
        }
    }
}

int  Manager::manager_drplive(wchar_t *s)
{
    itembar_t *itembar;
    unsigned k,needle=0;

    itembar=&items_list[RES_SLOTS];
    for(k=RES_SLOTS;k<items_list.size();k++,itembar++)
    if(itembar->hwidmatch&&StrStrIW(itembar->hwidmatch->getdrp_packname(),s))
    {
        if(itembar->isactive)
        {
            if(itembar->hwidmatch->getdrp_packontorrent())return 0;// Yes
        }
        needle=1;
    }
    return needle?1:1; // No/Unknown
}

void Manager::populate()
{
    int remap[1024];
    matcher->sorta(remap);
    items_list.resize(RES_SLOTS);

    for(unsigned i=0;i<matcher->getDwidmatch_list()->size();i++)
    {
        Devicematch *devicematch=matcher->getDevicematch_i(remap[i]);
        Hwidmatch *hwidmatch=matcher->getHwidmatch_i(devicematch->start_matches);

        for(unsigned j=0;j<devicematch->num_matches;j++,hwidmatch++)
        {
            items_list.push_back(itembar_t(devicematch,hwidmatch,i+RES_SLOTS,remap[i],j?2:2));
            items_list.push_back(itembar_t(devicematch,hwidmatch,i+RES_SLOTS,remap[i],j?0:1));
        }
        if(!devicematch->num_matches)
        {
            items_list.push_back(itembar_t(devicematch,nullptr,i+RES_SLOTS,remap[i],1));
        }
    }
    items_list.shrink_to_fit();
}

void Manager::filter(int options)
{
    Devicematch *devicematch;
    itembar_t *itembar,*itembar1,*itembar_drp=nullptr,*itembar_drpcur=nullptr;
    unsigned i,j,k;
    int cnt[NUM_STATUS+1];
    int ontorrent;
    int o1=options&FILTER_SHOW_ONE;

    itembar=&items_list[RES_SLOTS];

    for(i=RES_SLOTS;i<items_list.size();)
    {
        devicematch=itembar->devicematch;
        memset(cnt,0,sizeof(cnt));
        ontorrent=0;
        if(!devicematch){itembar++;i++;continue;}
        for(j=0;j<devicematch->num_matches;j++,itembar++,i++)
        {
            if(!itembar)log_err("ERROR a%d\n",j);
            itembar->isactive=0;
            //if(!itembar->hwidmatch)log_con("ERROR %d,%d\n",itembar->index,j);
            if(!itembar->hwidmatch)continue;


            if(itembar->first&2)
            {
                itembar->isactive=0;
                itembar_drp=itembar;
                j--;
                continue;
            }
            if(flags&FLAG_FILTERSP&&j)continue;

            if(itembar->checked||itembar->install_status)itembar->isactive=1;

            if((options&FILTER_SHOW_INVALID)==0&&!itembar->hwidmatch->isdrivervalid())
                continue;

            if((options&FILTER_SHOW_DUP)==0&&itembar->hwidmatch->getStatus()&STATUS_DUP)
                continue;

            if((options&FILTER_SHOW_DUP)&&itembar->hwidmatch->getStatus()&STATUS_DUP)
            {
                itembar1=&items_list[i];
                for(k=0;k<devicematch->num_matches-j;k++,itembar1++)
                    if(itembar1->first&2)k--;
                        else
                    if(itembar1->isactive&&
                       itembar1->index==itembar->index&&
                       itembar1->hwidmatch->getdrp_infcrc()==itembar->hwidmatch->getdrp_infcrc())
                        break;

                if(k!=j)
                    itembar->isactive=1;
            }

            if((!o1||!cnt[NUM_STATUS])&&(options&FILTER_SHOW_MISSING)&&itembar->hwidmatch->getStatus()&STATUS_MISSING)
            {
                itembar->isactive=1;
                if(itembar->hwidmatch->getdrp_packontorrent()&&!ontorrent)
                    ontorrent=1;
                else
                    cnt[NUM_STATUS]++;
            }

            if(flags&FLAG_FILTERSP&&itembar->hwidmatch->getAltsectscore()==2&&!itembar->hwidmatch->isvalidcat(matcher->getState()))
                itembar->hwidmatch->setAltsectscore(1);

            for(k=0;k<NUM_STATUS;k++)
                if((!o1||!cnt[NUM_STATUS])&&(options&statustnl[k].filter)&&itembar->hwidmatch->getStatus()&statustnl[k].status)
            {
                if((options&FILTER_SHOW_WORSE_RANK)==0/*&&(options&FILTER_SHOW_OLD)==0*/&&(options&FILTER_SHOW_INVALID)==0&&
                   devicematch->device->problem==0&&devicematch->driver&&itembar->hwidmatch->getAltsectscore()<2)continue;

                if((options&FILTER_SHOW_OLD)!=0&&(itembar->hwidmatch->getStatus()&STATUS_BETTER))continue;

                // hide if
                //[X] Newer
                //[ ] Worse
                //worse, no problem
                if((options&FILTER_SHOW_NEWER)!=0
                   &&(options&FILTER_SHOW_WORSE_RANK)==0&&(options&FILTER_SHOW_INVALID)==0
                   &&itembar->hwidmatch->getStatus()&STATUS_WORSE&&devicematch->device->problem==0&&devicematch->driver)continue;

                if(itembar->hwidmatch->getdrp_packontorrent()&&!ontorrent)
                    ontorrent=1;
                else
                {
                    cnt[k]++;
                    cnt[NUM_STATUS]++;
                }
                itembar->isactive=1;
            }


            if(itembar->isactive&&flags&FLAG_SHOWDRPNAMES2)
            {
                if(itembar_drp)
                {
                    if(!itembar_drpcur||(StrCmpW(itembar_drp->hwidmatch->getdrp_packname(),itembar_drpcur->hwidmatch->getdrp_packname())!=0))
                    {
                        itembar_drp->isactive=1;
                        itembar_drpcur=itembar_drp;
                    }
                }
            }

            if(!itembar->hwidmatch->getdrp_packontorrent())
                if(o1&&itembar->hwidmatch->getStatus()&STATUS_CURRENT)
                    cnt[NUM_STATUS]++;
        }
        if(!devicematch->num_matches)
        {
            itembar->isactive=0;
            if(options&FILTER_SHOW_NF_STANDARD&&devicematch->status&STATUS_NF_STANDARD)itembar->isactive=1;
            if(options&FILTER_SHOW_NF_UNKNOWN&&devicematch->status&STATUS_NF_UNKNOWN)itembar->isactive=1;
            if(options&FILTER_SHOW_NF_MISSING&&devicematch->status&STATUS_NF_MISSING)itembar->isactive=1;
            if(itembar->first&2)
            {
                itembar->isactive=0;
                itembar_drp=itembar;
            }
            itembar++;i++;
        }
    }
    i=0;
    itembar=&items_list[RES_SLOTS];
    for(k=RES_SLOTS;k<items_list.size();k++,itembar++)
        if(itembar->isactive&&itembar->hwidmatch)i++;else itembar->checked=0;

    items_list[SLOT_NOUPDATES].isactive=
        items_list.size()==RES_SLOTS||
        (i==0&&statemode==0&&matcher->getCol()->size()>1)?1:0;

    items_list[SLOT_RESTORE_POINT].isactive=statemode==
        STATEMODE_EMUL||i==0||(flags&FLAG_NORESTOREPOINT)?0:1;
    //set_rstpnt(0);

    if(!items_list[SLOT_RESTORE_POINT].install_status)
        items_list[SLOT_RESTORE_POINT].install_status=STR_RESTOREPOINT;
}

void Manager::print_tbl()
{
    int limits[7];

    if((log_verbose&LOG_VERBOSE_MANAGER)==0)return;
    log_file("{manager_print\n");
    memset(limits,0,sizeof(limits));

    for(auto itembar=items_list.begin()+RES_SLOTS;itembar!=items_list.end();++itembar)
        if(itembar->isactive&&itembar->hwidmatch)
            itembar->hwidmatch->calclen(limits);


    unsigned k=0,act=0;
    for(auto itembar=items_list.begin()+RES_SLOTS;itembar!=items_list.end();++itembar,k++)
        if(itembar->isactive&&(itembar->first&2)==0)
        {
            log_file("$%04d|",k);
            if(itembar->hwidmatch)
                itembar->hwidmatch->print_tbl(limits);
            else
                log_file("'%S'\n",matcher->getState()->textas.get(itembar->devicematch->device->Devicedesc));
            act++;
        }else
        {
//            log_file("$%04d|^^ %d,%d\n",k,itembar->devicematch->num_matches,(itembar->hwidmatch)?itembar->hwidmatch->status:-1);
        }

    log_file("}manager_print[%d]\n\n",act);
}

void Manager::print_hr()
{
    if((log_verbose&LOG_VERBOSE_MANAGER)==0)return;
    log_file("{manager_print\n");

    unsigned k=0,act=0;
    for(auto itembar=items_list.begin()+RES_SLOTS;itembar!=items_list.end();++itembar,k++)
        if(itembar->isactive&&(itembar->first&2)==0)
        {
            if(flags&FLAG_FILTERSP&&!itembar->hwidmatch->isvalidcat(matcher->getState()))continue;
            wchar_t buf[BUFLEN];
            itembar->str_status(buf);
            log_file("\n$%04d, %S\n",k,buf);
            if(itembar->devicematch->device)
            {
                itembar->devicematch->device->print(matcher->getState());
                //device_printHWIDS(itembar->devicematch->device,matcher->state);
            }
            if(itembar->devicematch->driver)
            {
                log_file("Installed driver\n");
                itembar->devicematch->driver->print(matcher->getState());
            }

            if(itembar->hwidmatch)
            {
                log_file("Available driver\n");
                itembar->hwidmatch->print_hr();
            }

            act++;
        }else
        {
//            log_file("$%04d|^^ %d,%d\n",k,itembar->devicematch->num_matches,(itembar->hwidmatch)?itembar->hwidmatch->status:-1);
        }

    log_file("}manager_print[%d]\n\n",act);
}

//{ User interaction
// Zones:
// 0 button
// 1 checkbox
// 2 downarrow
// 3 text
int setaa=0;
void Manager::hitscan(int x,int y,int *r,int *zone)
{
    itembar_t *itembar;
    unsigned i;
    int pos;
    int ofsy=getscrollpos();
    int cutoff=calc_cutoff()+D(DRVITEM_DIST_Y0);
    int ofs=0;
    int wx=XG(D(DRVITEM_WX),Xg(D(DRVITEM_OFSX),D(DRVITEM_WX)));

    *r=-2;
    *zone=0;
    int cnt=0;

    if(kbpanel==KB_FIELD)
    {
        int max_cnt=0;
        itembar=&items_list[0];
        for(i=0;i<items_list.size();i++,itembar++)
        if(itembar->isactive&&(itembar->first&2)==0)max_cnt++;

        if(kbitem[kbpanel]<0)kbitem[kbpanel]=max_cnt-1;
        if(kbitem[kbpanel]>=max_cnt)kbitem[kbpanel]=0;
    }

    y-=-D(DRVITEM_DIST_Y0);
    x-=Xg(D(DRVITEM_OFSX),D(DRVITEM_WX));
    if(kbpanel==KB_NONE)if(x<0||x>wx)return;
    itembar=&items_list[0];
    for(i=0;i<items_list.size();i++,itembar++)
    if(itembar->isactive&&(itembar->first&2)==0)
    {

        if(kbpanel==KB_FIELD)
        {
            *r=i;
            if(kbitem[kbpanel]==cnt)
            {
                if(setaa)
                {
                    animstart=GetTickCount();
                    offset_target=(itembar->curpos>>16);
                    SetTimer(hMain,1,1000/60,nullptr);
                    setaa=0;
                }
                return;
            }
            cnt++;
            continue;
        }
        pos=itembar->curpos>>16;
        if(i>=SLOT_RESTORE_POINT&&y<cutoff)continue;
        if(i>=SLOT_RESTORE_POINT)pos-=ofsy;
        if(y>pos&&y<pos+D(DRVITEM_WY))
        {
            x-=D(ITEM_CHECKBOX_OFS_X);
            y-=D(ITEM_CHECKBOX_OFS_Y)+pos;
            ofs=(itembar->first&1)?0:D(DRVITEM_LINE_INTEND);
            if(x-ofs>0)*r=i;
            if(x-ofs>0&&x-ofs<D(ITEM_CHECKBOX_SIZE)&&y>0&&y<D(ITEM_CHECKBOX_SIZE))*zone=1;
            if(x>wx-50&&!ofs)*zone=expertmode?2:2;
            if(!*zone&&(x-ofs<D(ITEM_CHECKBOX_SIZE)))*zone=3;
            if(!*zone&&(x>240+190))*zone=3;
            if(kbpanel==KB_NONE)return;
        }
    }
    *r=-1;
}

void Manager::clear()
{
    itembar_t *itembar;
    unsigned i;

    itembar=&items_list[RES_SLOTS];
    for(i=RES_SLOTS;i<items_list.size();i++,itembar++)
    {
        itembar->install_status=0;
        itembar->percent=0;
    }
    items_list[SLOT_EXTRACTING].isactive=0;
    items_list[SLOT_RESTORE_POINT].install_status=STR_RESTOREPOINT;
    filter(filters);
    setpos();
    invaidate(INVALIDATE_DEVICES|INVALIDATE_MANAGER);
}

void Manager::testitembars()
{
    itembar_t *itembar;
    unsigned i,j=0,index=RES_SLOTS+1;
    int prev_index=-1;

    itembar=&items_list[0];

    filter(FILTER_SHOW_CURRENT|FILTER_SHOW_NEWER);
    wcscpy(drpext_dir,L"drpext");
    items_list[SLOT_EMPTY].curpos=1;

    for(i=0;i<items_list.size();i++,itembar++)
    if(i>SLOT_EMPTY&&i<RES_SLOTS)
    {
        if(i==SLOT_VIRUS_HIDDEN||i==SLOT_VIRUS_RECYCLER||i==SLOT_NODRIVERS||i==SLOT_DPRDIR)continue;
        itembar_settext(i,1);
    }
    else if(itembar->isactive)
    {
        if(!itembar->devicematch||prev_index==itembar->index){itembar->isactive=0;continue;}
        prev_index=itembar->index;
        itembar->checked=0;
        if(j==0||j==6||j==9||j==18||j==21)index++;
        itembar->index=index;
        itembar->hwidmatch->setAltsectscore(2);
        switch(j++)
        {
            case  0:itembar->install_status=STR_INST_EXTRACT;itembar->percent=300;itembar->checked=1;break;
            case  1:itembar->install_status=STR_INST_INSTALL;itembar->percent=900;itembar->checked=1;break;
            case  2:itembar->install_status=STR_INST_EXTRACT;itembar->percent=400;break;
            case  3:itembar->install_status=STR_INST_OK;break;
            case  4:itembar->install_status=STR_INST_REBOOT;break;
            case  5:itembar->install_status=STR_INST_FAILED;break;

            case  6:itembar->hwidmatch->setStatus(STATUS_INVALID);break;
            case  7:itembar->hwidmatch->setStatus(STATUS_MISSING);break;
            case  8:itembar->hwidmatch->setStatus(STATUS_CURRENT|STATUS_SAME|STATUS_DUP);break;

            case  9:itembar->hwidmatch->setStatus(STATUS_NEW|STATUS_BETTER);break;
            case 10:itembar->hwidmatch->setStatus(STATUS_NEW|STATUS_SAME);break;
            case 11:itembar->hwidmatch->setStatus(STATUS_NEW|STATUS_WORSE);break;
            case 12:itembar->hwidmatch->setStatus(STATUS_CURRENT|STATUS_BETTER);break;
            case 13:itembar->hwidmatch->setStatus(STATUS_CURRENT|STATUS_SAME);break;
            case 14:itembar->hwidmatch->setStatus(STATUS_CURRENT|STATUS_WORSE);break;
            case 15:itembar->hwidmatch->setStatus(STATUS_OLD|STATUS_BETTER);break;
            case 16:itembar->hwidmatch->setStatus(STATUS_OLD|STATUS_SAME);break;
            case 17:itembar->hwidmatch->setStatus(STATUS_OLD|STATUS_WORSE);break;

            case 18:itembar->devicematch->status=STATUS_NF_MISSING;itembar->hwidmatch=nullptr;break;
            case 19:itembar->devicematch->status=STATUS_NF_STANDARD;itembar->hwidmatch=nullptr;break;
            case 20:itembar->devicematch->status=STATUS_NF_UNKNOWN;itembar->hwidmatch=nullptr;break;
            default:itembar->isactive=0;
        }
    }

}

void Manager::toggle(int index)
{
    itembar_t *itembar,*itembar1;
    unsigned i;
    int group;

#ifdef USE_TORRENT
    if(installmode&&!Updater.isPaused())
        return;
#endif

    itembar1=&items_list[index];
    if(index>=RES_SLOTS&&!itembar1->hwidmatch)return;
    itembar1->checked^=1;
    if(!itembar1->checked&&installmode)
    {
        itembar1->install_status=STR_INST_STOPPING;
    }
    if(index==SLOT_RESTORE_POINT)
    {
        set_rstpnt(itembar1->checked);
    }
    group=itembar1->index;

    itembar=&items_list[0];
    for(i=0;i<items_list.size();i++,itembar++)
        if(itembar!=itembar1&&itembar->index==group)
            itembar->checked&=~1;

    if(itembar1->checked&&itembar1->isactive&2)
        expand(index);
    else
        redrawmainwnd();
}

void Manager::expand(int index)
{
    itembar_t *itembar,*itembar1;
    unsigned i;
    int group;

    itembar1=&items_list[index];
    group=itembar1->index;

    itembar=&items_list[0];
    if((itembar1->isactive&2)==0)// collapsed
    {
        for(i=0;i<items_list.size();i++,itembar++)
            if(itembar->index==group&&itembar->hwidmatch&&(itembar->hwidmatch->getStatus()&STATUS_INVALID)==0&&(itembar->first&2)==0)
                {
                    itembar->isactive|=2; // expand
                }
    }
    else
    {
        for(i=0;i<items_list.size();i++,itembar++)
            if(itembar->index==group&&(itembar->first&2)==0)
            {
                itembar->isactive&=1; //collapse
                if(itembar->checked)itembar->isactive|=4;
            }
    }
    setpos();
}

void Manager::selectnone()
{
    itembar_t *itembar;
    unsigned i;

#ifdef USE_TORRENT
    if(installmode&&!Updater.isPaused())
        return;
#endif

    if(items_list[SLOT_RESTORE_POINT].isactive)
    {
        set_rstpnt(0);
    }
    itembar=&items_list[RES_SLOTS];
    for(i=RES_SLOTS;i<items_list.size();i++,itembar++)itembar->checked=0;
}

void Manager::selectall()
{
    itembar_t *itembar;
    unsigned i;
    int group=-1;

#ifdef USE_TORRENT
    if(installmode&&!Updater.isPaused())
        return;
#endif

    itembar=&items_list[SLOT_RESTORE_POINT];
    if(itembar->install_status==STR_RESTOREPOINT&&itembar->isactive)
        set_rstpnt(1);

    itembar=&items_list[RES_SLOTS];
    for(i=RES_SLOTS;i<items_list.size();i++,itembar++)
    {
        itembar->checked=0;
        if(itembar->isactive&&group!=itembar->index&&itembar->hwidmatch&&(itembar->first&2)==0)
        {
            if(itembar->install_status==0)itembar->checked=1;
            group=itembar->index;
        }
    }
}
//}

//{ Helpers
itembar_t::itembar_t(Devicematch *devicematch1,Hwidmatch *hwidmatch1,int groupindex,int rm1,int first1)
{
    memset(this,0,sizeof(itembar_t));
    devicematch=devicematch1;
    hwidmatch=hwidmatch1;
    curpos=(-D(DRVITEM_DIST_Y0))<<16;
    tagpos=(-D(DRVITEM_DIST_Y0))<<16;
    index=groupindex;
    rm=rm1;
    first=first1;
}

itembar_t::itembar_t()
{
}

void Manager::itembar_settext(int i,const wchar_t *txt1,int percent)
{
    itembar_t *itembar=&items_list[i];
    wcscpy(itembar->txt1,txt1);
    itembar->percent=percent;
    itembar->isactive=1;
    redrawfield();
}

void Manager::itembar_settext(int i,int act,const wchar_t *txt1,int val1v,int val2v,int percent)
{
    itembar_t *itembar=&items_list[i];
    if(txt1)wcscpy(itembar->txt1,txt1);
    if(val1v>=0)itembar->val1=val1v;
    if(val2v>=0)itembar->val2=val2v;
    if(!val2v)val2v++;
    itembar->percent=(percent>=0)?percent:val1v*1000/val2v;
    itembar->isactive=act;
    setpos();
    redrawfield();
}

void itembar_t::itembar_setpos(int *pos,int *cnt)
{
    if(isactive)
    {
        *pos+=*cnt?D(DRVITEM_DIST_Y1):D(DRVITEM_DIST_Y0);
        (*cnt)--;
    }
    oldpos=curpos;
    tagpos=*pos<<16;
    accel=(tagpos-curpos)/(1000/2);
    if(accel==0)accel=(tagpos<curpos)?500:-500;
}

void Manager::set_rstpnt(int checked)
{
    panels[11].setChecked(2,items_list[SLOT_RESTORE_POINT].checked=checked);
    //if(D(PANEL12_WY))manager_g->items_list[SLOT_RESTORE_POINT].isactive=checked;
    setpos();
    redrawfield();
}

int Hwidmatch::isdrivervalid()
{
    if(altsectscore>0&&decorscore>0)return 1;
    return 0;
}

void itembar_t::contextmenu(int x,int y)
{
    HMENU hPopupMenu=CreatePopupMenu();

    int flags1=checked?MF_CHECKED:0;
    if(!hwidmatch&&index!=SLOT_RESTORE_POINT)flags1|=MF_GRAYED;
    if(rtl)x=mainx_c-x;

    if(floating_itembar==SLOT_RESTORE_POINT)
    {
        InsertMenu(hPopupMenu,0,MF_BYPOSITION|MF_STRING|flags1,ID_SCHEDULE, STR(STR_REST_SCHEDULE));
        InsertMenu(hPopupMenu,1,MF_BYPOSITION|MF_STRING,       ID_SHOWALT,  STR(STR_REST_ROLLBACK));

        RECT rect;
        SetForegroundWindow(hMain);
        GetWindowRect(hField,&rect);
        TrackPopupMenu(hPopupMenu,TPM_LEFTALIGN,rect.left+x,rect.top+y,0,hMain,nullptr);
        return;
    }
    if(floating_itembar<RES_SLOTS)return;

    Driver *cur_driver=nullptr;

    char *t=manager_g->matcher->getState()->textas.get(0);
    if(devicematch->driver)cur_driver=devicematch->driver;
    int flags2=isactive&2?MF_CHECKED:0;
    int flags3=cur_driver?0:MF_GRAYED;
    if(manager_g->groupsize(index)<2)flags2|=MF_GRAYED;
    wchar_t buf[512];

    int i=0;
    HMENU hSub1=CreatePopupMenu();
    HMENU hSub2=CreatePopupMenu();
    if(devicematch->device->getHardwareID())
    {
        wchar_t *p=(wchar_t *)(t+devicematch->device->getHardwareID());
        while(*p)
        {
            escapeAmp(buf,p);
            InsertMenu(hSub1,i,MF_BYPOSITION|MF_STRING,ID_HWID_WEB+i,buf);
            InsertMenu(hSub2,i,MF_BYPOSITION|MF_STRING,ID_HWID_CLIP+i,buf);
            p+=lstrlenW(p)+1;
            i++;
        }
    }
    if(devicematch->device->getCompatibleIDs())
    {
        wchar_t *p=(wchar_t *)(t+devicematch->device->getCompatibleIDs());
        while(*p)
        {
            escapeAmp(buf,p);
            InsertMenu(hSub1,i,MF_BYPOSITION|MF_STRING,ID_HWID_WEB+i,buf);
            InsertMenu(hSub2,i,MF_BYPOSITION|MF_STRING,ID_HWID_CLIP+i,buf);
            p+=lstrlenW(p)+1;
            i++;
        }
    }
    int flagssubmenu=i?0:MF_GRAYED;

    i=0;
    InsertMenu(hPopupMenu,i++,MF_BYPOSITION|MF_STRING|flags1,ID_SCHEDULE, STR(STR_CONT_INSTALL));
    InsertMenu(hPopupMenu,i++,MF_BYPOSITION|MF_STRING|flags2,ID_SHOWALT,  STR(STR_CONT_SHOWALT));
    InsertMenu(hPopupMenu,i++,MF_BYPOSITION|MF_SEPARATOR,0,nullptr);
    InsertMenu(hPopupMenu,i++,MF_BYPOSITION|MF_STRING|MF_POPUP|flagssubmenu,(UINT_PTR)hSub1,STR(STR_CONT_HWID_SEARCH));
    InsertMenu(hPopupMenu,i++,MF_BYPOSITION|MF_STRING|MF_POPUP|flagssubmenu,(UINT_PTR)hSub2,STR(STR_CONT_HWID_CLIP));
    InsertMenu(hPopupMenu,i++,MF_BYPOSITION|MF_SEPARATOR,0,nullptr);
    InsertMenu(hPopupMenu,i++,MF_BYPOSITION|MF_STRING|flags3,ID_OPENINF,  STR(STR_CONT_OPENINF));
    InsertMenu(hPopupMenu,i++,MF_BYPOSITION|MF_STRING|flags3,ID_LOCATEINF,STR(STR_CONT_LOCATEINF));

    RECT rect;
    SetForegroundWindow(hMain);
    GetWindowRect(hField,&rect);
    TrackPopupMenu(hPopupMenu,TPM_LEFTALIGN,rect.left+x,rect.top+y,0,hMain,nullptr);
}

void itembar_t::str_status(wchar_t *buf)
{
    buf[0]=0;

    if(hwidmatch)
    {
        int status=hwidmatch->getStatus();
        if(status&STATUS_INVALID)
            wcscat(buf,STR(STR_STATUS_INVALID));
        else
        {
            if(status&STATUS_MISSING)
                wsprintf(buf,L"%s",STR(STR_STATUS_MISSING),devicematch->device->getProblem());
            else
            {
                if(status&STATUS_BETTER&&status&STATUS_NEW)        wcscat(buf,STR(STR_STATUS_BETTER_NEW));
                if(status&STATUS_SAME  &&status&STATUS_NEW)        wcscat(buf,STR(STR_STATUS_SAME_NEW));
                if(status&STATUS_WORSE &&status&STATUS_NEW)        wcscat(buf,STR(STR_STATUS_WORSE_NEW));

                if(status&STATUS_BETTER&&status&STATUS_CURRENT)    wcscat(buf,STR(STR_STATUS_BETTER_CUR));
                if(status&STATUS_SAME  &&status&STATUS_CURRENT)    wcscat(buf,STR(STR_STATUS_SAME_CUR));
                if(status&STATUS_WORSE &&status&STATUS_CURRENT)    wcscat(buf,STR(STR_STATUS_WORSE_CUR));

                if(status&STATUS_BETTER&&status&STATUS_OLD)        wcscat(buf,STR(STR_STATUS_BETTER_OLD));
                if(status&STATUS_SAME  &&status&STATUS_OLD)        wcscat(buf,STR(STR_STATUS_SAME_OLD));
                if(status&STATUS_WORSE &&status&STATUS_OLD)        wcscat(buf,STR(STR_STATUS_WORSE_OLD));
            }
        }
        if(status&STATUS_DUP)wcscat(buf,STR(STR_STATUS_DUP));
        if(hwidmatch->getAltsectscore()<2)
        {
            wcscat(buf,STR(STR_STATUS_NOTSIGNED));
        }
        if(hwidmatch->getdrp_packontorrent())wcscat(buf,STR(STR_UPD_WEBSTATUS));
    }
    else
    //if(devicematch)
    {
        if(devicematch->getStatus()&STATUS_NF_STANDARD)wcscat(buf,STR(STR_STATUS_NF_STANDARD));
        if(devicematch->getStatus()&STATUS_NF_UNKNOWN) wcscat(buf,STR(STR_STATUS_NF_UNKNOWN));
        if(devicematch->getStatus()&STATUS_NF_MISSING) wcscat(buf,STR(STR_STATUS_NF_MISSING));
    }
}

int itembar_t::box_status()
{
    switch(index)
    {
        case SLOT_VIRUS_AUTORUN:
        case SLOT_VIRUS_RECYCLER:
        case SLOT_VIRUS_HIDDEN:
            return BOX_DRVITEM_VI;

        case SLOT_NODRIVERS:
        case SLOT_DPRDIR:
        case SLOT_SNAPSHOT:
            return BOX_DRVITEM_IF;

        case SLOT_DOWNLOAD:
        case SLOT_NOUPDATES:
            return BOX_NOUPDATES;

        case SLOT_RESTORE_POINT:
            switch(install_status)
            {
                case STR_REST_CREATING:
                    return BOX_DRVITEM_D0;

                case STR_REST_CREATED:
                    return BOX_DRVITEM_D1;

                case STR_REST_FAILED:
                    return BOX_DRVITEM_DE;

                default:
                    break;
            }
            break;

        case SLOT_EXTRACTING:
            switch(install_status)
            {
                case STR_EXTR_EXTRACTING:
                case STR_INST_INSTALLING:
                    return BOX_DRVITEM_D0;

                case STR_INST_COMPLITED:
                    return BOX_DRVITEM_D1;

                case STR_INST_COMPLITED_RB:
                    return BOX_DRVITEM_D2;

                case STR_INST_STOPPING:
                    return BOX_DRVITEM_DE;

                default:break;
            }
            break;

        default:
            break;
    }
    if(hwidmatch)
    {
        int status=hwidmatch->getStatus();

        if(first&2)return BOX_DRVITEM_PN;

        if(status&STATUS_INVALID)
            return BOX_DRVITEM_IN;
        else
        {
            switch(install_status)
            {
                case STR_INST_EXTRACT:
                case STR_INST_INSTALL:
                    return BOX_DRVITEM_D0;

                case STR_INST_OK:
                case STR_EXTR_OK:
                    return BOX_DRVITEM_D1;

                case STR_INST_REBOOT:
                    return BOX_DRVITEM_D2;

                case STR_INST_FAILED:
                case STR_EXTR_FAILED:
                    return BOX_DRVITEM_DE;

                default:break;
            }
            if(status&STATUS_MISSING)
                return BOX_DRVITEM_MS;
            else
            {
                if(hwidmatch->getAltsectscore()<2)return BOX_DRVITEM_WO;

                if(status&STATUS_BETTER&&status&STATUS_NEW)        return BOX_DRVITEM_BN;
                if(status&STATUS_SAME  &&status&STATUS_NEW)        return BOX_DRVITEM_SN;
                if(status&STATUS_WORSE &&status&STATUS_NEW)        return BOX_DRVITEM_WN;

                if(status&STATUS_BETTER&&status&STATUS_CURRENT)    return BOX_DRVITEM_BC;
                if(status&STATUS_SAME  &&status&STATUS_CURRENT)    return BOX_DRVITEM_SC;
                if(status&STATUS_WORSE &&status&STATUS_CURRENT)    return BOX_DRVITEM_WC;

                if(status&STATUS_BETTER&&status&STATUS_OLD)        return BOX_DRVITEM_BO;
                if(status&STATUS_SAME  &&status&STATUS_OLD)        return BOX_DRVITEM_SO;
                if(status&STATUS_WORSE &&status&STATUS_OLD)        return BOX_DRVITEM_WO;
            }
        }
    }
    else
    if(devicematch)
    {
        if(devicematch->getStatus()&STATUS_NF_STANDARD)  return BOX_DRVITEM_NS;
        if(devicematch->getStatus()&STATUS_NF_UNKNOWN)   return BOX_DRVITEM_NU;
        if(devicematch->getStatus()&STATUS_NF_MISSING)   return BOX_DRVITEM_NM;
    }
        //if(status&STATUS_DUP)wcscat(buf,STR(STR_STATUS_DUP));
    return BOX_DRVITEM;
}

void Manager::itembar_setactive(int i,int val){items_list[i].isactive=val;}
void Manager::popup_drivercmp(Manager *manager,HDC hdcMem,RECT rect,int index){items_list[floating_itembar].popup_drivercmp(manager,hdcMem,rect,index);}
void Manager::contextmenu(int x,int y){items_list[floating_itembar].contextmenu(x,y);}
const wchar_t *Manager::getHWIDby(int id){return items_list[floating_itembar].devicematch->device->getHWIDby(id,matcher->getState());}

void Manager::getINFpath(int wp)
{
    wchar_t buf[BUFLEN];
    wsprintf(buf,L"%s%s%s",
            (wp==ID_LOCATEINF)?L"/select,":L"",
            matcher->getState()->textas.get(matcher->getState()->getWindir()),
            matcher->getState()->textas.get(items_list[floating_itembar].devicematch->driver->getInfPath()));

    if(wp==ID_OPENINF)
        run_command(buf,L"",SW_SHOW,0);
    else
        run_command(L"explorer.exe",buf,SW_SHOW,0);
}
//}

//{ Driver list
void Manager::setpos()
{
    Devicematch *devicematch;
    itembar_t *itembar,*lastitembar=nullptr;
    unsigned k;
    int cnt=0;
    int pos=D(DRVITEM_OFSY);
    //int pos=0;
    int group=0;
    int lastmatch=0;

//0:wide
//1:narrow

    itembar=&items_list[0];
    for(k=0;k<items_list.size();k++,itembar++)
    {
        devicematch=itembar->devicematch;
        cnt=group==itembar->index?1:0;

        //if(lastitembar&&lastitembar->index<SLOT_RESTORE_POINT&&itembar->index<SLOT_RESTORE_POINT)cnt=1;
        if(devicematch&&!devicematch->num_matches&&!lastmatch&&lastitembar&&lastitembar->index>=SLOT_RESTORE_POINT)cnt=1;

        itembar->itembar_setpos(&pos,&cnt);
        if(itembar->isactive)
        {
            lastitembar=itembar;
            group=itembar->index;
            if(devicematch)lastmatch=devicematch->num_matches;
        }
    }
    SetTimer(hMain,1,1000/60,nullptr);
    animstart=GetTickCount();
}

int Manager::animate()
{
    int chg=0;
    long tt1=GetTickCount()-animstart;

    // Move itembars
    for(auto &itembar:items_list)
    {
        if(itembar.curpos==itembar.tagpos)continue;
        chg=1;
        int pos=itembar.oldpos+itembar.accel*tt1;
        if(itembar.accel>0&&pos>itembar.tagpos)pos=itembar.tagpos;
        if(itembar.accel<0&&pos<itembar.tagpos)pos=itembar.tagpos;
        itembar.curpos=pos;
    }

    // Animate scrolling
    int i=getscrollpos();
    if(offset_target)
    {
        int v=offset_target-D(DRVITEM_DIST_Y0)*2;
        if(i>v)
        {
            i--;
            i-=(i-v)/10;
            if(i<v)i=v;
            setscrollpos(i);
            chg=1;
        }

        v=offset_target+D(DRVITEM_DIST_Y0)-mainy_c;
        if(i<v)
        {
            i++;
            i-=(i-v)/10;
            if(i>v)i=v;
            setscrollpos(i);
            chg=1;
        }
    }

    return chg||
        (installmode==MODE_NONE&&items_list[SLOT_EXTRACTING].install_status);
}

int Manager::groupsize(int index)
{
    int num=0;

    for(auto &itembar:items_list)
        if(itembar.index==index&&itembar.hwidmatch&&(itembar.hwidmatch->getStatus()&STATUS_INVALID)==0&&(itembar.first&2)==0)
            num++;

    return num;
}

int Manager::countItems()
{
    unsigned j,cnt=0;
    itembar_t *itembar;

    itembar=&items_list[RES_SLOTS];
    for(j=RES_SLOTS;j<items_list.size();j++,itembar++)
    if(itembar->checked)cnt++;
    return cnt;
}

void itembar_t::drawbutton(HDC hdc,int x,int pos,const wchar_t *str1,const wchar_t *str2)
{
    pos+=D(ITEM_TEXT_OFS_Y);
    SetTextColor(hdc,D(boxindex[box_status()]+14));
    TextOutH(hdc,x+D(ITEM_TEXT_OFS_X),pos,str1);
    SetTextColor(hdc,D(boxindex[box_status()]+15));
    TextOutH(hdc,x+D(ITEM_TEXT_OFS_X),pos+D(ITEM_TEXT_DIST_Y),str2);
}

int  Manager::drawitem(HDC hdc,int index,int ofsy,int zone,int cutoff)
{
    itembar_t *itembar=&items_list[index];

    HICON hIcon;
    wchar_t bufw[BUFLEN];
    HRGN hrgn=nullptr,hrgn2;
    int x=Xg(D(DRVITEM_OFSX),D(DRVITEM_WX));
    int wx=XG(D(DRVITEM_WX),x);
    int r=D(boxindex[itembar->box_status()]+3);
    int intend=0;
    int oldstyle=flags&FLAG_SHOWDRPNAMES1||flags&FLAG_OLDSTYLE;

    int pos=(itembar->curpos>>16)-D(DRVITEM_DIST_Y0);
    if(index>=SLOT_RESTORE_POINT)pos-=ofsy;

    if(!(itembar->first&1))
    {
        int i=index;

        while(i>=0&&!(items_list[i].first&1&&items_list[i].isactive))i--;
        if(items_list[i].index==itembar->index)intend=i;
        //itembar->index=intend;
    }
    if(intend)
    {
        x+=D(DRVITEM_LINE_INTEND);
        wx-=D(DRVITEM_LINE_INTEND);
    }
    if(pos<=-D(DRVITEM_DIST_Y0))return 0;
    if(pos>mainy_c)return 0;
    if(wx<0)return 0;

    SelectObject(hdc,hFont);

    if(index<SLOT_RESTORE_POINT)cutoff=D(DRVITEM_OFSY);
    hrgn2=CreateRectRgn(0,cutoff,x+wx,mainy_c);
    hrgn=CreateRoundRectRgn(x,(pos<cutoff)?cutoff:pos,x+wx,pos+D(DRVITEM_WY),r,r);
    int cl=((zone>=0)?1:0);
    if(index==SLOT_EXTRACTING&&itembar->install_status&&installmode==MODE_NONE)
        cl=((GetTickCount()-animstart)/200)%2;
    SelectClipRgn(hdc,hrgn2);
    if(intend&&D(DRVITEM_LINE_WIDTH)&&!(itembar->first&2))
    {
        HPEN oldpen,newpen;

        newpen=CreatePen(PS_SOLID,D(DRVITEM_LINE_WIDTH),D(DRVITEM_LINE_COLOR));
        oldpen=(HPEN)SelectObject(hdc,newpen);
        MoveToEx(hdc,x-D(DRVITEM_LINE_INTEND)/2,(items_list[intend].curpos>>16)-D(DRVITEM_DIST_Y0)+D(DRVITEM_WY)-ofsy,nullptr);
        LineTo(hdc,x-D(DRVITEM_LINE_INTEND)/2,pos+D(DRVITEM_WY)/2);
        LineTo(hdc,x,pos+D(DRVITEM_WY)/2);
        SelectObject(hdc,oldpen);
        DeleteObject(newpen);
    }
    drawbox(hdc,x,pos,x+wx,pos+D(DRVITEM_WY),itembar->box_status()+cl);
    SelectClipRgn(hdc,hrgn);

    if(itembar->percent)
    {
        //printf("%d\n",itembar->percent);
        int a=BOX_PROGR;
        //if(index==SLOT_EXTRACTING&&installmode==MODE_STOPPING)a=BOX_PROGR_S;
        //if(index>=RES_SLOTS&&(!itembar->checked||installmode==MODE_STOPPING))a=BOX_PROGR_S;
        drawbox(hdc,x,pos,x+wx*itembar->percent/1000.,pos+D(DRVITEM_WY),a);
    }

    SetTextColor(hdc,0); // todo: color
    switch(index)
    {
        case SLOT_RESTORE_POINT:
            drawcheckbox(hdc,x+D(ITEM_CHECKBOX_OFS_X),pos+D(ITEM_CHECKBOX_OFS_Y),
                         D(ITEM_CHECKBOX_SIZE),D(ITEM_CHECKBOX_SIZE),
                         itembar->checked,zone>=0);

            wcscpy(bufw,STR(itembar->install_status));
            TextOutH(hdc,x+D(ITEM_TEXT_OFS_X),pos+D(ITEM_TEXT_DIST_Y)/2,bufw);
            break;

        case SLOT_INDEXING:
            wsprintf(bufw,L"%s (%d%s%d)",STR(itembar->isactive==2?STR_INDEXLZMA:STR_INDEXING),
                        items_list[SLOT_INDEXING].val1,STR(STR_OF),
                        items_list[SLOT_INDEXING].val2);
            TextOutH(hdc,x+D(ITEM_TEXT_OFS_X),pos,bufw);

            if(*itembar->txt1)
            {
                wsprintf(bufw,L"%s",itembar->txt1);
                TextOutH(hdc,x+D(ITEM_TEXT_OFS_X),pos+D(ITEM_TEXT_DIST_Y),bufw);
            }
            break;

        case SLOT_EXTRACTING:
            pos+=D(ITEM_TEXT_OFS_Y);
            if(installmode)
            {
                if(installmode==MODE_INSTALLING)
                {
                wsprintf(bufw,L"%s (%d%s%d)",STR(itembar->install_status),
                        items_list[SLOT_EXTRACTING].val1+1,STR(STR_OF),
                        items_list[SLOT_EXTRACTING].val2);

                }
                else
                    if(itembar->install_status)wsprintf(bufw,STR(itembar->install_status),itembar->percent);

                SetTextColor(hdc,D(boxindex[itembar->box_status()]+14));
                TextOutH(hdc,x+D(ITEM_TEXT_OFS_X),pos,bufw);
                if(itembar_act>=RES_SLOTS)
                {
                    wsprintf(bufw,L"%S",items_list[itembar_act].hwidmatch->getdrp_drvdesc());
                    SetTextColor(hdc,D(boxindex[itembar->box_status()]+15));
                    TextOutH(hdc,x+D(ITEM_TEXT_OFS_X),pos+D(ITEM_TEXT_DIST_Y),bufw);
                }
            }else
            {
                wsprintf(bufw,L"%s",STR(itembar->install_status));
                SetTextColor(hdc,D(boxindex[itembar->box_status()]+14));
                TextOutH(hdc,x+D(ITEM_TEXT_OFS_X),pos,bufw);
                wsprintf(bufw,L"%s",STR(STR_INST_CLOSE));
                SetTextColor(hdc,D(boxindex[itembar->box_status()]+15));
                TextOutH(hdc,x+D(ITEM_TEXT_OFS_X),pos+D(ITEM_TEXT_DIST_Y),bufw);
            }
            break;

        case SLOT_NODRIVERS:
            itembar->drawbutton(hdc,x,pos,STR(STR_EMPTYDRP),matcher->getCol()->getDriverpack_dir());
            break;

        case SLOT_NOUPDATES:
            pos+=D(ITEM_TEXT_OFS_Y);
            wsprintf(bufw,L"%s",STR(items_list.size()>RES_SLOTS?STR_NOUPDATES:STR_INITIALIZING));
            SetTextColor(hdc,D(boxindex[itembar->box_status()]+14));
            TextOutH(hdc,x+D(ITEM_TEXT_OFS_X),pos+D(ITEM_TEXT_DIST_Y)/2,bufw);
            break;

        case SLOT_DOWNLOAD:
            if(itembar->val1>>8)
                wsprintf(bufw,STR(itembar->val1&0xFF?STR_UPD_AVAIL3:STR_UPD_AVAIL1),itembar->val1>>8,itembar->val1&0xFF);
            else
                wsprintf(bufw,STR(STR_UPD_AVAIL2),itembar->val1&0xFF);

#ifdef USE_TORRENT
            if(!Updater.isPaused())
            {
                Updater.showProgress(bufw);
                itembar->drawbutton(hdc,x,pos,bufw,STR(STR_UPD_MODIFY));
            }
            else
#endif
                itembar->drawbutton(hdc,x,pos,bufw,STR(STR_UPD_START));

            break;

        case SLOT_SNAPSHOT:
            itembar->drawbutton(hdc,x,pos,state_file,STR(STR_CLOSE_SNAPSHOT));
            break;

        case SLOT_DPRDIR:
            itembar->drawbutton(hdc,x,pos,drpext_dir,STR(STR_CLOSE_DRPEXT));
            break;

        case SLOT_VIRUS_AUTORUN:
            itembar->drawbutton(hdc,x,pos,STR(STR_VIRUS),STR(STR_VIRUS_AUTORUN));
            break;

        case SLOT_VIRUS_RECYCLER:
            itembar->drawbutton(hdc,x,pos,STR(STR_VIRUS),STR(STR_VIRUS_RECYCLER));
            break;

        case SLOT_VIRUS_HIDDEN:
            itembar->drawbutton(hdc,x,pos,STR(STR_VIRUS),STR(STR_VIRUS_HIDDEN));
            break;

        default:
            if(itembar->first&2)
            {
                    /*wsprintf(bufw,L"%ws",matcher->state->text+itembar->devicematch->device->Devicedesc);
                    SetTextColor(hdc,D(boxindex[box_status(index)]+14));
                    TextOutH(hdc,x+D(ITEM_TEXT_OFS_X),pos,bufw);*/

                    //str_status(bufw,itembar);
                    wsprintf(bufw,L"%ws",itembar->hwidmatch->getdrp_packname());
                    SetTextColor(hdc,D(boxindex[itembar->box_status()]+15));
                    TextOutH(hdc,x+D(ITEM_CHECKBOX_OFS_X),pos+D(ITEM_TEXT_DIST_Y)+5,bufw);
                    break;
            }
            if(itembar->hwidmatch)
            {
                // Checkbox
                drawcheckbox(hdc,x+D(ITEM_CHECKBOX_OFS_X),pos+D(ITEM_CHECKBOX_OFS_Y),
                         D(ITEM_CHECKBOX_SIZE),D(ITEM_CHECKBOX_SIZE),
                         itembar->checked,zone>=0);

                // Available driver desc
                pos+=D(ITEM_TEXT_OFS_Y);
                wsprintf(bufw,L"%S",itembar->hwidmatch->getdrp_drvdesc());
                SetTextColor(hdc,D(boxindex[itembar->box_status()]+14));
                RECT rect;
                int wx1=wx-D(ITEM_TEXT_OFS_X)-D(ITEM_ICON_OFS_X);
                rect.left=x+D(ITEM_TEXT_OFS_X);
                rect.top=pos;
                if(intend)wx1-=D(DRVITEM_LINE_INTEND);
                rect.right=rect.left+wx1/2;
                rect.bottom=rect.top+90;
                if(oldstyle)
                    TextOutH(hdc,x+D(ITEM_TEXT_OFS_X),pos,bufw);
                else
                    DrawText(hdc,bufw,-1,&rect,DT_WORDBREAK);


                // Available driver status
                SetTextColor(hdc,D(boxindex[itembar->box_status()]+15));
                itembar->str_status(bufw);
                switch(itembar->install_status)
                {
                    case STR_INST_FAILED:
                    case STR_EXTR_FAILED:
                        wsprintf(bufw,L"%s %X",STR(itembar->install_status),itembar->val1);
                        break;

                    case STR_INST_EXTRACT:
                        wsprintf(bufw,STR(STR_INST_EXTRACT),(itembar->percent+100)/10);
                        break;

                    case STR_EXTR_EXTRACTING:
                        wsprintf(bufw,L"%s %d%%",STR(STR_EXTR_EXTRACTING),itembar->percent/10);
                        break;

                    case 0:
                        break;

                    default:
                        wcscpy(bufw,STR(itembar->install_status));
                }
                rect.left=x+D(ITEM_TEXT_OFS_X)+wx1/2;
                rect.top=pos;
                rect.right=rect.left+wx1/2;
                rect.bottom=rect.top+90;
                if(oldstyle)
                    TextOutH(hdc,x+D(ITEM_TEXT_OFS_X),pos+D(ITEM_TEXT_DIST_Y),bufw);
                else
                    DrawText(hdc,bufw,-1,&rect,DT_WORDBREAK);

                if(flags&FLAG_SHOWDRPNAMES1)
                {
                    int len=wcslen(matcher->getCol()->getDriverpack_dir());
                    int lnn=len-wcslen(itembar->hwidmatch->getdrp_packpath());

                    SetTextColor(hdc,0);// todo: color
                    wsprintf(bufw,L"%ws%ws%ws",
                            itembar->hwidmatch->getdrp_packpath()+len+(lnn?1:0),
                            lnn?L"\\":L"",
                            itembar->hwidmatch->getdrp_packname());
                    TextOutH(hdc,x+wx-240,pos+D(ITEM_TEXT_DIST_Y),bufw);
                }
            }
            else
            {
                // Device desc
                if(itembar->devicematch)
                {
                    wsprintf(bufw,L"%ws",matcher->getState()->textas.get(itembar->devicematch->device->Devicedesc));
                    SetTextColor(hdc,D(boxindex[itembar->box_status()]+14));
                    RECT rect;
                    int wx1=wx-D(ITEM_TEXT_OFS_X)-D(ITEM_ICON_OFS_X);
                    rect.left=x+D(ITEM_TEXT_OFS_X);
                    rect.top=pos;
                    rect.right=rect.left+wx1/2;
                    rect.bottom=rect.top+90;
                    if(oldstyle)
                        TextOutH(hdc,x+D(ITEM_TEXT_OFS_X),pos,bufw);
                    else
                        DrawText(hdc,bufw,-1,&rect,DT_WORDBREAK);

                    itembar->str_status(bufw);
                    SetTextColor(hdc,D(boxindex[itembar->box_status()]+15));
                    rect.left=x+D(ITEM_TEXT_OFS_X)+wx1/2;
                    rect.top=pos;
                    rect.right=rect.left+wx1/2;
                    rect.bottom=rect.top+90;
                    if(oldstyle)
                        TextOutH(hdc,x+D(ITEM_TEXT_OFS_X),pos+D(ITEM_TEXT_DIST_Y),bufw);
                    else
                        DrawText(hdc,bufw,-1,&rect,DT_WORDBREAK);
                }
            }
            // Device icon
            if(itembar->devicematch&&SetupDiLoadClassIcon(&itembar->devicematch->device->DeviceInfoData.ClassGuid,&hIcon,nullptr))
            {
                if(rtl)
                {
                    HICON miricon;
                    miricon=CreateMirroredIcon(hIcon);
                    DestroyIcon(hIcon);
                    hIcon=miricon;
                }
                DrawIconEx(hdc,x+D(ITEM_ICON_OFS_X),pos+D(ITEM_ICON_OFS_Y),hIcon,D(ITEM_ICON_SIZE),D(ITEM_ICON_SIZE),0,nullptr,DI_NORMAL);
                DestroyIcon(hIcon);
            }

            // Expand icon
            if(groupsize(itembar->index)>1&&itembar->first&1)
            {
                int xo=x+wx-D(ITEM_ICON_SIZE)*2+10;
                icon[(itembar->isactive&2?0:2)+(zone==2?1:0)].draw(hdc,xo,pos,xo+32,pos+32,0,Image::HSTR|Image::VSTR);
            }
            break;

    }

    SelectClipRgn(hdc,nullptr);
    DeleteObject(hrgn);
    DeleteObject(hrgn2);
    return 1;
}

int Manager::isbehind(int pos,int ofsy,int j)
{
    itembar_t *itembar;

    if(j<SLOT_RESTORE_POINT)return 0;
    if(pos-ofsy<=-D(DRVITEM_DIST_Y0))return 1;
    if(pos-ofsy>mainy_c)return 1;

    itembar=&items_list[j-1];
    if((itembar->curpos>>16)==pos)return 1;

    return 0;
}

int Manager::calc_cutoff()
{
    int i,cutoff=0;

    for(i=0;i<SLOT_RESTORE_POINT;i++)
        if(items_list[i].isactive)cutoff=(items_list[i].curpos>>16);

    return cutoff;
}

void Manager::draw(HDC hdc,int ofsy)
{
    itembar_t *itembar;
    int i;
    int maxpos=0;
    int nm=0;
    int cur_i,zone;
    int cutoff=0;
    POINT p;
    RECT rect;

    GetCursorPos(&p);
    ScreenToClient(hField,&p);
    hitscan(p.x,p.y,&cur_i,&zone);

    GetClientRect(hField,&rect);
    drawbox(hdc,0,0,rect.right,rect.bottom,BOX_DRVLIST);

    cutoff=calc_cutoff();
    items_list[itembar_act].updatecur();
    updateoverall();
    for(i=items_list.size()-1;i>=0;i--)
    {
        itembar=&items_list[i];
        if(itembar->isactive)continue;

        if(isbehind((itembar->curpos>>16),ofsy,i))continue;
        nm+=drawitem(hdc,i,ofsy,-1,cutoff);
    }
    for(i=items_list.size()-1;i>=0;i--)
    {
        itembar=&items_list[i];
        if(itembar->isactive==0)continue;

        if(itembar->curpos>maxpos)maxpos=itembar->curpos;
        nm+=drawitem(hdc,i,ofsy,cur_i==i?zone:-1,cutoff);

    }
    //printf("nm:%3d, ofs:%d\n",nm,ofsy);
    setscrollrange((maxpos>>16)+20);
}

int itembar_cmp(itembar_t *a,itembar_t *b,wchar_t *ta,wchar_t *tb)
{
    if(a->hwidmatch&&b->hwidmatch)
    {
        if(a->hwidmatch->getHWID_index()==b->hwidmatch->getHWID_index())return 3;
        return 0;
    }
    if(wcslen(ta+a->devicematch->device->getDriver())>0)
    {
        if(!StrCmpW(ta+a->devicematch->device->getDriver(),tb+b->devicematch->device->getDriver()))return wcslen(ta+a->devicematch->device->getDriver())+10;
    }
    else
    {
        if(wcslen(ta+a->devicematch->device->getDescr())>0)
        {
            if(!StrCmpW(ta+a->devicematch->device->getDescr(),tb+b->devicematch->device->getDescr()))return 100+wcslen(ta+a->devicematch->device->getDescr());
        }
    }

    return 0;
}

void Manager::restorepos1(Manager *manager_prev)
{
    int i;

    memcpy(&items_list.front(),&manager_prev->items_list.front(),sizeof(itembar_t)*RES_SLOTS);
    populate();
    filter(filters);
    items_list[SLOT_SNAPSHOT].isactive=statemode==STATEMODE_EMUL?1:0;
    items_list[SLOT_DPRDIR].isactive=*drpext_dir?1:0;
    restorepos(manager_prev);
    //viruscheck(L"",0,0);
    setpos();
    log_con("}Sync\n");
    invaidate_set=0;
    LeaveCriticalSection(&sync);

#ifdef USE_TORRENT
    UpdateDialog.populate(0);
#endif
    //log_con("Mode in WM_BUNDLEREADY: %d\n",installmode);
    if(flags&FLAG_AUTOINSTALL)
    {
        int cnt=0;
        if(installmode==MODE_SCANNING)
        {
            if(!panels[11].isChecked(3))selectall();
            itembar_t *itembar=&items_list[RES_SLOTS];
            for(i=RES_SLOTS;(unsigned)i<items_list.size();i++,itembar++)
                if(itembar->checked)
            {
                cnt++;
            }

            if(!cnt)flags&=~FLAG_AUTOINSTALL;
            log_con("Autoinstall rescan: %d found\n",cnt);
        }

        if(installmode==MODE_NONE||(installmode==MODE_SCANNING&&cnt))
        {
            if(!panels[11].isChecked(3))selectall();
            if((flags&FLAG_EXTRACTONLY)==0)
            wsprintf(extractdir,L"%s\\SDI",matcher->getState()->textas.get(matcher->getState()->getTemp()));
            manager_install(INSTALLDRIVERS);
        }
        else
        {
            wchar_t buf[BUFLEN];

            installmode=MODE_NONE;
            if(panels[11].isChecked(3))
                wcscpy(buf,L" /c Shutdown.exe -r -t 3");
            else
                wsprintf(buf,L" /c %s",needreboot?finish_rb:finish);

            if(*(needreboot?finish_rb:finish)||panels[11].isChecked(3))
                run_command(L"cmd",buf,SW_HIDE,0);

            if(flags&FLAG_AUTOCLOSE)PostMessage(hMain,WM_CLOSE,0,0);
        }
    }
    else
        if(installmode==MODE_SCANNING)installmode=MODE_NONE;
}

//Keep when:
//* installing drivers
//* device update
//Discard when:
//* loading a snapshot
//* returning to real machine
//* driverpack update
void Manager::restorepos(Manager *manager_old)
{
    itembar_t *itembar_new,*itembar_old;
    wchar_t *t_new,*t_old;
    unsigned i,j;
    int show_changes=manager_old->items_list.size()>20;

    //if(statemode==STATEMODE_LOAD)show_changes=0;
    if((log_verbose&LOG_VERBOSE_DEVSYNC)==0)show_changes=0;
    //show_changes=1;

    t_old=manager_old->matcher->getState()->textas.getw(0);
    t_new=matcher->getState()->textas.getw(0);

    if(manager_old->items_list[SLOT_EMPTY].curpos==1)
    {
        return;
    }
    if(invaidate_set&INVALIDATE_MANAGER)return;
    if((instflag&RESTOREPOS)==0)
    {
        instflag^=RESTOREPOS;
        return;
    }

    log_con("{Updated %d->%d\n",manager_old->items_list.size(),items_list.size());
    log_console=0;
    itembar_new=&items_list[RES_SLOTS];
    for(i=RES_SLOTS;i<items_list.size();i++,itembar_new++)
    {
        itembar_old=&manager_old->items_list[RES_SLOTS];

        if(itembar_act&&itembar_cmp(itembar_new,&manager_old->items_list[itembar_act],t_new,t_old))
        {
            log_con("Act %d -> %d\n",itembar_act,i);
            itembar_act=i;
        }

        for(j=RES_SLOTS;j<manager_old->items_list.size();j++,itembar_old++)
        {
            if(itembar_old->isactive!=9)
            {
                if(itembar_cmp(itembar_new,itembar_old,t_new,t_old))
                {
                    wcscpy(itembar_new->txt1,itembar_old->txt1);
                    itembar_new->install_status=itembar_old->install_status;
                    itembar_new->val1=itembar_old->val1;
                    itembar_new->val2=itembar_old->val2;
                    itembar_new->percent=itembar_old->percent;

                    itembar_new->isactive=itembar_old->isactive;
                    itembar_new->checked=itembar_old->checked;

                    itembar_new->oldpos=itembar_old->oldpos;
                    itembar_new->curpos=itembar_old->curpos;
                    itembar_new->tagpos=itembar_old->tagpos;
                    itembar_new->accel=itembar_old->accel;

                    itembar_old->isactive=9;
                    break;
                }
            }
        }
        if(show_changes)
        if(j==manager_old->items_list.size())
        {
            log_con("\nAdded   $%04d|%S|%S|",i,t_new+itembar_new->devicematch->device->Driver,
                    t_new+itembar_new->devicematch->device->Devicedesc);

            if(itembar_new->hwidmatch)
            {
                int limits[7];
                memset(limits,0,sizeof(limits));
                log_con("%d|\n",itembar_new->hwidmatch->getHWID_index());
                itembar_new->hwidmatch->print_tbl(limits);
            }
            else
                itembar_new->devicematch->device->print(matcher->getState());
        }
    }

    itembar_old=&manager_old->items_list[RES_SLOTS];
    if(show_changes)
    for(j=RES_SLOTS;j<manager_old->items_list.size();j++,itembar_old++)
    {
        if(itembar_old->isactive!=9)
        {
            log_con("\nDeleted $%04d|%S|%S|",j,t_old+itembar_old->devicematch->device->Driver,
                    t_old+itembar_old->devicematch->device->getDescr());
            if(itembar_old->hwidmatch)
            {
                int limits[7];
                memset(limits,0,sizeof(limits));
                log_con("%d|\n",itembar_old->hwidmatch->getHWID_index());
                itembar_old->hwidmatch->print_tbl(limits);
            }
            else
                itembar_old->devicematch->device->print(manager_old->matcher->getState());

        }
    }
    log_console=0;
    log_con("}Updated\n");
}
//}

//{ Draw
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

//{ Popup
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

void Manager::popup_driverlist(HDC hdcMem,RECT rect,unsigned i)
{
    itembar_t *itembar;
    POINT p;
    wchar_t i_hwid[BUFLEN];
    wchar_t bufw[BUFLEN];
    int lne=D(POPUP_WY);
    unsigned k;
    int maxsz=0;
    int limits[30];
    int c0=D(POPUP_TEXT_COLOR);
    textdata_t td;

    if(i<RES_SLOTS)return;

    td.hdcMem=hdcMem;
    td.i=0;
    td.limits=limits;
    td.x=D(POPUP_OFSX)+horiz_sh;
    td.y=D(POPUP_OFSY);
    td.col=0;
    td.mode=1;

    int group=items_list[i].index;
    Driver *cur_driver=items_list[i].devicematch->driver;
    char *t=matcher->getState()->textas.get(0);

    memset(limits,0,sizeof(limits));

    itembar=&items_list[0];
    for(k=0;k<items_list.size();k++,itembar++)
        if(itembar->index==group&&itembar->hwidmatch)
            itembar->hwidmatch->popup_driverline(limits,hdcMem,td.y,0,k);


    TextOut_CM(hdcMem,10,td.y,STR(STR_HINT_INSTDRV),c0,&maxsz,1);td.y+=lne;

    if(cur_driver)
    {
        wsprintf(bufw,L"%s",t+cur_driver->MatchingDeviceId);
        for(k=0;bufw[k];k++)i_hwid[k]=toupper(bufw[k]);i_hwid[k]=0;
        cur_driver->version.str_date(bufw);

        TextOutP(&td,L"$%04d",i);
        td.x+=limits[td.i++];
        td.col=c0;
        TextOutP(&td,L"| %08X",cur_driver->calc_score_h(matcher->getState()));
        TextOutP(&td,L"| %s",bufw);
        for(k=0;k<6;k++)td.x+=limits[td.i++];
        TextOutP(&td,L"| %s%s",t+matcher->getState()->getWindir(),t+cur_driver->InfPath);
        TextOutP(&td,L"| %s",t+cur_driver->ProviderName);cur_driver->version.str_version(bufw);
        TextOutP(&td,L"| %s",bufw);
        TextOutP(&td,L"| %s",i_hwid);
        TextOutP(&td,L"| %s",t+cur_driver->DriverDesc);
        td.y+=lne;
    }
    td.y+=lne;
    TextOut_CM(hdcMem,10,td.y,STR(STR_HINT_AVAILDRVS),c0,&maxsz,1);td.y+=lne;

    itembar=&items_list[0];
    for(k=0;k<items_list.size();k++,itembar++)
        if(itembar->index==group&&itembar->hwidmatch&&(itembar->first&2)==0)
    {
        if(k==i)
        {
            SelectObject(hdcMem,GetStockObject(DC_BRUSH));
            SelectObject(hdcMem,GetStockObject(DC_PEN));
//            SetDCBrushColor(hdcMem,RGB(115,125,255));
            SetDCBrushColor(hdcMem,RGB(255,255,255));//todo: color
            Rectangle(hdcMem,D(POPUP_OFSX)+horiz_sh,td.y,rect.right+horiz_sh-D(POPUP_OFSX),td.y+lne);
        }
        itembar->hwidmatch->popup_driverline(limits,hdcMem,td.y,1,k);
        td.y+=lne;
    }

    GetWindowRect(GetDesktopWindow(),&rect);
    p.y=0;p.x=0;
    ClientToScreen(hPopup,&p);

    maxsz=0;
    for(k=0;k<30;k++)maxsz+=limits[k];
    if(p.x+maxsz+D(POPUP_OFSX)*3>rect.right)
    {
        td.y+=lne;
        TextOut_CM(hdcMem,D(POPUP_OFSX),td.y,STR(STR_HINT_SCROLL),c0,&maxsz,1);
        td.y+=lne;
    }
    popup_resize(maxsz+D(POPUP_OFSX)*3,td.y+D(POPUP_OFSY));
}

int Hwidmatch::pickcat(State *state)
{
    if(state->getArchitecture()==1&&*getdrp_drvcat(CatalogFile_ntamd64))
    {
        return CatalogFile_ntamd64;
    }
    else if(*getdrp_drvcat(CatalogFile_ntx86))
    {
        return CatalogFile_ntx86;
    }

    if(*getdrp_drvcat(CatalogFile_nt))
       return CatalogFile_nt;

    if(*getdrp_drvcat(CatalogFile))
       return CatalogFile;

    return 0;
}

int Hwidmatch::isvalidcat(State *state)
{
    CHAR bufa[BUFLEN];
    int n=pickcat(state);
    const char *s=getdrp_drvcat(n);

    int major,minor;
    state->getWinVer(&major,&minor);
    wsprintfA(bufa,"2:%d.%d",major,minor);
    if(!*s)return 0;
    return strstr(s,bufa)?1:0;
}

void itembar_t::popup_drivercmp(Manager *manager,HDC hdcMem,RECT rect,int index1)
{
    if(index1<RES_SLOTS)return;

    //itembar_t *itembar=&manager->items_list[index];
    Devicematch *devicematch_f=devicematch;
    Hwidmatch *hwidmatch_f=hwidmatch;
    State *state=manager->matcher->getState();

    wchar_t bufw[BUFLEN];
    wchar_t i_hwid[BUFLEN];
    wchar_t a_hwid[BUFLEN];

    char *t=state->textas.get(0);
    int maxln=0;
    int bolder=rect.right/2;
    wchar_t *p;
    Driver *cur_driver=nullptr;
    textdata_t td;
    version_t *a_v=nullptr;
    unsigned score=0;
    int cm_ver=0,cm_date=0,cm_score=0,cm_hwid=0;
    int c0=D(POPUP_TEXT_COLOR),cb=D(POPUP_CMP_BETTER_COLOR);
    int p0=D(POPUP_OFSX),p1=D(POPUP_OFSX)+10;


    td.y=D(POPUP_OFSY);
    td.wy=D(POPUP_WY);
    td.hdcMem=hdcMem;
    td.maxsz=0;

    if(devicematch_f->driver)
    {
        int i;
        cur_driver=devicematch_f->driver;
        wsprintf(bufw,L"%s",t+cur_driver->MatchingDeviceId);
        for(i=0;bufw[i];i++)i_hwid[i]=toupper(bufw[i]);i_hwid[i]=0;
    }
    if(hwidmatch_f)
    {
        a_v=hwidmatch_f->getdrp_drvversion();
        wsprintf(a_hwid,L"%S",hwidmatch_f->getdrp_drvHWID());
    }
    if(cur_driver&&hwidmatch_f)
    {
        int r=cmpdate(&cur_driver->version,a_v);
        if(r>0)cm_date=1;
        if(r<0)cm_date=2;

        score=cur_driver->calc_score_h(state);
        if(score<hwidmatch_f->getScore())cm_score=1;
        if(score>hwidmatch_f->getScore())cm_score=2;

        r=cmpversion(&cur_driver->version,a_v);
        if(r>0)cm_ver=1;
        if(r<0)cm_ver=2;
    }

    // Device info (hwidmatch_f,devicematch_f)
    td.x=p0;TextOutF(&td,c0,L"%s",STR(STR_HINT_ANALYSIS));td.x=p1;
    TextOutF(&td,c0,L"$%04d",index1);
    if(hwidmatch_f)
    {
        TextOutF(&td,hwidmatch_f->isvalidcat(state)?cb:D(POPUP_CMP_INVALID_COLOR),
                 L"%s(%d)%S",STR(STR_HINT_SIGNATURE),hwidmatch_f->pickcat(state),hwidmatch_f->getdrp_drvcat(hwidmatch_f->pickcat(state)));

        td.x=p0;TextOutF(&td,c0,L"%s",STR(STR_HINT_DRP));td.x=p1;
        TextOutF(&td,c0,L"%s\\%s",hwidmatch_f->getdrp_packpath(),hwidmatch_f->getdrp_packname());
        TextOutF(&td,hwidmatch_f->calc_notebook()?c0:D(POPUP_CMP_INVALID_COLOR)
                 ,L"%S%S",hwidmatch_f->getdrp_infpath(),hwidmatch_f->getdrp_infname());
    }

    SetupDiGetClassDescription(&devicematch_f->device->DeviceInfoData.ClassGuid,bufw,BUFLEN,nullptr);

    td.x=p0;TextOutF(&td,c0,L"%s",STR(STR_HINT_DEVICE));td.x=p1;
    TextOutF(&td,c0,L"%s",t+devicematch_f->device->getDescr());
    TextOutF(&td,c0,L"%s%s",STR(STR_HINT_MANUF),t+devicematch_f->device->Mfg);
    TextOutF(&td,c0,L"%s",bufw);
    TextOutF(&td,c0,L"%s",t+devicematch_f->device->Driver);
    wsprintf(bufw,STR(STR_STATUS_NOTPRESENT+devicematch_f->device->print_status()),devicematch_f->device->problem);
    TextOutF(&td,c0,L"%s",bufw);

    // HWID list (devicematch_f)
    maxln=td.y;
    td.y=D(POPUP_OFSY);
    if(devicematch_f->device->HardwareID)
    {
        td.x=p0+bolder;TextOutF(&td,c0,L"%s",STR(STR_HINT_HARDWAREID));td.x=p1+bolder;
        p=(wchar_t *)(t+devicematch_f->device->HardwareID);
        while(*p)
        {
            int pp=0;
            if(!StrCmpIW(i_hwid,p))pp|=1;
            if(!StrCmpIW(a_hwid,p))pp|=2;
            if(!cm_hwid&&(pp==1||pp==2))cm_hwid=pp;
            TextOutF(&td,pp?D(POPUP_HWID_COLOR):c0,L"%s",p);
            p+=lstrlenW(p)+1;
        }
    }
    if(devicematch_f->device->CompatibleIDs)
    {
        td.x=p0+bolder;TextOutF(&td,c0,L"%s",STR(STR_HINT_COMPID));td.x=p1+bolder;
        p=(wchar_t *)(t+devicematch_f->device->CompatibleIDs);
        while(*p)
        {
            int pp=0;
            if(!StrCmpIW(i_hwid,p))pp|=1;
            if(!StrCmpIW(a_hwid,p))pp|=2;
            if(!cm_hwid&&(pp==1||pp==2))cm_hwid=pp;
            TextOutF(&td,pp?D(POPUP_HWID_COLOR):c0,L"%s",p);
            p+=lstrlenW(p)+1;
        }
    }
    if(!cur_driver||!hwidmatch_f)cm_hwid=0;
    if(td.y>maxln)maxln=td.y;
    maxln+=td.wy;
    td.y=maxln;

    // Cur driver (cur_driver)
    if(cur_driver||hwidmatch_f)
    {
        MoveToEx(hdcMem,0,td.y-td.wy/2,nullptr);
        LineTo(hdcMem,rect.right,td.y-td.wy/2);
    }
    if(devicematch_f->device->HardwareID||hwidmatch_f)
    {
        MoveToEx(hdcMem,bolder,0,nullptr);
        LineTo(hdcMem,bolder,rect.bottom);
    }
    if(cur_driver)
    {
        cur_driver->version.str_date(bufw);

        td.x=p0;
        TextOutF(&td,               c0,L"%s",STR(STR_HINT_INSTDRV));td.x=p1;
        TextOutF(&td,               c0,L"%s",t+cur_driver->DriverDesc);
        TextOutF(&td,               c0,L"%s%s",STR(STR_HINT_PROVIDER),t+cur_driver->ProviderName);
        TextOutF(&td,cm_date ==1?cb:c0,L"%s%s",STR(STR_HINT_DATE),bufw);cur_driver->version.str_version(bufw);
        TextOutF(&td,cm_ver  ==1?cb:c0,L"%s%s",STR(STR_HINT_VERSION),bufw);
        TextOutF(&td,cm_hwid ==1?cb:c0,L"%s%s",STR(STR_HINT_ID),i_hwid);
        TextOutF(&td,               c0,L"%s%s",STR(STR_HINT_INF),t+cur_driver->InfPath);
        TextOutF(&td,               c0,L"%s%s%s",STR(STR_HINT_SECTION),t+cur_driver->InfSection,t+cur_driver->InfSectionExt);
        TextOutF(&td,cm_score==1?cb:c0,L"%s%08X",STR(STR_HINT_SCORE),score);
    }

    // Available driver (hwidmatch_f)
    if(hwidmatch_f)
    {
        td.y=maxln;
        a_v->str_date(bufw);
        hwidmatch_f->getdrp_drvsection((CHAR *)(bufw+500));

        td.x=p0+bolder;
        TextOutF(&td,               c0,L"%s",STR(STR_HINT_AVAILDRV));td.x=p1+bolder;wsprintf(bufw+1000,L"%S",hwidmatch_f->getdrp_drvdesc());
        TextOutF(&td,               c0,L"%s",bufw+1000);
        TextOutF(&td,               c0,L"%s%S",STR(STR_HINT_PROVIDER),hwidmatch_f->getdrp_drvmanufacturer());
        TextOutF(&td,cm_date ==2?cb:c0,L"%s%s",STR(STR_HINT_DATE),bufw);a_v->str_version(bufw);
        TextOutF(&td,cm_ver  ==2?cb:c0,L"%s%s",STR(STR_HINT_VERSION),bufw);
        TextOutF(&td,cm_hwid ==2?cb:c0,L"%s%S",STR(STR_HINT_ID),hwidmatch_f->getdrp_drvHWID());
        TextOutF(&td,               c0,L"%s%S%S",STR(STR_HINT_INF),hwidmatch_f->getdrp_infpath(),hwidmatch_f->getdrp_infname());
        TextOutF(&td,hwidmatch_f->getDecorscore()?c0:D(POPUP_CMP_INVALID_COLOR),L"%s%S",STR(STR_HINT_SECTION),bufw+500);
        TextOutF(&td,cm_score==2?cb:c0,L"%s%08X",STR(STR_HINT_SCORE),hwidmatch_f->getScore());
    }

    if(!devicematch_f->device->HardwareID&&!hwidmatch_f)td.maxsz/=2;

    popup_resize((td.maxsz+10+p0*2)*2,td.y+D(POPUP_OFSY));
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
