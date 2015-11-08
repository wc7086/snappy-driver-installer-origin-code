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

#ifndef MATCHER_H
#define MATCHER_H

#define NUM_DECS 12*4
#define NUM_MARKERS 39
#define NUM_FILTERS 22

enum DRIVER_STATUS
{
    STATUS_BETTER      = 0x001,
    STATUS_SAME        = 0x002,
    STATUS_WORSE       = 0x004,
    STATUS_INVALID     = 0x008,

    STATUS_MISSING     = 0x010,
    STATUS_NEW         = 0x020,
    STATUS_CURRENT     = 0x040,
    STATUS_OLD         = 0x080,

    STATUS_NF_MISSING  = 0x100,
    STATUS_NF_UNKNOWN  = 0x200,
    STATUS_NF_STANDARD = 0x400,
    STATUS_DUP         = 0x800,
};

class Devicematch;
class State;
class Collection;
class Hwidmatch;
class Canvas;
class Device;
class Driver;
class Driverpack;
class Version;

struct markers_t
{
    const char *name;
    int major,minor,arch;
};
extern const char *nts[NUM_DECS];
extern const markers_t markers[NUM_MARKERS];

// Calc
int calc_identifierscore(int dev_pos,int dev_ishw,int inf_pos);
unsigned calc_score(int catalogfile,int feature,int rank,State *state,int isnt);
int calc_secttype(const char *s);
int calc_signature(int catalogfile,State *state,int isnt);

// Misc
int cmpunsigned(unsigned a,unsigned b);

// Matcher is used as a storange for devicematch_list and hwidmatch_list
class Matcher
{
public:
    virtual void findHWIDs(Devicematch *device_match,wchar_t *hwid,int dev_pos,int ishw)=0;
    virtual void init(State *state1,Collection *col1)=0;
    virtual void populate()=0;
    virtual void print()=0;
    virtual void sorta(int *v)=0;

    virtual wchar_t *finddrp(const wchar_t *s)=0;
    virtual State *getState()=0;
    virtual Collection *getCol()=0;
    virtual size_t getDwidmatch_list()=0;
    virtual Devicematch *getDevicematch_i(int i)=0;
	virtual size_t getHwidmatch_list() = 0;
    virtual void Insert(const Hwidmatch &a)=0;
    virtual Hwidmatch *getHwidmatch_i(int i)=0;

    virtual ~Matcher()=0;
};

Matcher *CreateMatcher();

// Devicematch holds info about device and a list of alternative drivers
class Devicematch
{
    int start_matches;
    unsigned num_matches;
    int status;

public:
    Device *device;
    Driver *driver;

public:
    Devicematch(Device *cur_device,Driver *cur_driver,int items,Matcher *matcher);
    int isMissing(State *state);
    int getStatus(){return status;}

    friend class Manager; // TODO: friend
    friend class MatcherImp; // TODO: friend
};

// Hwidmatch is used to extract info about an available driver from indexes
class Hwidmatch
{
    Driverpack *drp;
    int HWID_index;

    Devicematch *devicematch;

    int identifierscore,decorscore,markerscore,altsectscore,status;
    unsigned score;

private:
    int isvalid_usb30hub(State *state,const wchar_t *str);
    int isblacklisted(State *state,const wchar_t *hwid,const char *section);
    int isvalid_ver(State *state);
    int calc_altsectscore(State *state,int curscore);
    int calc_status(State *state);
    int calc_decorscore(int id,State *state);
    int calc_markerscore(State *state,const char *path);

public:
    void setHWID_index(int index){HWID_index=index;}
    int getHWID_index(){return HWID_index;}
    void setStatus(int status1){status=status1;}
    int getStatus(){return status;}
    unsigned getScore(){return score;}
    int getDecorscore(){return decorscore;}
    int getAltsectscore(){return altsectscore;}
    void setAltsectscore(int val){altsectscore=val;}
    void markDup(){status|=STATUS_DUP;}

    Hwidmatch(Driverpack *drp,int HWID_index,int dev_pos,int ishw,State *state,Devicematch *devicematch);
    Hwidmatch(Driverpack *drp1,int HWID_index1);

    int calc_catalogfile();
    int calc_notebook();

    void minlen(char *s,int *len);
    void calclen(int *limits);
    void print_tbl(int *limits);
    void print_hr();
    void popup_driverline(int *limits,Canvas &canvas,int ln,int mode,int index);
    int  cmp(Hwidmatch *match2);
    int isdup(Hwidmatch *match2,char *sect1);
    int isdrivervalid();
    int isvalidcat(State *state);
    int pickcat(State *state);

// <<< GETTERS
    //driverpack
    wchar_t *getdrp_packpath();
    wchar_t *getdrp_packname();
    int   getdrp_packontorrent();
    //inffile
    char *getdrp_infpath();
    char *getdrp_infname();
    const char *getdrp_drvfield(int n);
    const char *getdrp_drvcat(int n);
    Version *getdrp_drvversion();
    int   getdrp_infsize();
    int   getdrp_infcrc();
    //manufacturer
    char *getdrp_drvmanufacturer();
    void  getdrp_drvsection(char *buf);
    //desc
    char *getdrp_drvdesc();
    char *getdrp_drvinstall();
    char *getdrp_drvinstallPicked();
    int   getdrp_drvfeature();
    //HWID
    int   getdrp_drvinfpos();
    char *getdrp_drvHWID();
// <<< GETTERS
};

#endif
