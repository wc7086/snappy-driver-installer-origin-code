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

#define INDEXING_H
#include "main.h"

//{ Global variables
int drp_count;
int drp_cur;
int loaded_unpacked=0;
int volatile cur_,count_;

class frm
{
public:
    Driverpack *drp;
    frm(Driverpack *a):drp(a){}
    frm():drp(nullptr){}
};

typedef boost::lockfree::queue<frm> drplist_t;
drplist_t queuedriverpack1(100);

typedef struct _tbl_t
{
    const char *s;
    int sz;
}tbl_t;
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
};
//}

//{ Version
int version_t::setDate(int d_,int m_,int y_)
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

void version_t::setVersion(int v1_,int v2_,int v3_,int v4_)
{
    v1=v1_;
    v2=v2_;
    v3=v3_;
    v4=v4_;
}

void version_t::str_date(wchar_t *buf)
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
        GetDateFormat(manager_g->matcher->getState()->locale,0,&tm,nullptr,buf,100);
}

void version_t::str_version(wchar_t *buf)
{
    if(v1<0)
        wsprintf(buf,STR(STR_HINT_UNKNOWN));
    else
        wsprintf(buf,L"%d.%d.%d.%d",v1,v2,v3,v4);
}

wchar_t unkver[128];
const wchar_t *str_version(version_t *ver)
{

    wsprintf(unkver,L"%s%s",STR(STR_HINT_VERSION),STR(STR_HINT_UNKNOWN));
    return ver->v1<0?unkver:L"%s%d.%d.%d.%d";
}

int cmpdate(version_t *t1,version_t *t2)
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

int cmpversion(version_t *t1,version_t *t2)
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

int Parser::readDate(version_t *t)
{

    while(strBeg<strEnd&&!(*strBeg>='0'&&*strBeg<='9'))strBeg++;
    int m=readNumber();
    int d=readNumber();
    int y=readNumber();
    return t->setDate(d,m,y);

}

void Parser::readVersion(version_t *t)
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

Parser::Parser(sect_data_t *lnk,Driverpack *drpv,std::unordered_map<std::string,std::string> &string_listv,const wchar_t *inf)
{
    blockBeg=lnk->blockbeg;
    blockEnd=lnk->blockend;
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
//}

//{ Collection
void Collection::updatedir()
{
    driverpack_dir=*drpext_dir?drpext_dir:drp_dir;
    populate();
}

Collection::Collection(wchar_t *driverpacks_dirv,const wchar_t *index_bin_dirv,const wchar_t *index_linear_dirv)
{
    driverpack_dir=driverpacks_dirv;
    index_bin_dir=index_bin_dirv;
    index_linear_dir=index_linear_dirv;
}


void Collection::init(wchar_t *driverpacks_dirv,const wchar_t *index_bin_dirv,const wchar_t *index_linear_dirv)
{
    driverpack_dir=driverpacks_dirv;
    index_bin_dir=index_bin_dirv;
    index_linear_dir=index_linear_dirv;
}

unsigned int __stdcall Collection::savedrp_thread(void *arg)
{
    drplist_t *drplist=reinterpret_cast<drplist_t *>(arg);
    frm data;
    int exit=0;

    while(!exit)
    {
        while(drplist->pop(data))
        {
            if(!data.drp){exit=1;break;}
            wchar_t bufw2[BUFLEN];
            wsprintf(bufw2,L"%ws\\%ws",data.drp->getPath(),data.drp->getFilename());
            log_con("Saving indexes for '%S'\n",bufw2);
            if(flags&COLLECTION_USE_LZMA)itembar_settext(SLOT_INDEXING,2,bufw2,cur_,count_);
            cur_++;
            data.drp->saveindex();
        }
    }
    return 0;
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
        driverpack_list[0].type=DRIVERPACK_TYPE_INDEXED;
    for(auto &driverpack:driverpack_list)
        if(driverpack.type==DRIVERPACK_TYPE_PENDING_SAVE)count_++;

    log_con("Saving indexes...\n");
    HANDLE thr[16];
    drplist_t queuedriverpack_loc(100);
    for(int i=0;i<num_cores;i++)
        thr[i]=(HANDLE)_beginthreadex(nullptr,0,&savedrp_thread,&queuedriverpack_loc,0,nullptr);
    for(auto &driverpack:driverpack_list)
    if(driverpack.type==DRIVERPACK_TYPE_PENDING_SAVE)
        queuedriverpack_loc.push(frm{&driverpack});

    for(int i=0;i<num_cores;i++)queuedriverpack_loc.push(frm{nullptr});
    for(int i=0;i<num_cores;i++)
    {
        WaitForSingleObject(thr[i],INFINITE);
        CloseHandle_log(thr[i],L"driverpack_genindex",L"thr");
    }
    itembar_settext(SLOT_INDEXING,0);
    log_con("DONE\n");

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
            if(StrStrIW(FindFileData.cFileName,L"\\_"))continue;
            wsprintf(filename,L"%s\\%s",index_bin_dir,FindFileData.cFileName);
            unsigned i;
            for(i=flags&FLAG_KEEPUNPACKINDEX?0:1;i<driverpack_list.size();i++)
            {
                driverpack_list[i].getindexfilename(index_bin_dir,L"bin",buf);
                if(!wcscmp(buf,filename))break;
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
            int i,len=lstrlen(FindFileData.cFileName);
            for(i=0;i<5;i++)
            if(StrStrIW(FindFileData.cFileName,olddrps[i]))
            {
                wsprintf(buf,L" /c del \"%s\\%s*.7z\" /Q /F",driverpack_dir,olddrps[i]);
                run_command(L"cmd",buf,SW_HIDE,1);
                break;
            }
            if(i==5&&StrCmpIW(FindFileData.cFileName+len-3,L".7z")==0)
            {
                Driverpack drp{path,FindFileData.cFileName,this};
                if(flags&COLLECTION_FORCE_REINDEXING||!drp.checkindex())cnt++;
            }
        }
    }
    FindClose(hFind);
    return cnt;
}

unsigned int __stdcall Collection::loaddrp_thread(void *arg)
{
    drplist_t *drplist=reinterpret_cast<drplist_t *>(arg);
    frm data;
    int exit=0;
    while(!exit)
    {
        while(drplist->pop(data))
        {
            Driverpack *drp=data.drp;
            if(data.drp==nullptr){exit=1;break;}
                if(flags&COLLECTION_FORCE_REINDEXING||!drp->loadindex())
                {
#ifndef NDEBUG
                    dr->objs=new boost::lockfree::queue<obj>(100);
#else
                    drp->objs=new boost::lockfree::queue<obj>;
#endif
                    queuedriverpack1.push(frm{drp});
                    drp->genindex();
                    driverpack_indexinf_async(drp,L"",L"",nullptr,0);
                }
        }
    }
    return 0;
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

void Collection::populate()
{
    Driverpack *unpacked_drp;

    time_indexes=GetTickCount();

    drp_count=scanfolder_count(driverpack_dir);
    driverpack_list.clear();
    driverpack_list.reserve(drp_count+1+100); // TODO

    registerall();
    driverpack_list.push_back(Driverpack(driverpack_dir,L"unpacked.7z",this));
    unpacked_drp=&driverpack_list.back();

//{thread
    int num_thr=num_cores;
    int num_thr_1=num_cores;
    if(drp_count&&num_thr>3)num_thr=3;
    log_con("Cores: %d\n",num_cores);

    HANDLE thr[16],cons[16];
    for(int i=0;i<num_thr_1;i++)
        thr[i]=(HANDLE)_beginthreadex(nullptr,0,&Driverpack::thread_indexinf,&queuedriverpack1,0,nullptr);

    drplist_t queuedriverpack(100);
    for(int i=0;i<num_thr;i++)
        cons[i]=(HANDLE)_beginthreadex(nullptr,0,&loaddrp_thread,&queuedriverpack,0,nullptr);
//}thread

    if(flags&FLAG_KEEPUNPACKINDEX)loaded_unpacked=unpacked_drp->loadindex();
    drp_cur=1;

    scanfolder(driverpack_dir,&queuedriverpack);
    for(int i=0;i<num_thr;i++)queuedriverpack.push(frm{nullptr});

    for(int i=0;i<num_thr;i++)
    {
        WaitForSingleObject(cons[i],INFINITE);
        CloseHandle_log(cons[i],L"driverpack_genindex",L"cons");
    }

    loadOnlineIndexes();
    manager_g->items_list[SLOT_INDEXING].isactive=0;
    if(driverpack_list.size()<=1&&(flags&FLAG_DPINSTMODE)==0)
        itembar_settext(manager_g,SLOT_NODRIVERS,L"",0);
    driverpack_list[0].genhashes();

//{thread
    for(int i=0;i<num_thr_1;i++)queuedriverpack1.push(frm{nullptr});

    for(int i=0;i<num_thr_1;i++)
    {
        WaitForSingleObject(thr[i],INFINITE);
        CloseHandle_log(thr[i],L"driverpack_genindex",L"thr");
    }
//}thread
    time_indexes=GetTickCount()-time_indexes;
    log_con("DONE Indexes\n");
    flags&=~COLLECTION_FORCE_REINDEXING;
    driverpack_list.shrink_to_fit();
}

void Collection::print_index_hr()
{
    time_indexprint=GetTickCount();

    for(auto &drp:driverpack_list)drp.print_index_hr();

    time_indexprint=GetTickCount()-time_indexprint;
}

void Collection::printstates()
{
    if((log_verbose&LOG_VERBOSE_DRP)==0)return;

    int sum=0,sizetx=0,sizeind=0;
    int num=0;
    log_file("Driverpacks\n");
    for(auto &drp:driverpack_list)
    {
        log_file("  %6d  %S\\%S\n",drp.HWID_list.size(),drp.getPath(),drp.getFilename());
        sum+=drp.HWID_list.size();
        sizetx+=drp.texta.getSize();

        num+=drp.cat_list.size()*sizeof(drp.cat_list);
        num+=drp.inffile.size()*sizeof(drp.inffile);
        num+=drp.manufacturer_list.size()*sizeof(drp.manufacturer_list);
        num+=drp.desc_list.size()*sizeof(drp.desc_list);
        num+=drp.HWID_list.size()*sizeof(drp.HWID_list);

        sizeind+=drp.indexesold.getSize()*sizeof(Hashitem);
    }
    log_file("  Sum: %d\n\n",sum);
    log_con("  Size: %d+%d*%d+%d[text]+%d[obj]+%d[ind]\n",sizeof(Collection),driverpack_list.size(),sizeof(Driverpack),sizetx,num,sizeind);
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
        if(StrStrIW(s,fnd)&&drp.type!=DRIVERPACK_TYPE_UPDATE)
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

void Collection::scanfolder(const wchar_t *path,void *arg)
{
    WIN32_FIND_DATA FindFileData;
    wchar_t buf[BUFLEN];

    wsprintf(buf,L"%s\\*.*",path);
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
            int len=lstrlen(FindFileData.cFileName);
            if(StrCmpIW(FindFileData.cFileName+len-3,L".7z")==0)
            {
                driverpack_list.push_back(Driverpack(path,FindFileData.cFileName,this));
                reinterpret_cast<drplist_t *>(arg)->push(frm{&driverpack_list.back()});
            }else
            if((StrCmpIW(FindFileData.cFileName+len-4,L".inf")==0||
               StrCmpIW(FindFileData.cFileName+len-4,L".cat")==0)&&loaded_unpacked==0)
            {
                FILE *f;
                char *buft;
                wsprintf(buf,L"%s\\%s",path,FindFileData.cFileName);
                f=_wfopen(buf,L"rb");
                fseek(f,0,SEEK_END);
                len=ftell(f);
                fseek(f,0,SEEK_SET);
                buft=(char *)malloc(len);
                fread(buft,len,1,f);
                fclose(f);
                wsprintf(buf,L"%s\\",path+wcslen(driverpack_dir)+1);

                if(StrCmpIW(FindFileData.cFileName+len-4,L".inf")==0)
                    driverpack_list[0].indexinf(buf,FindFileData.cFileName,buft,len);
                else
                    driverpack_list[0].parsecat(buf,FindFileData.cFileName,buft,len);

                free(buft);
            }
        }
    }
    FindClose(hFind);
}
//}

//{ Driverpack
Driverpack::Driverpack(wchar_t const *driverpack_path,wchar_t const *driverpack_filename,Collection *col_v)
{
    col=col_v;
    drppath=texta.strcpyw(driverpack_path);
    drpfilename=texta.strcpyw(driverpack_filename);
    indexesold.reset(0);
    type=DRIVERPACK_TYPE_PENDING_SAVE;
}

Driverpack::~Driverpack()
{

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
        texta.getSize()+
        indexesold.getSize()*sizeof(Hashitem)+sizeof(int)+
        6*sizeof(int)*2;

    p=mem=(char *)malloc(sz);
    fwrite("SDW",3,1,f);
    fwrite(&version,sizeof(int),1,f);

    p=vector_save(&inffile,p);
    p=vector_save(&manufacturer_list,p);
    p=vector_save(&desc_list,p);
    p=vector_save(&HWID_list,p);
    p=vector_save(texta.getVector(),p);
    p=indexesold.save(p);

    if(flags&COLLECTION_USE_LZMA)
    {
        mem_pack=(char *)malloc(sz);
        sz=encode(mem_pack,sz,mem,sz);
        fwrite(mem_pack,sz,1,f);
        free(mem_pack);
    }
    else fwrite(mem,sz,1,f);

    free(mem);
    fclose(f);
}

int Driverpack::checkindex()
{
    wchar_t filename[BUFLEN];
    CHAR buf[3];
    FILE *f;
    int sz;
    int version;

    getindexfilename(col->getIndex_bin_dir(),L"bin",filename);
    f=_wfopen(filename,L"rb");
    if(!f)return 0;

    fseek(f,0,SEEK_END);
    sz=ftell(f);
    fseek(f,0,SEEK_SET);

    fread(buf,3,1,f);
    fread(&version,sizeof(int),1,f);
    sz-=3+sizeof(int);

    if(memcmp(buf,"SDW",3)||version!=VER_INDEX)return 0;
    if(*drpext_dir)return 0;

    fclose(f);
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

    if(memcmp(buf,"SDW",3)||version!=VER_INDEX)return 0;
    if(*drpext_dir)return 0;

    p=mem=(char *)malloc(sz);
    fread(mem,sz,1,f);

    if(flags&COLLECTION_USE_LZMA)
    {
        UInt64 sz_unpack;

        Lzma86_GetUnpackSize((Byte *)p,sz,&sz_unpack);
        mem_unpack=(char *)malloc(sz_unpack);
        decode(mem_unpack,sz_unpack,mem,sz);
        p=mem_unpack;
    }

    p=vector_load(&inffile,p);
    p=vector_load(&manufacturer_list,p);
    p=vector_load(&desc_list,p);
    p=vector_load(&HWID_list,p);
    p=vector_load(texta.getVector(),p);
    p=indexesold.load(p);

    free(mem);
    if(mem_unpack)free(mem_unpack);
    fclose(f);
    texta.shrink();

    type=StrStrIW(filename,L"\\_")?DRIVERPACK_TYPE_UPDATE:DRIVERPACK_TYPE_INDEXED;
    return 1;
}

void Driverpack::getindexfilename(const wchar_t *dir,const wchar_t *ext,wchar_t *indfile)
{
    wchar_t *p;
    wchar_t buf[BUFLEN];
    int len=wcslen(getFilename());

    wsprintf(buf,L"%s",getFilename());

    if(*(getPath()))
        wsprintf(buf+(len-3)*1,L"%s.%s",getPath()+lstrlen(col->getDriverpack_dir()),ext);
    else
        wsprintf(buf+(len-3)*1,L".%s",ext);

    p=buf;
    while(*p){if(*p==L'\\'||*p==L' ')*p=L'_';p++;}
    wsprintf(indfile,L"%s\\%s",dir,buf);
}

void Driverpack::print_index_hr()
{
    int pos;
    unsigned inffile_index,manuf_index,HWID_index,desc_index;
    unsigned n=inffile.size();
    version_t *t;
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
        fprintf(f,"  %s%s (%d bytes)\n",texta.get(d_i->infpath),texta.get(d_i->inffilename),d_i->infsize);
        for(i=0;i<(int)n;i++)if(i!=(int)inffile_index&&d_i->infcrc==inffile[i].infcrc)
        fprintf(f,"**%s%s\n",texta.get(inffile[i].infpath),texta.get(inffile[i].inffilename));
        t=&d_i->version;
        fprintf(f,"    date\t\t\t%d/%d/%d\n",t->d,t->m,t->y);
        fprintf(f,"    version\t\t\t%d.%d.%d.%d\n",t->v1,t->v2,t->v3,t->v4);
        for(i=0;i<NUM_VER_NAMES;i++)
            if(d_i->fields[i])
            {
                fprintf(f,"    %-28s%s\n",table_version[i].s,texta.get(d_i->fields[i]));
                if(d_i->cats[i])fprintf(f,"      %s\n",texta.get(d_i->cats[i]));

            }

        memset(cnts,-1,sizeof(cnts));plain=0;
        for(manuf_index=manuf_index_last;manuf_index<manufacturer_list.size();manuf_index++)
            if(manufacturer_list[manuf_index].inffile_index==inffile_index)
        {
            manuf_index_last=manuf_index;
            //hwidmatch.HWID_index=HWID_index_last;
            if(manufacturer_list[manuf_index].manufacturer)
                fprintf(f,"      {%s}\n",texta.get(manufacturer_list[manuf_index].manufacturer));
            for(pos=0;pos<manufacturer_list[manuf_index].sections_n;pos++)
            {
                getdrp_drvsectionAtPos(buf,pos,manuf_index);
                i=calc_secttype(buf);
                if(i>=0&&cnts[i]<0)cnts[i]=0;
                if(i<0&&pos>0)fprintf(f,"!!![%s]\n",buf);
                fprintf(f,"        [%s]\n",buf);
//                strcpy(buf+1000,buf);
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
                //if(strcmp(buf+1000,buf+500))
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

    //hash_stats(&indexes);
/*    for(i=0;i<indexes.items_handle.items;i++)
    {
        fprintf(f,"%d,%d,%d\n",i,indexes.items[i].key,indexes.items[i].next);
    }*/
    fclose(f);
}

void Driverpack::genhashes()
{
    // Driver signatures
    for(auto &it:inffile)
    {
        char filename[BUFLEN];
        strcpy(filename,texta.get(it.infpath));
        char *field=filename+strlen(filename);

        for(int j=CatalogFile;j<=CatalogFile_ntamd64;j++)if(it.fields[j])
        {
            strcpy(field,texta.get(it.fields[j]));
            strtolower(filename,strlen(filename));

            auto got=cat_list.find(filename);
            if(got!=cat_list.end())it.cats[j]=got->second;
        }
    }

    // Hashtable for fast search
    indexesold.reset(HWID_list.size()/2);
    for(unsigned i=0;i<HWID_list.size();i++)
    {
        char *vv=texta.get(HWID_list[i].HWID);
        int val=indexesold.gethashcode(vv,strlen(vv));
        indexesold.additem(val,i);
    }
}

unsigned int __stdcall Driverpack::thread_indexinf(void *arg)
{
    drplist_t *drplist=reinterpret_cast<drplist_t *>(arg);
    obj t;
    frm data;
    int exit=0,exit1=0;
    long long tm=0,last=0;
    //int tt;

    while(!exit)
    {
        while(drplist->pop(data))
        {
            if(!data.drp){exit=1;break;}

            {
                wchar_t bufw1[BUFLEN];
                wchar_t bufw2[BUFLEN];
                if(!drp_count)drp_count=1;
                wsprintf(bufw1,L"Indexing %d/%d",drp_cur,drp_count);
                wsprintf(bufw2,L"%s\\%s",data.drp->getPath(),data.drp->getFilename());
                manager_g->items_list[SLOT_INDEXING].isactive=1;
                manager_g->items_list[SLOT_INDEXING].val1=drp_cur;
                manager_g->items_list[SLOT_INDEXING].val2=drp_count;
                itembar_settext(manager_g,SLOT_INDEXING,bufw2,(drp_cur)*1000/drp_count);
                manager_g->setpos();
                drp_cur++;
            }

            //log_con("Str %ws\n",data.drp->getFilename());
            //t.inffile[0]=1;
            //do
            exit1=0;
            //tt=0;
            while(!exit1)
            while(data.drp->objs->pop(t))
            {
                //log_con("c1\n");
                if(last)tm+=GetTickCount()-last;
                //log_con("c2\n");
                //log_con("?");
                //if(!t.drp){exit=1;break;}
                if(!*t.inffile)
                {
                    t.drp->genhashes();
                    t.drp->texta.shrink();
                    free(t.adr);
                    last=GetTickCount();
                    //log_con("Trm %ws(%d,%d)\n",data.drp->getFilename(),t.drp->indexesold.size,tt);
                    delete data.drp->objs;
                    exit1=1;
                    break;
                }
                //log_con(".");
                //tt++;
                if(StrStrIW(t.inffile,L".inf"))
                    t.drp->indexinf_ansi(t.pathinf,t.inffile,t.adr,t.len);
                else
                    t.drp->parsecat(t.pathinf,t.inffile,t.adr,t.len);

                free(t.adr);
                last=GetTickCount();
                //log_con("c4\n");
            }
            //while(*t.inffile&&!exit);
            //delete data.drp->objs;
            //log_con("Fin %ws\n",data.drp->getFilename());
        }
    }
    log_con("Starved for %ld\n",tm);
    return 0;
}

void driverpack_indexinf_async(Driverpack *drp,wchar_t const *pathinf,wchar_t const *inffile,char *adr,int len)
{
    obj data;

    if(len>4&&((adr[0]==-1&&adr[3]==0)||adr[0]==0))
    {
        data.adr=(char *)malloc(len+2);
        if(!data.adr)
        {
            log_err("ERROR in driverpack_indexinf: malloc(%d)\n",len+2);
            return;
        }
        len=unicode2ansi(adr,data.adr,len);
    }
    else
    {
        data.adr=(char *)malloc(len);
        memmove(data.adr,adr,len);
    }

    data.len=len;
    wcscpy(data.pathinf,pathinf);
    wcscpy(data.inffile,inffile);
    data.drp=drp;
    if(drp&&drp->objs)drp->objs->push(data);
}

void driverpack_parsecat_async(Driverpack *drp,wchar_t const *pathinf,wchar_t const *inffile,char *adr,int len)
{
    obj data;

    data.adr=(char *)malloc(len);
    memmove(data.adr,adr,len);
    data.len=len;
    wcscpy(data.pathinf,pathinf);
    wcscpy(data.inffile,inffile);
    data.drp=drp;
    if(drp&&drp->objs)drp->objs->push(data);
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

void Driverpack::parsecat(wchar_t const *pathinf,wchar_t const *inffilename,char *adr,int len)
{
    CHAR bufa[BUFLEN];

    findosattr(bufa,adr,len);
    if(*bufa)
    {
        CHAR filename[BUFLEN];
        wsprintfA(filename,"%ws%ws",pathinf,inffilename);
        strtolower(filename,strlen(filename));
        cat_list.insert({filename,texta.memcpyz_dup(bufa,strlen(bufa))});
        //log_con("(%s)\n##%s\n",filename,bufa);
    }
    else
    {
        log_con("Not found singature in '%ws%ws'(%d)\n",pathinf,inffilename,len);
    }

}

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
    allocImp.Alloc=SzAlloc;
    allocImp.Free=SzFree;
    allocTempImp.Alloc=SzAllocTemp;
    allocTempImp.Free=SzFreeTemp;

    CSzArEx db;
    SzArEx_Init(&db);
    SRes res=SzArEx_Open(&db,&lookStream.s,&allocImp,&allocTempImp);
    int cc=0;
    if(res==SZ_OK)
    {
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
                if(res!=SZ_OK)
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
                if(StrStrIW(infname,L".inf"))
                    driverpack_indexinf_async(this,infpath,infname,(char *)(outBuffer+offset),outSizeProcessed);
                else
                    parsecat(infpath,infname,(char *)(outBuffer+offset),outSizeProcessed);
                    //driverpack_parsecat_async(this,infpath,infname,(char *)(outBuffer+offset),outSizeProcessed);
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


    //log_con("%ws, %d\n",getFilename(),cc);

    //driverpack_indexinf_async(this,L"",L"",nullptr,0);
    return 1;
}

void Driverpack::indexinf(wchar_t const *drpdir,wchar_t const *iinfdilename,char *bb,int inf_len)
{
    if(inf_len>4&&((bb[0]==-1&&bb[3]==0)||bb[0]==0))
    {
        char *buf_out;
        int size=inf_len;

        buf_out=(char *)malloc(size+2);
        if(!buf_out)
        {
            log_err("ERROR in driverpack_indexinf: malloc(%d)\n",size+2);
            return;
        }
        size=unicode2ansi(bb,buf_out,size);
        indexinf_ansi(drpdir,iinfdilename,buf_out,size);
        free(buf_out);
    }
    else
    {
        indexinf_ansi(drpdir,iinfdilename,bb,inf_len);
    }
}

// http://msdn.microsoft.com/en-us/library/ff547485(v=VS.85).aspx
void Driverpack::indexinf_ansi(wchar_t const *drpdir,wchar_t const *inffilename,char *inf_base,int inf_len)
{
    version_t *cur_ver;

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
    cur_inffile->infpath=texta.strcpy(line);
    wsprintfA(line,"%ws",inffilename);
    cur_inffile->inffilename=texta.strcpy(line);
    cur_inffile->infsize=inf_len;
    cur_inffile->infcrc=0;

    wchar_t inffull[BUFLEN];
    wcscpy(inffull,drpdir);
    wcscat(inffull,inffilename);

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

        Parser parse_info{lnk,this,string_list,inffull};
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
    //char date_s[256];
    //char build_s[256];
    //date_s[0]=0;
    //build_s[0]=0;
    cur_ver=&cur_inffile->version;
    cur_ver->setInvalid();

    range=section_list.equal_range("version");
    if(range.first==range.second)log_index("ERROR: missing [version] in %S\n",inffull);
    //if(range.first==range.second)log_index("NOTE:  multiple [version] in %S\n",inffull);
    for(auto got=range.first;got!=range.second;++got)
    {
        sect_data_t *lnk=&got->second;
        char *s1b,*s1e;

        Parser parse_info{lnk,this,string_list,inffull};
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

                        //wsprintfA(date_s,"%02d/%02d/%04d",cur_ver->m,cur_ver->d,cur_ver->y);

                        // version
                        if(parse_info.parseField())
                        {
                            parse_info.readVersion(cur_ver);
                        }
                        //wsprintfA(build_s,"%d.%d.%d.%d",cur_ver->v1,cur_ver->v2,cur_ver->v3,cur_ver->v4);

                }else
                {
                    parse_info.parseField();
                    parse_info.readStr(&s1b,&s1e);
                    cur_inffile->fields[i]=texta.t_memcpyz(s1b,s1e-s1b);
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
        Parser parse_info{lnk,this,string_list,inffull};
        while(parse_info.parseItem())
        {
            char *s1b,*s1e;
            parse_info.readStr(&s1b,&s1e);

            cur_manuf_index=manufacturer_list.size();
            manufacturer_list.resize(cur_manuf_index+1);
            cur_manuf=&manufacturer_list[cur_manuf_index];
            cur_manuf->inffile_index=cur_inffile_index;
            cur_manuf->manufacturer=texta.t_memcpyz(s1b,s1e-s1b);
            cur_manuf->sections_n=0;

            if(parse_info.parseField())
            {
                parse_info.readStr(&s1b,&s1e);
                strtolower(s1b,s1e-s1b);
                strs[cur_manuf->sections_n++]=texta.t_memcpyz(s1b,s1e-s1b);
                while(1)
                {
                    if(cur_manuf->sections_n>1)
                        wsprintfA(secttry,"%s.%s",
                                texta.get(strs[0]),
                                texta.get(strs[cur_manuf->sections_n-1]));
                    else
                        wsprintfA(secttry,"%s",texta.get(strs[0]));

                    strtolower(secttry,strlen(secttry));

                    auto range2=section_list.equal_range(secttry);
                    if(range2.first==range2.second)log_index("ERROR: missing [%s] in %S\n",secttry,inffull);
                    for(auto got2=range2.first;got2!=range2.second;++got2)
                    {
                        sect_data_t *lnk2=&got2->second;
                        Parser parse_info2{lnk2,this,string_list,inffull};
                        while(parse_info2.parseItem())
                        {
                            parse_info2.readStr(&s1b,&s1e);
                            int desc_c=texta.memcpyz_dup(s1b,s1e-s1b);

                            parse_info2.parseField();
                            parse_info2.readStr(&s1b,&s1e);
                            int inst_c=texta.memcpyz_dup(s1b,s1e-s1b);

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
                                        strcat(installsection,".");strcat(installsection,texta.get(strs[cur_manuf->sections_n-1]));
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
                                install_picket_c=texta.memcpyz_dup(installsection,strlen(installsection));
                            }
                            else
                            {
                                install_picket_c=texta.memcpyz_dup("{missing}",9);
                            }

                            for(auto got3=range3.first;got3!=range3.second;++got3)
                            {
                                lnk3=&got3->second;
                                Parser parse_info3{lnk3,this,string_list,inffull};
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

                                HWID_list.push_back(data_HWID_t(cur_desc_index,hwid_pos++,texta.memcpyz_dup(s1b,s1e-s1b)));
                            }
                        }
                    }

                    if(!parse_info.parseField())break;
                    parse_info.readStr(&s1b,&s1e);
                    if(s1b>s1e)break;
                    strtolower(s1b,s1e-s1b);
                    strs[cur_manuf->sections_n++]=texta.t_memcpyz(s1b,s1e-s1b);
                }
            }
            cur_manuf->sections=texta.t_memcpyz((char *)strs,sizeof(int)*cur_manuf->sections_n);
        }
    }
}
//}
