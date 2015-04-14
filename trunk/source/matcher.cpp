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
#include "device.h"

//{ Global variables

/*
Invalid:
    me          Windows ME
    ntx64       ntAMD64
    ntai64      ntIA64
    ntarm       ARM
    ntx64.6.0   ntAMD64
    nt.7        future
*/
const char *nts[NUM_DECS]=
{
    "nt.5",  "ntx86.5",  "ntamd64.5"  ,"ntia64.5",   // 2000
    "nt.5.0","ntx86.5.0","ntamd64.5.0","ntia64.5.0", // 2000
    "nt.5.1","ntx86.5.1","ntamd64.5.1","ntia64.5.1", // XP
    "nt.5.2","ntx86.5.2","ntamd64.5.2","ntia64.5.2", // Server 2003
    "nt.6",  "ntx86.6",  "ntamd64.6"  ,"ntia64.6",   // Vista
    "nt.6.0","ntx86.6.0","ntamd64.6.0","ntia64.6.0", // Vista
    "nt.6.1","ntx86.6.1","ntamd64.6.1","ntia64.6.1", // 7
    "nt.6.2","ntx86.6.2","ntamd64.6.2","ntia64.6.2", // 8
    "nt.6.3","ntx86.6.3","ntamd64.6.3","ntia64.6.3", // 10
    "nt.10.0","ntx86.10.3","ntamd64.10.0","ntia64.10.0", // 10
    "nt",    "ntx86",    "ntamd64",    "ntia64",
    "nt..",  "ntx86..",  "ntamd64..",  "ntia64..",
};

const int nts_version[NUM_DECS]=
{
    50,    50,    50,    50, // 2000
    50,    50,    50,    50, // 2000
    51,    51,    51,    51, // XP
    52,    52,    52,    52, // Server 2003
    60,    60,    60,    60, // Vista
    60,    60,    60,    60, // Vista
    61,    61,    61,    61, // 7
    62,    62,    62,    62, // 8
    63,    63,    63,    63, // 10
    63,    63,    63,    63, // 10
     0,     0,     0,     0,
     0,     0,     0,     0,
};

const int nts_arch[NUM_DECS]=
{
    0,  1,  2,  3, // 2000
    0,  1,  2,  3, // 2000
    0,  1,  2,  3, // XP
    0,  1,  2,  3, // Serve
    0,  1,  2,  3, // Vista
    0,  1,  2,  3, // Vista
    0,  1,  2,  3, // 7
    0,  1,  2,  3, // 8
    0,  1,  2,  3, // 10
    0,  1,  2,  3, // 10
    0,  1,  2,  3,
    0,  1,  2,  3,
};

const int nts_score[NUM_DECS]=
{
    50,   150,   150,   150, // 2000
    50,   150,   150,   150, // 2000
    51,   151,   151,   151, // XP
    52,   152,   152,   152, // Server 2003
    60,   160,   160,   160, // Vista
    60,   160,   160,   160, // Vista
    61,   161,   161,   161, // 7
    62,   162,   162,   162, // 8
    63,   163,   163,   163, // 10
    10,   100,   100,   100, // 10
    10,   100,   100,   100,
    10,   100,   100,   100,
};

const markers_t markers[NUM_MARKERS]=
{
    // Exact x86
    {"5x86",    5, 1, 0},
    {"6x86",    6, 0, 0},
    {"7x86",    6, 1, 0},
    {"8x86",    6, 2, 0},
    {"81x86",   6, 3, 0},
    {"9x86",    6, 4, 0},
    {"10x86",  10, 0, 0},

    // Exact x64
    {"5x64",    5, 2, 1},
    {"6x64",    6, 0, 1},
    {"7x64",    6, 1, 1},
    {"8x64",    6, 2, 1},
    {"81x64",   6, 3, 1},
    {"9x64",    6, 4, 1},
    {"10x64",  10, 0, 1},

    // Each OS, ignore arch
    {"allnt",   4, 0,-1},
    {"allxp",   5, 0,-1},
    {"all6",    6, 0,-1},
    {"all7",    6, 1,-1},
    {"all8\\",  6, 2,-1},
    {"all81",   6, 3,-1},
    {"all9",    6, 4,-1},
    {"all10",  10, 0,-1},

    // arch
    {"allx86", -1,-1, 0},
    {"allx64", -1,-1, 1},
    {"all8x86", 6, 2, 0},
    {"all8x64", 6, 2, 1},
    {"ntx86",  -1,-1, 0},
    {"ntx64",  -1,-1, 1},
    {"x86\\",  -1,-1, 0},
    {"x64\\",  -1,-1, 1},

    {"winall",  0, 0, 0},
};

char marker[BUFLEN];
int isLaptop;

const WCHAR *Filter_1[]={L"Acer",L"acer",L"emachines",L"packard",L"bell",L"gateway",L"aspire",0};
const WCHAR *Filter_2[]={L"Apple",L"apple",0};
const WCHAR *Filter_3[]={L"Asus",L"asus",0};
const WCHAR *Filter_4[]={L"OEM",L"clevo",L"eurocom",L"sager",L"iru",L"viewsonic",L"viewbook",0};
const WCHAR *Filter_5[]={L"Dell",L"dell",L"alienware",L"arima",L"jetway",L"gericom",0};
const WCHAR *Filter_6[]={L"Fujitsu",L"fujitsu",L"sieme",0};
const WCHAR *Filter_7[]={L"OEM",L"ecs",L"elitegroup",L"roverbook",L"rover",L"shuttle",0};
const WCHAR *Filter_8[]={L"HP",L"hp",L"hewle",L"compaq",0};
const WCHAR *Filter_9[]={L"OEM",L"intel",L"wistron",0};
const WCHAR *Filter_10[]={L"Lenovo",L"lenovo",L"compal",L"ibm",0};
const WCHAR *Filter_11[]={L"LG",L"lg",0};
const WCHAR *Filter_12[]={L"OEM",L"mitac",L"mtc",L"depo",L"getac",0};
const WCHAR *Filter_13[]={L"MSI",L"msi",L"micro-star",0};
const WCHAR *Filter_14[]={L"Panasonic",L"panasonic",L"matsushita",0};
const WCHAR *Filter_15[]={L"OEM",L"quanta",L"prolink",L"nec",L"k-systems",L"benq",L"vizio",0};
const WCHAR *Filter_16[]={L"OEM",L"pegatron",L"medion",0};
const WCHAR *Filter_17[]={L"Samsung",L"samsung",0};
const WCHAR *Filter_18[]={L"Gigabyte",L"gigabyte",0};
const WCHAR *Filter_19[]={L"Sony",L"sony",L"vaio",0};
const WCHAR *Filter_20[]={L"Toshiba",L"toshiba",0};
const WCHAR *Filter_21[]={L"OEM",L"twinhead",L"durabook",0};
const WCHAR *Filter_22[]={L"NEC",L"Nec_brand",L"nec",0};

const WCHAR **filter_list[]=
{
    Filter_1, Filter_2, Filter_3, Filter_4, Filter_5, Filter_6, Filter_7,
    Filter_8, Filter_9, Filter_10,Filter_11,Filter_12,Filter_13,Filter_14,
    Filter_15,Filter_16,Filter_17,Filter_18,Filter_19,Filter_20,Filter_21,
    Filter_22,
};
//}

//{ Calc
void State::genmarker()
{
    int i,j;
    WCHAR *str;

    *marker=0;

    str=getManuf();
    if(!str)return;

    //log_con("Manuf '%S'\n",str);
    for(i=0;i<21;i++)
    {
        //log_con("%d: '%S'\n",i+1,filter_list[i][0]);
        for(j=1;filter_list[i][j];j++)
        {
            if(StrStrI(str,filter_list[i][j]))
            {
                wsprintfA(marker,"%S_nb",filter_list[i][0]);
                log_con("Matched marker: '%s'(%S)\n",marker,filter_list[i][j]);
            }
        }
    }
    if(!*marker)wsprintfA(marker,"OEM_nb");
    log_con("Marker: '%s'\n",marker);
}

int calc_identifierscore(int dev_pos,int dev_ishw,int inf_pos)
{
    if(dev_ishw&&inf_pos==0)    // device hardware ID and a hardware ID in an INF
        return 0x0000+dev_pos;

    if(dev_ishw)                // device hardware ID and a compatible ID in an INF
        return 0x1000+dev_pos+0x100*inf_pos;

    if(inf_pos==0)              // device compatible ID and a hardware ID in an INF
        return 0x2000+dev_pos;

                                // device compatible ID and a compatible ID in an INF
        return 0x3000+dev_pos+0x100*inf_pos;
}

int Hwidmatch::calc_catalogfile()
{
    int r=0,i;

    for(i=CatalogFile;i<=CatalogFile_ntamd64;i++)
        if(*getdrp_drvfield(i))r+=1<<i;

    //if(!isvalidcat(hwidmatch,state))r=0;
    //r=0;
    return r;
}

int calc_signature(int catalogfile,State *state,int isnt)
{
    if(state->architecture)
    {
        if(catalogfile&(1<<CatalogFile|1<<CatalogFile_nt|1<<CatalogFile_ntamd64|1<<CatalogFile_ntia64))
            return 0;
    }
    else
    {
        if(catalogfile&(1<<CatalogFile|1<<CatalogFile_nt|1<<CatalogFile_ntx86))
            return 0;
    }
    if(isnt)return 0x8000;
    return 0xC000;
}

unsigned calc_score(int catalogfile,int feature,int rank,State *state,int isnt)
{
    if(state->platform.dwMajorVersion>=6)
        return (calc_signature(catalogfile,state,isnt)<<16)+(feature<<16)+rank;
    else
        return calc_signature(catalogfile,state,isnt)+rank;
}

unsigned calc_score_h(Driver *driver,State *state)
{
    return calc_score(driver->catalogfile,driver->feature,driver->identifierscore,
        state,StrStrI(state->textas.getw(driver->InfSectionExt),L".nt")?1:0);
}

int calc_secttype(const char *s)
{
    char buf[BUFLEN];
    char *p=buf,*s1;
    int i;

    while(1)
    {
        s1=strchr(s,'.');
        if(s1)
        {
            s=s1;
            s++;
            if(!memcmp(s,"nt",2))break;
        }else break;

    }

    strcpy(buf,s);

    if((p=strchr(p,'.')))
        if((p=strchr(p+1,'.')))
            if((p=strchr(p+1,'.')))*p=0;
    for(i=0;i<NUM_DECS;i++)if(!StrCmpIA(buf,nts[i]))return i;
//log_file("'%s'\n",buf);
    return -1;
}

int calc_decorscore(int id,State *state)
{
    int major=state->platform.dwMajorVersion,
        minor=state->platform.dwMinorVersion,
        arch=state->architecture+1;

    if(id<0)
    {
        return 1;
    }
    if(nts_version[id]&&major*10+minor<nts_version[id])return 0;
    if(nts_arch[id]&&arch!=nts_arch[id])return 0;
    return nts_score[id];
}

int calc_markerscore(State *state,char *path)
{
    char buf[BUFLEN];
    int majver=state->platform.dwMajorVersion,
        minver=state->platform.dwMinorVersion,
        arch=state->architecture,
        curmaj=-1,curmin=-1,curarch=-1;
    int i;
    //int winall=0;
    int score=0;

    strcpy(buf,path);
    strtolower(buf,strlen(buf));

    for(i=0;i<NUM_MARKERS;i++)
    {
        if(strstr(buf,markers[i].name))
        {
            score=1;
            if(markers[i].major==0){/*winall=1;*/continue;}
            if(markers[i].major>curmaj)curmaj=markers[i].major;
            if(markers[i].minor>curmin)curmin=markers[i].minor;
            if(markers[i].arch>curarch)curarch=markers[i].arch;
        }
    }

    if(curmaj>=0&&curmin>=0&&majver>=curmaj&&minver>=curmin)score+=2+8;
    if(curmaj<0&&score)score+=2;
    if(curarch>=0&&curarch==arch)score+=4+16;
    if(curarch<0&&score)score+=4;
    return score;
}

intptr_t Hwidmatch::isvalid_usb30hub(State *state,const WCHAR *str)
{
    //log_con("Intel USB3.0 HUB '%S'\n",state->text+hwidmatch->devicematch->device->HardwareID);
    return (intptr_t)StrStrI(state->textas.getw(devicematch->device->HardwareID),str);
}

int Hwidmatch::isblacklisted(State *state,const WCHAR *hwid,const char *section)
{
    char buf[BUFLEN];

    if(StrStrI(state->textas.getw(devicematch->device->HardwareID),hwid))
    {
        getdrp_drvsection(buf);
        if(StrStrIA(buf,section))return 1;
    }
    return 0;
}

int Hwidmatch::isvalid_ver(State *state)
{
    version_t *v;
    int major=state->platform.dwMajorVersion;
    int minor=state->platform.dwMinorVersion;

    v=getdrp_drvversion();
    switch(v->v1)
    {
        case 5:if(major!=5)return 0;break;
        case 6:if(major==5)return 0;break;
        case 106:if(major!=6||minor!=0)return 0;break;
        default:break;
    }
    return 1;
}

int Hwidmatch::calc_notebook()
{
    if(StrStrIA(getdrp_infpath(),"_nb\\")||
       StrStrIA(getdrp_infpath(),"Touchpad_Mouse\\"))
    {
        if(!isLaptop)return 0;
        if(!*marker)return 0;
        if(!StrStrIA(getdrp_infpath(),marker))return 0;
    }
    return 1;
}

int Hwidmatch::calc_altsectscore(State *state,int curscore)
{
    char buf[BUFLEN];
    int pos;
    int desc_index,manufacturer_index;

    desc_index=drp->HWID_list[HWID_index].desc_index;
    manufacturer_index=drp->desc_list[desc_index].manufacturer_index;

    for(pos=0;pos<drp->manufacturer_list[manufacturer_index].sections_n;pos++)
    {
        getdrp_drvsectionAtPos(drp,buf,pos,manufacturer_index);
        if(calc_decorscore(calc_secttype(buf),state)>curscore)return 0;
    }

    if(!calc_notebook())return 0;

    if(StrStrIA(getdrp_infpath(),"intel_2nd\\"))
        if(!isvalid_usb30hub(state,L"IUSB3\\ROOT_HUB30&VID_8086&PID_1E31"))return 0;

    if(StrStrIA(getdrp_infpath(),"intel_4th\\"))
        if(!isvalid_usb30hub(state,L"IUSB3\\ROOT_HUB30&VID_8086&PID_8C31")&&
           !isvalid_usb30hub(state,L"IUSB3\\ROOT_HUB30&VID_8086&PID_9C31")&&
           !isvalid_usb30hub(state,L"IUSB3\\ROOT_HUB30&VID_8086&PID_0F35")&&
//           !isvalid_usb30hub(hwidmatch,state,L"pnp0a08")&&
           !isvalid_usb30hub(state,L"IUSB3\\ROOT_HUB30&VID_8086&PID_8CB1"))return 0;

    if(StrStrIA(getdrp_infpath(),"matchver\\")||
       StrStrIA(getdrp_infpath(),"L\\Realtek\\")||
       StrStrIA(getdrp_infpath(),"L\\R\\"))
        if(!isvalid_ver(state))return 0;

    if(isblacklisted(state,L"VEN_168C&DEV_002B&SUBSYS_30A117AA","Realtek"))return 0;

    if(StrStrIA(getdrp_infpath(),"matchmarker\\"))
        if((calc_markerscore(state,getdrp_infpath())&7)!=7)return 0;

    //log_file("Sc:%d\n\n",curscore);
    if(flags&FLAG_FILTERSP)return 2;

    if(StrStrIA(getdrp_infpath(),"tweak"))return 1;
    return isvalidcat(this,state)?2:1;
}

int isMissing(Device *device,Driver *driver,State *state)
{
    if(device->problem==CM_PROB_DISABLED)return 0;
    if(driver)
    {
        if(!StrCmpIW((WCHAR*)(state->textas.get(driver->MatchingDeviceId)),L"PCI\\CC_0300"))return 1;
        if(device->problem&&device->HardwareID)return 1;
    }else
    {
        if(device->problem&&device->HardwareID)return 1;
    }
    return 0;
}

int Hwidmatch::calc_status(State *state)
{
    int r=0,res;
    int scorev;
    Driver *cur_driver=devicematch->driver;

    if(isMissing(devicematch->device,cur_driver,state))return STATUS_MISSING;

    if(devicematch->driver)
    {
        if(getdrp_drvversion())
        {
            res=cmpdate(&devicematch->driver->version,getdrp_drvversion());
            if(res<0)r+=STATUS_NEW;else
            if(res>0)r+=STATUS_OLD;else
                r+=STATUS_CURRENT;
        }

        scorev=calc_score_h(cur_driver,state);
        res=cmpunsigned(scorev,score);
        if(res>0)r+=STATUS_BETTER;else
        if(res<0)r+=STATUS_WORSE;else
            r+=STATUS_SAME;
    }
    else
        r+=STATUS_BETTER;

    if(!altsectscore)r+=STATUS_INVALID;
    return r;
}
//}

//{ Misc
void findHWID_in_list(char *s,int list,int str,int *dev_pos)
{
    *dev_pos=0;
    WCHAR *p=(WCHAR *)(s+list);
    while(*p)
    {
        if(!StrCmpIW(p,(WCHAR *)(s+str)))return;
        p+=lstrlen(p)+1;
        (*dev_pos)++;
    }
    *dev_pos=-1;
}

void getdd(Device *cur_device,Driver *cur_driver,State *state,int *ishw,int *dev_pos)
{
    //Driver *cur_driver=&state->Drivers_list[cur_device->driver_index];

    *ishw=1;
    findHWID_in_list(state->textas.get(0),cur_device->HardwareID,cur_driver->MatchingDeviceId,dev_pos);
    if(*dev_pos<0&&cur_device->CompatibleIDs)
    {
        *ishw=0;
        findHWID_in_list(state->textas.get(0),cur_device->CompatibleIDs,cur_driver->MatchingDeviceId,dev_pos);
    }
}

int cmpunsigned(unsigned a,unsigned b)
{
    if(a>b)return 1;
    if(a<b)return -1;
    return 0;
}

int cmpdate(version_t *t1,version_t *t2)
{
    int res;

    if(flags&FLAG_FILTERSP&&t2->y<1000)return 0;

    res=t1->y-t2->y;
    if(res)return res;

    res=t1->m-t2->m;
    if(res)return res;

    res=t1->d-t2->d;
    if(res)return res;

    return 0;
}

int cmpversion(version_t *t1,version_t *t2)
{
    int res;

    if(flags&FLAG_FILTERSP&&t2->v1<0)return 0;

    res=t1->v1-t2->v1;
    if(res)return res;

    res=t1->v2-t2->v2;
    if(res)return res;

    res=t1->v3-t2->v3;
    if(res)return res;

    res=t1->v4-t2->v4;
    if(res)return res;

    return 0;

}

void devicematch_init(devicematch_t *devicematch,Device *cur_device,Driver *driver,int items)
{
    devicematch->device=cur_device;
    devicematch->driver=driver;
    devicematch->start_matches=items;
    devicematch->num_matches=0;
}
//}

//{ hwidmatch
void Hwidmatch::init(Driverpack *drp1,int HWID_index1,int dev_pos,int ishw,State *state,devicematch_t *devicematch1)
{
    char buf[BUFLEN];

    drp=drp1;
    HWID_index=HWID_index1;
    devicematch=devicematch1;

    getdrp_drvsection(buf);

    identifierscore=calc_identifierscore(dev_pos,ishw,drp->HWID_list[HWID_index].inf_pos);
    decorscore=calc_decorscore(calc_secttype(buf),state);
    markerscore=calc_markerscore(state,getdrp_infpath());
    altsectscore=calc_altsectscore(state,decorscore);
    score=calc_score(calc_catalogfile(),getdrp_drvfeature(),
        identifierscore,state,strstr(getdrp_drvinstallPicked(),".nt")?1:0);
    status=calc_status(state);
}

void Hwidmatch::initbriefly(Driverpack *drp1,int HWID_index1)
{
    drp=drp1;
    HWID_index=HWID_index1;
}
/*
0 section
1 driverpack
2 inffile
3 manuf
4 version
5 hwid
6 desc
*/

void minlen(CHAR *s,int *len)
{
    int l=strlen(s);
    if(*len<l)*len=l;
}

void Hwidmatch::calclen(int *limits)
{
    char buf[BUFLEN];
    version_t *v;

    getdrp_drvsection(buf);
    minlen(buf,&limits[0]);
    wsprintfA(buf,"%ws%ws",getdrp_packpath(),getdrp_packname());
    minlen(buf,&limits[1]);
    wsprintfA(buf,"%s%s",getdrp_infpath(),getdrp_infname());
    minlen(buf,&limits[2]);
    minlen(getdrp_drvmanufacturer(),&limits[3]);
    v=getdrp_drvversion();
    wsprintfA(buf,"%d.%d.%d.%d",v->v1,v->v2,v->v3,v->v4);
    minlen(buf,&limits[4]);
    minlen(getdrp_drvHWID(),&limits[5]);
    minlen(getdrp_drvdesc(),&limits[6]);
}

void Hwidmatch::print_tbl(int *limits)
{
    CHAR buf[BUFLEN];
    version_t *v;

    v=getdrp_drvversion();
    log_file("  %d |",               altsectscore);
    log_file(" %08X |",              score);
    log_file(" %2d.%02d.%4d |",      v->d,v->m,v->y);
    log_file(" %3d |",               decorscore);
    log_file(" %d |",                markerscore);
    log_file(" %3X |",               status);
                                getdrp_drvsection(buf);
    log_file(" %-*s |",limits[0],buf);

    wsprintfA(buf,"%ws\\%ws",       getdrp_packpath(),getdrp_packname());
    log_file(" %-*s |",limits[1],buf);
    log_file(" %8X |",              getdrp_infcrc());
    wsprintfA(buf,"%s%s",         getdrp_infpath(),getdrp_infname());
    log_file(" %-*s |",limits[2],buf);
    log_file(" %-*s |",limits[3],    getdrp_drvmanufacturer());
    wsprintfA(buf,"%d.%d.%d.%d",  v->v1,v->v2,v->v3,v->v4);
    log_file(" %*s |",limits[4],buf);
    log_file(" %-*s |",limits[5],    getdrp_drvHWID());
    log_file(" %-*s",limits[6],      getdrp_drvdesc());
    log_file("\n");
}

void Hwidmatch::print_hr()
{
    CHAR buf[BUFLEN];
    version_t *v;

    v=getdrp_drvversion();
/*    log_file("  Alt:   %d\n",               altsectscore);
    log_file("  Decor: %3d\n",               decorscore);
    log_file("  CRC:  %8X%\n",              getdrp_infcrc(this));
    log_file("  Marker %d\n",                markerscore);
    log_file("  Status %3X\n",               status);*/
    log_file("  Pack:     %S\\%S\n",       getdrp_packpath(),getdrp_packname());

    log_file("  Name:     %s\n",getdrp_drvdesc());
    log_file("  Provider: %s\n",    getdrp_drvmanufacturer());
    log_file("  Date:     %2d.%02d.%4d\n",      v->d,v->m,v->y);
    log_file("  Version:  %d.%d.%d.%d\n",  v->v1,v->v2,v->v3,v->v4);
    log_file("  HWID:     %s\n",getdrp_drvHWID());
    getdrp_drvsection(buf);
    log_file("  inf:      %s%s,%s\n",         getdrp_infpath(),getdrp_infname(),buf);
    log_file("  Score:    %08X\n",              score);
    log_file("\n");
}

int Hwidmatch::cmp(Hwidmatch *match2)
{
    int res;

    res=altsectscore-match2->altsectscore;
    if(res)return res;

    res=cmpunsigned(score,match2->score);
    if(res)return -res;

    res=cmpdate(getdrp_drvversion(),match2->getdrp_drvversion());
    if(res)return res;

    res=decorscore-match2->decorscore;
    if(res)return res;

    res=markerscore-match2->markerscore;
    if(res)return res;

    res=(status&~STATUS_DUP)-(match2->status&~STATUS_DUP);
    if(res)return res;

    return 0;
}
//}

//{ Matcher
void Matcher::init(State *state1,Collection *col1)
{
    state=state1;
    col=col1;

    //heap_init(&devicematch_handle,ID_MATCHER,(void **)&devicematch_list,0,sizeof(devicematch_t));
    //heap_init(&hwidmatch_handle,ID_MATCHER,(void **)&hwidmatch_list,0,sizeof(Hwidmatch));
}

void Matcher::release()
{
    //heap_free(&devicematch_handle);
    //heap_free(&hwidmatch_handle);
}

void Matcher::findHWIDs(devicematch_t *devicematch,char *hwid,int dev_pos,int ishw)
{
    Driverpack *drp;
    Hwidmatch *hwidmatch;
    unsigned i;
    int code;
    int sz;
    int isfound;

    sz=strlen(hwid);
    strtoupper(hwid,sz);
    code=hash_getcode(hwid,sz);

    for(i=0;i<col->driverpack_list.size();i++)
    {
        drp=&col->driverpack_list[i];
        int val=hash_find(&drp->indexes,(char *)&code,4,&isfound);
        while(isfound)
        {
            hwidmatch_list.push_back(Hwidmatch());
            hwidmatch=&hwidmatch_list.back();
            hwidmatch->init(drp,val,dev_pos,ishw,state,devicematch);
            devicematch->num_matches++;
            val=hash_findnext_b(&drp->indexes,&isfound);
        }
    }
}

void Matcher::populate()
{
    devicematch_t *devicematch;
    Driver *cur_driver;
    Device *cur_device;
    WCHAR *p;
    char *s=state->textas.get(0);
    char buf[BUFLEN];
    int dev_pos;
    unsigned int i;

    time_matcher=GetTickCount();
    cur_device=&state->Devices_list[0];
    devicematch_list.clear();
    hwidmatch_list.clear();

    isLaptop=state->isLaptop;
    //wcscpy(marker,state->marker);

    for(i=0;i<state->Devices_list.size();i++,cur_device++)
    {
        cur_driver=0;
        if(cur_device->driver_index>=0)cur_driver=&state->Drivers_list[cur_device->driver_index];
        devicematch_list.push_back(devicematch_t());
        devicematch=&devicematch_list.back();
        devicematch_init(devicematch,cur_device,cur_driver,hwidmatch_list.size());
        if(cur_device->HardwareID)
        {
            p=(WCHAR *)(s+cur_device->HardwareID);
            dev_pos=0;
            while(*p)
            {
                wsprintfA(buf,"%ws",p);
                findHWIDs(devicematch,buf,dev_pos,1);
                p+=lstrlen(p)+1;
                dev_pos++;
            }
        }

        if(cur_device->CompatibleIDs)
        {
            p=(WCHAR *)(s+cur_device->CompatibleIDs);
            dev_pos=0;
            while(*p)
            {
                wsprintfA(buf,"%ws",p);
                findHWIDs(devicematch,buf,dev_pos,0);
                p+=lstrlen(p)+1;
                dev_pos++;
            }
        }
        if(!devicematch->num_matches)
        {
            hwidmatch_list.push_back(Hwidmatch());

            if(isMissing(cur_device,cur_driver,state))devicematch->status=STATUS_NF_MISSING;else
            if(devicematch->driver)
            {
                if(!wcscmp(state->textas.getw(devicematch->driver->ProviderName),L"Microsoft")||
                   !wcscmp(state->textas.getw(devicematch->driver->ProviderName),L"Майкрософт"))
                    devicematch->status=STATUS_NF_STANDARD;
                else
                {
                    if(*state->textas.get(devicematch->driver->MatchingDeviceId))
                        devicematch->status=STATUS_NF_UNKNOWN;
                    else
                        devicematch->status=STATUS_NF_STANDARD;
                }
            }
            else
            {
                if(devicematch->device->problem)
                    devicematch->status=devicematch->device->HardwareID?STATUS_NF_MISSING:STATUS_NF_STANDARD;
                else
                    devicematch->status=STATUS_NF_STANDARD;
            }
        }
    }
}

void Matcher::sort()
{
    devicematch_t *devicematch;
    Hwidmatch *match1,*match2,*bestmatch;
    Hwidmatch matchtmp;

    char sect1[BUFLEN],sect2[BUFLEN];
    unsigned k;
    unsigned i,j;
    devicematch=&devicematch_list[0];
    for(k=0;k<devicematch_list.size();k++,devicematch++)
    {
        // Sort
        match1=&hwidmatch_list[devicematch->start_matches];
        for(i=0;i+1<devicematch->num_matches;i++,match1++)
        {
            match2=&hwidmatch_list[devicematch->start_matches+i+1];
            bestmatch=match1;
            for(j=i+1;j<devicematch->num_matches;j++,match2++)
                if(bestmatch->cmp(match2)<0)bestmatch=match2;

            if(bestmatch!=match1)
            {
                memcpy(&matchtmp,match1,sizeof(Hwidmatch));
                memcpy(match1,bestmatch,sizeof(Hwidmatch));
                memcpy(bestmatch,&matchtmp,sizeof(Hwidmatch));
            }

        }

        // Mark dups
        match1=&hwidmatch_list[devicematch->start_matches];
        for(i=0;i+1<devicematch->num_matches;i++,match1++)
        {
            match1->getdrp_drvsection(sect1);

            match2=&hwidmatch_list[devicematch->start_matches+i+1];
            for(j=i+1;j<devicematch->num_matches;j++,match2++)
            {
                match2->getdrp_drvsection(sect2);

                if(match1->getdrp_infcrc()==match2->getdrp_infcrc()&&
                   !strcmp(match1->getdrp_drvHWID(),match2->getdrp_drvHWID())&&
                   !strcmp(sect1,sect2))match2->status|=STATUS_DUP;

            }
        }
    }
    time_matcher=GetTickCount()-time_matcher;
}

void Matcher::print()
{
    devicematch_t *devicematch;
    Device *cur_device;
    Hwidmatch *hwidmatch;
    int limits[7];
    unsigned i;
    unsigned j;

    if((log_verbose&LOG_VERBOSE_MATCHER)==0)return;
    log_file("\n{matcher_print[devices=%d,hwids=%d]\n",devicematch_list.size(),hwidmatch_list.size());
    devicematch=&devicematch_list[0];
    for(i=0;i<devicematch_list.size();i++,devicematch++)
    {
        cur_device=devicematch->device;
        cur_device->print(state);
        log_file("DriverInfo\n");
        if(devicematch->driver)
            devicematch->driver->print(state);
        else
            log_file("  NoDriver\n");

        memset(limits,0,sizeof(limits));
        hwidmatch=&hwidmatch_list[devicematch->start_matches];
        for(j=0;j<devicematch_list[i].num_matches;j++,hwidmatch++)
            hwidmatch->calclen(limits);

        hwidmatch=&hwidmatch_list[devicematch->start_matches];
        for(j=0;j<devicematch_list[i].num_matches;j++,hwidmatch++)
            hwidmatch->print_tbl(limits);
        log_file("\n");
    }
    log_file("}matcher_print\n\n");
}
//}

//{ Getters
//driverpack
WCHAR *Hwidmatch::getdrp_packpath()
{
    return drp->getPath();
}
WCHAR *Hwidmatch::getdrp_packname()
{
    return drp->getFilename();
}
int Hwidmatch::getdrp_packontorrent()
{
    return drp->type==DRIVERPACK_TYPE_UPDATE;
}

//inffile
char *Hwidmatch::getdrp_infpath()
{
    int desc_index=drp->HWID_list[HWID_index].desc_index;
    int manufacturer_index=drp->desc_list[desc_index].manufacturer_index;
    int inffile_index=drp->manufacturer_list[manufacturer_index].inffile_index;
    return drp->texta.get(drp->inffile[inffile_index].infpath);
}
char *Hwidmatch::getdrp_infname()
{
    int desc_index=drp->HWID_list[HWID_index].desc_index;
    int manufacturer_index=drp->desc_list[desc_index].manufacturer_index;
    int inffile_index=drp->manufacturer_list[manufacturer_index].inffile_index;
    return drp->texta.get(drp->inffile[inffile_index].inffilename);
}
const char *Hwidmatch::getdrp_drvfield(int n)
{
    int desc_index=drp->HWID_list[HWID_index].desc_index;
    int manufacturer_index=drp->desc_list[desc_index].manufacturer_index;
    int inffile_index=drp->manufacturer_list[manufacturer_index].inffile_index;
    if(!drp->inffile[inffile_index].fields[n])return "";
    return drp->texta.get(drp->inffile[inffile_index].fields[n]);
}
const char *Hwidmatch::getdrp_drvcat(int n)
{
    int desc_index=drp->HWID_list[HWID_index].desc_index;
    int manufacturer_index=drp->desc_list[desc_index].manufacturer_index;
    int inffile_index=drp->manufacturer_list[manufacturer_index].inffile_index;
    if(!drp->inffile[inffile_index].cats[n])return "";
    return drp->texta.get(drp->inffile[inffile_index].cats[n]);
}
version_t *Hwidmatch::getdrp_drvversion()
{
    int desc_index=drp->HWID_list[HWID_index].desc_index;
    int manufacturer_index=drp->desc_list[desc_index].manufacturer_index;
    int inffile_index=drp->manufacturer_list[manufacturer_index].inffile_index;
    return &drp->inffile[inffile_index].version;
}
int Hwidmatch::getdrp_infsize()
{
    int desc_index=drp->HWID_list[HWID_index].desc_index;
    int manufacturer_index=drp->desc_list[desc_index].manufacturer_index;
    int inffile_index=drp->manufacturer_list[manufacturer_index].inffile_index;
    return drp->inffile[inffile_index].infsize;
}
int Hwidmatch::getdrp_infcrc()
{
    int desc_index=drp->HWID_list[HWID_index].desc_index;
    int manufacturer_index=drp->desc_list[desc_index].manufacturer_index;
    int inffile_index=drp->manufacturer_list[manufacturer_index].inffile_index;
    return drp->inffile[inffile_index].infcrc;
}

//manufacturer
char *Hwidmatch::getdrp_drvmanufacturer()
{
    int desc_index=drp->HWID_list[HWID_index].desc_index;
    int manufacturer_index=drp->desc_list[desc_index].manufacturer_index;
    return drp->texta.get(drp->manufacturer_list[manufacturer_index].manufacturer);
}
void getdrp_drvsectionAtPos(Driverpack *drp,char *buf,int pos,int manuf_index)
{
    intptr_t rr=(intptr_t)drp->texta.get(drp->manufacturer_list[manuf_index].sections);
    if(pos)
    {
        char *ext=drp->texta.get(((int *)rr)[pos]);
        wsprintfA(buf,"%s.%s",drp->texta.get(((int *)rr)[0]),ext);
    }
    else
        wsprintfA(buf,"%s",drp->texta.get(((int *)rr)[pos]));
}
void Hwidmatch::getdrp_drvsection(char *buf)
{
    int desc_index=drp->HWID_list[HWID_index].desc_index;
    int manufacturer_index=drp->desc_list[desc_index].manufacturer_index;
    getdrp_drvsectionAtPos(drp,buf,drp->desc_list[desc_index].sect_pos,manufacturer_index);
}

//desc
char *Hwidmatch::getdrp_drvdesc()
{
    int desc_index=drp->HWID_list[HWID_index].desc_index;
    return drp->texta.get(drp->desc_list[desc_index].desc);
}
char *Hwidmatch::getdrp_drvinstall()
{
    int desc_index=drp->HWID_list[HWID_index].desc_index;
    return drp->texta.get(drp->desc_list[desc_index].install);
}
char *Hwidmatch::getdrp_drvinstallPicked()
{
    int desc_index=drp->HWID_list[HWID_index].desc_index;
    return drp->texta.get(drp->desc_list[desc_index].install_picked);
}
int Hwidmatch::getdrp_drvfeature()
{
    char *p;
    p=StrStrIA(getdrp_infpath(),"feature_");
    if(p)return atoi(p+8);

    int desc_index=drp->HWID_list[HWID_index].desc_index;
    return drp->desc_list[desc_index].feature&0xFF;
}

//HWID
short Hwidmatch::getdrp_drvinfpos()
{
    return drp->HWID_list[HWID_index].inf_pos;
}
char *Hwidmatch::getdrp_drvHWID()
{
    return drp->texta.get(drp->HWID_list[HWID_index].HWID);
}
//}
