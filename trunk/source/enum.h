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

// Declarations
class Manager;
class State;
class Device;
class infdata_t;
typedef std::unordered_map <std::wstring,infdata_t> inflist_tp;

// Misc struct
class infdata_t
{
    int catalogfile;
    int feature;
    int inf_pos;
    ofst cat;
    int start_index;

public:
    infdata_t(int vcatalogfile,int vfeature,int vinf_pos,ofst vcat,int vindex):
        catalogfile(vcatalogfile),feature(vfeature),inf_pos(vinf_pos),cat(vcat),start_index(vindex){};

    friend class Driver;
};

struct SP_DEVINFO_DATA_32
{
    DWORD     cbSize;
    GUID      ClassGuid;
    DWORD     DevInst;
    int       Reserved;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
//warning: 'class Driverpack' has pointer data members
    SP_DEVINFO_DATA_32():cbSize(sizeof(SP_DEVINFO_DATA_32)),DevInst(0),Reserved(0){}
#pragma GCC diagnostic pop
};

// Device
class Device
{
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
    int  getDriverIndex()const{return driver_index;}
    ofst getHardwareID()const{return HardwareID;}
    ofst getCompatibleIDs()const{return CompatibleIDs;}
    ofst getDriver()const{return Driver;}
    ofst getDescr()const{return Devicedesc;}
    ofst getRet()const{return ret;}
    ofst getProblem()const{return problem;}

    int  print_status();
    void print(State *state);
    void printHWIDS(State *state);
    const wchar_t *getHWIDby(int num,State *state);

    //Device(const Device &)=delete;
    //Device &operator=(const Device &)=delete;
    //Device(Device &&)=default;
    Device(HDEVINFO hDevInfo,State *state,int i);
    Device():driver_index(-1),Devicedesc(0),HardwareID(0),CompatibleIDs(0),Driver(0),
        Mfg(0),FriendlyName(0),Capabilities(0),ConfigFlags(0),
        InstanceId(0),status(0),problem(0),ret(0),DeviceInfoData(){}

    friend class Manager; // TODO: friend
    friend class itembar_t; // TODO: friend
};

// Driver
class Driver
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

private:
    void read_reg_val(HKEY hkey,State *state,const wchar_t *key,ofst *val);
    void scaninf(State *state,Driverpack *unpacked_drp,int &inf_pos);
    int findHWID_in_list(const wchar_t *p,const wchar_t *str);
    void calc_dev_pos(Device *cur_device,State *state,int *ishw,int *dev_pos);

public:
    ofst getInfPath()const{return InfPath;}
    ofst getMatchingDeviceId(){return MatchingDeviceId;}
    version_t *getVersion(){return &version;}

    unsigned calc_score_h(State *state);
    void print(State *state);


    //Driver(const Driver &)=delete;
    //Driver &operator=(const Driver &)=delete;
    //Driver(Driver &&)=default;
    Driver(State *state,Device *cur_device,HKEY hkey,Driverpack *unpacked_drp);
    Driver():DriverDesc(0),ProviderName(0),DriverDate(0),DriverVersion(0),MatchingDeviceId(0),
        InfPath(0),InfSection(0),InfSectionExt(0),cat(0),version(),catalogfile(0),feature(0),identifierscore(0){}

    friend class Manager; // TODO: friend
    friend class itembar_t; // TODO: friend
};

// State (POD)
class state_m_t
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
};

// State
class State
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

    // --- End of POD ---

    std::vector<Device> Devices_list;
    std::vector<Driver> Drivers_list;

public:
    Txt textas;
    inflist_tp inf_list_new;
    int isLaptop;

private:
    void fakeOSversion();

public:
    ofst getWindir(){return windir;}
    ofst getTemp(){return temp;}
    int getLocale(){return locale;}
    int getArchitecture(){return architecture;}
    void getWinVer(int *major,int *minor);
    wchar_t *get_szCSDVersion(){return platform.szCSDVersion;}
    std::vector<Device> *getDevices_list(){return &Devices_list;}
    Driver *getCurrentDriver(Device *dev){return (dev->getDriverIndex()>=0)?&Drivers_list[dev->getDriverIndex()]:nullptr;}

    wchar_t *getProduct();
    wchar_t *getManuf();
    wchar_t *getModel();

    State();
    void print();
    void popup_sysinfo(HDC hdcMem);
    void contextmenu2(int x,int y);

    void save(const wchar_t *filename);
    int  load(const wchar_t *filename);
    void getsysinfo_fast();
    void getsysinfo_slow();
    void getsysinfo_slow(State *prev);
    void scanDevices();
    void init();

    const wchar_t *get_winverstr();
    int  opencatfile(Driver *cur_driver);
    void genmarker(); // in matcher.cpp
    void isnotebook_a();
};

// Monitor info
int GetMonitorDevice(wchar_t* adapterName,DISPLAY_DEVICE *ddMon);
int GetMonitorSizeFromEDID(wchar_t* adapterName,int *Width,int *Height);
int iswide(int x,int y);

//  Misc (in baseboard.cpp)
int getbaseboard(wchar_t *manuf,wchar_t *model,wchar_t *product,wchar_t *cs_manuf,wchar_t *cs_model,int *type);
void ShowProgressInTaskbar(HWND hwnd,TBPFLAG flags,int complited,int total);
