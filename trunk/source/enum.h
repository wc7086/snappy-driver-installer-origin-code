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

//{ Global variables
extern int isLaptop;
//}

//{ enum structures

#ifndef _WIN64
#define DISPLAY_DEVICE_ACTIVE         1
#define DISPLAY_DEVICE_ATTACHED       2
#endif

class State;

typedef struct _infdata_t
{
    int catalogfile;
    int feature;
    int inf_pos;
    ofst cat;
}infdata_t;

typedef struct _SP_DEVINFO_DATA32
{
    DWORD     cbSize;
    GUID      ClassGuid;
    DWORD     DevInst;
    int       Reserved;
} SP_DEVINFO_DATA_32, *PSP_DEVINFO_DATA_32;

//typedef struct _driver_t driver_t;
typedef struct _driver_t
{
    ofst DriverDesc;
    ofst ProviderName;
    ofst DriverDate;
    ofst DriverVersion;
    ofst MatchingDeviceId;
    ofst InfPath;
    ofst InfSection;
    ofst InfSectionExt;
    ofst cat;
    version_t version;

    int catalogfile;
    int feature;
    int identifierscore;

    //SP_DRVINSTALL_PARAMS drvparams;
    //SP_DRVINFO_DETAIL_DATA drvdet;
}driver_t;

typedef struct _state_m_t
{
    OSVERSIONINFOEX platform;
    int locale;
    int architecture;

    ofst manuf;
    ofst model;
    ofst product;
    ofst monitors;
    ofst battery;

    ofst windir;
    ofst temp;

    ofst cs_manuf;
    ofst cs_model;
    int ChassisType;
    int revision;
    char reserved[1024];

    char reserved1[676];
//    driverpack_t windirinf;
}state_m_t;

//}
class Device
{
public:
    int driver_index;

    ofst Devicedesc;
    ofst HardwareID;
    ofst CompatibleIDs;
    ofst Driver;
    ofst Mfg;
    ofst FriendlyName;
    int Capabilities;
    int ConfigFlags;

    ofst InstanceId;
    ULONG status,problem;
    int ret;

    SP_DEVINFO_DATA_32 DeviceInfoData;     // ClassGuid,DevInst

public:
    int print_status();
    void device_print(State *state);
    void device_printHWIDS(State *state);
};

class State
{
public:
    OSVERSIONINFOEX platform;
    int locale;
    int architecture;

    ofst manuf;
    ofst model;
    ofst product;
    ofst monitors;
    ofst battery;

    ofst windir;
    ofst temp;

    ofst cs_manuf;
    ofst cs_model;
    int ChassisType;
    int revision;
    char reserved[1024];

    char reserved1[676];
//    driverpack_t windirinf;

/*    ofst *profile_list;
    heap_t profile_handle;
    int profile_current;*/

    Device *devices_list;
    heap_t drivers_handle;

    driver_t *drivers_list;
    heap_t devices_handle;

    char *text;
    heap_t text_handle;

public:
    void init();
    void release();
    void save(const WCHAR *filename);
    int  load(const WCHAR *filename);
    void fakeOSversion();
    void log_add(CHAR const *format,...);
    void print();
    WCHAR *getProduct();
    WCHAR *getManuf();
    WCHAR *getModel();
    void getsysinfo_fast();
    void getsysinfo_slow();
//    void state_scandevices();
    int opencatfile(driver_t *cur_driver);
    void genmarker();
};
extern const char *deviceststus_str[];


void driver_print(driver_t *cur_driver,State *state);
void state_scandevices(State *state);

void print_guid(GUID *g);
void print_appinfo();
void read_device_property(HDEVINFO hDevInfo,SP_DEVINFO_DATA *DeviceInfoData,State *state,int id,ofst *val);
void read_reg_val(HKEY hkey,State *state,const WCHAR *key,ofst *val);
int GetMonitorDevice(WCHAR* adapterName,DISPLAY_DEVICE *ddMon);
int GetMonitorSizeFromEDID(WCHAR* adapterName,int *Width,int *Height);
int iswide(int x,int y);
void isnotebook_a(State *state);

int getbaseboard(WCHAR *manuf,WCHAR *model,WCHAR *product,WCHAR *cs_manuf,WCHAR *cs_model,int *type);
void ShowProgressInTaskbar(HWND hwnd,TBPFLAG flags,int complited,int total);
