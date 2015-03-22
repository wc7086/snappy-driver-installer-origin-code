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
#include <vector>
#define STR_LN 4096
typedef int ofst;
//#define MERGE_FINDER

enum FLAG
{
    COLLECTION_FORCE_REINDEXING = 0x00000001,
    COLLECTION_USE_LZMA         = 0x00000002,
    COLLECTION_PRINT_INDEX      = 0x00000004,
    FLAG_NOGUI                  = 0x00000010,
    FLAG_CHECKUPDATES           = 0x00000020,
    FLAG_DISABLEINSTALL         = 0x00000040,
    FLAG_AUTOINSTALL            = 0x00000080,
    FLAG_FAILSAFE               = 0x00000100,
    FLAG_AUTOCLOSE              = 0x00000200,
    FLAG_NORESTOREPOINT         = 0x00000400,
    FLAG_NOLOGFILE              = 0x00000800,
    FLAG_NOSNAPSHOT             = 0x00001000,
    FLAG_NOSTAMP                = 0x00002000,
    // FLAG_NOFEATURESCORE       =   0x00004000
    FLAG_PRESERVECFG            = 0x00008000,
    FLAG_EXTRACTONLY            = 0x00010000,
    FLAG_KEEPUNPACKINDEX        = 0x00020000,
    FLAG_KEEPTEMPFILES          = 0x00040000,
    FLAG_SHOWDRPNAMES1          = 0x00080000,
    FLAG_DPINSTMODE             = 0x00100000,
    FLAG_SHOWCONSOLE            = 0x00200000,
    FLAG_DELEXTRAINFS           = 0x00400000,
    FLAG_SHOWDRPNAMES2          = 0x00800000,
    FLAG_ONLYUPDATES            = 0x01000000,
    FLAG_AUTOUPDATE             = 0x02000000,
    FLAG_FILTERSP               = 0x04000000,
    FLAG_OLDSTYLE               = 0x08000000,
};

class Collection;
class Driverpack;
class data_manufacturer_t;
class data_inffile_t;
class data_desc_t;
class data_HWID_t;

enum
{
    ClassGuid,
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
    WCHAR pathinf[BUFLEN];
    WCHAR inffile[BUFLEN];
    char *adr;
    int len;

    HANDLE dataready;
    HANDLE slotvacant;
    int type;
}inflist_t;

typedef struct _filedata_t
{
    int size;
    unsigned crc;
    WCHAR *str;
}filedata_t;

typedef struct _sect_data_t
{
    int ofs,len;
}sect_data_t;

typedef struct _tbl_t
{
    const char *s;
    int sz;
}tbl_t;

typedef struct _version_t
{
    int d,m,y;
    int v1,v2,v3,v4;
}version_t;
//}

enum DRIVERPACK_TYPE
{
    DRIVERPACK_TYPE_PENDING_SAVE   = 0,
    DRIVERPACK_TYPE_INDEXED        = 1,
    DRIVERPACK_TYPE_UPDATE         = 2,
    DRIVERPACK_TYPE_EMPTY          = 3,
};

//{ Indexing strucures
//#include <unordered_map>

class Txt
{
private:
//    typedef std::unordered_map<std::string,ofst> stringmap;
public:
    std::vector<char> text;
    Driverpack *drp;
//    stringmap dub;

    char *get(ofst offset);
    WCHAR *getw(ofst offset);
//    char *get(ofst offset){return &text[offset];}
//    WCHAR *getw(ofst offset){return (WCHAR *)&text[offset];}

/*    ofst addStr(char *pb,char *pe)
    {
        int r=text.size();
        text.insert(text.end(),pb,pe);

        return r;
    }
    ofst heap_memcpyz(char *pb,int len)
    {
        int r=text.size();
        text.insert(text.end(),pb,pb+len);
        text.push_back(0);
        return r;
    }
    ofst addStr(char *pb)
    {
        int r=text.size();
        text.insert(text.end(),pb,pb+strlen(pb)+1);
        return r;
    }
    ofst addStr(const WCHAR *pb)
    {
        int r=text.size();
        text.insert(text.end(),pb,pb+wcslen(pb)*2+2);
        return r;
    }*/
};

class Driverpack
{
private:
public:
    ofst drppath;
    ofst drpfilename;
    int type;

    Collection *col;

    hashtable_t section_list;
    hashtable_t string_list;
    hashtable_t indexes;
    hashtable_t cat_list;

    std::vector<data_inffile_t> inffile;
    std::vector<data_manufacturer_t> manufacturer_list;
    std::vector<data_desc_t> desc_list;
    std::vector<data_HWID_t> HWID_list;
    Txt texta;

    char *text;
    heap_t text_handle;

public:
    void init(WCHAR const *driverpack_path,WCHAR const *driverpack_filename,Collection *col);
    void release();
    void saveindex();
    int  checkindex();
    int  loadindex();
    void getindexfilename(const WCHAR *dir,const WCHAR *ext,WCHAR *indfile);
    void print();
    void genhashes();
    void parsecat(WCHAR const *pathinf,WCHAR const *inffile,char *adr,int len);
    int  genindex();
    void indexinf_ansi(WCHAR const *drpdir,WCHAR const *inffile,char *inf_base,int inf_len);
    void indexinf(WCHAR const *drpdir,WCHAR const *inffile,char *inf_base,int inf_len);
};

class data_inffile_t // 80
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

class data_desc_t // 12+1
{
public:
    unsigned manufacturer_index;
    int sect_pos;

    ofst desc;
    ofst install;
    ofst install_picked;
    char feature;
};

class data_HWID_t // 8
{
public:
    unsigned desc_index;
    short inf_pos;

    ofst HWID;
};

class Collection
{
public:
    WCHAR *driverpack_dir;
    const WCHAR *index_bin_dir;
    const WCHAR *index_linear_dir;
    int flags;

    Driverpack *driverpack_list;
    heap_t driverpack_handle;

    inflist_t *inflist;
    int pos_in,pos_out;

public:
    friend unsigned int __stdcall thread_indexinf(void *arg);
    friend void driverpack_indexinf_async(Driverpack *drp,Collection *colv,WCHAR const *pathinf,WCHAR const *inffile,char *adr,int len);

    WCHAR *getDriverpack_dir(){return driverpack_dir;}
    const WCHAR *getIndex_bin_dir(){return index_bin_dir;}
    const WCHAR *getIndex_linear_dir(){return index_linear_dir;}
    int getFlags(){return flags;}

    void init(WCHAR *driverpacks_dir,const WCHAR *index_bin_dir,const WCHAR *index_linear_dir,int flags);
    void release();
    void save();
    void updatedindexes();
    void load();
    void print();
    WCHAR *finddrp(WCHAR *s);
    void printstates();
    void scanfolder(const WCHAR *path);
    int  scanfolder_count(const WCHAR *path);
};

class Parser_str
{
private:
    Driverpack *pack;
    heap_t strings;
    char *text;

    char *blockBeg;
    char *blockEnd;
    char *strBeg;
    char *strEnd;

    void str_sub();
    void trimtoken();
    void parseWhitespace(bool eatnewline);

public:
    int  parseItem();
    int  parseField();

    int  readNumber();
    int  readHex();
    int  readDate(version_t *t);
    void readVersion(version_t *t);
    void readStr(char **vb,char **ve);

    void init(Driverpack *drp);
    void release();
    void setRange(char *inf_base,sect_data_t *lnk);
    void setRange(char *vb,char *ve){strBeg=vb;strEnd=ve;}
};

// Misc
int  unicode2ansi(char *s,char *out,int size);
void extracttest();
int  encode(char *dest,int dest_sz,char *src,int src_sz);
int  decode(char *dest,int dest_sz,char *src,int src_sz);
int  checkfolders(WCHAR *folder1,WCHAR *folder2,hashtable_t *filename2path,hashtable_t *path2filename,int sub);
void hash_clearfiles(hashtable_t *t);
WCHAR *finddrp(WCHAR *s);

void driverpack_indexinf_async(Driverpack *drp,Collection *colv,WCHAR const *pathinf,WCHAR const *inffile,char *adr,int len);
unsigned int __stdcall thread_indexinf(void *arg);
