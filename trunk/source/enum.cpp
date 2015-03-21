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

const char *deviceststus_str[]=
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

//{ Print
void print_guid(GUID *g)
{
    WCHAR buffer[BUFLEN];
    /*log_file("%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",g->Data1,g->Data2,g->Data3,
        (int)(g->Data4[0]),(int)(g->Data4[1]),
        (int)(g->Data4[2]),(int)(g->Data4[3]),(int)(g->Data4[4]),
        (int)(g->Data4[5]),(int)(g->Data4[6]),(int)(g->Data4[7]));*/

    memset(buffer,0,sizeof(buffer));
    if(!SetupDiGetClassDescription(g,buffer,BUFLEN,0))
    {
        //int lr=GetLastError();
        //print_error(lr,L"print_guid()");
    }
    log_file("%ws\n",buffer);
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
//}

//{ Device/driver
void read_device_property(HDEVINFO hDevInfo,SP_DEVINFO_DATA *DeviceInfoData,State *state,int id,ofst *val)
{
    DWORD buffersize=0;
    int lr;
    DWORD DataT=0;
    PBYTE p;
    BYTE buf[BUFLEN];

    memset(buf,0,BUFLEN);

    if(!SetupDiGetDeviceRegistryProperty(hDevInfo,DeviceInfoData,id,&DataT,0,0,&buffersize))
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
        *val=heap_alloc(&state->text_handle_st,buffersize);
        p=(PBYTE)(state->text+*val);
    }
    memset(p,0,buffersize);
    if(!SetupDiGetDeviceRegistryProperty(hDevInfo,DeviceInfoData,id,&DataT,(PBYTE)buf,buffersize,&buffersize))
    {
        lr=GetLastError();
        log_file("Property %d\n",id);
        print_error(lr,L"read_device_property()");
        return;
    }
    memcpy(p,buf,buffersize);
}

void read_reg_val(HKEY hkey,State *state,const WCHAR *key,ofst *val)
{
    DWORD dwType,dwSize=0;
    int lr;

    *val=0;
    lr=RegQueryValueEx(hkey,key,NULL,0,0,&dwSize);
    if(lr==ERROR_FILE_NOT_FOUND)return;
    if(lr!=ERROR_SUCCESS)
    {
        log_file("Key %ws\n",key);
        print_error(lr,L"RegQueryValueEx()");
        return;
    }

    *val=heap_alloc(&state->text_handle_st,(int)(dwSize));
    lr=RegQueryValueEx(hkey,key,NULL,&dwType,(unsigned char *)(state->text+*val),&dwSize);
    if(lr!=ERROR_SUCCESS)
    {
        log_file("Key %ws\n",key);
        print_error(lr,L"read_reg_val()");
    }
}

void Device::print(State *state)
{
    /*log_file("Device[%d]\n",i);
    log_file("  InstanceID:'%ws'\n",state->text+InstanceId);
    log_file("  ClassGuid:");printguid(&DeviceInfoData.ClassGuid);
    log_file("  DevInst: %d\n",DeviceInfoData.DevInst);*/

    char *s=state->text;

    log_file("DeviceInfo\n");
    log_file("##Name:#########%ws\n",s+Devicedesc);
    log_file("##Status:#######");
    log_file(deviceststus_str[print_status()],problem);
    log_file("\n##Manufacturer:#%ws\n",s+Mfg);
    log_file("##HWID_reg######%ws\n",s+Driver);
    log_file("##Class:########");    print_guid(&DeviceInfoData.ClassGuid);
    log_file("##Location:#####\n");
    log_file("##ConfigFlags:##%d\n", ConfigFlags);
    log_file("##Capabilities:#%d\n", Capabilities);
}

void Device::printHWIDS(State *state)
{
    WCHAR *p;
    char *s=state->text;

    if(HardwareID)
    {
        p=(WCHAR *)(s+HardwareID);
        log_file("HardwareID\n");
        while(*p)
        {
            log_file("  %ws\n",p);
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
            log_file("  %ws\n",p);
            p+=lstrlen(p)+1;
        }
    }
}

void Driver::print(State *state)
{
    char *s=state->text;
    WCHAR buf[BUFLEN];

    str_date(&version,buf);
    log_file("##Name:#####%ws\n",s+DriverDesc);
    log_file("##Provider:#%ws\n",s+ProviderName);
    log_file("##Date:#####%ws\n",buf);
    log_file("##Version:##%d.%d.%d.%d\n",version.v1,version.v2,version.v3,version.v4);
    log_file("##HWID:#####%ws\n",s+MatchingDeviceId);
    log_file("##inf:######%ws%ws,%ws%ws\n",(s+state->windir),s+InfPath,s+InfSection,s+InfSectionExt);
    int score=calc_score_h(this,state);
    log_file("##Score:####%08X %04x\n",score,identifierscore);

    if(log_verbose&LOG_VERBOSE_BATCH)
        log_file("##Filter:###\"%ws\"=a,%ws\n",s+DriverDesc,s+MatchingDeviceId);
}
//}

//{ State
void State::init()
{
    memset(this,0,sizeof(State));
    heap_init(&text_handle_st,ID_STATE_TEXT,(void **)&text,0,1);
    heap_alloc(&text_handle_st,2);
    text[0]=0;
    text[1]=0;
    revision=SVN_REV;

    //log_file("sizeof(Device)=%d\nsizeof(Driver)=%d\n\n",sizeof(Device),sizeof(Driver));
}

void State::release()
{
    Devices_list.clear();
    Drivers_list.clear();
    heap_free(&text_handle_st);
}

template <class T>
char *vector_save(std::vector<T> *v,char *p)
{
    int used=v->size()*sizeof(T);
    int val;

    memcpy(p,&used,sizeof(int));p+=sizeof(int);

    val=v->size();
    memcpy(p,&val,sizeof(int));p+=sizeof(int);

    memcpy(p,&v->front(),used);p+=used;
    return p;
}

template <class T>
char *vector_load(std::vector<T> *v,char *p)
{
    int sz,num;

    memcpy(&sz,p,sizeof(int));p+=sizeof(int);
    memcpy(&num,p,sizeof(int));p+=sizeof(int);
    v->resize(num);
    log_con("SZ %d,%d\n",num,v->size());
    memcpy(v->data(),p,sz);p+=sz;
    return p;
}

void State::save(const WCHAR *filename)
{
    FILE *f;
    int sz;
    int version=VER_STATE;
    char *mem,*mem_pack,*p;

    if(flags&FLAG_NOSNAPSHOT)return;
    log_file("Saving state in '%ws'...",filename);
    if(!canWrite(filename))
    {
        log_err("ERROR in state_save(): Write-protected,'%ws'\n",filename);
        return;
    }
    f=_wfopen(filename,L"wb");
    if(!f)
    {
        log_err("ERROR in state_save(): failed _wfopen(%ws)\n",errno_str());
        return;
    }

    sz=
        sizeof(state_m_t)+
        Drivers_list.size()*sizeof(Driver)+
        Devices_list.size()*sizeof(Device)+
        text_handle_st.used+
        2*3*sizeof(int);  // 3 heaps

    p=mem=(char *)malloc(sz);

    fwrite(VER_MARKER,3,1,f);
    fwrite(&version,sizeof(int),1,f);

    memcpy(p,this,sizeof(state_m_t));p+=sizeof(state_m_t);
    p=vector_save(&Devices_list,p);
    p=vector_save(&Drivers_list,p);
    p=heap_save(&text_handle_st,p);

    if(1)
    {
        mem_pack=(char *)malloc(sz);
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
    //WCHAR txt2[256];
    FILE *f;
    int sz;
    int version;
    char *mem,*p,*mem_unpack=0;

    log_con("Loading state from '%ws'...",filename);
    f=_wfopen(filename,L"rb");
    if(!f)
    {
        log_err("FAILED(%ws)\n",errno_str());
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
        log_err("FAILED(invalid snapshot)\n");
        return 0;
    }
    if(version!=VER_STATE)
    {
        log_err("FAILED(invalid version)\n");
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
    p=heap_load(&text_handle_st,p);

    fakeOSversion();

    free(mem);
    if(mem_unpack)free(mem_unpack);
    fclose(f);
    log_con("OK\n");
    return 1;
}

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

void State::print()
{
    int x,y;
    unsigned i;
    Device *cur_device;
    WCHAR *buf;
    SYSTEM_POWER_STATUS *batteryloc;

    if(log_verbose&LOG_VERBOSE_SYSINFO&&log_verbose&LOG_VERBOSE_BATCH)
    {
        log_file("%ws (%d.%d.%d), ",get_winverstr(manager_g),platform.dwMajorVersion,platform.dwMinorVersion,platform.dwBuildNumber);
        log_file("%s\n",architecture?"64-bit":"32-bit");
        log_file("%s, ",isLaptop?"Laptop":"Desktop");
        log_file("Product='%ws', ",text+product);
        log_file("Model='%ws', ",text+model);
        log_file("Manuf='%ws'\n",text+manuf);
    }else
    if(log_verbose&LOG_VERBOSE_SYSINFO)
    {
        log_file("Windows\n");
        log_file("  Version:     %ws (%d.%d.%d)\n",get_winverstr(manager_g),platform.dwMajorVersion,platform.dwMinorVersion,platform.dwBuildNumber);
        log_file("  PlatformId:  %d\n",platform.dwPlatformId);
        log_file("  Update:      %ws\n",platform.szCSDVersion);
        if(platform.dwOSVersionInfoSize == sizeof(OSVERSIONINFOEX))
        {
            log_file("  ServicePack: %d.%d\n",platform.wServicePackMajor,platform.wServicePackMinor);
            log_file("  SuiteMask:   %d\n",platform.wSuiteMask);
            log_file("  ProductType: %d\n",platform.wProductType);
        }
        log_file("\nEnvironment\n");
        log_file("  windir:      %ws\n",text+windir);
        log_file("  temp:        %ws\n",text+temp);

        log_file("\nMotherboard\n");
        log_file("  Product:     %ws\n",text+product);
        log_file("  Model:       %ws\n",text+model);
        log_file("  Manuf:       %ws\n",text+manuf);
        log_file("  cs_Model:    %ws\n",text+cs_model);
        log_file("  cs_Manuf:    %ws\n",text+cs_manuf);
        log_file("  Chassis:     %d\n",ChassisType);

        log_file("\nBattery\n");
        batteryloc=(SYSTEM_POWER_STATUS *)(text+battery);
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

        buf=(WCHAR *)(text+monitors);
        log_file("\nMonitors\n");
        for(i=0;i<buf[0];i++)
        {
            x=buf[1+i*2];
            y=buf[2+i*2];
            log_file("  %dcmx%dcm (%.1fin)\t%.3f %s\n",x,y,sqrt(x*x+y*y)/2.54,(double)y/x,iswide(x,y)?"wide":"");
        }

        log_file("\nMisc\n");
        log_file("  Type:        %s\n",isLaptop?"Laptop":"Desktop");
        log_file("  Locale:      %X\n",locale);
        log_file("  CPU_Arch:    %s\n",architecture?"64-bit":"32-bit");
        log_file("\n");

    }

    if(log_verbose&LOG_VERBOSE_DEVICES)
    for(i=0;i<Devices_list.size();i++)
    {
        cur_device=&Devices_list[i];

        cur_device->print(this);

        log_file("DriverInfo\n",cur_device->getDriverIndex());
        if(cur_device->getDriverIndex()>=0)
            Drivers_list[cur_device->getDriverIndex()].print(this);
        else
            log_file("##NoDriver\n");

        cur_device->printHWIDS(this);
        log_file("\n\n");
    }
    //log_file("Errors: %d\n",error_count);
}

void State::getsysinfo_fast()
{
    SYSTEM_POWER_STATUS *batteryloc;
    DISPLAY_DEVICE DispDev;
    int x,y,i=0;
    WCHAR buf[BUFLEN];

    time_test=GetTickCount();

    heap_reset(&text_handle_st,2);
    // Battery
    battery=heap_alloc(&text_handle_st,sizeof(SYSTEM_POWER_STATUS));
    batteryloc=(SYSTEM_POWER_STATUS *)(text+battery);
    GetSystemPowerStatus(batteryloc);

    // Monitors
    memset(&DispDev,0,sizeof(DispDev));
    DispDev.cb=sizeof(DispDev);
    buf[0]=0;
    while(EnumDisplayDevices(0,i,&DispDev,0))
    {
        GetMonitorSizeFromEDID(DispDev.DeviceName,&x,&y);
        if(x&&y)
        {
            buf[buf[0]*2+1]=x;
            buf[buf[0]*2+2]=y;
            buf[0]++;
        }
        i++;
    }
    monitors=heap_memcpy(&text_handle_st,buf,(1+buf[0]*2)*2);

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
    windir=heap_memcpy(&text_handle_st,buf,wcslen(buf)*2+2);

    GetEnvironmentVariable(L"TEMP",buf,BUFLEN);
    temp=heap_memcpy(&text_handle_st,buf,wcslen(buf)*2+2);

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

WCHAR *State::getProduct()
{
    WCHAR *s=(WCHAR *)(text+product);

    if(StrStrIW(s,L"Product"))return (WCHAR *)(text+cs_model);
    return s;
}

WCHAR *State::getManuf()
{
    WCHAR *s=(WCHAR *)(text+manuf);

    if(StrStrIW(s,L"Vendor"))return (WCHAR *)(text+cs_manuf);
    return s;
}

WCHAR *State::getModel()
{
    WCHAR *s=(WCHAR *)(text+model);

    if(!*s)return (WCHAR *)(text+cs_model);
    return s;
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
    manuf=heap_memcpy(&text_handle_st,smanuf,wcslen(smanuf)*2+2);
    product=heap_memcpy(&text_handle_st,sproduct,wcslen(sproduct)*2+2);
    model=heap_memcpy(&text_handle_st,smodel,wcslen(smodel)*2+2);
    cs_manuf=heap_memcpy(&text_handle_st,scs_manuf,wcslen(scs_manuf)*2+2);
    cs_model=heap_memcpy(&text_handle_st,scs_model,wcslen(scs_model)*2+2);

    time_sysinfo=GetTickCount()-time_sysinfo;
}

int State::opencatfile(Driver *cur_driver)
{
    WCHAR filename[BUFLEN];
    CHAR bufa[BUFLEN];

    wcscpy(filename,(WCHAR *)(text+windir));
    wsprintf(filename+wcslen(filename)-4,
             L"system32\\CatRoot\\{F750E6C3-38EE-11D1-85E5-00C04FC295EE}\\%ws",
             (WCHAR *)(text+cur_driver->InfPath));

    wcscpy(filename+wcslen(filename)-3,L"cat");
    {
        FILE *f;
        char *buft;
        int len;

        *bufa=0;
        f=_wfopen(filename,L"rb");
        if(f)
        {
            //log_con("Open '%ws'\n",filename);
            fseek(f,0,SEEK_END);
            len=ftell(f);
            fseek(f,0,SEEK_SET);
            buft=(char *)malloc(len);
            fread(buft,len,1,f);
            fclose(f);
            {
                unsigned bufal=0;
                char *p=buft;

                while(p+11<buft+len)
                {
                    if(*p=='O'&&!memcmp(p,L"OSAttr",11))
                    {
                        if(!*bufa||bufal<wcslen((WCHAR *)(p+19)))
                        {
                            sprintf(bufa,"%ws",p+19);
                            bufal=strlen(bufa);
                        }
                    }
                    p++;
                }
                //if(*bufa)log_con("[%s]\n",bufa);
            }
            free(buft);
        }
    }

    return *bufa?heap_strcpy(&text_handle_st,bufa):0;
}

int Device::device_readprop(HDEVINFO hDevInfo,State *state,int i)
{
    DWORD buffersize;
    SP_DEVINFO_DATA *DeviceInfoDataloc;
    int r;

    DeviceInfoDataloc=(SP_DEVINFO_DATA *)&DeviceInfoData;
    memset(&DeviceInfoData,0,sizeof(SP_DEVINFO_DATA));
    DeviceInfoData.cbSize=sizeof(SP_DEVINFO_DATA);

    if(!SetupDiEnumDeviceInfo(hDevInfo,i,DeviceInfoDataloc))
    {
        r=GetLastError();
        if(r==ERROR_NO_MORE_ITEMS)
            state->Devices_list.pop_back();
        else
            print_error(r,L"SetupDiEnumDeviceInfo()");
        return 1;
    }

    SetupDiGetDeviceInstanceId(hDevInfo,DeviceInfoDataloc,0,0,&buffersize);
    InstanceId=heap_alloc(&state->text_handle_st,buffersize);
    SetupDiGetDeviceInstanceId(hDevInfo,DeviceInfoDataloc,(WCHAR *)(state->text+InstanceId),BUFLEN,0);
    read_device_property(hDevInfo,DeviceInfoDataloc,state,SPDRP_DEVICEDESC,    &Devicedesc);
    read_device_property(hDevInfo,DeviceInfoDataloc,state,SPDRP_HARDWAREID,    &HardwareID);
    read_device_property(hDevInfo,DeviceInfoDataloc,state,SPDRP_COMPATIBLEIDS, &CompatibleIDs);
    read_device_property(hDevInfo,DeviceInfoDataloc,state,SPDRP_DRIVER,        &Driver);
    read_device_property(hDevInfo,DeviceInfoDataloc,state,SPDRP_MFG,           &Mfg);
    read_device_property(hDevInfo,DeviceInfoDataloc,state,SPDRP_FRIENDLYNAME,  &FriendlyName);
    read_device_property(hDevInfo,DeviceInfoDataloc,state,SPDRP_CAPABILITIES,  &Capabilities);
    read_device_property(hDevInfo,DeviceInfoDataloc,state,SPDRP_CONFIGFLAGS,   &ConfigFlags);

    r=CM_Get_DevNode_Status(&status,&problem,DeviceInfoDataloc->DevInst,0);
    ret=r;
    if(r!=CR_SUCCESS)
    {
        log_file("Error %d with CM_Get_DevNode_Status()\n",r);
    }
    return 0;
}

void scaninf(State *state,Driver *cur_driver,hashtable_t *inf_list,driverpack_t *unpacked_drp,int &inf_pos)
{
    WCHAR filename[BUFLEN];
    infdata_t *infdata;
    WCHAR buf[BUFLEN];
    char bufa[BUFLEN];

    FILE *f;
    char *buft;
    int len;
    int HWID_index;

    wsprintf(filename,L"%s%s",state->text+state->windir,state->text+cur_driver->InfPath);

    int isfound;
    sprintf(bufa,"%ws%ws",filename,state->text+cur_driver->MatchingDeviceId);
    infdata=(infdata_t *)hash_find(inf_list,bufa,strlen(bufa),&isfound);
    if(flags&FLAG_FAILSAFE)
    {
        inf_pos=0;
    }
    else
    if(isfound)
    {
        //log_file("Skipped '%ws'\n",filename,inf_pos);
        cur_driver->feature=infdata->feature;
        cur_driver->catalogfile=infdata->catalogfile;
        cur_driver->cat=infdata->cat;
        inf_pos=infdata->inf_pos;
    }else
    {
        //log_file("Looking for '%ws'\n",filename);
        f=_wfopen(filename,L"rb");
        if(!f)
        {
            log_err("ERROR: Not found '%ws'\n",filename);
            return;
        }
        fseek(f,0,SEEK_END);
        len=ftell(f);
        if(len<0)len=0;
        fseek(f,0,SEEK_SET);
        buft=(char *)malloc(len);
        fread(buft,len,1,f);
        fclose(f);

        wsprintf(buf,L"%ws",state->text+state->windir);
        if(len>0)
        driverpack_indexinf(unpacked_drp,buf,(WCHAR *)(state->text+cur_driver->InfPath),buft,len);
        free(buft);

        char sect[BUFLEN];
        sprintf(sect,"%ws%ws",state->text+cur_driver->InfSection,state->text+cur_driver->InfSectionExt);
        sprintf(bufa,"%ws",state->text+cur_driver->MatchingDeviceId);
        for(HWID_index=0;HWID_index<unpacked_drp->HWID_list_handle.items;HWID_index++)
        if(!strcmpi(unpacked_drp->text+unpacked_drp->HWID_list[HWID_index].HWID,bufa))
        {
            hwidmatch_t hwidmatch;

            hwidmatch_initbriefly(&hwidmatch,unpacked_drp,HWID_index);
            if(StrStrIA(sect,getdrp_drvinstall(&hwidmatch)))
            {
                cur_driver->feature=getdrp_drvfeature(&hwidmatch);
                cur_driver->catalogfile=calc_catalogfile(&hwidmatch);
                if(inf_pos<0||inf_pos>getdrp_drvinfpos(&hwidmatch))inf_pos=getdrp_drvinfpos(&hwidmatch);
            }
        }
        if(inf_pos==-1)
            log_err("ERROR: not found '%s'\n",sect);

        cur_driver->cat=state->opencatfile(cur_driver);

        //log_file("Added '%ws',%d\n",filename,inf_pos);
        infdata=(infdata_t *)malloc(sizeof(infdata_t));
        infdata->catalogfile=cur_driver->catalogfile;
        infdata->feature=cur_driver->feature;
        infdata->cat=cur_driver->cat;
        infdata->inf_pos=inf_pos;
        sprintf(bufa,"%ws%ws",filename,state->text+cur_driver->MatchingDeviceId);
        hash_add(inf_list,bufa,strlen(bufa),(intptr_t)infdata,HASH_MODE_INTACT);
    }
}

void driver_read(Driver *cur_driver,State *state,Device *cur_device,HKEY hkey,hashtable_t *inf_list,driverpack_t *unpacked_drp)
{
    char bufa[BUFLEN];
    int dev_pos,ishw,inf_pos=-1;
    Parser_str pi;

    read_reg_val(hkey,state,L"DriverDesc",         &cur_driver->DriverDesc);
    read_reg_val(hkey,state,L"ProviderName",       &cur_driver->ProviderName);
    read_reg_val(hkey,state,L"DriverDate",         &cur_driver->DriverDate);
    read_reg_val(hkey,state,L"DriverVersion",      &cur_driver->DriverVersion);
    read_reg_val(hkey,state,L"MatchingDeviceId",   &cur_driver->MatchingDeviceId);

    read_reg_val(hkey,state,L"InfPath",            &cur_driver->InfPath);
    read_reg_val(hkey,state,L"InfSection",         &cur_driver->InfSection);
    read_reg_val(hkey,state,L"InfSectionExt",      &cur_driver->InfSectionExt);

    getdd(cur_device,state,&ishw,&dev_pos);

    if(cur_driver->InfPath)
        scaninf(state,cur_driver,inf_list,unpacked_drp,inf_pos);

    cur_driver->identifierscore=calc_identifierscore(dev_pos,ishw,inf_pos);
    //log_file("%d,%d,%d\n",dev_pos,ishw,inf_pos);

    sprintf(bufa,"%ws",state->text+cur_driver->DriverDate);
    pi.setRange(bufa,bufa+strlen(bufa));
    pi.readDate(&cur_driver->version);

    sprintf(bufa,"%ws",state->text+cur_driver->DriverVersion);
    pi.setRange(bufa,bufa+strlen(bufa));
    pi.readVersion(&cur_driver->version);
}

void State::state_scandevices()
{
    HDEVINFO hDevInfo;
    HKEY   hkey;
    WCHAR buf[BUFLEN];
    Driver *cur_driver;
    Collection collection;
    driverpack_t unpacked_drp;
    hashtable_t inf_list;
    unsigned i;
    int lr;

    time_devicescan=GetTickCount();
    collection.collection_init((WCHAR *)(text+windir),L"",L"",0);
    driverpack_init(&unpacked_drp,L"",L"windir.7z",&collection);
    hash_init(&inf_list,ID_INF_LIST,200,HASH_FLAG_KEYS_ARE_POINTERS);
    Devices_list.clear();

    hDevInfo=SetupDiGetClassDevs(0,0,0,DIGCF_PRESENT|DIGCF_ALLCLASSES);
    if(hDevInfo==INVALID_HANDLE_VALUE)
    {
        print_error(GetLastError(),L"SetupDiGetClassDevs()");
        return;
    }

    i=0;
    while(1)
    {
        //log_file("%d,%d/%d\n",i,text_handle.used,text_handle.allocated);
        heap_refresh(&text_handle_st);
        Devices_list.push_back(Device());
        Device *cur_device=&Devices_list.back();

        if(cur_device->device_readprop(hDevInfo,this,i))break;


        // Driver
        cur_device->setDriverIndex(-1);
        if(!cur_device->getDriver()){i++;continue;}
        wsprintf(buf,L"SYSTEM\\CurrentControlSet\\Control\\Class\\%s",
             (WCHAR *)(text+cur_device->getDriver()));

        lr=RegOpenKeyEx(HKEY_LOCAL_MACHINE,buf,0,KEY_QUERY_VALUE,&hkey);
        if(lr==ERROR_FILE_NOT_FOUND)
        {

        }
        else if(lr!=ERROR_SUCCESS)
        {
            print_error(lr,L"RegOpenKeyEx()");
        }
        else if(lr==ERROR_SUCCESS)
        {
            cur_device->setDriverIndex(Drivers_list.size());
            Drivers_list.push_back(Driver());
            cur_driver=&Drivers_list.back();

            driver_read(cur_driver,this,cur_device,hkey,&inf_list,&unpacked_drp);
        }
        i++;
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
    collection.collection_free();
    hash_free(&inf_list);
    SetupDiDestroyDeviceInfoList(hDevInfo);
    time_devicescan=GetTickCount()-time_devicescan;
}
//}

//{ laptop/desktop detection
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

    buf=(WCHAR *)(text+monitors);
    batteryloc=(SYSTEM_POWER_STATUS *)(text+battery);

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
        char *s=text;

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
