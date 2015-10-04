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
class Collection;
class Driverpack;
class data_manufacturer_t;
class data_inffile_t;
class data_desc_t;
class data_HWID_t;
class driverpack_task;
class Hashtable;
class Txt;

template<typename Data>
class concurrent_queue;

#include <queue>
#include <unordered_map>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma GCC diagnostic ignored "-Weffc++"
#define BOOST_SYSTEM_NO_DEPRECATED
#include <boost/thread/condition_variable.hpp>
#pragma GCC diagnostic pop

typedef unsigned ofst;
typedef concurrent_queue<driverpack_task> drplist_t;

// CatalogFile
struct tbl_t
{
    const char *s;
    int sz;
};
enum
{
    ClassGuid_,
    Class,
    Provider,
    CatalogFile,
    CatalogFile_nt,
    CatalogFile_ntx86,
    CatalogFile_ntia64,
    CatalogFile_ntamd64,
    DriverVer,
    DriverPackageDisplayName,
    DriverPackageType,
    NUM_VER_NAMES
};

// Driverpack type
enum DRIVERPACK_TYPE
{
    DRIVERPACK_TYPE_PENDING_SAVE   = 0,
    DRIVERPACK_TYPE_INDEXED        = 1,
    DRIVERPACK_TYPE_UPDATE         = 2,
    DRIVERPACK_TYPE_EMPTY          = 3,
};

// Misc functions
void findosattr(char *bufa,char *adr,int len);
void *mySzAlloc(void *p,size_t size);
void mySzFree(void *p,void *address);

// Driverpack_task
class driverpack_task
{
    Driverpack *drp;
    driverpack_task(Driverpack *a):drp(a){}
    driverpack_task():drp(nullptr){}

    friend class Driverpack;
    friend class Collection;
};

// Inffile_task
class inffile_task
{
    Driverpack *drp;
    wchar_t *pathinf;
    wchar_t *inffile;
    char *adr;
    int len;

    friend class Driverpack;
};

// Sect_data_t
class sect_data_t
{
    char *blockbeg,*blockend;

public:
    sect_data_t(char *bb,char *be):blockbeg(bb),blockend(be){}

    friend class Parser;
    friend class Driverpack;
};

// Parser
class Parser
{
    Driverpack *pack;
    std::unordered_map<std::string,std::string> *string_list;
    const wchar_t *inffile;
    Txt textholder;

    char *blockBeg;
    char *blockEnd;
    char *strBeg;
    char *strEnd;

private:
    void parseWhitespace(bool eatnewline);
    void trimtoken();
    void subStr();

public:
    int  parseItem();
    int  parseField();

    int  readNumber();
    int  readHex();
    int  readDate(Version *t);
    void readVersion(Version *t);
    void readStr(char **vb,char **ve);

    Parser(const Parser&)=delete;
    Parser &operator=(const Parser&)=delete;
    Parser(Driverpack *drp,std::unordered_map<std::string,std::string> &string_listv,const wchar_t *inf);
    Parser(char *vb,char *ve);

    void setRange(sect_data_t *lnk);
};

// Indexes
class data_inffile_t // 132
{
    ofst infpath;
    ofst inffilename;
    ofst fields[NUM_VER_NAMES];
    ofst cats[NUM_VER_NAMES];
    Version version;
    int infsize;
    int infcrc;

    friend class Driverpack;
    friend class Hwidmatch;
};

class data_manufacturer_t // 16
{
    unsigned inffile_index;

    ofst manufacturer;
    ofst sections;
    int sections_n;

    friend class Driverpack;
    friend class Hwidmatch;
};

class data_desc_t // 24
{
    unsigned manufacturer_index;
    int sect_pos;

    ofst desc;
    ofst install;
    ofst install_picked;
    unsigned int feature;

public:
    data_desc_t():manufacturer_index(0),sect_pos(0),desc(0),install(0),install_picked(0),feature(0){}
    data_desc_t(unsigned manufacturer_indexv,int sect_posv,ofst descv,ofst installv,ofst install_pickedv,unsigned int featurev):
        manufacturer_index(manufacturer_indexv),sect_pos(sect_posv),desc(descv),install(installv),install_picked(install_pickedv),feature(featurev){}

    friend class Driverpack;
    friend class Hwidmatch;
};

class data_HWID_t // 12
{
    unsigned desc_index;
    int inf_pos;

    ofst HWID;

public:
    ofst getHWID(){return HWID;}
    data_HWID_t():desc_index(0),inf_pos(0),HWID(0){}
    data_HWID_t(unsigned desc_indexv,int inf_posv,ofst HWIDv):desc_index(desc_indexv),inf_pos(inf_posv),HWID(HWIDv){}

    friend class Driverpack;
    friend class Hwidmatch;
};

// Collection
class Collection
{
    const wchar_t *index_bin_dir;
    const wchar_t *index_linear_dir;
    std::vector<Driverpack> driverpack_list;
    wchar_t *driverpack_dir;

private:
    int  scanfolder_count(const wchar_t *path);
    void scanfolder(const wchar_t *path,void *arg);
    void loadOnlineIndexes();

public:
    wchar_t *getDriverpack_dir()const{return driverpack_dir;}
    const wchar_t *getIndex_bin_dir()const{return index_bin_dir;}
    const wchar_t *getIndex_linear_dir()const{return index_linear_dir;}
    int size(){return driverpack_list.size();}
    std::vector<Driverpack> *getList(){return &driverpack_list;}

    void init(wchar_t *driverpacks_dir,const wchar_t *index_bin_dir,const wchar_t *index_linear_dir);
    Collection(wchar_t *driverpacks_dir,const wchar_t *index_bin_dir,const wchar_t *index_linear_dir);
    Collection():index_bin_dir(nullptr),index_linear_dir(nullptr),driverpack_dir(nullptr){}

    void updatedir();
    void populate();
    void save();
    void printstats();
    void print_index_hr();

    wchar_t *finddrp(wchar_t *s);
};

// Driverpack
class Driverpack
{
    ofst drppath;
    ofst drpfilename;

    int type;

    Collection *col;

    Hashtable indexes;
    std::unordered_map<std::string,ofst> cat_list;

    loadable_vector<data_inffile_t> inffile;
    loadable_vector<data_manufacturer_t> manufacturer_list;
    loadable_vector<data_desc_t> desc_list;
    loadable_vector<data_HWID_t> HWID_list;
    Txt text_ind;
    concurrent_queue<inffile_task> *objs_new;

private:
    int  genindex();
    void driverpack_parsecat_async(wchar_t const *pathinf,wchar_t const *inffile,char *adr,int len);
    void driverpack_indexinf_async(wchar_t const *pathinf,wchar_t const *inffile,char *adr,int len);
    void indexinf_ansi(wchar_t const *drpdir,wchar_t const *inffile,char *inf_base,int inf_len);
    void getdrp_drvsectionAtPos(char *buf,int pos,int manuf_index);

    static unsigned int __stdcall loaddrp_thread(void *arg);
    static unsigned int __stdcall indexinf_thread(void *arg);
    static unsigned int __stdcall savedrp_thread(void *arg);

public:
    wchar_t *getPath(){return text_ind.getw2(drppath);}
    wchar_t *getFilename(){return text_ind.getw2(drpfilename);}
    int getType(){return type;}
    int getSize(){return HWID_list.size();}
    int setType(int val){return type=val;}
    int find(int key,int *isFound){return indexes.find(key,isFound);}
    int findnext(int *isFound){return indexes.findnext(isFound);}

    Driverpack(const Driverpack&)=default;
    Driverpack &operator=(const Driverpack&)=default;
    Driverpack(wchar_t const *driverpack_path,wchar_t const *driverpack_filename,Collection *col);

    int  checkindex();
    int  loadindex();
    void saveindex();
    void genhashes();
    int  printstats();
    void print_index_hr();

    void fillinfo(char *sect,char *hwid,unsigned start_index,int *inf_pos,ofst *cat,int *catalogfile,int *feature);
    void getindexfilename(const wchar_t *dir,const wchar_t *ext,wchar_t *indfile);
    void parsecat(wchar_t const *pathinf,wchar_t const *inffile,char *adr,int len);
    void indexinf(wchar_t const *drpdir,wchar_t const *inffile,char *inf_base,int inf_len);

    friend class Hwidmatch;
    friend class Collection;
};
