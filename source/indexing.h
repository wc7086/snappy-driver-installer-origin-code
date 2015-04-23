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
#define STR_LN 4096
typedef int ofst;
//#define MERGE_FINDER

class Collection;
class Driverpack;
class data_manufacturer_t;
class data_inffile_t;
class data_desc_t;
class data_HWID_t;

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

//{ Misc
#define LSTCNT 1000
typedef struct _inflist_t
{
    Driverpack *drp;
    wchar_t pathinf[BUFLEN];
    wchar_t inffile[BUFLEN];
    char *adr;
    int len;

    HANDLE dataready;
    HANDLE slotvacant;
    int type;
}inflist_t;

class sect_data_t
{
public:
    char *blockbeg,*blockend;
    sect_data_t(char *bb,char *be):blockbeg(bb),blockend(be){}
};

typedef struct _tbl_t
{
    const char *s;
    int sz;
}tbl_t;

struct version_t
{
    int d,m,y;
    int v1,v2,v3,v4;

    version_t():d(0),m(0),y(0),v1(-2),v2(0),v3(0),v4(0){}
};
//}

enum DRIVERPACK_TYPE
{
    DRIVERPACK_TYPE_PENDING_SAVE   = 0,
    DRIVERPACK_TYPE_INDEXED        = 1,
    DRIVERPACK_TYPE_UPDATE         = 2,
    DRIVERPACK_TYPE_EMPTY          = 3,
};

//{ Indexing strucures
class Driverpack
{
private:
    ofst drppath;
    ofst drpfilename;
public:
    int type;

    Collection *col;

    hashtable_t indexesold;

    std::unordered_map<std::string,std::string> string_list;
    std::unordered_map<std::string,ofst> cat_list;

    std::vector<data_inffile_t> inffile;
    std::vector<data_manufacturer_t> manufacturer_list;
    std::vector<data_desc_t> desc_list;
    std::vector<data_HWID_t> HWID_list;
    Txt texta;

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
    void print();
    void genhashes();
    void parsecat(wchar_t const *pathinf,wchar_t const *inffile,char *adr,int len);
    int  genindex();
    void indexinf_ansi(wchar_t const *drpdir,wchar_t const *inffile,char *inf_base,int inf_len);
    void indexinf(wchar_t const *drpdir,wchar_t const *inffile,char *inf_base,int inf_len);
};

class data_inffile_t // 132
{
public:
    ofst infpath;
    ofst inffilename;
    ofst fields[NUM_VER_NAMES];
    ofst cats[NUM_VER_NAMES];
    version_t version;
    int infsize;
    int infcrc;
};

class data_manufacturer_t // 16
{
public:
    unsigned inffile_index;

    ofst manufacturer;
    ofst sections;
    int sections_n;
};

class data_desc_t // 24
{
public:
    unsigned manufacturer_index;
    int sect_pos;

    ofst desc;
    ofst install;
    ofst install_picked;
    unsigned int feature;
};

class data_HWID_t // 12
{
public:
    unsigned desc_index;
    int inf_pos;

    ofst HWID;
};

class Collection
{
public:
    wchar_t *driverpack_dir;
    const wchar_t *index_bin_dir;
    const wchar_t *index_linear_dir;
    //int flags;

    std::vector<Driverpack> driverpack_list;

    inflist_t *inflist;
    int pos_in,pos_out;

public:
    friend unsigned int __stdcall thread_indexinf(void *arg);
    friend void driverpack_indexinf_async(Driverpack *drp,Collection *colv,wchar_t const *pathinf,wchar_t const *inffile,char *adr,int len);

    wchar_t *getDriverpack_dir()const{return driverpack_dir;}
    const wchar_t *getIndex_bin_dir()const{return index_bin_dir;}
    const wchar_t *getIndex_linear_dir()const{return index_linear_dir;}

    void init(wchar_t *driverpacks_dir,const wchar_t *index_bin_dir,const wchar_t *index_linear_dir);
    void release();
    void save();
    void updatedindexes();
    void load();
    void print();
    wchar_t *finddrp(wchar_t *s);
    void printstates();
    void scanfolder(const wchar_t *path);
    int  scanfolder_count(const wchar_t *path);
};

// Parser
class Parser
{
private:
    Driverpack *pack;
    const wchar_t *inffile;
    Txt textholder;

    char *blockBeg;
    char *blockEnd;
    char *strBeg;
    char *strEnd;

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
    Parser(sect_data_t *lnk,Driverpack *drp,const wchar_t *inf);
    Parser(char *vb,char *ve);
};

// Misc
void driverpack_indexinf_async(Driverpack *drp,Collection *colv,wchar_t const *pathinf,wchar_t const *inffile,char *adr,int len);
unsigned int __stdcall thread_indexinf(void *arg);
void findosattr(char *bufa,char *adr,int len);
