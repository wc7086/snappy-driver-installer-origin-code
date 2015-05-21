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
    wchar_t buffer[BUFLEN];
    /*log_file("%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",g->Data1,g->Data2,g->Data3,
        (int)(g->Data4[0]),(int)(g->Data4[1]),
        (int)(g->Data4[2]),(int)(g->Data4[3]),(int)(g->Data4[4]),
        (int)(g->Data4[5]),(int)(g->Data4[6]),(int)(g->Data4[7]));*/

    *buffer=0;
    if(!SetupDiGetClassDescription(g,buffer,BUFLEN,nullptr))
    {
        //int lr=GetLastError();
        //print_error(lr,L"print_guid()");
    }
    log_file("%S\n",buffer);
}

void Device::read_device_property(HDEVINFO hDevInfo,State *state,int id,ofst *val)
{
    DWORD buffersize=0;
    DWORD DataT=0;
    PBYTE p;
    auto DeviceInfoDataloc=(SP_DEVINFO_DATA *)&DeviceInfoData;

    *val=0;
    if(!SetupDiGetDeviceRegistryProperty(hDevInfo,DeviceInfoDataloc,id,&DataT,nullptr,0,&buffersize))
    {
        int ret_er=GetLastError();
        if(ret_er==ERROR_INVALID_DATA)return;
        if(ret_er!=ERROR_INSUFFICIENT_BUFFER)
        {
            log_file("Property %d\n",id);
            print_error(ret_er,L"read_device_property()");
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
        int ret_er=GetLastError();
        log_file("Property %d\n",id);
        print_error(ret_er,L"read_device_property()");
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
    log_file("  Name:         %S\n",s+Devicedesc);
    log_file("  Status:       ");
    log_file(deviceststus_str[print_status()],problem);
    log_file("\n  Manufacturer: %S\n",s+Mfg);
    log_file("  HWID_reg      %S\n",s+Driver);
    log_file("  Class:        ");print_guid(&DeviceInfoData.ClassGuid);
    log_file("  Location:     \n");
    log_file("  ConfigFlags:  %d\n", ConfigFlags);
    log_file("  Capabilities: %d\n", Capabilities);
}

void Device::printHWIDS(State *state)
{
    if(HardwareID)
    {
        wchar_t *p=state->textas.getw(HardwareID);
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
        wchar_t *p=state->textas.getw(CompatibleIDs);
        log_file("CompatibleID\n");
        while(*p)
        {
            log_file("  %S\n",p);
            p+=lstrlen(p)+1;
        }
    }
}

const wchar_t *Device::getHWIDby(int num)
{
    int i=0;

    if(HardwareID)
    {
        wchar_t *p=manager_g->matcher->getState()->textas.getw(HardwareID);
        while(*p)
        {
            if(i==num)return p;
            p+=lstrlen(p)+1;
            i++;
        }
    }
    if(CompatibleIDs)
    {
        wchar_t *p=manager_g->matcher->getState()->textas.getw(CompatibleIDs);
        while(*p)
        {
            if(i==num)return p;
            p+=lstrlen(p)+1;
            i++;
        }
    }
    return L"";
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

    SetupDiGetDeviceInstanceId(hDevInfo,DeviceInfoDataloc,nullptr,0,&buffersize);
    InstanceId=state->textas.alloc(buffersize);
    SetupDiGetDeviceInstanceId(hDevInfo,DeviceInfoDataloc,state->textas.getw(InstanceId),buffersize,nullptr);

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
        log_err("ERROR %d with CM_Get_DevNode_Status()\n",ret);
    }
}
//}

//{ Driver
void Driver::read_reg_val(HKEY hkey,State *state,const wchar_t *key,ofst *val)
{
    DWORD dwType,dwSize=0;
    int lr;

    *val=0;
    lr=RegQueryValueEx(hkey,key,nullptr,nullptr,nullptr,&dwSize);
    if(lr==ERROR_FILE_NOT_FOUND)return;
    if(lr!=ERROR_SUCCESS)
    {
        log_err("Key %S\n",key);
        print_error(lr,L"RegQueryValueEx()");
        return;
    }

    *val=state->textas.alloc(dwSize);
    lr=RegQueryValueEx(hkey,key,nullptr,&dwType,reinterpret_cast<unsigned char*>(state->textas.get(*val)),&dwSize);
    if(lr!=ERROR_SUCCESS)
    {
        log_err("Key %S\n",key);
        print_error(lr,L"read_reg_val()");
    }
}

void Driver::scaninf(State *state,Driverpack *unpacked_drp,int &inf_pos)
{
    wchar_t filename[BUFLEN];
    wchar_t fnm_hwid[BUFLEN];
    auto inf_list=&state->inf_list_new;

    unsigned HWID_index,start_index=0;

    if(flags&FLAG_FAILSAFE)
    {
        inf_pos=0;
        return;
    }

    wsprintf(filename,L"%s%s",state->textas.get(state->getWindir()),state->textas.get(InfPath));
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
        infdata_t *infdata=&got->second;
        cat=infdata->cat;
        catalogfile=infdata->catalogfile;
        start_index=infdata->start_index;
        //log_file("Match_inf  '%S',%d\n",filename,cat,catalogfile);
    }
    else
    {
        FILE *f;
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
        std::unique_ptr<char[]> buft(new char[len]);
        fread(buft.get(),len,1,f);
        fclose(f);

        if(len>0)
        {
            start_index=unpacked_drp->HWID_list.size();
            unpacked_drp->indexinf(state->textas.getw(state->getWindir()),state->textas.getw(InfPath),buft.get(),len);
        }

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
        Hwidmatch hwidmatch(unpacked_drp,HWID_index);
        if(StrStrIA(sect,hwidmatch.getdrp_drvinstall()))
        {
            feature=hwidmatch.getdrp_drvfeature();
            if(got==inf_list->end())catalogfile=hwidmatch.calc_catalogfile();
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

    inf_list->insert({std::wstring(fnm_hwid),infdata_t(catalogfile,feature,inf_pos,cat,start_index)});
    inf_list->insert({std::wstring(filename),infdata_t(catalogfile,0,0,cat,start_index)});
}

int Driver::findHWID_in_list(const wchar_t *p,const wchar_t *str)
{
    int dev_pos=0;
    while(*p)
    {
        if(!StrCmpIW(p,str))return dev_pos;
        p+=lstrlen(p)+1;
        dev_pos++;
    }
    return -1;
}

void Driver::calc_dev_pos(Device *cur_device,State *state,int *ishw,int *dev_pos)
{
    *ishw=1;
    *dev_pos=findHWID_in_list(state->textas.getw(cur_device->getHardwareID()),state->textas.getw(MatchingDeviceId));
    if(*dev_pos<0&&cur_device->getCompatibleIDs())
    {
        *ishw=0;
        *dev_pos=findHWID_in_list(state->textas.getw(cur_device->getCompatibleIDs()),state->textas.getw(MatchingDeviceId));
    }
}

unsigned Driver::calc_score_h(State *state)
{
    return calc_score(catalogfile,feature,identifierscore, // TODO: check signature
        state,StrStrI(state->textas.getw(InfSectionExt),L".nt")?1:0);
}

void Driver::print(State *state)
{
    char *s=state->textas.get(0);
    wchar_t buf[BUFLEN];

    log_file("  Name:     %S\n",s+DriverDesc);
    log_file("  Provider: %S\n",s+ProviderName);
    version.str_date(buf);
    log_file("  Date:     %S\n",buf);
    version.str_version(buf);
    log_file("  Version:  %S\n");
    log_file("  HWID:     %S\n",s+MatchingDeviceId);
    log_file("  inf:      %S%S,%S%S\n",(s+state->getWindir()),s+InfPath,s+InfSection,s+InfSectionExt);
    log_file("  Score:    %08X %04x\n",calc_score_h(state),identifierscore);
    //log_file("  Sign:     '%s'(%d)\n",s+cat,catalogfile);

    if(log_verbose&LOG_VERBOSE_BATCH)
        log_file("  Filter:   \"%S\"=a,%S\n",s+DriverDesc,s+MatchingDeviceId);
}

Driver::Driver(State *state,Device *cur_device,HKEY hkey,Driverpack *unpacked_drp)
{
    char bufa[BUFLEN];
    int dev_pos,ishw,inf_pos=-1;
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

    calc_dev_pos(cur_device,state,&ishw,&dev_pos);

    if(InfPath)
        scaninf(state,unpacked_drp,inf_pos);

    identifierscore=calc_identifierscore(dev_pos,ishw,inf_pos);
    //log_file("%d,%d,%d,(%x)\n",dev_pos,ishw,inf_pos,identifierscore);

    if(DriverDate)
    {
        wsprintfA(bufa,"%ws",state->textas.get(DriverDate));
        Parser pi{bufa,bufa+strlen(bufa)};
        pi.readDate(&version);
    }

    if(DriverVersion)
    {
        wsprintfA(bufa,"%ws",state->textas.get(DriverVersion));
        Parser pi{bufa,bufa+strlen(bufa)};
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

void State::getWinVer(int *major,int *minor)
{
    *major=platform.dwMajorVersion;
    *minor=platform.dwMinorVersion;
}

wchar_t *State::getProduct()
{
    wchar_t *s=textas.getw(product);

    if(StrStrIW(s,L"Product"))return textas.getw(cs_model);
    return s;
}

wchar_t *State::getManuf()
{
    wchar_t *s=textas.getw(manuf);

    if(StrStrIW(s,L"Vendor"))return textas.getw(cs_manuf);
    return s;
}

wchar_t *State::getModel()
{
    wchar_t *s=textas.getw(model);

    if(!*s)return textas.getw(cs_model);
    return s;
}

State::State()
{
    memset(this,0,sizeof(state_m_t));
    revision=SVN_REV;

    //log_con("sizeof(Device)=%d\nsizeof(Driver)=%d\n\n",sizeof(Device),sizeof(Driver));
}

void State::print()
{
    unsigned i;
    wchar_t *buf;
    SYSTEM_POWER_STATUS *batteryloc;

    if(log_verbose&LOG_VERBOSE_SYSINFO&&log_verbose&LOG_VERBOSE_BATCH)
    {
        log_file("%S (%d.%d.%d), ",get_winverstr(),platform.dwMajorVersion,platform.dwMinorVersion,platform.dwBuildNumber);
        log_file("%s\n",architecture?"64-bit":"32-bit");
        log_file("%s, ",isLaptop?"Laptop":"Desktop");
        log_file("Product='%S', ",textas.getw(product));
        log_file("Model='%S', ",textas.get(model));
        log_file("Manuf='%S'\n",textas.get(manuf));
    }else
    if(log_verbose&LOG_VERBOSE_SYSINFO)
    {
        log_file("Windows\n");
        log_file("  Version:     %S (%d.%d.%d)\n",get_winverstr(),platform.dwMajorVersion,platform.dwMinorVersion,platform.dwBuildNumber);
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

        log_file("DriverInfo\n");
        if(cur_device.getDriverIndex()>=0)
            Drivers_list[cur_device.getDriverIndex()].print(this);
        else
            log_file("  NoDriver\n");

        cur_device.printHWIDS(this);
        log_file("\n\n");
    }

    log_con("State: %d+%d+%d*%d+%d*%d\n",sizeof(State),textas.getSize(),Devices_list.size(),sizeof(Device),Drivers_list.size(),sizeof(Driver));
    //log_file("Errors: %d\n",error_count);
}

void State::popup_sysinfo(HDC hdcMem)
{
    wchar_t bufw[BUFLEN];
    int i,x,y;
    int p0=D(POPUP_OFSX),p1=D(POPUP_OFSX)+10;

    textdata_t td;
    td.col=D(POPUP_TEXT_COLOR);
    td.y=D(POPUP_OFSY);
    td.wy=D(POPUP_WY);
    td.hdcMem=hdcMem;
    td.maxsz=0;

    td.x=p0;
    TextOutF(&td,td.col,STR(STR_SYSINF_WINDOWS));td.x=p1;

    TextOutSF(&td,STR(STR_SYSINF_VERSION),L"%s (%d.%d.%d)",get_winverstr(),platform.dwMajorVersion,platform.dwMinorVersion,platform.dwBuildNumber);
    TextOutSF(&td,STR(STR_SYSINF_UPDATE),L"%s",platform.szCSDVersion);
    TextOutSF(&td,STR(STR_SYSINF_CPU_ARCH),L"%s",architecture?STR(STR_SYSINF_64BIT):STR(STR_SYSINF_32BIT));
    TextOutSF(&td,STR(STR_SYSINF_LOCALE),L"%X",locale);
    //TextOutSF(&td,STR(STR_SYSINF_PLATFORM),L"%d",platform.dwPlatformId);
    /*if(platform.dwOSVersionInfoSize == sizeof(OSVERSIONINFOEX))
    {
        TextOutSF(&td,STR(STR_SYSINF_SERVICEPACK),L"%d.%d",platform.wServicePackMajor,platform.wServicePackMinor);
        TextOutSF(&td,STR(STR_SYSINF_SUITEMASK),L"%d",platform.wSuiteMask);
        TextOutSF(&td,STR(STR_SYSINF_PRODUCTTYPE),L"%d",platform.wProductType);
    }*/
    td.x=p0;
    TextOutF(&td,td.col,STR(STR_SYSINF_ENVIRONMENT));td.x=p1;
    TextOutSF(&td,STR(STR_SYSINF_WINDIR),L"%s",textas.get(windir));
    TextOutSF(&td,STR(STR_SYSINF_TEMP),L"%s",textas.get(temp));

    td.x=p0;
    TextOutF(&td,td.col,STR(STR_SYSINF_MOTHERBOARD));td.x=p1;
    TextOutSF(&td,STR(STR_SYSINF_PRODUCT),L"%s",getProduct());
    TextOutSF(&td,STR(STR_SYSINF_MODEL),L"%s",getModel());
    TextOutSF(&td,STR(STR_SYSINF_MANUF),L"%s",getManuf());
    TextOutSF(&td,STR(STR_SYSINF_TYPE),L"%s[%d]",isLaptop?STR(STR_SYSINF_LAPTOP):STR(STR_SYSINF_DESKTOP),ChassisType);

    td.x=p0;
    TextOutF(&td,td.col,STR(STR_SYSINF_BATTERY));td.x=p1;
    SYSTEM_POWER_STATUS *battery_loc=(SYSTEM_POWER_STATUS *)(textas.get(battery));
    switch(battery_loc->ACLineStatus)
    {
        case 0:wcscpy(bufw,STR(STR_SYSINF_OFFLINE));break;
        case 1:wcscpy(bufw,STR(STR_SYSINF_ONLINE));break;
        default:
        case 255:wcscpy(bufw,STR(STR_SYSINF_UNKNOWN));break;
    }
    TextOutSF(&td,STR(STR_SYSINF_AC_STATUS),L"%s",bufw);

    i=battery_loc->BatteryFlag;
    *bufw=0;
    if(i&1)wcscat(bufw,STR(STR_SYSINF_HIGH));
    if(i&2)wcscat(bufw,STR(STR_SYSINF_LOW));
    if(i&4)wcscat(bufw,STR(STR_SYSINF_CRITICAL));
    if(i&8)wcscat(bufw,STR(STR_SYSINF_CHARGING));
    if(i&128)wcscat(bufw,STR(STR_SYSINF_NOBATTERY));
    if(i==255)wcscat(bufw,STR(STR_SYSINF_UNKNOWN));
    TextOutSF(&td,STR(STR_SYSINF_FLAGS),L"%s",bufw);

    if(battery_loc->BatteryLifePercent!=255)
        TextOutSF(&td,STR(STR_SYSINF_CHARGED),L"%d%%",battery_loc->BatteryLifePercent);
    if(battery_loc->BatteryLifeTime!=0xFFFFFFFF)
        TextOutSF(&td,STR(STR_SYSINF_LIFETIME),L"%d %s",battery_loc->BatteryLifeTime/60,STR(STR_SYSINF_MINS));
    if(battery_loc->BatteryFullLifeTime!=0xFFFFFFFF)
        TextOutSF(&td,STR(STR_SYSINF_FULLLIFETIME),L"%d %s",battery_loc->BatteryFullLifeTime/60,STR(STR_SYSINF_MINS));

    wchar_t *buf=(wchar_t *)(textas.get(monitors));
    td.x=p0;
    TextOutF(&td,td.col,STR(STR_SYSINF_MONITORS));td.x=p1;
    for(i=0;i<buf[0];i++)
    {
        x=buf[1+i*2];
        y=buf[2+i*2];
        td.maxsz+=POPUP_SYSINFO_OFS;
        TextOutF(&td,td.col,L"%d%s x %d%s (%.1f %s) %.3f %s",
                  x,STR(STR_SYSINF_CM),
                  y,STR(STR_SYSINF_CM),
                  sqrt(x*x+y*y)/2.54,STR(STR_SYSINF_INCH),
                  (double)y/x,
                  iswide(x,y)?STR(STR_SYSINF_WIDE):L"");

        td.maxsz-=POPUP_SYSINFO_OFS;
    }

    td.x=p0;
    td.maxsz+=POPUP_SYSINFO_OFS;
    td.y+=td.wy;
    TextOutF(&td,D(POPUP_CMP_BETTER_COLOR),STR(STR_SYSINF_MISC));td.x=p1;
    td.maxsz-=POPUP_SYSINFO_OFS;
    popup_resize((td.maxsz+POPUP_SYSINFO_OFS+p0+p1),td.y+D(POPUP_OFSY));
}

void State::contextmenu2(int x,int y)
{
    HMENU hPopupMenu=CreatePopupMenu();
    HMENU hSub1=CreatePopupMenu();
    int ver=platform.dwMinorVersion+10*platform.dwMajorVersion;

    for(int i=0;i<NUM_OS-1;i++)
    {
        InsertMenu(hSub1,i,MF_BYPOSITION|MF_STRING|(ver==windows_ver[i]?MF_CHECKED:0),
                   ID_WIN_2000+i,windows_name[i]);
    }

    int i=0;
    InsertMenu(hPopupMenu,i++,MF_BYPOSITION|MF_STRING|MF_POPUP,(UINT_PTR)hSub1,STR(STR_SYS_WINVER));
    InsertMenu(hPopupMenu,i++,MF_BYPOSITION|MF_STRING|(architecture==0?MF_CHECKED:0),ID_EMU_32,STR(STR_SYS_32));
    InsertMenu(hPopupMenu,i++,MF_BYPOSITION|MF_STRING|(architecture==1?MF_CHECKED:0),ID_EMU_64,STR(STR_SYS_64));
    InsertMenu(hPopupMenu,i++,MF_BYPOSITION|MF_SEPARATOR,0,nullptr);
    InsertMenu(hPopupMenu,i++,MF_BYPOSITION|MF_STRING,ID_DEVICEMNG,STR(STR_SYS_DEVICEMNG));
    InsertMenu(hPopupMenu,i++,MF_BYPOSITION|MF_SEPARATOR,0,nullptr);
    InsertMenu(hPopupMenu,i++,MF_BYPOSITION|MF_STRING|(flags&FLAG_DISABLEINSTALL?MF_CHECKED:0),ID_DIS_INSTALL,STR(STR_SYS_DISINSTALL));
    InsertMenu(hPopupMenu,i++,MF_BYPOSITION|MF_STRING|(flags&FLAG_NORESTOREPOINT?MF_CHECKED:0),ID_DIS_RESTPNT,STR(STR_SYS_DISRESTPNT));

    RECT rect;
    SetForegroundWindow(hMain);
    GetWindowRect(hMain,&rect);
    TrackPopupMenu(hPopupMenu,TPM_LEFTALIGN,rect.left+x,rect.top+y,0,hMain,nullptr);
}

void State::save(const wchar_t *filename)
{
    FILE *f;
    int sz;
    int version=VER_STATE;

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
        textas.getSize()+
        2*3*sizeof(int);  // 3 heaps

    std::unique_ptr<char[]> mem(new char[sz]);
    char *p=mem.get();

    fwrite(VER_MARKER,3,1,f);
    fwrite(&version,sizeof(int),1,f);

    memcpy(p,this,sizeof(state_m_t));p+=sizeof(state_m_t);
    p=vector_save(&Devices_list,p);
    p=vector_save(&Drivers_list,p);
    p=vector_save(textas.getVector(),p);

    if(1)
    {
        std::unique_ptr<char[]> mem_pack(new char[sz]);
        sz=encode(mem_pack.get(),sz,mem.get(),sz);
        fwrite(mem_pack.get(),sz,1,f);
    }
    else fwrite(mem.get(),sz,1,f);

    fclose(f);
    log_con("OK\n");
}

int  State::load(const wchar_t *filename)
{
    char buf[BUFLEN];
    FILE *f;
    int sz;
    int version;

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

    std::unique_ptr<char[]> mem(new char[sz]);
    char *p=mem.get();
    fread(mem.get(),sz,1,f);

    UInt64 sz_unpack;
    Lzma86_GetUnpackSize((Byte *)p,sz,&sz_unpack);
    std::unique_ptr<char[]> mem_unpack(new char[sz_unpack]);
    decode(mem_unpack.get(),sz_unpack,mem.get(),sz);
    p=mem_unpack.get();

    memcpy(this,p,sizeof(state_m_t));p+=sizeof(state_m_t);
    p=vector_load(&Devices_list,p);
    p=vector_load(&Drivers_list,p);
    p=vector_load(textas.getVector(),p);

    fakeOSversion();

    fclose(f);
    log_con("OK\n");
    return 1;
}

void State::getsysinfo_fast()
{
    wchar_t buf[BUFLEN];

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
    while(EnumDisplayDevices(nullptr,i,&DispDev,0))
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
    monitors=textas.t_memcpy((char *)buf,(1+buf[0]*2)*2);

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
    windir=textas.strcpyw(buf);

    GetEnvironmentVariable(L"TEMP",buf,BUFLEN);
    temp=textas.strcpyw(buf);

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
    wchar_t smanuf[BUFLEN];
    wchar_t smodel[BUFLEN];
    wchar_t sproduct[BUFLEN];
    wchar_t scs_manuf[BUFLEN];
    wchar_t scs_model[BUFLEN];

    if((invaidate_set&INVALIDATE_SYSINFO)==0)return;
    time_sysinfo=GetTickCount();

    getbaseboard(smanuf,smodel,sproduct,scs_manuf,scs_model,&ChassisType);

    manuf=textas.strcpyw(smanuf);
    product=textas.strcpyw(sproduct);
    model=textas.strcpyw(smodel);
    cs_manuf=textas.strcpyw(scs_manuf);
    cs_model=textas.strcpyw(scs_model);

    time_sysinfo=GetTickCount()-time_sysinfo;
}

void State::getsysinfo_slow(State *prev)
{
    time_sysinfo=0;
    manuf=textas.strcpyw(prev->textas.getw(prev->manuf));
    product=textas.strcpyw(prev->textas.getw(prev->product));
    model=textas.strcpyw(prev->textas.getw(prev->model));
    cs_manuf=textas.strcpyw(prev->textas.getw(prev->cs_manuf));
    cs_model=textas.strcpyw(prev->textas.getw(prev->cs_model));
}

void State::scanDevices()
{
    HDEVINFO hDevInfo;
    HKEY   hkey;
    wchar_t buf[BUFLEN];
    Collection collection{textas.getw(windir),L"",L""};
    Driverpack unpacked_drp{L"",L"windir.7z",&collection};

    if((invaidate_set&INVALIDATE_DEVICES)==0)return;

    time_devicescan=GetTickCount();
    //collection.init(textas.getw(windir),L"",L"");
    Devices_list.clear();
    Drivers_list.clear();
    inf_list_new.clear();

    hDevInfo=SetupDiGetClassDevs(nullptr,nullptr,nullptr,DIGCF_PRESENT|DIGCF_ALLCLASSES);
    if(hDevInfo==INVALID_HANDLE_VALUE)
    {
        print_error(GetLastError(),L"SetupDiGetClassDevs()");
        return;
    }

    for(unsigned i=0;;i++)
    {
        // Device
        Devices_list.emplace_back((Device(hDevInfo,this,i)));
        Device *cur_device=&Devices_list.back();

        int ret=cur_device->getRet();
        if(ret)
        {
            Devices_list.pop_back();
            if(ret==ERROR_NO_MORE_ITEMS)
                break;
            else
                continue;
        }

        // Driver
        if(!cur_device->getDriver())continue;
        wsprintf(buf,L"SYSTEM\\CurrentControlSet\\Control\\Class\\%s",textas.getw(cur_device->getDriver()));
        ret=RegOpenKeyEx(HKEY_LOCAL_MACHINE,buf,0,KEY_QUERY_VALUE,&hkey);
        switch(ret)
        {
            case ERROR_SUCCESS:
                cur_device->setDriverIndex(Drivers_list.size());
                Drivers_list.emplace_back(Driver(this,cur_device,hkey,&unpacked_drp));
                break;

            default:
                print_error(ret,L"RegOpenKeyEx()");

            case ERROR_FILE_NOT_FOUND:
                break;
        }
        RegCloseKey(hkey);
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);
    time_devicescan=GetTickCount()-time_devicescan;
}

const wchar_t *State::get_winverstr()
{
    int i;
    int ver=platform.dwMinorVersion;
    ver+=10*platform.dwMajorVersion;

    if(ver==64)ver=100;
    if(ver==52)
    {
        if(architecture)
            ver=51;
        else
            return L"Windows Server 2003";
    }
    for(i=0;i<NUM_OS;i++)if(windows_ver[i]==ver)return windows_name[i];
    return windows_name[NUM_OS-1];
}

int State::opencatfile(Driver *cur_driver)
{
    wchar_t filename[BUFLEN];
    CHAR bufa[BUFLEN];
    FILE *f;
    *bufa=0;

    wcscpy(filename,textas.getw(windir));
    wsprintf(filename+wcslen(filename)-4,
             L"system32\\CatRoot\\{F750E6C3-38EE-11D1-85E5-00C04FC295EE}\\%ws",
             textas.getw(cur_driver->getInfPath()));
    wcscpy(filename+wcslen(filename)-3,L"cat");

    f=_wfopen(filename,L"rb");
    //log_con("Open '%S'\n",filename);
    if(f)
    {
        fseek(f,0,SEEK_END);
        int len=ftell(f);
        fseek(f,0,SEEK_SET);
        std::unique_ptr<char[]> buft(new char[len]);
        fread(buft.get(),len,1,f);
        fclose(f);

        findosattr(bufa,buft.get(),len);
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
    int min_v=99,min_x=0,min_y=0;
    int batdev=0;
    wchar_t *buf;
    SYSTEM_POWER_STATUS *batteryloc;

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
        int x=buf[1+i*2];
        int y=buf[2+i*2];
        int diag=sqrt(x*x+y*y)/2.54;

        if(diag<min_v||(diag==min_v&&iswide(x,y)))
        {
            min_v=diag;
            min_x=x;
            min_y=y;
        }
    }

    for(auto &cur_device:Devices_list)
    {
        if(cur_device.getHardwareID())
        {
            wchar_t *p=textas.getw(cur_device.getHardwareID());
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
int GetMonitorDevice(wchar_t* adapterName,DISPLAY_DEVICE *ddMon)
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

int GetMonitorSizeFromEDID(wchar_t* adapterName,int *Width,int *Height)
{
    DISPLAY_DEVICE ddMon;
    ZeroMemory(&ddMon,sizeof(ddMon));
    ddMon.cb=sizeof(ddMon);

    *Width=0;
    *Height=0;
    if(GetMonitorDevice(adapterName,&ddMon))
    {
        wchar_t model[18];
        wchar_t* s=wcschr(ddMon.DeviceID,L'\\')+1;
        size_t len=wcschr(s,L'\\')-s;
        wcsncpy(model,s,len);
        model[len]=0;

        wchar_t *path=wcschr(ddMon.DeviceID,L'\\')+1;
        wchar_t str[MAX_PATH]=L"SYSTEM\\CurrentControlSet\\Enum\\DISPLAY\\";
        wcsncat(str,path,wcschr(path, L'\\')-path);
        path=wcschr(path,L'\\')+1;
        HKEY hKey;
        if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,str,0,KEY_READ,&hKey)==ERROR_SUCCESS)
        {
            DWORD i=0;
            DWORD size=MAX_PATH;
            FILETIME ft;
            while(RegEnumKeyEx(hKey,i,str,&size,nullptr,nullptr,nullptr,&ft)==ERROR_SUCCESS)
            {
                HKEY hKey2;
                if(RegOpenKeyEx(hKey,str,0,KEY_READ,&hKey2)==ERROR_SUCCESS)
                {
                    size=MAX_PATH;
                    if(RegQueryValueEx(hKey2, L"Driver",nullptr,nullptr,(LPBYTE)&str,&size)==ERROR_SUCCESS)
                    {
                        if(wcscmp(str,path)==0)
                        {
                            HKEY hKey3;
                            if(RegOpenKeyEx(hKey2,L"Device Parameters",0,KEY_READ,&hKey3)==ERROR_SUCCESS)
                            {
                                BYTE EDID[256];
                                size=256;
                                if(RegQueryValueEx(hKey3,L"EDID",nullptr,nullptr,(LPBYTE)&EDID,&size)==ERROR_SUCCESS)
                                {
                                    DWORD p=8;
                                    wchar_t model2[9];

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
    return (static_cast<double>(y)/x)>1.35?1:0;
}
//}
