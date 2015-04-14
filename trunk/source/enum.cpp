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
int isLaptop;

static const char *deviceststus_str[]=
{
    "Device is not present",
    "Device is disabled",
    "The device has the following problem: %d",
    "The driver reported a problem with the device",
    "Driver is running",
    "Device is currently stopped"
};

#ifdef STORE_PROPS
const dev devtbl[NUM_PROPS]=
{
/**/{SPDRP_DEVICEDESC                   ,"Devicedesc"},
/**/{SPDRP_HARDWAREID                   ,"HardwareID"},
/**/{SPDRP_COMPATIBLEIDS                ,"CompatibleIDs"},
//   SPDRP_UNUSED0           3
    {SPDRP_SERVICE                      ,"Service"},
//   SPDRP_UNUSED1           5
//   SPDRP_UNUSED2           6
    {SPDRP_CLASS                        ,"Class"},
    {SPDRP_CLASSGUID                    ,"ClassGUID"},
/**/{SPDRP_DRIVER                       ,"Driver"},
    {SPDRP_CONFIGFLAGS                  ,"ConfigFlags"},
/**/{SPDRP_MFG                          ,"Mfg"},
/**/{SPDRP_FRIENDLYNAME                 ,"FriendlyName"},
    {SPDRP_LOCATION_INFORMATION         ,"LocationInformation"},
    {SPDRP_PHYSICAL_DEVICE_OBJECT_NAME  ,"PhysicalDeviceObjectName"},
    {SPDRP_CAPABILITIES                 ,"Capabilities"},
    {SPDRP_UI_NUMBER                    ,"UINumber"},
//   SPDRP_UPPERFILTERS      17
//   SPDRP_LOWERFILTERS      18
    {SPDRP_BUSTYPEGUID                  ,"BusTypeGUID"},
    {SPDRP_LEGACYBUSTYPE                ,"LegacyBusType"},
    {SPDRP_BUSNUMBER                    ,"BusNumber"},
    {SPDRP_ENUMERATOR_NAME              ,"EnumeratorName"},
//   SPDRP_SECURITY          23
//   SPDRP_SECURITY_SDS      24
//   SPDRP_DEVTYPE           25
//   SPDRP_EXCLUSIVE         26
//   SPDRP_CHARACTERISTICS   27
    {SPDRP_ADDRESS                      ,"Address"},
//                           29
    {SPDRP_UI_NUMBER_DESC_FORMAT        ,"UINumberDescFormat"},
};
#endif // STORE_PROPS
//}

//{ Device
void Device::print_guid(GUID *g)
{
    WCHAR buffer[BUFLEN];
    /*log_file("%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",g->Data1,g->Data2,g->Data3,
        (int)(g->Data4[0]),(int)(g->Data4[1]),
        (int)(g->Data4[2]),(int)(g->Data4[3]),(int)(g->Data4[4]),
        (int)(g->Data4[5]),(int)(g->Data4[6]),(int)(g->Data4[7]));*/

    *buffer=0;
    if(!SetupDiGetClassDescription(g,buffer,BUFLEN,0))
    {
        //int lr=GetLastError();
        //print_error(lr,L"print_guid()");
    }
    log_file("%S\n",buffer);
}

void Device::read_device_property(HDEVINFO hDevInfo,State *state,int id,ofst *val)
{
    DWORD buffersize=0;
    int lr;
    DWORD DataT=0;
    PBYTE p;
    auto DeviceInfoDataloc=(SP_DEVINFO_DATA *)&DeviceInfoData;

    *val=0;
    if(!SetupDiGetDeviceRegistryProperty(hDevInfo,DeviceInfoDataloc,id,&DataT,0,0,&buffersize))
    {
        lr=GetLastError();
        if(lr==ERROR_INVALID_DATA)return;
        if(lr!=ERROR_INSUFFICIENT_BUFFER)
        {
            log_file("Property %d\n",id);
            print_error(lr,L"read_device_property()");
            return;
        }
    }

    if(DataT==REG_DWORD)
    {
        p=(PBYTE)val;
    }
    else
    {
        *val=state->textas.alloc(buffersize);
        p=(PBYTE)(state->textas.get(*val));
        *p=0;
    }
    if(!SetupDiGetDeviceRegistryProperty(hDevInfo,DeviceInfoDataloc,id,&DataT,(PBYTE)p,buffersize,&buffersize))
    {
        lr=GetLastError();
        log_file("Property %d\n",id);
        print_error(lr,L"read_device_property()");
        return;
    }
}

int Device::print_status()
{
    int isPhantom=0;

    if(ret!=CR_SUCCESS)
    {
        if((ret==CR_NO_SUCH_DEVINST)||(ret==CR_NO_SUCH_VALUE))isPhantom=1;
    }

    if(isPhantom)
        return 0;
    else
    {
        if((status&DN_HAS_PROBLEM)&&problem==CM_PROB_DISABLED)
            return 1;
        else
        {
            if(status&DN_HAS_PROBLEM)
                return 2;
            else if(status&DN_PRIVATE_PROBLEM)
                return 3;
            else if(status&DN_STARTED)
                return 4;
            else
                return 5;
        }
    }
}

void Device::print(State *state)
{
    char *s=state->textas.get(0);
    log_file("DeviceInfo\n");
    log_file("##Name:#########%S\n",s+Devicedesc);
    log_file("##Status:#######");
    log_file(deviceststus_str[print_status()],problem);
    log_file("\n##Manufacturer:#%S\n",s+Mfg);
    log_file("##HWID_reg######%S\n",s+Driver);
    log_file("##Class:########");    print_guid(&DeviceInfoData.ClassGuid);
    log_file("##Location:#####\n");
    log_file("##ConfigFlags:##%d\n", ConfigFlags);
    log_file("##Capabilities:#%d\n", Capabilities);
}

void Device::printHWIDS(State *state)
{
    WCHAR *p;
    char *s=state->textas.get(0);

    if(HardwareID)
    {
        p=(WCHAR *)(s+HardwareID);
        log_file("HardwareID\n");
        while(*p)
        {
            log_file("  %S\n",p);
            p+=lstrlen(p)+1;
        }
    }
    else
    {
        log_file("NoID\n");
    }

    if(CompatibleIDs)
    {
        p=(WCHAR *)(s+CompatibleIDs);
        log_file("CompatibleID\n");
        while(*p)
        {
            log_file("  %S\n",p);
            p+=lstrlen(p)+1;
        }
    }
}

Device::Device(HDEVINFO hDevInfo,State *state,int i)
{
    DWORD buffersize;
    SP_DEVINFO_DATA *DeviceInfoDataloc;

    DeviceInfoDataloc=(SP_DEVINFO_DATA *)&DeviceInfoData;
    memset(&DeviceInfoData,0,sizeof(SP_DEVINFO_DATA));
    DeviceInfoData.cbSize=sizeof(SP_DEVINFO_DATA);

    driver_index=-1;
    if(!SetupDiEnumDeviceInfo(hDevInfo,i,DeviceInfoDataloc))
    {
        ret=GetLastError();
        return;
    }

    SetupDiGetDeviceInstanceId(hDevInfo,DeviceInfoDataloc,0,0,&buffersize);
    InstanceId=state->textas.alloc(buffersize);
    SetupDiGetDeviceInstanceId(hDevInfo,DeviceInfoDataloc,state->textas.getw(InstanceId),buffersize,0);

    read_device_property(hDevInfo,state,SPDRP_DEVICEDESC,    &Devicedesc);
    read_device_property(hDevInfo,state,SPDRP_HARDWAREID,    &HardwareID);
    read_device_property(hDevInfo,state,SPDRP_COMPATIBLEIDS, &CompatibleIDs);
    read_device_property(hDevInfo,state,SPDRP_DRIVER,        &Driver);
    read_device_property(hDevInfo,state,SPDRP_MFG,           &Mfg);
    read_device_property(hDevInfo,state,SPDRP_FRIENDLYNAME,  &FriendlyName);
    read_device_property(hDevInfo,state,SPDRP_CAPABILITIES,  &Capabilities);
    read_device_property(hDevInfo,state,SPDRP_CONFIGFLAGS,   &ConfigFlags);

    ret=CM_Get_DevNode_Status(&status,&problem,DeviceInfoDataloc->DevInst,0);
    if(ret!=CR_SUCCESS)
    {
        log_file("ERROR %d with CM_Get_DevNode_Status()\n",ret);
    }
}
//}

//{ Driver
void Driver::read_reg_val(HKEY hkey,State *state,const WCHAR *key,ofst *val)
{
    DWORD dwType,dwSize=0;
    int lr;

    *val=0;
    lr=RegQueryValueEx(hkey,key,NULL,0,0,&dwSize);
    if(lr==ERROR_FILE_NOT_FOUND)return;
    if(lr!=ERROR_SUCCESS)
    {
        log_file("Key %S\n",key);
        print_error(lr,L"RegQueryValueEx()");
        return;
    }

    *val=state->textas.alloc(dwSize);
    lr=RegQueryValueEx(hkey,key,NULL,&dwType,(unsigned char *)(state->textas.get(*val)),&dwSize);
    if(lr!=ERROR_SUCCESS)
    {
        log_file("Key %S\n",key);
        print_error(lr,L"read_reg_val()");
    }
}

void Driver::scaninf(State *state,Driverpack *unpacked_drp,int &inf_pos)
{
    WCHAR filename[BUFLEN];
    WCHAR fnm_hwid[BUFLEN];
    auto inf_list=&state->inf_list_new;

    unsigned HWID_index,start_index=0;

    if(flags&FLAG_FAILSAFE)
    {
        inf_pos=0;
        return;
    }

    wsprintf(filename,L"%s%s",state->textas.get(state->windir),state->textas.get(InfPath));
    wsprintf(fnm_hwid,L"%s%s",filename,state->textas.get(MatchingDeviceId));
    auto got=inf_list->find(std::wstring(fnm_hwid));
    if(got!=inf_list->end())
    {
        infdata_t *infdata=&got->second;
        //log_file("Match_hwid '%S' %d,%d,%d,%d\n",fnm_hwid,infdata->feature,infdata->catalogfile,infdata->cat,infdata->inf_pos);
        feature=infdata->feature;
        catalogfile=infdata->catalogfile;
        cat=infdata->cat;
        inf_pos=infdata->inf_pos;
        return;
    }

    got=inf_list->find(std::wstring(filename));
    if(got!=inf_list->end())
    {
        //infdata_t *infdata=&got->second;
        //log_file("Match_inf  '%S',%d\n",filename,infdata->feature,infdata->catalogfile,infdata->cat,infdata->inf_pos);
    }
    else
    {
        FILE *f;
        char *buft;
        int len;

        //log_file("Reading '%S' for (%S)\n",filename,state->textas.get(MatchingDeviceId));
        f=_wfopen(filename,L"rb");
        if(!f)
        {
            log_err("ERROR: file not found '%S'\n",filename);
            return;
        }
        fseek(f,0,SEEK_END);
        len=ftell(f);
        if(len<0)len=0;
        fseek(f,0,SEEK_SET);
        buft=(char *)malloc(len);
        fread(buft,len,1,f);
        fclose(f);

        if(len>0)
        {
            start_index=unpacked_drp->HWID_list.size();
            unpacked_drp->indexinf(state->textas.getw(state->windir),state->textas.getw(InfPath),buft,len);
        }
        free(buft);

        cat=state->opencatfile(this);
    }

    char sect[BUFLEN];
    char hwid[BUFLEN];
    inf_pos=-1;
    wsprintfA(sect,"%ws%ws",state->textas.get(InfSection),state->textas.get(InfSectionExt));
    wsprintfA(hwid,"%ws",state->textas.get(MatchingDeviceId));
    for(HWID_index=start_index;HWID_index<unpacked_drp->HWID_list.size();HWID_index++)
    if(!StrCmpIA(unpacked_drp->texta.get(unpacked_drp->HWID_list[HWID_index].HWID),hwid))
    {
        Hwidmatch hwidmatch;

        hwidmatch.initbriefly(unpacked_drp,HWID_index);
        if(StrStrIA(sect,hwidmatch.getdrp_drvinstall()))
        {
            feature=hwidmatch.getdrp_drvfeature();
            catalogfile=hwidmatch.calc_catalogfile();
            if(inf_pos<0||inf_pos>hwidmatch.getdrp_drvinfpos())inf_pos=hwidmatch.getdrp_drvinfpos();
        }
    }
    if(inf_pos==-1)
    {
        inf_pos=0;
        cat=0;
        feature=0xFF;
        log_err("ERROR: sect not found '%s'\n",sect);
    }

    //log_file("Added  %d,%d,%d,%d\n",feature,catalogfile,cat,inf_pos);

    inf_list->insert({std::wstring(fnm_hwid),infdata_t(catalogfile,feature,cat,inf_pos)});
    inf_list->insert({std::wstring(filename),infdata_t(0,0,0,0)});
}

void Driver::print(State *state)
{
    char *s=state->textas.get(0);
    WCHAR buf[BUFLEN];

    str_date(&version,buf);
    log_file("##Name:#####%S\n",s+DriverDesc);
    log_file("##Provider:#%S\n",s+ProviderName);
    log_file("##Date:#####%S\n",buf);
    log_file("##Version:##%d.%d.%d.%d\n",version.v1,version.v2,version.v3,version.v4);
    log_file("##HWID:#####%S\n",s+MatchingDeviceId);
    log_file("##inf:######%S%S,%S%S\n",(s+state->windir),s+InfPath,s+InfSection,s+InfSectionExt);
    log_file("##Score:####%08X %04x\n",calc_score_h(this,state),identifierscore);

    if(log_verbose&LOG_VERBOSE_BATCH)
        log_file("##Filter:###\"%S\"=a,%S\n",s+DriverDesc,s+MatchingDeviceId);
}

Driver::Driver(State *state,Device *cur_device,HKEY hkey,Driverpack *unpacked_drp)
{
    char bufa[BUFLEN];
    int dev_pos,ishw,inf_pos=-1;
    Parser_str pi;
    DriverDate=0;
    DriverVersion=0;

    read_reg_val(hkey,state,L"DriverDesc",         &DriverDesc);
    read_reg_val(hkey,state,L"ProviderName",       &ProviderName);
    read_reg_val(hkey,state,L"DriverDate",         &DriverDate);
    read_reg_val(hkey,state,L"DriverVersion",      &DriverVersion);
    read_reg_val(hkey,state,L"MatchingDeviceId",   &MatchingDeviceId);

    read_reg_val(hkey,state,L"InfPath",            &InfPath);
    read_reg_val(hkey,state,L"InfSection",         &InfSection);
    read_reg_val(hkey,state,L"InfSectionExt",      &InfSectionExt);

    getdd(cur_device,this,state,&ishw,&dev_pos);

    if(InfPath)
        scaninf(state,unpacked_drp,inf_pos);

    identifierscore=calc_identifierscore(dev_pos,ishw,inf_pos);
    //log_file("%d,%d,%d\n",dev_pos,ishw,inf_pos);

    if(DriverDate)
    {
        wsprintfA(bufa,"%ws",state->textas.get(DriverDate));
        pi.setRange(bufa,bufa+strlen(bufa));
        pi.readDate(&version);
    }

    if(DriverVersion)
    {
        wsprintfA(bufa,"%ws",state->textas.get(DriverVersion));
        pi.setRange(bufa,bufa+strlen(bufa));
        pi.readVersion(&version);
    }
}
//}

//{ State
void State::fakeOSversion()
{
    if(virtual_arch_type==32)architecture=0;
    if(virtual_arch_type==64)architecture=1;
    if(virtual_os_version)
    {
        platform.dwMajorVersion=virtual_os_version/10;
        platform.dwMinorVersion=virtual_os_version%10;
    }
}

WCHAR *State::getProduct()
{
    WCHAR *s=textas.getw(product);

    if(StrStrIW(s,L"Product"))return textas.getw(cs_model);
    return s;
}

WCHAR *State::getManuf()
{
    WCHAR *s=textas.getw(manuf);

    if(StrStrIW(s,L"Vendor"))return textas.getw(cs_manuf);
    return s;
}

WCHAR *State::getModel()
{
    WCHAR *s=textas.getw(model);

    if(!*s)return textas.getw(cs_model);
    return s;
}

void State::init()
{
    textas.text.reserve(1024*1024*1);
    //Drivers_list.reserve(500);
    //Devices_list.reserve(500);
    textas.alloc(2);
    textas.text[0]=0;
    textas.text[1]=0;

    revision=SVN_REV;

    //log_file("sizeof(Device)=%d\nsizeof(Driver)=%d\n\n",sizeof(Device),sizeof(Driver));
}

void State::release()
{
    Devices_list.clear();
    Drivers_list.clear();
}

State::~State()
{
//    log_con("Text_size: %d\n",textas.text.size());
}

void State::print()
{
    unsigned i;
    WCHAR *buf;
    SYSTEM_POWER_STATUS *batteryloc;

    if(log_verbose&LOG_VERBOSE_SYSINFO&&log_verbose&LOG_VERBOSE_BATCH)
    {
        log_file("%S (%d.%d.%d), ",get_winverstr(manager_g),platform.dwMajorVersion,platform.dwMinorVersion,platform.dwBuildNumber);
        log_file("%s\n",architecture?"64-bit":"32-bit");
        log_file("%s, ",isLaptop?"Laptop":"Desktop");
        log_file("Product='%S', ",textas.getw(product));
        log_file("Model='%S', ",textas.get(model));
        log_file("Manuf='%S'\n",textas.get(manuf));
    }else
    if(log_verbose&LOG_VERBOSE_SYSINFO)
    {
        log_file("Windows\n");
        log_file("  Version:     %S (%d.%d.%d)\n",get_winverstr(manager_g),platform.dwMajorVersion,platform.dwMinorVersion,platform.dwBuildNumber);
        log_file("  PlatformId:  %d\n",platform.dwPlatformId);
        log_file("  Update:      %S\n",platform.szCSDVersion);
        if(platform.dwOSVersionInfoSize == sizeof(OSVERSIONINFOEX))
        {
            log_file("  ServicePack: %d.%d\n",platform.wServicePackMajor,platform.wServicePackMinor);
            log_file("  SuiteMask:   %d\n",platform.wSuiteMask);
            log_file("  ProductType: %d\n",platform.wProductType);
        }
        log_file("\nEnvironment\n");
        log_file("  windir:      %S\n",textas.get(windir));
        log_file("  temp:        %S\n",textas.get(temp));

        log_file("\nMotherboard\n");
        log_file("  Product:     %S\n",textas.get(product));
        log_file("  Model:       %S\n",textas.get(model));
        log_file("  Manuf:       %S\n",textas.get(manuf));
        log_file("  cs_Model:    %S\n",textas.get(cs_model));
        log_file("  cs_Manuf:    %S\n",textas.get(cs_manuf));
        log_file("  Chassis:     %d\n",ChassisType);

        log_file("\nBattery\n");
        batteryloc=(SYSTEM_POWER_STATUS *)(textas.get(battery));
        log_file("  AC_Status:   ");
        switch(batteryloc->ACLineStatus)
        {
            case 0:log_file("Offline\n");break;
            case 1:log_file("Online\n");break;
            default:
            case 255:log_file("Unknown\n");break;
        }
        i=batteryloc->BatteryFlag;
        log_file("  Flags:       %d",i);
        if(i&1)log_file("[high]");
        if(i&2)log_file("[low]");
        if(i&4)log_file("[critical]");
        if(i&8)log_file("[charging]");
        if(i&128)log_file("[no battery]");
        if(i==255)log_file("[unknown]");
        log_file("\n");
        if(batteryloc->BatteryLifePercent!=255)
            log_file("  Charged:      %d\n",batteryloc->BatteryLifePercent);
        if(batteryloc->BatteryLifeTime!=0xFFFFFFFF)
            log_file("  LifeTime:     %d mins\n",batteryloc->BatteryLifeTime/60);
        if(batteryloc->BatteryFullLifeTime!=0xFFFFFFFF)
            log_file("  FullLifeTime: %d mins\n",batteryloc->BatteryFullLifeTime/60);

        buf=textas.getw(monitors);
        log_file("\nMonitors\n");
        for(i=0;i<buf[0];i++)
        {
            int x=buf[1+i*2];
            int y=buf[2+i*2];
            log_file("  %dcmx%dcm (%.1fin)\t%.3f %s\n",x,y,sqrt(x*x+y*y)/2.54,(double)y/x,iswide(x,y)?"wide":"");
        }

        log_file("\nMisc\n");
        log_file("  Type:        %s\n",isLaptop?"Laptop":"Desktop");
        log_file("  Locale:      %X\n",locale);
        log_file("  CPU_Arch:    %s\n",architecture?"64-bit":"32-bit");
        log_file("\n");
    }

    if(log_verbose&LOG_VERBOSE_DEVICES)
    for(auto &cur_device:Devices_list)
    {
        cur_device.print(this);

        log_file("DriverInfo\n",cur_device.getDriverIndex());
        if(cur_device.getDriverIndex()>=0)
            Drivers_list[cur_device.getDriverIndex()].print(this);
        else
            log_file("##NoDriver\n");

        cur_device.printHWIDS(this);
        log_file("\n\n");
    }
    //log_file("Errors: %d\n",error_count);
}

void State::save(const WCHAR *filename)
{
    FILE *f;
    int sz;
    int version=VER_STATE;
    char *mem,*p;

    if(flags&FLAG_NOSNAPSHOT)return;
    log_file("Saving state in '%S'...",filename);
    if(!canWrite(filename))
    {
        log_err("ERROR in state_save(): Write-protected,'%S'\n",filename);
        return;
    }
    f=_wfopen(filename,L"wb");
    if(!f)
    {
        log_err("ERROR in state_save(): failed _wfopen(%S)\n",errno_str());
        return;
    }

    sz=
        sizeof(state_m_t)+
        Drivers_list.size()*sizeof(Driver)+
        Devices_list.size()*sizeof(Device)+
        textas.text.size()+
        2*3*sizeof(int);  // 3 heaps

    p=mem=(char *)malloc(sz);

    fwrite(VER_MARKER,3,1,f);
    fwrite(&version,sizeof(int),1,f);

    memcpy(p,this,sizeof(state_m_t));p+=sizeof(state_m_t);
    p=vector_save(&Devices_list,p);
    p=vector_save(&Drivers_list,p);
    p=vector_save(&textas.text,p);

    if(1)
    {
        char *mem_pack=(char *)malloc(sz);
        sz=encode(mem_pack,sz,mem,sz);
        fwrite(mem_pack,sz,1,f);
        free(mem_pack);
    }
    else fwrite(mem,sz,1,f);

    free(mem);
    fclose(f);
    log_con("OK\n");
}

int  State::load(const WCHAR *filename)
{
    char buf[BUFLEN];
    FILE *f;
    int sz;
    int version;
    char *mem,*p,*mem_unpack=0;

    log_con("Loading state from '%S'...",filename);
    f=_wfopen(filename,L"rb");
    if(!f)
    {
        log_err("ERROR in State::load(): failed _wfopen(%S)\n",errno_str());
        return 0;
    }

    fseek(f,0,SEEK_END);
    sz=ftell(f);
    fseek(f,0,SEEK_SET);

    fread(buf,3,1,f);
    fread(&version,sizeof(int),1,f);
    sz-=3+sizeof(int);

    if(memcmp(buf,VER_MARKER,3))
    {
        log_err("ERROR in State::load(): invalid snapshot\n");
        return 0;
    }
    if(version!=VER_STATE)
    {
        log_err("ERROR in State::load(): invalid version(%d)\n",version);
        return 0;
    }

    p=mem=(char *)malloc(sz);
    fread(mem,sz,1,f);

    if(1)
    {
        UInt64 sz_unpack;

        Lzma86_GetUnpackSize((Byte *)p,sz,&sz_unpack);
        mem_unpack=(char *)malloc(sz_unpack);
        decode(mem_unpack,sz_unpack,mem,sz);
        p=mem_unpack;
    }

    memcpy(this,p,sizeof(state_m_t));p+=sizeof(state_m_t);
    p=vector_load(&Devices_list,p);
    p=vector_load(&Drivers_list,p);
    p=vector_load(&textas.text,p);

    fakeOSversion();

    free(mem);
    if(mem_unpack)free(mem_unpack);
    fclose(f);
    log_con("OK\n");
    return 1;
}

void State::getsysinfo_fast()
{
    WCHAR buf[BUFLEN];

    time_test=GetTickCount();
    textas.reset(2);

    // Battery
    battery=textas.alloc(sizeof(SYSTEM_POWER_STATUS));
    SYSTEM_POWER_STATUS *batteryloc=(SYSTEM_POWER_STATUS *)(textas.get(battery));
    GetSystemPowerStatus(batteryloc);

    // Monitors
    DISPLAY_DEVICE DispDev;
    memset(&DispDev,0,sizeof(DispDev));
    DispDev.cb=sizeof(DispDev);
    buf[0]=0;
    int i=0;
    while(EnumDisplayDevices(0,i,&DispDev,0))
    {
        int x,y;
        GetMonitorSizeFromEDID(DispDev.DeviceName,&x,&y);
        if(x&&y)
        {
            buf[buf[0]*2+1]=x;
            buf[buf[0]*2+2]=y;
            buf[0]++;
        }
        i++;
    }
    monitors=textas.memcpy((char *)buf,(1+buf[0]*2)*2);

    // Windows version
    platform.dwOSVersionInfoSize=sizeof(OSVERSIONINFOEX);
    if(!(GetVersionEx((OSVERSIONINFO*)&platform)))
    {
        platform.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
        if(!GetVersionEx((OSVERSIONINFO*)&platform))
        print_error(GetLastError(),L"GetVersionEx()");
    }
    locale=GetUserDefaultLCID();

    // Environment
    GetEnvironmentVariable(L"windir",buf,BUFLEN);
    wcscat(buf,L"\\inf\\");
    windir=textas.memcpy((char *)buf,wcslen(buf)*2+2);

    GetEnvironmentVariable(L"TEMP",buf,BUFLEN);
    temp=textas.memcpy((char *)buf,wcslen(buf)*2+2);

    // 64-bit detection
    architecture=0;
    *buf=0;
    GetEnvironmentVariable(L"PROCESSOR_ARCHITECTURE",buf,BUFLEN);
    if(!lstrcmpi(buf,L"AMD64"))architecture=1;
    *buf=0;
    GetEnvironmentVariable(L"PROCESSOR_ARCHITEW6432",buf,BUFLEN);
    if(*buf)architecture=1;

    fakeOSversion();
}

void State::getsysinfo_slow()
{
    WCHAR smanuf[BUFLEN];
    WCHAR smodel[BUFLEN];
    WCHAR sproduct[BUFLEN];
    WCHAR scs_manuf[BUFLEN];
    WCHAR scs_model[BUFLEN];

    time_sysinfo=GetTickCount();

    getbaseboard(smanuf,smodel,sproduct,scs_manuf,scs_model,&ChassisType);

    manuf=textas.memcpy((char*)smanuf,wcslen(smanuf)*2+2);
    product=textas.memcpy((char*)sproduct,wcslen(sproduct)*2+2);
    model=textas.memcpy((char*)smodel,wcslen(smodel)*2+2);
    cs_manuf=textas.memcpy((char*)scs_manuf,wcslen(scs_manuf)*2+2);
    cs_model=textas.memcpy((char*)scs_model,wcslen(scs_model)*2+2);

    time_sysinfo=GetTickCount()-time_sysinfo;
}

void State::scanDevices()
{
    HDEVINFO hDevInfo;
    HKEY   hkey;
    WCHAR buf[BUFLEN];
    Collection collection;
    Driverpack unpacked_drp;
    inflist_tp inflist;
    unsigned i;
    int lr;

    time_devicescan=GetTickCount();
    collection.init(textas.getw(windir),L"",L"",0);
    unpacked_drp.init(L"",L"windir.7z",&collection);
    //Driverpack unpacked_drp(L"",L"windir.7z",&collection);
    Devices_list.clear();
    inf_list_new.clear();

    hDevInfo=SetupDiGetClassDevs(0,0,0,DIGCF_PRESENT|DIGCF_ALLCLASSES);
    if(hDevInfo==INVALID_HANDLE_VALUE)
    {
        print_error(GetLastError(),L"SetupDiGetClassDevs()");
        return;
    }

    i=0;
    while(1)
    {
        // Device
        Devices_list.push_back(Device(hDevInfo,this,i++));
        Device *cur_device=&Devices_list.back();

        lr=cur_device->ret;
        if(lr)
        {
            Devices_list.pop_back();
            if(lr==ERROR_NO_MORE_ITEMS)
                break;
            else
                continue;
        }

        // Driver
        if(!cur_device->getDriver())continue;
        wsprintf(buf,L"SYSTEM\\CurrentControlSet\\Control\\Class\\%s",textas.getw(cur_device->getDriver()));
        lr=RegOpenKeyEx(HKEY_LOCAL_MACHINE,buf,0,KEY_QUERY_VALUE,&hkey);
        switch(lr)
        {
            case ERROR_SUCCESS:
                cur_device->setDriverIndex(Drivers_list.size());
                Drivers_list.emplace_back(Driver(this,cur_device,hkey,&unpacked_drp));
                break;

            default:
                print_error(lr,L"RegOpenKeyEx()");

            case ERROR_FILE_NOT_FOUND:
                break;
        }
    }
/*
    SP_DRVINFO_DATA drvinfo;
    drvinfo.cbSize=sizeof(SP_DRVINFO_DATA);
    SP_DRVINSTALL_PARAMS drvparams;
    drvparams.cbSize=sizeof(SP_DRVINSTALL_PARAMS);
    SetupDiBuildDriverInfoList(hDevInfo,DeviceInfoData,SPDIT_CLASSDRIVER);
    for (j=0;SetupDiEnumDriverInfo(hDevInfo,DeviceInfoData,SPDIT_CLASSDRIVER,j,&drvinfo);j++)
    {
        SYSTEMTIME t;
        SetupDiGetDriverInstallParams(hDevInfo,DeviceInfoData,&drvinfo,&drvparams);
        SP_DRVINFO_DETAIL_DATA drvdet;
        drvdet.cbSize=sizeof(SP_DRVINFO_DETAIL_DATA);
        SetupDiGetDriverInfoDetail(hDevInfo,DeviceInfoData,&drvinfo,&drvdet,BUFLEN,0);
    //Error(GetLastError());
        FileTimeToSystemTime(&drvinfo.DriverDate,&t);
    }*/
    //driverpack_print(&unpacked_drp);
    collection.release();
    SetupDiDestroyDeviceInfoList(hDevInfo);
    time_devicescan=GetTickCount()-time_devicescan;
}

int State::opencatfile(Driver *cur_driver)
{
    WCHAR filename[BUFLEN];
    CHAR bufa[BUFLEN];
    FILE *f;
    char *buft;
    int len;
    *bufa=0;

    wcscpy(filename,textas.getw(windir));
    wsprintf(filename+wcslen(filename)-4,
             L"system32\\CatRoot\\{F750E6C3-38EE-11D1-85E5-00C04FC295EE}\\%ws",
             textas.getw(cur_driver->InfPath));
    wcscpy(filename+wcslen(filename)-3,L"cat");

    f=_wfopen(filename,L"rb");
    //log_con("Open '%S'\n",filename);
    if(f)
    {
        fseek(f,0,SEEK_END);
        len=ftell(f);
        fseek(f,0,SEEK_SET);
        buft=(char *)malloc(len);
        fread(buft,len,1,f);
        fclose(f);

        findosattr(bufa,buft,len);
        free(buft);
    }

    if(*bufa)
    {
        //log_con("'%s'\n",bufa);
        return textas.strcpy(bufa);
    }
    return 0;
}

/*
 1   ***, XX..14, 4:3         ->  desktop
 2   ***, 15..16, 4:3         ->  desktop
 3   ***, 17..XX, 4:3         ->  desktop
 4   ***, XX..14, Widescreen  ->  desktop
 5   ***, 15..16, Widescreen  ->  desktop
 6   ***, 17..XX, Widescreen  ->  desktop
 7   +/-, XX..14, 4:3         ->  assume desktop
 8   +/-, 15..16, 4:3         ->  desktop
 9   +/-, 17..XX, 4:3         ->  desktop
10   +/-, XX..14, Widescreen  ->  assume laptop
11   +/-, 15..18, Widescreen  ->  assume laptop
12   +/-, 18..XX, Widescreen  ->  assume desktop
*/
void State::isnotebook_a()
{
    unsigned int i;
    int x,y;
    int min_v=99,min_x=0,min_y=0;
    int diag;
    int batdev=0;
    WCHAR *buf;
    SYSTEM_POWER_STATUS *batteryloc;
    Device *cur_device;

    buf=textas.getw(monitors);
    batteryloc=(SYSTEM_POWER_STATUS *)(textas.get(battery));

    if(ChassisType==3)
    {
        isLaptop=0;
        return;
    }
    if(ChassisType==10)
    {
        isLaptop=1;
        return;
    }

    for(i=0;i<buf[0];i++)
    {
        x=buf[1+i*2];
        y=buf[2+i*2];
        diag=sqrt(x*x+y*y)/2.54;

        if(diag<min_v||(diag==min_v&&iswide(x,y)))
        {
            min_v=diag;
            min_x=x;
            min_y=y;
        }
    }

    for(i=0;i<Devices_list.size();i++)
    {
        cur_device=&Devices_list[i];
        WCHAR *p;
        char *s=textas.get(0);

        if(cur_device->getHardwareID())
        {
            p=(WCHAR *)(s+cur_device->getHardwareID());
            while(*p)
            {
                if(StrStrI(p,L"*ACPI0003"))batdev=1;
                p+=lstrlen(p)+1;
            }
        }
    }

    if((batteryloc->BatteryFlag&128)==0||batdev)
    {
        if(!buf[0])
            isLaptop=1;
        else if(iswide(min_x,min_y))
            isLaptop=min_v<=18?1:0;
        else
            isLaptop=0;
    }
    else
       isLaptop=0;
}
//}

//{ Monitor info
int GetMonitorDevice(WCHAR* adapterName,DISPLAY_DEVICE *ddMon)
{
    DWORD devMon = 0;

    while(EnumDisplayDevices(adapterName,devMon,ddMon,0))
    {
        if (ddMon->StateFlags&DISPLAY_DEVICE_ACTIVE&&
            ddMon->StateFlags&DISPLAY_DEVICE_ATTACHED) // for ATI, Windows XP
                break;
        devMon++;
    }
    return *ddMon->DeviceID!=0;
}

int GetMonitorSizeFromEDID(WCHAR* adapterName,int *Width,int *Height)
{
    DISPLAY_DEVICE ddMon;
    ZeroMemory(&ddMon,sizeof(ddMon));
    ddMon.cb=sizeof(ddMon);

    *Width=0;
    *Height=0;
    if(GetMonitorDevice(adapterName,&ddMon))
    {
        WCHAR model[18];
        WCHAR* s=wcschr(ddMon.DeviceID,L'\\')+1;
        size_t len=wcschr(s,L'\\')-s;
        wcsncpy(model,s,len);
        model[len]=0;

        WCHAR *path=wcschr(ddMon.DeviceID,L'\\')+1;
        WCHAR str[MAX_PATH]=L"SYSTEM\\CurrentControlSet\\Enum\\DISPLAY\\";
        wcsncat(str,path,wcschr(path, L'\\')-path);
        path=wcschr(path,L'\\')+1;
        HKEY hKey;
        if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,str,0,KEY_READ,&hKey)==ERROR_SUCCESS)
        {
            DWORD i=0;
            DWORD size=MAX_PATH;
            FILETIME ft;
            while(RegEnumKeyEx(hKey,i,str,&size,NULL,NULL,NULL,&ft)==ERROR_SUCCESS)
            {
                HKEY hKey2;
                if(RegOpenKeyEx(hKey,str,0,KEY_READ,&hKey2)==ERROR_SUCCESS)
                {
                    size=MAX_PATH;
                    if(RegQueryValueEx(hKey2, L"Driver",NULL,NULL,(LPBYTE)&str,&size)==ERROR_SUCCESS)
                    {
                        if(wcscmp(str,path)==0)
                        {
                            HKEY hKey3;
                            if(RegOpenKeyEx(hKey2,L"Device Parameters",0,KEY_READ,&hKey3)==ERROR_SUCCESS)
                            {
                                BYTE EDID[256];
                                size=256;
                                if(RegQueryValueEx(hKey3,L"EDID",NULL,NULL,(LPBYTE)&EDID,&size)==ERROR_SUCCESS)
                                {
                                    DWORD p=8;
                                    WCHAR model2[9];

                                    char byte1=EDID[p];
                                    char byte2=EDID[p+1];
                                    model2[0]=((byte1&0x7C)>>2)+64;
                                    model2[1]=((byte1&3)<<3)+((byte2&0xE0)>>5)+64;
                                    model2[2]=(byte2&0x1F)+64;
                                    wsprintf(model2+3,L"%X%X%X%X",(EDID[p+3]&0xf0)>>4,EDID[p+3]&0xf,(EDID[p+2]&0xf0)>>4,EDID[p+2]&0x0f);
                                    if(wcscmp(model,model2)==0)
                                    {
                                        *Width=EDID[22];
                                        *Height=EDID[21];
                                        return 1;
                                    }
                                }
                                RegCloseKey(hKey3);
                            }
                        }
                    }
                    RegCloseKey(hKey2);
                }
                i++;
            }
            RegCloseKey(hKey);
        }
    }
    return 0;
}

int iswide(int x,int y)
{
    return ((double)y/x)>1.35?1:0;
}
//}
