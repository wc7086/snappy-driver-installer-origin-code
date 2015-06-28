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

//#define MERGE_FINDER
#include "main.h"

//{ Global variables
int drp_count;
int drp_cur;
int loaded_unpacked=0;
int volatile cur_,count_;
drplist_t *queuedriverpack_p;

const tbl_t table_version[NUM_VER_NAMES]=
{
    {"classguid",                  9},
    {"class",                      5},
    {"provider",                   8},
    {"catalogfile",                11},
    {"catalogfile.nt",             14},
    {"catalogfile.ntx86",          17},
    {"catalogfile.ntia64",         18},
    {"catalogfile.ntamd64",        19},
    {"driverver",                  9},
    {"driverpackagedisplayname",   24},
    {"driverpackagetype",          17}
};
const wchar_t *olddrps[]=
{
    L"DP_Video_Server_1",
    L"DP_Video_Others_1",
    L"DP_Video_nVIDIA_1",
    L"DP_Video_AMD_1",
    L"DP_Videos_AMD_1",
    L"DP_LAN_Realtek_1",
};
//}

//{ Misc functions
void *mySzAlloc(void *p,size_t size)
{
    UNREFERENCED_PARAMETER(p)

    void *mem=nullptr;

    if (size == 0)return nullptr;
    try
    {
        mem=((void*)(new char[size]));
    }catch(std::bad_alloc)
    {
        log_err("Failed to alloc\n");
        //log_err("%10ld, Failed to allocate %ld MB \n",nvwa::total_mem_alloc/1024/1024,size/1024/1024);
    }catch(...)
    {
        log_err("Failed to alloc\n");
    }

    //if(size>1024*1024)log_err("%10ld, Allocated %ld MB\n",nvwa::total_mem_alloc/1024/1024,size/1024/1024);

    //if(!mem)log_err("Failed to alloc a\n");
    return mem;

}

void mySzFree(void *p,void *address)
{
    UNREFERENCED_PARAMETER(p)
    try
    {
        delete[] (char*)(address);
    }catch(...)
    {
        log_err("Failed to free\n");
    }
}

void findosattr(char *bufa,char *adr,int len)
{
    unsigned bufal=0;
    char *p=adr;

    *bufa=0;
    while(p+11<adr+len)
    {
        if(*p=='O'&&!memcmp(p,L"OSAttr",10))
        {
            int ofs=p[19]=='2'||p[19]=='1'?1:0;
            if(!*bufa||bufal<wcslen((wchar_t *)(p+18+ofs)))
            {
                wsprintfA(bufa,"%ws",p+18+ofs);
                bufal=strlen(bufa);
            }
        }
        p++;
    }
}
//}

//{ Version
int Version::setDate(int d_,int m_,int y_)
{
    d=d_;
    m=m_;
    y=y_;

    int flag=0;
    if(y<100)y+=1900;
    if(y<1990)flag=1;
    if(y>2013)flag=2;
    switch(m)
    {
        case 1:case 3:case 5:case 7:case 8:case 10:case 12:
            if(d<1||d>31)flag=3;
            break;
        case 4:case 6:case 9:case 11:
            if(d<1||d>30)flag=4;
            break;
        case 2:
            if(d<1||d>((((y%4==0)&&(y%100))||(y%400==0))?29:28))flag=5;
            break;
        default:
            flag=6;
    }
    return flag;
}

void Version::setVersion(int v1_,int v2_,int v3_,int v4_)
{
    v1=v1_;
    v2=v2_;
    v3=v3_;
    v4=v4_;
}

void Version::str_date(wchar_t *buf)
{
    SYSTEMTIME tm;
    FILETIME ft;

    memset(&tm,0,sizeof(SYSTEMTIME));
    tm.wDay=d;
    tm.wMonth=m;
    tm.wYear=y;
    SystemTimeToFileTime(&tm,&ft);
    FileTimeToSystemTime(&ft,&tm);

    if(y<1000)
        wsprintf(buf,STR(STR_HINT_UNKNOWN));
    else
        GetDateFormat(manager_g->matcher->getState()->getLocale(),0,&tm,nullptr,buf,100);
}

void Version::str_version(wchar_t *buf)
{
    if(v1<0)
        wsprintf(buf,STR(STR_HINT_UNKNOWN));
    else
        wsprintf(buf,L"%d.%d.%d.%d",v1,v2,v3,v4);
}

int cmpdate(Version *t1,Version *t2)
{
    int res;

    if(flags&FLAG_FILTERSP&&t2->y<1000)return 0;

    res=t1->y-t2->y;
    if(res)return res;

    res=t1->m-t2->m;
    if(res)return res;

    res=t1->d-t2->d;
    if(res)return res;

    return 0;
}

int cmpversion(Version *t1,Version *t2)
{
    int res;

    if(flags&FLAG_FILTERSP&&t2->v1<0)return 0;

    res=t1->v1-t2->v1;
    if(res)return res;

    res=t1->v2-t2->v2;
    if(res)return res;

    res=t1->v3-t2->v3;
    if(res)return res;

    res=t1->v4-t2->v4;
    if(res)return res;

    return 0;
}
//}

//{ Parser
void Parser::parseWhitespace(bool eatnewline=false)
{
    while(blockBeg<blockEnd)
    {
        switch(*blockBeg)
        {
//            case 0x1A:
            case '\n':
            case '\r':
                if(eatnewline==false)return;
                blockBeg++;
                break;

            case 32:  // space
            case '\t':// tab
                blockBeg++;
                break;

            case ';': // comment
                blockBeg++;
                while(blockBeg<blockEnd&&*blockBeg!='\n'&&*blockBeg!='\r')blockBeg++;
                break;

            case '\\': // continue line
                if(blockBeg+3<blockEnd&&blockBeg[1]=='\r'&&blockBeg[2]=='\n'){blockBeg+=3;break;}

            default:
                return;
        }
    }
}

void Parser::trimtoken()
{
    while(strEnd>strBeg&&(strEnd[-1]==32||strEnd[-1]=='\t')&&strEnd[-1]!='\"')strEnd--;
    if(*strBeg=='\"')strBeg++;
    if(*(strEnd-1)=='\"')strEnd--;
}

void Parser::subStr()
{
    if(!pack)return;

    // Fast string substitution
    char *v1b=strBeg;
    if(*v1b=='%')
    {
        v1b++;
        int vers_len=strEnd-v1b-1;
        if(strEnd[-1]!='%')vers_len++;
        if(vers_len<0)vers_len=0;

        strtolower(v1b,vers_len);
        auto rr=string_list->find(std::string(v1b,vers_len));
        if(rr!=string_list->end())
        {
            strBeg=const_cast<char *>(rr->second.c_str());
            strEnd=strBeg+strlen(strBeg);
            return;
        }
    }

    // Advanced string substitution
    char static_buf[BUFLEN];
    char *p_s=static_buf;
    int flag=0;
    v1b=strBeg;
    while(v1b<strEnd)
    {
        while(*v1b!='%'&&v1b<strEnd)*p_s++=*v1b++;
        if(*v1b=='%')
        {
            char *p=v1b+1;
            while(*p!='%'&&p<strEnd)p++;
            if(*p=='%')
            {
                strtolower(v1b+1,p-v1b-1);
                auto rr=string_list->find(std::string(v1b+1,p-v1b-1));
                if(rr!=string_list->end())
                {
                    char *res=const_cast<char *>(rr->second.c_str());
                    strcpy(p_s,res);
                    p_s+=strlen(res);
                    v1b=p+1;
                    flag=1;
                }
#ifdef DEBUG_EXTRACHECKS
                else log_con("String '%s' not found in %S(%S)\n",std::string(v1b+1,p-v1b-1).c_str(),pack->getFilename(),inffile);
#endif
            }
            if(v1b<strEnd)*p_s++=*v1b++;
        }
    }
    if(!flag)return;

    *p_s=0;
    strBeg=textholder.get(textholder.strcpy(static_buf));
    strEnd=strBeg+strlen(strBeg);
}

int Parser::parseItem()
{
    parseWhitespace(true);
    strBeg=blockBeg;

    char *p=blockBeg;

    while(p<blockEnd-1)
    {
        switch(*p)
        {
            case '=':               // Item found
                blockBeg=p;
                strEnd=p;
                trimtoken();
                subStr();
                return 1;

            case '\n':case '\r':    // No item found
                blockBeg=p++;
                parseWhitespace(true);
                strBeg=blockBeg;
#ifdef DEBUG_EXTRACHECKS
                log_con("ERROR: no item '%s' found in %S(%S){%s}\n\n",std::string(blockBeg,30).c_str(),pack->getFilename(),inffile,std::string(blockEnd,30).c_str());
#endif
                break;
            default:
                p++;
        }
    }
    strBeg=nullptr;
    strEnd=nullptr;
    return 0;
}

int Parser::parseField()
{
    if(blockBeg[0]!='='&&blockBeg[0]!=',')return 0;
    blockBeg++;
    parseWhitespace();

    char *p=blockBeg;
    int flag=0;

    strBeg=strEnd=p;

    if(*p=='\"')    // "str"
    {
        strBeg++;
        p++;
        while(p<blockEnd)
        {
            switch(*p)
            {
                case '\r':case '\n': // no closing "
                    p++;
#ifdef DEBUG_EXTRACHECKS
                    log_file("ERR2 '%.*s'\n",30,s1b-1);
#endif
                case '\"':          // "str"
                    strEnd=p;
                    blockBeg=strEnd+1;
                    subStr();
                    return 1;

                default:
                    p++;
            }
        }
    }
    else
    {
        while(p<blockEnd&&!flag)
        {
            switch(*p)
            {
                case '\n':case '\r':
                case ';':
                case ',':
                    strEnd=p;
                    blockBeg=p;
                    trimtoken();
                    subStr();
                    return strEnd!=strBeg||*p==',';

                default:
                    p++;
            }
        }
    }
    return 0;
}

int Parser::readNumber()
{
    int n=atoi(strBeg);

    while(strBeg<strEnd&&*strBeg>='0'&&*strBeg<='9')strBeg++;
    if(strBeg<strEnd)strBeg++;
    return n;
}

int Parser::readHex()
{
    int val=0;

    while(strBeg<strEnd&&(*strBeg=='0'||*strBeg=='x'))strBeg++;
    if(strBeg<strEnd)
        val=toupper(*strBeg)-(*strBeg<='9'?'0':'A'-10);

    strBeg++;
    if(strBeg<strEnd)
    {
        val<<=4;
        val+=toupper(*strBeg)-(*strBeg<='9'?'0':'A'-10);
    }
    return val;
}

int Parser::readDate(Version *t)
{

    while(strBeg<strEnd&&!(*strBeg>='0'&&*strBeg<='9'))strBeg++;
    int m=readNumber();
    int d=readNumber();
    int y=readNumber();
    return t->setDate(d,m,y);

}

void Parser::readVersion(Version *t)
{
    int v1=readNumber();
    int v2=readNumber();
    int v3=readNumber();
    int v4=readNumber();
    t->setVersion(v1,v2,v3,v4);
}

void Parser::readStr(char **vb,char **ve)
{
    *vb=strBeg;
    *ve=strEnd;
}

Parser::Parser(Driverpack *drpv,std::unordered_map<std::string,std::string> &string_listv,const wchar_t *inf)
{
    pack=drpv;
    string_list=&string_listv;
    inffile=inf;
}

Parser::Parser(char *vb,char *ve)
{
    strBeg=vb;
    strEnd=ve;
    pack=nullptr;
}

void Parser::setRange(sect_data_t *lnk)
{
    blockBeg=lnk->blockbeg;
    blockEnd=lnk->blockend;
}
//}

//{ Collection
int Collection::scanfolder_count(const wchar_t *path)
{
    int cnt=0;

    wchar_t buf[BUFLEN];
    wsprintf(buf,L"%ws\\*.*",path);
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind=FindFirstFile(buf,&FindFileData);
    while(FindNextFile(hFind,&FindFileData)!=0)
    {
        if(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
        {
            if(lstrcmp(FindFileData.cFileName,L"..")==0)continue;
            wsprintf(buf,L"%ws\\%ws",path,FindFileData.cFileName);
            cnt+=scanfolder_count(buf);
        } else
        {
            int i,len=lstrlenW(FindFileData.cFileName);
            for(i=0;i<6;i++)
            if(StrStrIW(FindFileData.cFileName,olddrps[i]))
            {
                wsprintf(buf,L" /c del \"%s\\%s*.7z\" /Q /F",driverpack_dir,olddrps[i]);
                run_command(L"cmd",buf,SW_HIDE,1);
                break;
            }
            if(i==6&&StrCmpIW(FindFileData.cFileName+len-3,L".7z")==0)
            {
                Driverpack drp{path,FindFileData.cFileName,this};
                if(flags&COLLECTION_FORCE_REINDEXING||!drp.checkindex())cnt++;
            }
        }
    }
    FindClose(hFind);
    return cnt;
}

void Collection::scanfolder(const wchar_t *path,void *arg)
{
    wchar_t buf[BUFLEN];
    wsprintf(buf,L"%s\\*.*",path);
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind=FindFirstFile(buf,&FindFileData);

    while(FindNextFile(hFind,&FindFileData)!=0)
    {
        if(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
        {
            if(lstrcmp(FindFileData.cFileName,L"..")==0)continue;
            wsprintf(buf,L"%s\\%s",path,FindFileData.cFileName);
            scanfolder(buf,arg);
        } else
        {
            int len=lstrlenW(FindFileData.cFileName);
            if(StrCmpIW(FindFileData.cFileName+len-3,L".7z")==0)
            {
                driverpack_list.push_back(Driverpack(path,FindFileData.cFileName,this));
                reinterpret_cast<drplist_t *>(arg)->push(driverpack_task{&driverpack_list.back()});
            }else
            if((StrCmpIW(FindFileData.cFileName+len-4,L".inf")==0||
               StrCmpIW(FindFileData.cFileName+len-4,L".cat")==0)&&loaded_unpacked==0)
            {
                wsprintf(buf,L"%s\\%s",path,FindFileData.cFileName);
                FILE *f=_wfopen(buf,L"rb");
                fseek(f,0,SEEK_END);
                len=ftell(f);
                fseek(f,0,SEEK_SET);
                char *buft=new char[len];
                fread(buft,len,1,f);
                fclose(f);
                wsprintf(buf,L"%s\\",path+wcslen(driverpack_dir)+1);

                if(len)
                {
                    if(StrCmpIW(FindFileData.cFileName+lstrlenW(FindFileData.cFileName)-4,L".inf")==0)
                        driverpack_list[0].indexinf(buf,FindFileData.cFileName,buft,len);
                    else
                        driverpack_list[0].parsecat(buf,FindFileData.cFileName,buft,len);
                }

                delete []buft;
            }
        }
    }
    FindClose(hFind);
}

void Collection::loadOnlineIndexes()
{
    wchar_t buf[BUFLEN];
    wsprintf(buf,L"%ws\\_*.*",index_bin_dir);
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind=FindFirstFile(buf,&FindFileData);

    while(FindNextFile(hFind,&FindFileData)!=0)
    {
        wchar_t filename[BUFLEN];
        wsprintf(filename,L"%ws",FindFileData.cFileName);
        wcscpy(filename+wcslen(FindFileData.cFileName)-3,L"7z");

        wsprintf(buf,L"drivers\\%ws",filename);
        buf[8]=L'D';
        if(PathFileExists(buf))
        {
            log_con("Skip %S\n",buf);
            continue;
        }

        driverpack_list.push_back(Driverpack(driverpack_dir,filename,this));
        driverpack_list.back().loadindex();
    }
    FindClose(hFind);
}

void Collection::init(wchar_t *driverpacks_dirv,const wchar_t *index_bin_dirv,const wchar_t *index_linear_dirv)
{
    driverpack_dir=driverpacks_dirv;
    index_bin_dir=index_bin_dirv;
    index_linear_dir=index_linear_dirv;
    driverpack_list.clear();
}

Collection::Collection(wchar_t *driverpacks_dirv,const wchar_t *index_bin_dirv,const wchar_t *index_linear_dirv)
{
    driverpack_dir=driverpacks_dirv;
    index_bin_dir=index_bin_dirv;
    index_linear_dir=index_linear_dirv;
}

void Collection::updatedir()
{
    driverpack_dir=*drpext_dir?drpext_dir:drp_dir;
    populate();
}

void Collection::populate()
{
    Driverpack *unpacked_drp;

    time_indexes=GetTickCount();

    drp_count=scanfolder_count(driverpack_dir);
    driverpack_list.reserve(drp_count+1+100); // TODO

    driverpack_list.push_back(Driverpack(driverpack_dir,L"unpacked.7z",this));
    unpacked_drp=&driverpack_list.back();

//{thread
    drplist_t queuedriverpack1;
    queuedriverpack_p=&queuedriverpack1;
    int num_thr=num_cores;
    int num_thr_1=num_cores;
    #ifndef _WIN64
    if(drp_count&&num_thr>3)num_thr=3;
    #endif

    HANDLE thr[16],cons[16];
    for(int i=0;i<num_thr_1;i++)
        thr[i]=(HANDLE)_beginthreadex(nullptr,0,&Driverpack::indexinf_thread,&queuedriverpack1,0,nullptr);

    drplist_t queuedriverpack;
    for(int i=0;i<num_thr;i++)
        cons[i]=(HANDLE)_beginthreadex(nullptr,0,&Driverpack::loaddrp_thread,&queuedriverpack,0,nullptr);
//}thread

    if(flags&FLAG_KEEPUNPACKINDEX)loaded_unpacked=unpacked_drp->loadindex();
    drp_cur=1;

    scanfolder(driverpack_dir,&queuedriverpack);
    for(int i=0;i<num_thr;i++)queuedriverpack.push(driverpack_task{nullptr});

    for(int i=0;i<num_thr;i++)
    {
        WaitForSingleObject(cons[i],INFINITE);
        CloseHandle_log(cons[i],L"driverpack_genindex",L"cons");
    }

    loadOnlineIndexes();

    manager_g->itembar_setactive(SLOT_INDEXING,0);
    if(driverpack_list.size()<=1&&(flags&FLAG_DPINSTMODE)==0)
        manager_g->itembar_settext(SLOT_NODRIVERS,L"",0);
    driverpack_list[0].genhashes();

//{thread
    for(int i=0;i<num_thr_1;i++)queuedriverpack1.push(driverpack_task{nullptr});

    for(int i=0;i<num_thr_1;i++)
    {
        WaitForSingleObject(thr[i],INFINITE);
        CloseHandle_log(thr[i],L"driverpack_genindex",L"thr");
    }
//}thread
    flags&=~COLLECTION_FORCE_REINDEXING;
    driverpack_list.shrink_to_fit();
    time_indexes=GetTickCount()-time_indexes;
}

void Collection::save()
{
    #ifdef CONSOLE_MODE
    return;
    #endif
    if(*drpext_dir)return;
    if(!canWrite(index_bin_dir))
    {
        log_err("ERROR in collection_save(): Write-protected,'%S'\n",index_bin_dir);
        return;
    }
    time_indexsave=GetTickCount();

    // Save indexes
    count_=0;
    cur_=1;
    if((flags&FLAG_KEEPUNPACKINDEX)==0)
        driverpack_list[0].setType(DRIVERPACK_TYPE_INDEXED);
    for(auto &driverpack:driverpack_list)
        if(driverpack.getType()==DRIVERPACK_TYPE_PENDING_SAVE)count_++;

    if(count_)log_con("Saving indexes...\n");
    HANDLE thr[16];
    drplist_t queuedriverpack_loc;
    for(int i=0;i<num_cores;i++)
        thr[i]=(HANDLE)_beginthreadex(nullptr,0,&Driverpack::savedrp_thread,&queuedriverpack_loc,0,nullptr);
    for(auto &driverpack:driverpack_list)
    if(driverpack.getType()==DRIVERPACK_TYPE_PENDING_SAVE)
        queuedriverpack_loc.push(driverpack_task{&driverpack});

    for(int i=0;i<num_cores;i++)queuedriverpack_loc.push(driverpack_task{nullptr});
    for(int i=0;i<num_cores;i++)
    {
        WaitForSingleObject(thr[i],INFINITE);
        CloseHandle_log(thr[i],L"driverpack_genindex",L"thr");
    }
    manager_g->itembar_settext(SLOT_INDEXING,0);
    if(count_)log_con("DONE\n");

    // Delete unused indexes
    WIN32_FIND_DATA FindFileData;
    wchar_t filename[BUFLEN];
    wchar_t buf[BUFLEN];
    wsprintf(buf,L"%ws\\*.*",index_bin_dir);
    HANDLE hFind=FindFirstFile(buf,&FindFileData);
    while(FindNextFile(hFind,&FindFileData)!=0)
    {
        if(!(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
        {
            if(FindFileData.cFileName[0]==L'_')continue;
            wsprintf(filename,L"%s\\%s",index_bin_dir,FindFileData.cFileName);
            unsigned i;
            for(i=flags&FLAG_KEEPUNPACKINDEX?0:1;i<driverpack_list.size();i++)
            {
                wchar_t buf1[BUFLEN];
                driverpack_list[i].getindexfilename(index_bin_dir,L"bin",buf1);
                if(!StrCmpIW(buf1,filename))break;
            }
            if(i==driverpack_list.size())
            {
                log_con("Deleting %S\n",filename);
                _wremove(filename);
            }
        }
    }
    time_indexsave=GetTickCount()-time_indexsave;
}

void Collection::printstats()
{
    if((log_verbose&LOG_VERBOSE_DRP)==0)return;

    int sum=0;
    log_file("Driverpacks\n");
    for(auto &drp:driverpack_list)
        sum+=drp.printstats();

    log_file("  Sum: %d\n\n",sum);
}

void Collection::print_index_hr()
{
    time_indexprint=GetTickCount();

    for(auto &drp:driverpack_list)drp.print_index_hr();

    time_indexprint=GetTickCount()-time_indexprint;
}

wchar_t *Collection::finddrp(wchar_t *fnd)
{
    int j;
    wchar_t *s,*d,*n_s;

    j=0;
    n_s=nullptr;
    for(auto &drp:driverpack_list)
    {
        s=drp.getFilename();
        if(StrStrIW(s,fnd)&&drp.getType()!=DRIVERPACK_TYPE_UPDATE)
        {
            d=s;
            while(*d)
            {
                if(*d==L'_'&&d[1]>=L'0'&&d[1]<=L'9')
                {
                    if(j<_wtoi_my(d+1))
                    {
                        j=_wtoi_my(d+1);
                        n_s=s;
                    }
                    break;
                }
                d++;
            }
        }
    }
    return n_s;
}
//}

#ifdef MERGE_FINDER
class Filedata
{
    std::wstring str;
    int size;
    unsigned crc;

public:
    const wchar_t *getStr(){return str.c_str();}
    const wchar_t *getPath(){return str.c_str();}
    const wchar_t *getFilename(){return str.c_str();}
    bool checksize(int _size){return size==_size;}
    bool checkCRC(unsigned _CRC){return crc==_CRC;}
    bool checkself(const wchar_t *_str){return wcscmp(str.c_str(),_str);}
    Filedata(std::wstring _str,int _size,unsigned _crc):str(_str),size(_size),crc(_crc){}

    friend class Merger;
};

class Merger
{
    std::unordered_set<std::wstring> merged;
    std::unordered_set<std::wstring> dirlist;
    std::unordered_multimap<std::wstring,Filedata> filename2path;
    std::unordered_multimap<std::wstring,Filedata> path2filename;
    std::unordered_multimap<std::wstring,std::wstring> dir2dir;
    FILE *f;
    CSzArEx *db;

public:
    Merger(CSzArEx *_db,const wchar_t *fullname);
    ~Merger();
    void makerecords(int i);
    int checkfolders(const std::wstring dir1,const std::wstring dir2,int sub);
    void find_dups();
    int combine(std::wstring dir1,std::wstring dir2,int sz);
    void process_file(int i,unsigned *CRC,int *size,
                  std::wstring &_filename,std::wstring &_filepath,
                  std::wstring &_subdir1,std::wstring &_subdir2);
};

Merger::Merger(CSzArEx *_db,const wchar_t *fullname)
{
    f=_wfopen(fullname,L"wt");
    log_con("Making %ws\n",fullname);
    db=_db;
}

Merger::~Merger()
{
    fclose(f);
}

void Merger::process_file(int i,unsigned *CRC,int *size,
                  std::wstring &_filename,std::wstring &_filepath,
                  std::wstring &_subdir1,std::wstring &_subdir2)
{
    *CRC=db->CRCs.Vals[i];
    *size=SzArEx_GetFileSize(db,i);
    wchar_t fullname[BUFLEN];
    SzArEx_GetFileNameUtf16(db,i,(UInt16 *)fullname);

    _filename.clear();
    _filepath.clear();
    _subdir1.clear();
    _subdir2.clear();

    wchar_t buf[BUFLEN];
    wchar_t filepath[BUFLEN];
    wcscpy(buf,fullname);
    wcscpy(filepath,fullname);
    wchar_t *p=buf,*filename=nullptr,*subdir1=buf,*subdir2=nullptr;

    while((p=wcschr(p,L'/'))!=nullptr)
    {
        p++;
        subdir2=filename;
        filename=p;
    }
    if(subdir1&&subdir2)
    {
        subdir2[-1]=0;
        subdir1[filename-buf-1]=0;
        _subdir1=subdir1;
        _subdir2=subdir2;
    }
    if(filename)
    {
        filepath[filename-buf-1]=0;
        _filename=filename;
        _filepath=filepath;
    }
    std::transform(_filename.begin(),_filename.end(),_filename.begin(),::tolower);
    std::transform(_filepath.begin(),_filepath.end(),_filepath.begin(),::tolower);
    std::transform(_subdir1.begin(),_subdir1.end(),_subdir1.begin(),::tolower);
    std::transform(_subdir2.begin(),_subdir2.end(),_subdir2.begin(),::tolower);
}

void Merger::makerecords(int i)
{
    std::wstring filename,filepath,subdir1,subdir2;
    unsigned CRC;
    int sz;

    process_file(i,&CRC,&sz,filename,filepath,subdir1,subdir2);
    //log_con("%8X,%10d,{%ws},{%ws},{%ws},{%ws}\n",CRC,sz,filepath.c_str(),filename.c_str(),subdir1.c_str(),subdir2.c_str());

    if(!filename.empty())
    {
        filename2path.insert({filename,{filepath,sz,CRC}});
        path2filename.insert({filepath,{filename,sz,CRC}});
    }

    if(!subdir1.empty()&&!subdir2.empty())
    {
        std::wstring tstr=subdir1+subdir2;
        if(dirlist.find(tstr)==dirlist.end())
        {
            dir2dir.insert({subdir1,subdir2});
            dirlist.insert(tstr);
        }
    }
}

int Merger::checkfolders(const std::wstring dir1,const std::wstring dir2,int sub)
{
    int sizecom=0,sizedif=0;
    bool hasINF=false;

    if(merged.find(dir1)!=merged.end())return -1;
    if(merged.find(dir2)!=merged.end())return -1;

    auto range1=path2filename.equal_range(dir1);
    for(auto it1=range1.first;it1!=range1.second;it1++)
    {
        Filedata *d1=&it1->second;
        if(StrStrIW(d1->getStr(),L".inf"))hasINF=true;

        auto range2=filename2path.equal_range(d1->getStr());
        for(auto it2=range2.first;it2!=range2.second;it2++)
        {
            Filedata *d2=&it2->second;
        //log_con("* %ws,%ws\n",it2->second.getStr(),dir2);
            if(d2->str==dir2)
            {
                if(d1->size==d2->size&&d1->crc==d2->crc)
                    sizecom+=d1->size;
                else
                    sizedif+=d1->size;
        //log_con("* %ws\n",d1->getStr());

            }
        }
    }
    if(sub==0&&hasINF==false)
    {
        if(dir1.find(L"/")!=std::wstring::npos&&dir2.find(L"/")!=std::wstring::npos)
        {
            std::wstring dir1new=dir1;
            std::wstring dir2new=dir2;
            dir1new.resize(dir1new.rfind(L"/"));
            dir2new.resize(dir2new.rfind(L"/"));
            checkfolders(dir1new,dir2new,0);
            //log_con("{%ws},{%ws}\n",dir1.c_str(),dir1new.c_str());
        }
        return -1;
    }

    auto range2=dir2dir.equal_range(dir1);
    for(auto it2=range2.first;it2!=range2.second;it2++)
    {
        int sz=checkfolders(dir1+L'/'+it2->second,dir2+L'/'+it2->second,1);
        //int sz=-1;
        if(sz<0)return -1;
        sizecom+=sz;
        //log_con("# %ws\n",it2->second.c_str());
    }

    if(sizedif)return -1;
    //if(sub==0&&sizecom)log_con("\n%d,%d\n%ws\n%ws\n",sizecom,sizedif,dir1.c_str(),dir2.c_str());

    if(sub==0&&sizecom>0)
    {
        if(combine(dir1,dir2,sizecom))
        {
            merged.insert(dir1);
            merged.insert(dir2);
        }
    }

    return sizecom;
}

void detectmarker(std::wstring str,int *i)
{
    char buf[BUFLEN];
    wsprintfA(buf,"%S",str.c_str());

    for(*i=0;*i<NUM_MARKERS;(*i)++)
    if(StrStrIA(buf,markers[*i].name))
    {
        return;
    }
    log_con("Unk marker {%s}\n",buf);
    *i=-1;
}

int Merger::combine(std::wstring dir1,std::wstring dir2,int sz)
{
    int m1,m2;
    std::wstring dest(dir1);

    if(dir1.find(dir2)!=std::string::npos)return 0;
    if(dir2.find(dir1)!=std::string::npos)return 0;
    std::transform(dest.begin(),dest.end(),dest.begin(),::tolower);

    detectmarker(dir1,&m1);
    detectmarker(dir2,&m2);
    if(m1>=0&&m2>=0)
    {
        int major=markers[m1].major;
        int minor=markers[m1].minor;
        int arch=-1;

        if(markers[m1].major>=0&&markers[m1].minor>=0&&
           markers[m1].major>=markers[m2].major&&markers[m1].minor>=markers[m2].minor)
        {
            major=markers[m2].major;
            minor=markers[m2].minor;
        }
        else
        {
            major=markers[m1].major;
            minor=markers[m1].minor;
        }

        if(markers[m1].arch>=0)arch=markers[m1].arch;
        if(markers[m2].arch>=0)arch=markers[m2].arch;
        if(markers[m1].arch==0&&markers[m2].arch==1)arch=-1;
        if(markers[m1].arch==1&&markers[m2].arch==0)arch=-1;

        int i;
        for(i=0;i<NUM_MARKERS;i++)
        {
            if(markers[i].arch==arch&&
               markers[i].major==major&&
               markers[i].minor==minor)
            {
                wchar_t buf1[BUFLEN];
                wchar_t buf2[BUFLEN];
                wsprintfW(buf1,L"%S",markers[m1].name);
                wsprintfW(buf2,L"%S",markers[i].name);
                dest.replace(dest.find(buf1),wcslen(buf1),buf2);
                break;
            }
        }
        if(i==NUM_MARKERS)
        {
            /*wchar_t buf1[BUFLEN];
            wchar_t buf2[BUFLEN];
            wsprintfW(buf2,L" #(%d,%d,%d)",major,minor,arch);
            dest+=buf2;*/
        }
    }
    fprintf(f,"rem %d\n",sz);
    std::replace(dir1.begin(),dir1.end(),'/','\\');
    std::replace(dir2.begin(),dir2.end(),'/','\\');
    std::replace(dest.begin(),dest.end(),'/','\\');
    fprintf(f,"movefiles %S %S\n",dir1.c_str(),dest.c_str());
    fprintf(f,"movefiles %S %S\n\n",dir2.c_str(),dest.c_str());
    return 1;
}

void Merger::find_dups()
{
    log_con("{");
    for(unsigned i=0;i<db->NumFiles;i++)
    {
        std::wstring filename,filepath,subdir1,subdir2;
        unsigned CRC;
        int sz;

        process_file(i,&CRC,&sz,filename,filepath,subdir1,subdir2);

        if(filename.empty())continue;
        if(merged.find(filepath)!=merged.end())continue;

        auto range=filename2path.equal_range(filename);
        for(auto it=range.first;it!=range.second;it++)
        {
            Filedata *d=&it->second;
            if(d->checkCRC(CRC)&&d->checksize(sz)&&d->checkself(filepath.c_str()))
            if(merged.find(d->getStr())==merged.end())
            {
                //log_con(".");
                int szcom=checkfolders(filepath,d->str,0);
/*                if(szcom>1024*1024)
                {
                    combine(filepath,d->str,szcom);
                    //fprintf(f,"%S\n%S\n\n",filepath.c_str(),d->getStr());
                    merged.insert(filepath);
                    merged.insert(d->getStr());
                }*/
            }
        }
    }
    log_con("}\n");
}
#endif

//{ Driverpack
int Driverpack::genindex()
{
    CFileInStream archiveStream;
    CLookToRead lookStream;

    wchar_t fullname[BUFLEN];
    wchar_t infpath[BUFLEN];
    wchar_t *infname;

    wchar_t name[BUFLEN];
    wsprintf(name,L"%ws\\%ws",getPath(),getFilename());
    log_con("Indexing %S\n",name);
    if(InFile_OpenW(&archiveStream.file,name))return 1;

    FileInStream_CreateVTable(&archiveStream);
    LookToRead_CreateVTable(&lookStream,False);
    lookStream.realStream=&archiveStream.s;
    LookToRead_Init(&lookStream);

    ISzAlloc allocImp;
    ISzAlloc allocTempImp;
    allocImp.Alloc=mySzAlloc;
    allocImp.Free=mySzFree;
    allocTempImp.Alloc=SzAllocTemp;
    allocTempImp.Free=SzFreeTemp;

    CSzArEx db;
    SzArEx_Init(&db);
    SRes res=SzArEx_Open(&db,&lookStream.s,&allocImp,&allocTempImp);
    int cc=0;
    if(res==SZ_OK)
    {
#ifdef MERGE_FINDER
        getindexfilename(col->getIndex_linear_dir(),L"7z.bat",fullname);
        Merger merger{&db,fullname};
        for(unsigned i=0;i<db.NumFiles;i++)if(!SzArEx_IsDir(&db,i))merger.makerecords(i);
        merger.find_dups();
#endif
      /*
      if you need cache, use these 3 variables.
      if you use external function, you can make these variable as static.
      */
        UInt32 blockIndex=0xFFFFFFFF; /* it can have any value before first call (if outBuffer = 0) */
        Byte *outBuffer=nullptr; /* it must be 0 before first call for each new archive. */
        size_t outBufferSize=0;  /* it can have any value before first call (if outBuffer = 0) */

        for(unsigned i=0;i<db.NumFiles;i++)
        {
            size_t offset=0;
            size_t outSizeProcessed=0;
            if(SzArEx_IsDir(&db,i))continue;

            if(SzArEx_GetFileNameUtf16(&db,i,nullptr)>BUFLEN)
            {
                res=SZ_ERROR_MEM;
                log_err("ERROR: mem\n");
                break;
            }
            SzArEx_GetFileNameUtf16(&db,i,(UInt16 *)fullname);

            if(StrCmpIW(fullname+wcslen(fullname)-4,L".inf")==0||
                StrCmpIW(fullname+wcslen(fullname)-4,L".cat")==0)
            {
                //log_con("{");

                tryagain:
                res = SzArEx_Extract(&db,&lookStream.s,i,
                    &blockIndex,&outBuffer,&outBufferSize,
                    &offset,&outSizeProcessed,
                    &allocImp,&allocTempImp);
                //log_con("}");
                if(res!=SZ_ERROR_MEM)
                {
                    log_err("ERROR with %S:%d\n",getFilename(),res);
                    Sleep(100);
                    goto tryagain;
                    //continue;
                }

                wchar_t *ii=fullname;
                while(*ii){if(*ii=='/')*ii='\\';ii++;}
                infname=ii;
                while(infname!=fullname&&*infname!='\\')infname--;
                if(*infname=='\\'){*infname++=0;}
                wsprintf(infpath,L"%ws\\",fullname);

                cc++;
                if(outSizeProcessed)
                {
                    if(StrStrIW(infname,L".inf"))
                        driverpack_indexinf_async(infpath,infname,(char *)(outBuffer+offset),outSizeProcessed);
                    else
                        parsecat(infpath,infname,(char *)(outBuffer+offset),outSizeProcessed);
                        //driverpack_parsecat_async(this,infpath,infname,(char *)(outBuffer+offset),outSizeProcessed);
                }
            }
        }
        IAlloc_Free(&allocImp,outBuffer);
    }
    else
    {
        log_err("ERROR with %S:%d\n",getFilename(),res);
    }
    SzArEx_Free(&db,&allocImp);
    File_Close(&archiveStream.file);
    return 1;
}

void Driverpack::driverpack_parsecat_async(wchar_t const *pathinf,wchar_t const *inffile1,char *adr,int len)
{
    inffile_task data;

    data.adr=new char[len];
    memmove(data.adr,adr,len);
    data.len=len;
    wcscpy(data.pathinf,pathinf);
    wcscpy(data.inffile,inffile1);
    data.drp=this;
    objs_new->push(data);
}

void Driverpack::driverpack_indexinf_async(wchar_t const *pathinf,wchar_t const *inffile1,char *adr,int len)
{
    inffile_task data;

    data.drp=this;
    if(!adr)
    {
        data.adr=nullptr;
        if(objs_new)objs_new->push(data);
        return;
    }

    if(len>4&&((adr[0]==-1&&adr[3]==0)||adr[0]==0))
    {
        data.adr=new char[len+2];
        if(!data.adr)
        {
            log_err("ERROR in driverpack_indexinf: malloc(%d)\n",len+2);
            return;
        }
        len=unicode2ansi(adr,data.adr,len);
    }
    else
    {
        data.adr=new char[len];
        memmove(data.adr,adr,len);
    }

    data.len=len;
    data.pathinf=new wchar_t[wcslen(pathinf)+1];
    data.inffile=new wchar_t[wcslen(inffile1)+1];

    wcscpy(data.pathinf,pathinf);
    wcscpy(data.inffile,inffile1);
    objs_new->push(data);
}

void Driverpack::indexinf_ansi(wchar_t const *drpdir,wchar_t const *inffilename,char *inf_base,int inf_len)
{
    // http://msdn.microsoft.com/en-us/library/ff547485(v=VS.85).aspx
    Version *cur_ver;

    int cur_inffile_index;
    data_inffile_t *cur_inffile;
    int cur_manuf_index;
    data_manufacturer_t *cur_manuf;
    int cur_desc_index;

    char secttry[256];
    char line[2048];
    int  strs[16];

    std::unordered_map<std::string,std::string> string_list;
    std::unordered_multimap<std::string,sect_data_t> section_list;

    char *p=inf_base,*strend=inf_base+inf_len;
    char *p2,*sectnmend;
    sect_data_t *lnk_s2=nullptr;

    cur_inffile_index=inffile.size();
    inffile.resize(cur_inffile_index+1);
    cur_inffile=&inffile[cur_inffile_index];
    wsprintfA(line,"%ws",drpdir);
    cur_inffile->infpath=text_ind.strcpy(line);
    wsprintfA(line,"%ws",inffilename);
    cur_inffile->inffilename=text_ind.strcpy(line);
    cur_inffile->infsize=inf_len;
    cur_inffile->infcrc=0;

    wchar_t inffull[BUFLEN];
    wcscpy(inffull,drpdir);
    wcscat(inffull,inffilename);

    Parser parse_info{this,string_list,inffull};
    Parser parse_info2{this,string_list,inffull};
    Parser parse_info3{this,string_list,inffull};
    //log_con("%S%S\n",drpdir,inffilename);

    // Populate sections
    while(p<strend)
    {
        switch(*p)
        {
            case ' ':case '\t':case '\n':case '\r':
                p++;
                break;

            case ';':
                p++;
                while(p<strend&&*p!='\n'&&*p!='\r')p++;
                break;

            case '[':
                if(lnk_s2)
                    lnk_s2->blockend=p;
#ifdef DEBUG_EXTRACHECKS
/*					char *strings_base=inf_base+(*it).second.ofs;
					int strings_len=(*it).second.len-(*it).second.ofs;
					if(*(strings_base-1)!=']')
						log_file("B'%.*s'\n",1,strings_base-1);
					if(*(strings_base+strings_len)!='[')
						log_file("E'%.*s'\n",1,strings_base+strings_len);*/
#endif
                p++;
                p2=p;

                while(*p2!=']'&&p2<strend)
                {
#ifdef DEBUG_EXTRACHECKS
                    if(*p2=='\\')log_file("Err \\\n");else
                    if(*p2=='"')log_file("Err \"\n");else
                    if(*p2=='%')log_file("Err %\n");else
                    if(*p2==';')log_file("Err ;\n");
#endif
                    cur_inffile->infcrc+=*p2++;
                }
                sectnmend=p2;
                p2++;

                {
                    strtolower(p,sectnmend-p);
                    auto a=section_list.insert({std::string(p,sectnmend-p),sect_data_t(p2,inf_base+inf_len)});
                    lnk_s2=&a->second;
                    //log_con("  %8d,%8d '%s' \n",strlink.ofs,strlink.len,std::string(p,sectnmend-p).c_str());
                }
                p=p2;
                break;

            default:
                //b=p;
                //while(*p!='\n'&&*p!='\r'&&*p!='['&&p<strend)p++;
                //if(*p=='['&&p<strend)log_file("ERROR in %S%S:\t\t\t'%.*s'(%d/%d)\n",drpdir,inffile,p-b+20,b,p,strend);
                while(p<strend&&*p!='\n'&&*p!='\r'/*&&*p!='['*/)cur_inffile->infcrc+=*p++;
        }
    }

    // Find [strings]
    auto range=section_list.equal_range("strings");
    if(range.first==range.second)log_index("ERROR: missing [strings] in %S\n",inffull);
    for(auto got=range.first;got!=range.second;++got)
    {
        sect_data_t *lnk=&got->second;
        char *s1b,*s1e,*s2b,*s2e;

        parse_info.setRange(lnk);
        while(parse_info.parseItem())
        {
            parse_info.readStr(&s1b,&s1e);
            parse_info.parseField();
            parse_info.readStr(&s2b,&s2e);
            strtolower(s1b,s1e-s1b);
            string_list.insert({std::string(s1b,s1e-s1b),std::string(s2b,s2e-s2b)});
        }
    }

    // Find [version]
    cur_ver=&cur_inffile->version;
    cur_ver->setInvalid();

    range=section_list.equal_range("version");
    if(range.first==range.second)log_index("ERROR: missing [version] in %S\n",inffull);
    //if(range.first==range.second)log_index("NOTE:  multiple [version] in %S\n",inffull);
    for(auto got=range.first;got!=range.second;++got)
    {
        sect_data_t *lnk=&got->second;
        char *s1b,*s1e;

        parse_info.setRange(lnk);
        while(parse_info.parseItem())
        {
            parse_info.readStr(&s1b,&s1e);
            strtolower(s1b,s1e-s1b);
            //log_con("tolower '%.10s'\n",s1b);

            int i,sz=s1e-s1b;
            for(i=0;i<NUM_VER_NAMES;i++)
            if(table_version[i].sz==sz&&!memcmp(s1b,table_version[i].s,sz))
            {
                if(i==DriverVer)
                {
                        // date
                        parse_info.parseField();
                        i=parse_info.readDate(cur_ver);
                        /*if(i)log_index("ERROR: invalid date(%d.%d.%d)[%d] in %S\n",
                                 cur_ver->d,cur_ver->m,cur_ver->y,i,inffull);*/

                        // version
                        if(parse_info.parseField())
                        {
                            parse_info.readVersion(cur_ver);
                        }

                }else
                {
                    parse_info.parseField();
                    parse_info.readStr(&s1b,&s1e);
                    cur_inffile->fields[i]=text_ind.t_memcpyz(s1b,s1e-s1b);
                }
                break;
            }
            if(i==NUM_VER_NAMES)
            {
                //s1e=parse_info.se;
                //log_file("QQ '%.*s'\n",s1e-s1b,s1b);
            }
            while(parse_info.parseField());
        }
    }
    //if(cur_ver->y==-1) log_index("ERROR: missing date in %S\n",inffull);
    //if(cur_ver->v1==-1)log_index("ERROR: missing build number in %S\n",inffull);

    // Find [manufacturer] section
    range=section_list.equal_range("manufacturer");
    if(range.first==range.second)log_index("ERROR: missing [manufacturer] in %S\n",inffull);
    //if(lnk)log_index("NOTE:  multiple [manufacturer] in %S%S\n",drpdir,inffilename);
    for(auto got=range.first;got!=range.second;++got)
    {
        sect_data_t *lnk=&got->second;
        parse_info.setRange(lnk);
        while(parse_info.parseItem())
        {
            char *s1b,*s1e;
            parse_info.readStr(&s1b,&s1e);

            cur_manuf_index=manufacturer_list.size();
            manufacturer_list.resize(cur_manuf_index+1);
            cur_manuf=&manufacturer_list[cur_manuf_index];
            cur_manuf->inffile_index=cur_inffile_index;
            cur_manuf->manufacturer=text_ind.t_memcpyz(s1b,s1e-s1b);
            cur_manuf->sections_n=0;

            if(parse_info.parseField())
            {
                parse_info.readStr(&s1b,&s1e);
                strtolower(s1b,s1e-s1b);
                strs[cur_manuf->sections_n++]=text_ind.t_memcpyz(s1b,s1e-s1b);
                while(1)
                {
                    if(cur_manuf->sections_n>1)
                        wsprintfA(secttry,"%s.%s",
                                text_ind.get(strs[0]),
                                text_ind.get(strs[cur_manuf->sections_n-1]));
                    else
                        wsprintfA(secttry,"%s",text_ind.get(strs[0]));

                    strtolower(secttry,strlen(secttry));

                    auto range2=section_list.equal_range(secttry);
                    if(range2.first==range2.second)log_index("ERROR: missing [%s] in %S\n",secttry,inffull);
                    for(auto got2=range2.first;got2!=range2.second;++got2)
                    {
                        sect_data_t *lnk2=&got2->second;
                        parse_info2.setRange(lnk2);
                        while(parse_info2.parseItem())
                        {
                            parse_info2.readStr(&s1b,&s1e);
                            int desc_c=text_ind.memcpyz_dup(s1b,s1e-s1b);

                            parse_info2.parseField();
                            parse_info2.readStr(&s1b,&s1e);
                            int inst_c=text_ind.memcpyz_dup(s1b,s1e-s1b);

                            //{ featurescore and install section
                            int feature_c=0xFF,install_picket_c;

                            char installsection[BUFLEN];
                            sect_data_t *lnk3;

                            memcpy(installsection,s1b,s1e-s1b);installsection[s1e-s1b]=0;
                            strcat(installsection,".nt");
                            strtolower(installsection,strlen(installsection));
                            auto range3=section_list.equal_range(installsection);
                            if(range3.first==range3.second)
                            {
                                memcpy(installsection,s1b,s1e-s1b);installsection[s1e-s1b]=0;
                                strtolower(installsection,strlen(installsection));
                                range3=section_list.equal_range(installsection);
                            }
                            if(range3.first==range3.second)
                            {
                                if(cur_manuf->sections_n>1)
                                {
                                        memcpy(installsection,s1b,s1e-s1b);installsection[s1e-s1b]=0;
                                        strcat(installsection,".");strcat(installsection,text_ind.get(strs[cur_manuf->sections_n-1]));
                                }
                                else
                                {
                                    memcpy(installsection,s1b,s1e-s1b);installsection[s1e-s1b]=0;
                                }

                                strtolower(installsection,strlen(installsection));
                                while(strlen(installsection)>=(unsigned)(s1e-s1b))
                                {
                                    range3=section_list.equal_range(installsection);
                                    if(range3.first!=range3.second)break;
                                    //log_file("Tried '%s'\n",installsection);
                                    installsection[strlen(installsection)-1]=0;
                                }
                            }
                            char iii[BUFLEN];
                            *iii=0;
                            //int cnt=0;
                            if(range3.first==range3.second)
                            {
                                int i;
                                for(i=0;i<NUM_DECS;i++)
                                {
                                    //sprintf(installsection,"%.*s.%s",s1e-s1b,s1b,nts[i]);
                                    memcpy(installsection,s1b,s1e-s1b);installsection[s1e-s1b]=0;
                                    strcat(installsection,".");strcat(installsection,nts[i]);
                                    strtolower(installsection,strlen(installsection));
                                    auto range4=section_list.equal_range(installsection);
                                    if(range4.first!=range4.second)
                                    {
                                        //lnk3=tlnk;
                                        range3=range4;
                                        strcat(iii,installsection);
                                        strcat(iii,",");
                                    }
                                    //if(lnk3){log_file("Found '%s'\n",installsection);cnt++;}
                                }
                            }
                            //if(cnt>1)log_file("@num: %d\n",cnt);
                            //if(cnt>1&&!lnk3)log_file("ERROR in %S%S:\t\t\tMissing [%s]\n",drpdir,inffilename,iii);
                            if(range3.first!=range3.second)
                            {
                                if(*iii)wsprintfA(installsection,"$%s",iii);
                                install_picket_c=text_ind.memcpyz_dup(installsection,strlen(installsection));
                            }
                            else
                            {
                                install_picket_c=text_ind.memcpyz_dup("{missing}",9);
                            }

                            for(auto got3=range3.first;got3!=range3.second;++got3)
                            {
                                lnk3=&got3->second;
                                parse_info3.setRange(lnk3);
                                if(!strcmp(secttry,installsection))
                                {
                                    log_index("ERROR: [%s] refers to itself in %S\n",installsection,inffull);
                                    break;
                                }

                                while(parse_info3.parseItem())
                                {
                                    parse_info3.readStr(&s1b,&s1e);
                                    strtolower(s1b,s1e-s1b);
                                    int sz=s1e-s1b;
                                    if(sz==12&&!memcmp(s1b,"featurescore",sz))
                                    {
                                        parse_info3.parseField();
                                        feature_c=parse_info3.readHex();
                                    }
                                    while(parse_info3.parseField());
                                }
                            }
                            //} feature and install_picked section

                            cur_desc_index=desc_list.size();

                            desc_list.push_back(data_desc_t(cur_manuf_index,
                                manufacturer_list[cur_manuf_index].sections_n-1,
                                desc_c,inst_c,install_picket_c,feature_c));

                            int hwid_pos=0;
                            while(parse_info2.parseField())
                            {
                                parse_info2.readStr(&s1b,&s1e);
                                if(s1b>=s1e)continue;
                                strtoupper(s1b,s1e-s1b);

                                HWID_list.push_back(data_HWID_t(cur_desc_index,hwid_pos++,text_ind.memcpyz_dup(s1b,s1e-s1b)));
                            }
                        }
                    }

                    if(!parse_info.parseField())break;
                    parse_info.readStr(&s1b,&s1e);
                    if(s1b>s1e)break;
                    strtolower(s1b,s1e-s1b);
                    strs[cur_manuf->sections_n++]=text_ind.t_memcpyz(s1b,s1e-s1b);
                }
            }
            cur_manuf->sections=text_ind.t_memcpyz((char *)strs,sizeof(int)*cur_manuf->sections_n);
        }
    }
}

void Driverpack::getdrp_drvsectionAtPos(char *buf,int pos,int manuf_index)
{
    int *rr=reinterpret_cast<int *>(text_ind.get(manufacturer_list[manuf_index].sections));
    if(pos)
    {
        strcpy(buf,text_ind.get(rr[0]));
        strcat(buf,".");
        strcat(buf,text_ind.get(rr[pos]));
    }
    else
        strcpy(buf,text_ind.get(rr[pos]));
}

Driverpack::Driverpack(wchar_t const *driverpack_path,wchar_t const *driverpack_filename,Collection *col_v)
{
    col=col_v;
    drppath=text_ind.strcpyw(driverpack_path);
    drpfilename=text_ind.strcpyw(driverpack_filename);
    indexes.reset(0);
    type=DRIVERPACK_TYPE_PENDING_SAVE;
}

unsigned int __stdcall Driverpack::loaddrp_thread(void *arg)
{
    drplist_t *drplist=reinterpret_cast<drplist_t *>(arg);
    driverpack_task data;

    while(1)
    {
        drplist->wait_and_pop(data);
        if(data.drp==nullptr)break;

        Driverpack *drp=data.drp;
        if(flags&COLLECTION_FORCE_REINDEXING||!drp->loadindex())
        {
            drp->objs_new=new concurrent_queue<inffile_task>;
            queuedriverpack_p->push(driverpack_task{drp});
            drp->genindex();
            drp->driverpack_indexinf_async(L"",L"",nullptr,0);
        }
    }
    return 0;
}

unsigned int __stdcall Driverpack::indexinf_thread(void *arg)
{
    drplist_t *drplist=reinterpret_cast<drplist_t *>(arg);
    inffile_task t;
    driverpack_task data;
    long long tm=0,last=0;

    while(1)
    {
        drplist->wait_and_pop(data);
        if(!data.drp)break;

        wchar_t bufw2[BUFLEN];
        if(!drp_count)drp_count=1;
        wsprintf(bufw2,L"%s\\%s",data.drp->getPath(),data.drp->getFilename());
        manager_g->itembar_settext(SLOT_INDEXING,1,bufw2,drp_cur,drp_count,(drp_cur)*1000/drp_count);
        drp_cur++;

        //log_con("Str %ws\n",data.drp->getFilename());
        while(1)
        {
            data.drp->objs_new->wait_and_pop(t);
            if(last)tm+=GetTickCount()-last;
            if(!t.adr)
            {
                t.drp->genhashes();
                t.drp->text_ind.shrink();
                last=GetTickCount();
                //log_con("Trm %ws\n",data.drp->getFilename());
                delete data.drp->objs_new;
                break;
            }
            if(StrStrIW(t.inffile,L".inf"))
                t.drp->indexinf_ansi(t.pathinf,t.inffile,t.adr,t.len);
            else
                t.drp->parsecat(t.pathinf,t.inffile,t.adr,t.len);

            delete[] t.pathinf;
            delete[] t.inffile;
            delete[] t.adr;
            last=GetTickCount();
        }
        //log_con("Fin %ws\n",data.drp->getFilename());
    }
    //log_con("Starved for %ld\n",tm);
    return 0;
}

unsigned int __stdcall Driverpack::savedrp_thread(void *arg)
{
    drplist_t *drplist=reinterpret_cast<drplist_t *>(arg);
    driverpack_task data;

    while(1)
    {
        drplist->wait_and_pop(data);
        if(!data.drp)break;

        wchar_t bufw2[BUFLEN];
        wsprintf(bufw2,L"%ws\\%ws",data.drp->getPath(),data.drp->getFilename());
        log_con("Saving indexes for '%S'\n",bufw2);
        if(flags&COLLECTION_USE_LZMA)manager_g->itembar_settext(SLOT_INDEXING,2,bufw2,cur_,count_);
        cur_++;
        data.drp->saveindex();
    }
    return 0;
}

int Driverpack::checkindex()
{
    if(*drpext_dir)return 0;

    wchar_t filename[BUFLEN];
    getindexfilename(col->getIndex_bin_dir(),L"bin",filename);
    FILE *f=_wfopen(filename,L"rb");
    if(!f)return 0;

    fseek(f,0,SEEK_END);
    int sz=ftell(f);
    fseek(f,0,SEEK_SET);

    char buf[3];
    int version;
    fread(buf,3,1,f);
    fread(&version,sizeof(int),1,f);
    sz-=3+sizeof(int);
    fclose(f);

    if(memcmp(buf,"SDW",3)||version!=VER_INDEX)if(version!=0x204)return 0;

    return 1;
}

int Driverpack::loadindex()
{
    wchar_t filename[BUFLEN];
    CHAR buf[3];
    FILE *f;
    int sz;
    int version;
    char *mem,*p,*mem_unpack=nullptr;

    getindexfilename(col->getIndex_bin_dir(),L"bin",filename);
    f=_wfopen(filename,L"rb");
    if(!f)return 0;

    fseek(f,0,SEEK_END);
    sz=ftell(f);
    fseek(f,0,SEEK_SET);

    fread(buf,3,1,f);
    fread(&version,sizeof(int),1,f);
    sz-=3+sizeof(int);

    if(memcmp(buf,"SDW",3)||version!=VER_INDEX)if(version!=0x204)return 0;
    if(*drpext_dir)return 0;

    p=mem=new char[sz];
    log_con("");// A fix for a compiler bug
    fread(mem,sz,1,f);

    if(flags&COLLECTION_USE_LZMA)
    {
        UInt64 sz_unpack;

        Lzma86_GetUnpackSize((Byte *)p,sz,&sz_unpack);
        mem_unpack=new char[sz_unpack];
        decode(mem_unpack,sz_unpack,mem,sz);
        p=mem_unpack;
    }

    p=vector_load(&inffile,p);
    p=vector_load(&manufacturer_list,p);
    p=vector_load(&desc_list,p);
    p=vector_load(&HWID_list,p);
    p=vector_load(text_ind.getVector(),p);
    p=indexes.load(p);

    delete[] mem;
    if(mem_unpack)delete[] mem_unpack;
    fclose(f);
    text_ind.shrink();

    type=StrStrIW(filename,L"\\_")?DRIVERPACK_TYPE_UPDATE:DRIVERPACK_TYPE_INDEXED;
    return 1;
}

void Driverpack::saveindex()
{
    wchar_t filename[BUFLEN];
    FILE *f;
    int sz;
    int version=VER_INDEX;
    char *mem,*p,*mem_pack;

    getindexfilename(col->getIndex_bin_dir(),L"bin",filename);
    if(!canWrite(filename))
    {
        log_err("ERROR in driverpack_saveindex(): Write-protected,'%S'\n",filename);
        return;
    }
    f=_wfopen(filename,L"wb");

    sz=
        inffile.size()*sizeof(data_inffile_t)+
        manufacturer_list.size()*sizeof(data_manufacturer_t)+
        desc_list.size()*sizeof(data_desc_t)+
        HWID_list.size()*sizeof(data_HWID_t)+
        text_ind.getSize()+
        indexes.getSize()*sizeof(Hashitem)+sizeof(int)+
        6*sizeof(int)*2;

    p=mem=new char[sz];
    fwrite("SDW",3,1,f);
    fwrite(&version,sizeof(int),1,f);

    p=vector_save(&inffile,p);
    p=vector_save(&manufacturer_list,p);
    p=vector_save(&desc_list,p);
    p=vector_save(&HWID_list,p);
    p=vector_save(text_ind.getVector(),p);
    p=indexes.save(p);

    if(flags&COLLECTION_USE_LZMA)
    {
        mem_pack=new char[sz];
        sz=encode(mem_pack,sz,mem,sz);
        fwrite(mem_pack,sz,1,f);
        delete[] mem_pack;
    }
    else fwrite(mem,sz,1,f);

    delete[] mem;
    fclose(f);
    type=DRIVERPACK_TYPE_INDEXED;
}

void Driverpack::genhashes()
{
    // Driver signatures
    for(auto &it:inffile)
    {
        char filename[BUFLEN];
        strcpy(filename,text_ind.get(it.infpath));
        char *field=filename+strlen(filename);

        for(int j=CatalogFile;j<=CatalogFile_ntamd64;j++)if(it.fields[j])
        {
            strcpy(field,text_ind.get(it.fields[j]));
            strtolower(filename,strlen(filename));

            auto got=cat_list.find(filename);
            if(got!=cat_list.end())it.cats[j]=got->second;
        }
    }

    // Hashtable for fast search
    indexes.reset(HWID_list.size()/2);
    for(unsigned i=0;i<HWID_list.size();i++)
    {
        char *vv=text_ind.get(HWID_list[i].HWID);
        int val=indexes.gethashcode(vv,strlen(vv));
        indexes.additem(val,i);
    }
}

int Driverpack::printstats()
{
    int sum=0;

    log_file("  %6d  %S\\%S\n",HWID_list.size(),getPath(),getFilename());
    sum+=HWID_list.size();
    return sum;
}

void Driverpack::print_index_hr()
{
    int pos;
    unsigned inffile_index,manuf_index,HWID_index,desc_index;
    unsigned n=inffile.size();
    Version *t;
    data_inffile_t *d_i;
    Hwidmatch hwidmatch(this,0);
    char buf[BUFLEN];
    wchar_t filename[BUFLEN];
    FILE *f;
    int cnts[NUM_DECS],plain;
    unsigned HWID_index_last=0;
    unsigned manuf_index_last=0;
    int i;

    getindexfilename(col->getIndex_linear_dir(),L"txt",filename);
    f=_wfopen(filename,L"wt");

    log_con("Saving %S\n",filename);
    fwprintf(f,L"%s\\%s (%d inf files)\n",getPath(),getFilename(),n);
    for(inffile_index=0;inffile_index<n;inffile_index++)
    {
        d_i=&inffile[inffile_index];
        fprintf(f,"  %s%s (%d bytes)\n",text_ind.get(d_i->infpath),text_ind.get(d_i->inffilename),d_i->infsize);
        for(i=0;i<(int)n;i++)if(i!=(int)inffile_index&&d_i->infcrc==inffile[i].infcrc)
        fprintf(f,"**%s%s\n",text_ind.get(inffile[i].infpath),text_ind.get(inffile[i].inffilename));
        t=&d_i->version;
        fprintf(f,"    date\t\t\t%d/%d/%d\n",t->d,t->m,t->y);
        fprintf(f,"    version\t\t\t%d.%d.%d.%d\n",t->v1,t->v2,t->v3,t->v4);
        for(i=0;i<NUM_VER_NAMES;i++)
            if(d_i->fields[i])
            {
                fprintf(f,"    %-28s%s\n",table_version[i].s,text_ind.get(d_i->fields[i]));
                if(d_i->cats[i])fprintf(f,"      %s\n",text_ind.get(d_i->cats[i]));

            }

        memset(cnts,-1,sizeof(cnts));plain=0;
        for(manuf_index=manuf_index_last;manuf_index<manufacturer_list.size();manuf_index++)
            if(manufacturer_list[manuf_index].inffile_index==inffile_index)
        {
            manuf_index_last=manuf_index;
            //hwidmatch.HWID_index=HWID_index_last;
            if(manufacturer_list[manuf_index].manufacturer)
                fprintf(f,"      {%s}\n",text_ind.get(manufacturer_list[manuf_index].manufacturer));
            for(pos=0;pos<manufacturer_list[manuf_index].sections_n;pos++)
            {
                getdrp_drvsectionAtPos(buf,pos,manuf_index);
                i=calc_secttype(buf);
                if(i>=0&&cnts[i]<0)cnts[i]=0;
                if(i<0&&pos>0)fprintf(f,"!!![%s]\n",buf);
                fprintf(f,"        [%s]\n",buf);

                for(desc_index=0;desc_index<desc_list.size();desc_index++)
                    if(desc_list[desc_index].manufacturer_index==manuf_index&&
                       desc_list[desc_index].sect_pos==pos)
                {
                    for(HWID_index=HWID_index_last;HWID_index<HWID_list.size();HWID_index++)
                        if(HWID_list[HWID_index].desc_index==desc_index)
                    {
                        if(HWID_index_last+1!=HWID_index&&HWID_index)fprintf(f,"Skip:%d,%d\n",HWID_index_last,HWID_index);
                        HWID_index_last=HWID_index;
                        hwidmatch.setHWID_index(HWID_index_last);

                //if(text+manufacturer_list[manuf_index].manufacturer!=get_manufacturer(&hwidmatch))
                //fprintf(f,"*%s\n",get_manufacturer(&hwidmatch));
                //get_section(&hwidmatch,buf+500);
                //fprintf(f,"*%s,%s\n",buf+1000,buf+500);
                        if(i>=0)cnts[i]++;
                        if(pos==0&&i<0)plain++;

                        if(hwidmatch.getdrp_drvinfpos())
                            wsprintfA(buf,"%-2d",hwidmatch.getdrp_drvinfpos());
                        else
                            wsprintfA(buf,"  ");

                        fprintf(f,"       %s %-50s%-20s\t%s\n",buf,
                            hwidmatch.getdrp_drvHWID(),
                            hwidmatch.getdrp_drvinstall(),
                            hwidmatch.getdrp_drvdesc());
                        fprintf(f,"          feature:%-42hX%-20s\n\n",
                            hwidmatch.getdrp_drvfeature()&0xFF,
                            hwidmatch.getdrp_drvinstallPicked());
                    }
                    else if(HWID_index!=HWID_index_last)break;
                }
            }
        }
        else if(manuf_index!=manuf_index_last)break;

        fprintf(f,"  Decors:\n");
        fprintf(f,"    %-15s%d\n","plain",plain);
        for(i=0;i<NUM_DECS;i++)
        {
            if(cnts[i]>=0)fprintf(f,"    %-15s%d\n",nts[i],cnts[i]);
        }
        fprintf(f,"\n");
    }
    fprintf(f,"  HWIDS:%d/%d\n",HWID_index_last+1,(int)HWID_list.size());
    fclose(f);
}

void Driverpack::fillinfo(char *sect,char *hwid,unsigned start_index,int *inf_pos,ofst *cat,int *catalogfile,int *feature)
{
    *inf_pos=-1;
    //log_file("Search[%s,%s,%d]\n",sect,hwid,start_index);
    for(unsigned HWID_index=start_index;HWID_index<HWID_list.size();HWID_index++)
    {
        if(!strcmpi(text_ind.get(HWID_list[HWID_index].getHWID()),hwid))
        {
            Hwidmatch hwidmatch(this,HWID_index);
            if(!strcmpi(hwidmatch.getdrp_drvinstallPicked(),sect)||
               StrStrIA(hwidmatch.getdrp_drvinstall(),sect))
            {
                if(*inf_pos<0||*inf_pos>hwidmatch.getdrp_drvinfpos())
                {
                    *feature=hwidmatch.getdrp_drvfeature();
                    *catalogfile=hwidmatch.calc_catalogfile();
                    *inf_pos=hwidmatch.getdrp_drvinfpos();
                }
                //log_file("Sect %s, %d, %d, %d (%d),%s\n",sect,*catalogfile,*feature,*inf_pos,HWID_index,hwidmatch.getdrp_drvinstallPicked());
            }
        }
    }
    if(*inf_pos==-1)
    {
        *inf_pos=0;
        *cat=0;
        *feature=0xFF;
        log_err("ERROR: sect not found '%s'\n",sect);
    }
}

void Driverpack::getindexfilename(const wchar_t *dir,const wchar_t *ext,wchar_t *indfile)
{
    wchar_t *p;
    wchar_t buf[BUFLEN];
    int len=wcslen(getFilename());

    wsprintf(buf,L"%s",getFilename());

    if(*(getPath()))
        wsprintf(buf+(len-3)*1,L"%s.%s",getPath()+lstrlenW(col->getDriverpack_dir()),ext);
    else
        wsprintf(buf+(len-3)*1,L".%s",ext);

    p=buf;
    while(*p){if(*p==L'\\'||*p==L' ')*p=L'_';p++;}
    wsprintf(indfile,L"%s\\%s",dir,buf);
}

void Driverpack::parsecat(wchar_t const *pathinf,wchar_t const *inffilename,char *adr,int len)
{
    CHAR bufa[BUFLEN];

    findosattr(bufa,adr,len);
    if(*bufa)
    {
        CHAR filename[BUFLEN];
        wsprintfA(filename,"%ws%ws",pathinf,inffilename);
        strtolower(filename,strlen(filename));
        cat_list.insert({filename,text_ind.memcpyz_dup(bufa,strlen(bufa))});
        //log_con("(%s)\n##%s\n",filename,bufa);
    }
    else
    {
        log_con("Not found singature in '%ws%ws'(%d)\n",pathinf,inffilename,len);
    }

}

void Driverpack::indexinf(wchar_t const *drpdir,wchar_t const *iinfdilename,char *bb,int inf_len)
{
    if(inf_len>4&&((bb[0]==-1&&bb[3]==0)||bb[0]==0))
    {
        int size=inf_len;

        char *buf_out=new char[size+2];
        if(!buf_out)
        {
            log_err("ERROR in driverpack_indexinf: malloc(%d)\n",size+2);
            return;
        }
        size=unicode2ansi(bb,buf_out,size);
        indexinf_ansi(drpdir,iinfdilename,buf_out,size);
        delete[] buf_out;
    }
    else
    {
        indexinf_ansi(drpdir,iinfdilename,bb,inf_len);
    }
}
//}
