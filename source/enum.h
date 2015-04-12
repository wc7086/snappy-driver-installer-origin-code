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

// Global variables
extern int isLaptop;

// Misc struct
class State;
class Device;

class infdata_t
{
public:
    int catalogfile;
    int feature;
    int inf_pos;
    ofst cat;

public:
    infdata_t(int vcatalogfile,int vfeature,int vinf_pos,ofst vcat):
        catalogfile(vcatalogfile),feature(vfeature),inf_pos(vinf_pos),cat(vcat){};
};
typedef std::unordered_map <std::string,infdata_t> inflist_tp;

typedef struct _SP_DEVINFO_DATA32
{
    DWORD     cbSize;
    GUID      ClassGuid;
    DWORD     DevInst;
    int       Reserved;
} SP_DEVINFO_DATA_32, *PSP_DEVINFO_DATA_32;

// Device
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

private:
    void print_guid(GUID *g);
    void read_device_property(HDEVINFO hDevInfo,State *state,int id,ofst *val);

public:
    void setDriverIndex(int v){driver_index=v;}
    int  getDriverIndex(){return driver_index;}
    ofst getHardwareID(){return HardwareID;}
    ofst getDriver(){return Driver;}
    ofst getDescr(){return Devicedesc;}

    int  print_status();
    void print(State *state);
    void printHWIDS(State *state);

    Device(HDEVINFO hDevInfo,State *state,int i);
    Device():driver_index(-1),Devicedesc(0),HardwareID(0),CompatibleIDs(0),Driver(0),
        Mfg(0),FriendlyName(0),Capabilities(0),ConfigFlags(0),
        InstanceId(0),status(0),problem(0),ret(0){}
};

// Driver
class Driver
{
public:
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

private:
    void read_reg_val(HKEY hkey,State *state,const WCHAR *key,ofst *val);
    void scaninf(State *state,Driverpack *unpacked_drp,int &inf_pos);

public:
    void print(State *state);

    Driver(State *state,Device *cur_device,HKEY hkey,Driverpack *unpacked_drp);
    Driver():DriverDesc(0),ProviderName(0),DriverDate(0),DriverVersion(0),MatchingDeviceId(0),
        InfPath(0),InfSection(0),InfSectionExt(0),cat(0),catalogfile(0),feature(0),identifierscore(0){}
};

// State (POD)
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
}state_m_t;

// State
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

    // --- End of POD ---

    std::vector<Device> Devices_list;
    std::vector<Driver> Drivers_list;
    Txt textas;
    inflist_tp inf_list_new;

private:
    void fakeOSversion();

public:
    WCHAR *getProduct();
    WCHAR *getManuf();
    WCHAR *getModel();

    void init();
    void release();
    ~State();
    void print();

    void save(const WCHAR *filename);
    int  load(const WCHAR *filename);
    void getsysinfo_fast();
    void getsysinfo_slow();
    void scanDevices();

    int  opencatfile(Driver *cur_driver);
    void genmarker(); // in matcher.cpp
    void isnotebook_a();
};

// Monitor info
int GetMonitorDevice(WCHAR* adapterName,DISPLAY_DEVICE *ddMon);
int GetMonitorSizeFromEDID(WCHAR* adapterName,int *Width,int *Height);
int iswide(int x,int y);


//  Misc (in baseboard.cpp)
int getbaseboard(WCHAR *manuf,WCHAR *model,WCHAR *product,WCHAR *cs_manuf,WCHAR *cs_model,int *type);
void ShowProgressInTaskbar(HWND hwnd,TBPFLAG flags,int complited,int total);

// Vector templates
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
    memcpy(v->data(),p,sz);p+=sz;
    return p;
}

