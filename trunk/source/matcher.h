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

#define NUM_DECS 12*4
#define NUM_MARKERS 31

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

extern const char *nts[NUM_DECS];

typedef struct _markers_t
{
    const char *name;
    int major,minor,arch;
}markers_t;

class devicematch_t
{
public:
    Device *device;
    Driver *driver;
    int start_matches;
    unsigned num_matches;
    int status;
};

class Hwidmatch
{
public:
    Driverpack *drp;
    int HWID_index;

    devicematch_t *devicematch;
    int identifierscore,decorscore,markerscore,altsectscore,status;
    unsigned score;

public:
    //driverpack
    WCHAR *getdrp_packpath();
    WCHAR *getdrp_packname();
    int   getdrp_packontorrent();
    //inffile
    char *getdrp_infpath();
    char *getdrp_infname();
    const char *getdrp_drvfield(int n);
    const char *getdrp_drvcat(int n);
    version_t *getdrp_drvversion();
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
    short getdrp_drvinfpos();
    char *getdrp_drvHWID();

    int isvalid_usb30hub(State *state,const WCHAR *str);
    int isblacklisted(State *state,const WCHAR *hwid,const char *section);
    int isvalid_ver(State *state);
    int calc_notebook();
    int calc_altsectscore(State *state,int curscore);
    int calc_status(State *state);
    int calc_catalogfile();

    void init(Driverpack *drp,int HWID_index,int dev_pos,int ishw,State *state,devicematch_t *devicematch);
    void initbriefly(Driverpack *drp,int HWID_index);
    void calclen(int *limits);
    void print_tbl(int *limits);
    void print_hr();
    int  cmp(Hwidmatch *match2);
};

class Matcher
{
public:
    State *state;
    Collection *col;

    std::vector<devicematch_t> devicematch_list;
    std::vector<Hwidmatch> hwidmatch_list;

public:
    void init(State *state,Collection *col);
    void release();
    void findHWIDs(devicematch_t *device_match,char *hwid,int dev_pos,int ishw);
    void populate();
    void sort();
    void print();
};

// Calc
void genmarker(State *state);
int isMissing(Device *device,Driver *driver,State *state);
int calc_identifierscore(int dev_pos,int dev_ishw,int inf_pos);
int calc_signature(int catalogfile,State *state,int isnt);
unsigned calc_score(int catalogfile,int feature,int rank,State *state,int isnt);
unsigned calc_score_h(Driver *driver,State *state);
int calc_secttype(const char *s);
int calc_decorscore(int id,State *state);
int calc_markerscore(State *state,char *path);

// Misc
void findHWID_in_list(char *s,int list,int str,int *dev_pos);
void getdd(Device *cur_device,Driver *cur_driver,State *state,int *ishw,int *dev_pos);
int  cmpunsigned(unsigned a,unsigned b);
int  cmpdate(version_t *t1,version_t *t2);
int  cmpversion(version_t *t1,version_t *t2);
void devicematch_init(devicematch_t *devicematch,Device *cur_device,Driver *driver,int items);

// hwidmatch
void minlen(CHAR *s,int *len);

// Matcher
void  getdrp_drvsectionAtPos(Driverpack *drp,char *buf,int pos,int manuf_index);

