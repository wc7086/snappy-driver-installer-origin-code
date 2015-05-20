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

#define STR_LN 4096
typedef int ofst;

// Misc
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

enum DRIVERPACK_TYPE
{
    DRIVERPACK_TYPE_PENDING_SAVE   = 0,
    DRIVERPACK_TYPE_INDEXED        = 1,
    DRIVERPACK_TYPE_UPDATE         = 2,
    DRIVERPACK_TYPE_EMPTY          = 3,
};

class obj
{
    Driverpack *drp;
    wchar_t pathinf[BUFLEN];
    wchar_t inffile[BUFLEN];
    char *adr;
    int len;

    friend class Driverpack;
    friend void driverpack_indexinf_async(Driverpack *drp,wchar_t const *pathinf,wchar_t const *inffile,char *adr,int len);
    friend void driverpack_parsecat_async(Driverpack *drp,wchar_t const *pathinf,wchar_t const *inffile,char *adr,int len);
};

class sect_data_t
{
    char *blockbeg,*blockend;

public:
    sect_data_t(char *bb,char *be):blockbeg(bb),blockend(be){}

    friend class Parser;
    friend class Driverpack;
};

// Version
class version_t
{
    int d,m,y;
    int v1,v2,v3,v4;

public:
    void str_date(wchar_t *buf);
    void str_version(wchar_t *buf);
    int setDate(int d_,int m_,int y_);
    void setVersion(int v1_,int v2_,int v3_,int v4_);
    void setInvalid(){y=v1=-1;}

    version_t():d(0),m(0),y(0),v1(-2),v2(0),v3(0),v4(0){}
    version_t(int d1,int m1,int y1):d(d1),m(m1),y(y1),v1(-2),v2(0),v3(0),v4(0){}

    friend class Driverpack;
    friend class Hwidmatch;
    friend int cmpdate(version_t *t1,version_t *t2);
    friend int cmpversion(version_t *t1,version_t *t2);
};
int cmpdate(version_t *t1,version_t *t2);
int cmpversion(version_t *t1,version_t *t2);

// Indexes
class data_inffile_t // 132
{
    ofst infpath;
    ofst inffilename;
    ofst fields[NUM_VER_NAMES];
    ofst cats[NUM_VER_NAMES];
    version_t version;
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
    //data_desc_t(){}
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
    //data_HWID_t(){}
    data_HWID_t():desc_index(0),inf_pos(0),HWID(0){}
    data_HWID_t(unsigned desc_indexv,int inf_posv,ofst HWIDv):desc_index(desc_indexv),inf_pos(inf_posv),HWID(HWIDv){}

    friend class Driverpack;
    friend class Hwidmatch;
    friend class Driver; // TODO: friend
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
    int  readDate(version_t *t);
    void readVersion(version_t *t);
    void readStr(char **vb,char **ve);

    Parser(const Parser&)=delete;
    Parser &operator=(const Parser&)=delete;
    Parser(sect_data_t *lnk,Driverpack *drp,std::unordered_map<std::string,std::string> &string_listv,const wchar_t *inf);
    Parser(char *vb,char *ve);
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
    static unsigned int __stdcall loaddrp_thread(void *arg);
    static unsigned int __stdcall savedrp_thread(void *arg);

public:
    wchar_t *getDriverpack_dir()const{return driverpack_dir;}
    const wchar_t *getIndex_bin_dir()const{return index_bin_dir;}
    const wchar_t *getIndex_linear_dir()const{return index_linear_dir;}
    int size(){return driverpack_list.size();}
    void updatedir();
    std::vector<Driverpack> *getList(){return &driverpack_list;}

    void init(wchar_t *driverpacks_dir,const wchar_t *index_bin_dir,const wchar_t *index_linear_dir);
    Collection(wchar_t *driverpacks_dir,const wchar_t *index_bin_dir,const wchar_t *index_linear_dir);
    Collection():index_bin_dir(nullptr),index_linear_dir(nullptr),driverpack_dir(nullptr){}
    void save();
    void populate();
    void print_index_hr();
    void printstates();
    wchar_t *finddrp(wchar_t *s);
};

// Driverpack
class Driverpack
{
    ofst drppath;
    ofst drpfilename;

    int type;

    Collection *col;

    Hashtable indexesold;
    std::unordered_map<std::string,ofst> cat_list;

    std::vector<data_inffile_t> inffile;
    std::vector<data_manufacturer_t> manufacturer_list;
    std::vector<data_desc_t> desc_list;
    std::vector<data_HWID_t> HWID_list;
    Txt texta;

#ifdef INDEXING_H
    boost::lockfree::queue<obj> *objs;
#else
    void *objs;
#endif

private:
    void indexinf_ansi(wchar_t const *drpdir,wchar_t const *inffile,char *inf_base,int inf_len);
    int  genindex();

public:
    wchar_t *getPath(){return texta.getw(drppath);}
    wchar_t *getFilename(){return texta.getw(drpfilename);}

    //Driverpack(const Driverpack&)=delete;
    Driverpack &operator=(const Driverpack&)=delete;
    Driverpack(wchar_t const *driverpack_path,wchar_t const *driverpack_filename,Collection *col);
    ~Driverpack();

    void saveindex();
    int  checkindex();
    int  loadindex();
    void getindexfilename(const wchar_t *dir,const wchar_t *ext,wchar_t *indfile);
    void print_index_hr();
    void parsecat(wchar_t const *pathinf,wchar_t const *inffile,char *adr,int len);
    void indexinf(wchar_t const *drpdir,wchar_t const *inffile,char *inf_base,int inf_len);
    void genhashes();
    static unsigned int __stdcall thread_indexinf(void *arg);
    void getdrp_drvsectionAtPos(char *buf,int pos,int manuf_index); // in matcher.h

    friend class Driver; // TODO: friend
    friend class Collection; // TODO: friend
    friend class Hwidmatch; // TODO: friend
    friend class Matcher; // TODO: friend
    friend void driverpack_indexinf_async(Driverpack *drp,wchar_t const *pathinf,wchar_t const *inffile,char *adr,int len);
    friend void driverpack_parsecat_async(Driverpack *drp,wchar_t const *pathinf,wchar_t const *inffile,char *adr,int len);
};
void driverpack_indexinf_async(Driverpack *drp,wchar_t const *pathinf,wchar_t const *inffile,char *adr,int len);

// Misc
void findosattr(char *bufa,char *adr,int len);
