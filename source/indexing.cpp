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

#include "main.h"

//#define DEBUG_EXTRACHECKS

//{ Global variables
int drp_count;
int drp_cur;
int loaded_unpacked=0;
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
const WCHAR *olddrps[]=
{
    L"DP_Video_Server_1",
    L"DP_Video_Others_1",
    L"DP_Video_nVIDIA_1",
    L"DP_Video_AMD_1",
    L"DP_Videos_AMD_1",
};
//}

//{ Parse
void Parser_str::parseWhitespace(bool eatnewline=false)
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

int Parser_str::parseItem()
{
    parseWhitespace(true);
    strBeg=blockBeg;

    char *p=blockBeg;

    while(p<blockEnd)
    {
        switch(*p)
        {
            case '=':               // Item found
                blockBeg=p;
                strEnd=p;
                while((strEnd[-1]==32||strEnd[-1]=='\t')&&strEnd[-1]!='\"')strEnd--; // trim spaces
                if(*strBeg=='\"'){strBeg++;strEnd--;}
                str_sub();
                return 1;

            case '\n':case '\r':    // No item found
                p++;
                blockBeg=p;
                strBeg=blockBeg;
#ifdef DEBUG_EXTRACHECKS
                log_file("ERR1 '%.*s'\n",30,s1b);
#endif
                break;
            default:
                p++;
        }
    }
    strBeg=0;
    strEnd=0;
    return 0;
}

int Parser_str::parseField()
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
                    str_sub();
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
                    blockBeg=strEnd;
                    while(strEnd>strBeg&&(strEnd[-1]==32||strEnd[-1]=='\t'))strEnd--; // trim spaces
                    str_sub();
                    return strEnd!=strBeg||*p==',';

                default:
                    p++;
            }
        }
    }
    return 0;
}

int Parser_str::readNumber()
{
    int n=atoi(strBeg);

    while(*strBeg>='0'&&*strBeg<='9'&&strBeg<strEnd)strBeg++;
    if(strBeg<strEnd)strBeg++;
    return n;
}

int Parser_str::readHex()
{
    int val=0;

    while((*strBeg=='0'||*strBeg=='x')&&strBeg<strEnd)strBeg++;
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

int Parser_str::readDate(version_t *t)
{
    int flag=0;

    while(!(*strBeg>='0'&&*strBeg<='9')&&strBeg<strEnd)strBeg++;
    t->m=readNumber();
    t->d=readNumber();
    t->y=readNumber();
    if(t->y<100)t->y+=1900;

    if(t->y<1990)flag=1;
    if(t->y>2013)flag=2;
    switch(t->m)
    {
        case 1:case 3:case 5:case 7:case 8:case 10:case 12:
            if(t->d<1||t->d>31)flag=3;
            break;
        case 4:case 6:case 9:case 11:
            if(t->d<1||t->d>30)flag=4;
            break;
        case 2:
            if(t->d<1||t->d>((((t->y%4==0)&&(t->y%100))||(t->y%400==0))?29:28))flag=5;
            break;
        default:
            flag=6;
    }
    return flag;
}

void Parser_str::readVersion(version_t *t)
{
    t->v1=readNumber();
    t->v2=readNumber();
    t->v3=readNumber();
    t->v4=readNumber();
}

void Parser_str::init(Driverpack *drp)
{
    pack=drp;
    heap_init(&strings,ID_PARSE,(void **)&text,1024*64,1);
}

void Parser_str::release()
{
    if(strings.used)log_file("Parse_used %d\n",strings.used);
    heap_free(&strings);
}

void Parser_str::setRange(char *inf_base,sect_data_t *lnk)
{
    blockBeg=inf_base+lnk->ofs;
    blockEnd=inf_base+lnk->len;
}

void Parser_str::readStr(char **vb,char **ve)
{
    *vb=strBeg;
    *ve=strEnd;
}

void Parser_str::str_sub()
{
    static char static_buf[BUFLEN];
    int vers_len;
    char *v1b,*v1e;
    char *res;
    int isfound;

    v1b=strBeg;
    v1e=strEnd;

    strEnd=v1e;

    if(*v1b=='%'/*&&v1e[-1]=='%'*/)
    {
        v1b++;
        vers_len=v1e-v1b-1;
        if(v1e[-1]!='%')vers_len++;
        if(vers_len<0)vers_len=0;

        strtolower(v1b,vers_len);
        res=(char *)hash_find(&pack->string_list,v1b,vers_len,&isfound);
        if(isfound)
        {
            strBeg=res;
            strEnd=res+strlen(res);
            return;
        }else
        {
            //if(memcmp(v1b,"system",5))
            //log_file("ERROR: string '%.*s' not found\n",vers_len+2,v1b-1);
            //return;
        }
    }
    char *p,*p_s=static_buf;
    v1b=strBeg;
    int flag=0;
    while(v1b<v1e)
    {
        while(*v1b!='%'&&v1b<v1e)*p_s++=*v1b++;
        if(*v1b=='%')
        {
            //log_file("Deep replace %.*s\n",v1e-v1b,v1b);
            p=v1b+1;
            while(*p!='%'&&p<v1e)p++;
            if(*p=='%')
            {
                strtolower(v1b+1,p-v1b-1);
                res=(char *)hash_find(&pack->string_list,v1b+1,p-v1b-1,&isfound);
                if(isfound)
                {
                    strcpy(p_s,res);
                    p_s+=strlen(res);
                    v1b=p+1;
                    flag=1;
                }
            }
            if(v1b<v1e)*p_s++=*v1b++;
        }
    }
    if(!flag)return;

    *p_s++=0;*p_s=0;
    p_s=static_buf;

    p_s=text+heap_alloc(&strings,strlen(p_s)+1);
    strcpy(p_s,static_buf);

    strBeg=p_s;
    strEnd=p_s+strlen(p_s);
}
//}

//{ Misc
int unicode2ansi(char *s,char *out,int size)
{
    int ret,flag;
    size/=2;
    if(!out)log_err("Error out:\n");
    if(!s)log_err("Error in:\n");
    if(size<0)log_err("Error size:\n");
    ret=WideCharToMultiByte(CP_ACP,0,(WCHAR *)(s+(s[0]==-1?2:0)),size-(s[0]==-1?1:0),(CHAR *)out,size,0,&flag);
    if(!ret)log_err("Error:%d\n",GetLastError());
    out[size]=0;
    return ret;
}

int encode(char *dest,int dest_sz,char *src,int src_sz)
{
    Lzma86_Encode((Byte *)dest,(SizeT *)&dest_sz,(const Byte *)src,src_sz,0,1<<23,SZ_FILTER_AUTO);
    return dest_sz;
}

int decode(char *dest,int dest_sz,char *src,int src_sz)
{
    Lzma86_Decode((Byte *)dest,(SizeT *)&dest_sz,(const Byte *)src,(SizeT *)&src_sz);
    return dest_sz;
}

WCHAR *finddrp(WCHAR *s)
{
    return manager_g->matcher->col->collection_finddrp(s);
}

//}

//{ Collection
void Collection::collection_init(WCHAR *driverpacks_dirv,const WCHAR *index_bin_dirv,const WCHAR *index_linear_dirv,int flags_l)
{
    heap_init(&driverpack_handle,ID_DRIVERPACK,(void **)&driverpack_list,0,sizeof(Driverpack));
    flags=flags_l;

    driverpack_dir=driverpacks_dirv;
    index_bin_dir=index_bin_dirv;
    index_linear_dir=index_linear_dirv;
}

void Collection::collection_free()
{
    int i;

    for(i=0;i<driverpack_handle.items;i++)
        driverpack_list[i].driverpack_free();

    heap_free(&driverpack_handle);
}

void Collection::collection_save()
{
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA FindFileData;
    WCHAR buf1[BUFLEN];
    WCHAR buf2[BUFLEN];
    WCHAR buf3[BUFLEN];
    int i;

    time_indexsave=GetTickCount();

#ifndef CONSOLE_MODE
    // Save indexes
    if(*drpext_dir==0)
    {
        int count=0,cur=1;

        for(i=0;i<driverpack_handle.items;i++)
            if(driverpack_list[i].type==DRIVERPACK_TYPE_PENDING_SAVE)count++;

        log_con("Saving indexes...");
        for(i=0;i<driverpack_handle.items;i++)
        {
            if((flags&FLAG_KEEPUNPACKINDEX)==0&&!i)
            {
                cur++;
                continue;
            }
            if(driverpack_list[i].type==DRIVERPACK_TYPE_PENDING_SAVE)
            {
                if(flags&COLLECTION_USE_LZMA)
                {
                    WCHAR bufw2[BUFLEN];

                    wsprintf(bufw2,L"%ws\\%ws",
                        driverpack_list[i].text+driverpack_list[i].drppath,
                        driverpack_list[i].text+driverpack_list[i].drpfilename);

                    log_con("Saving indexes for '%ws'\n",bufw2);
                    manager_g->items_list[SLOT_INDEXING].isactive=2;
                    manager_g->items_list[SLOT_INDEXING].val1=cur-1;
                    manager_g->items_list[SLOT_INDEXING].val2=count-1;
                    wcscpy(manager_g->items_list[SLOT_INDEXING].txt1,bufw2);
                    manager_g->items_list[SLOT_INDEXING].percent=(cur)*1000/count;
                    manager_setpos(manager_g);
                    redrawfield();
                    cur++;
                }

                driverpack_list[i].driverpack_saveindex();
            }
        }
        manager_g->items_list[SLOT_INDEXING].isactive=0;
        manager_setpos(manager_g);
        log_con("DONE\n");
    }

    // Delete unused indexes
    if(*drpext_dir)return;
    if(!canWrite(index_bin_dir))
    {
        log_err("ERROR in collection_save(): Write-protected,'%ws'\n",index_bin_dir);
        return;
    }
    wsprintf(buf1,L"%ws\\*.*",index_bin_dir);
    hFind=FindFirstFile(buf1,&FindFileData);
    while(FindNextFile(hFind,&FindFileData)!=0)
    {
        if(!(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
        {
            wsprintf(buf3,L"%s\\%s",index_bin_dir,FindFileData.cFileName);
            for(i=flags&FLAG_KEEPUNPACKINDEX?0:1;i<driverpack_handle.items;i++)
            {
                driverpack_list[i].driverpack_getindexfilename(index_bin_dir,L"bin",buf2);
                if(!wcscmp(buf2,buf3))break;
            }
            if(i==driverpack_handle.items&&!StrStrIW(buf3,L"\\_"))
            {
                printf("Deleting %ws\n",buf3);
                _wremove(buf3);
            }
        }
    }
#endif
    time_indexsave=GetTickCount()-time_indexsave;
}

int Collection::collection_scanfolder_count(const WCHAR *path)
{
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA FindFileData;
    WCHAR buf[BUFLEN];
    Driverpack drp;
    int cnt;
    int i;

    cnt=0;

    wsprintf(buf,L"%ws\\*.*",path);
    hFind=FindFirstFile(buf,&FindFileData);

    while(FindNextFile(hFind,&FindFileData)!=0)
    {
        if(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
        {
            if(lstrcmp(FindFileData.cFileName,L"..")==0)continue;
            wsprintf(buf,L"%ws\\%ws",path,FindFileData.cFileName);
            cnt+=collection_scanfolder_count(buf);
        } else
        {
            int len=lstrlen(FindFileData.cFileName);
            for(i=0;i<5;i++)
            if(StrStrIW(FindFileData.cFileName,olddrps[i]))
            {
                wsprintf(buf,L" /c del \"%s\\%s*.7z\" /Q /F",driverpack_dir,olddrps[i]);
                RunSilent(L"cmd",buf,SW_HIDE,1);
                break;
            }
            if(i==5&&_wcsicmp(FindFileData.cFileName+len-3,L".7z")==0)
            {
                drp.driverpack_init(path,FindFileData.cFileName,this);
                //log_con("<%ws><%ws>\n",path,FindFileData.cFileName);
                if(flags&COLLECTION_FORCE_REINDEXING||!drp.driverpack_checkindex())cnt++;
                drp.driverpack_free();
            }
        }
    }
    FindClose(hFind);
    return cnt;
}

void Collection::collection_updatedindexes()
{
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA FindFileData;
    WCHAR buf[BUFLEN];
    WCHAR filename[BUFLEN];
    Driverpack *drp;

    wsprintf(buf,L"%ws\\_*.*",index_bin_dir);
    hFind=FindFirstFile(buf,&FindFileData);

    while(FindNextFile(hFind,&FindFileData)!=0)
    {
        wsprintf(filename,L"%ws",FindFileData.cFileName);
        wcscpy(filename+wcslen(FindFileData.cFileName)-3,L"7z");

        wsprintf(buf,L"drivers\\%ws",filename);
        buf[8]=L'D';
        if(PathFileExists(buf))
        {
            log_con("Skip %ws\n",buf);
            continue;
        }

        int index=heap_allocitem_i(&driverpack_handle);

        drp=&driverpack_list[index];
        drp->driverpack_init(driverpack_dir,filename,this);
//        log_con("Load '%ws'\n",filename);
        drp->driverpack_loadindex();
    }
    FindClose(hFind);
}

void Collection::collection_load()
{
    Driverpack *unpacked_drp;

    time_indexes=GetTickCount();
    registerall();
    heap_reset(&driverpack_handle,0);
    unpacked_drp=(Driverpack *)heap_allocitem_ptr(&driverpack_handle);
    unpacked_drp->driverpack_init(driverpack_dir,L"unpacked.7z",this);

//{thread
    int i;
    HANDLE thr;
    inflist=(inflist_t *)malloc(LSTCNT*sizeof(inflist_t));
    if(!inflist){log_con("ERROR 1\n");return;}
    for(i=0;i<LSTCNT;i++)
    {
        inflist[i].dataready=CreateEvent(0,0,0,0);
        inflist[i].slotvacant=CreateEvent(0,0,1,0);
        if(!inflist[i].dataready){log_con("ERROR 2\n");return;}
        if(!inflist[i].slotvacant){log_con("ERROR 3\n");return;}
    }
    pos_in=pos_out=0;
    thr=(HANDLE)_beginthreadex(0,0,&thread_indexinf,this,0,0);
//}thread

    if(flags&FLAG_KEEPUNPACKINDEX)loaded_unpacked=unpacked_drp->driverpack_loadindex();
    drp_count=collection_scanfolder_count(driverpack_dir);
    drp_cur=0;
    collection_scanfolder(driverpack_dir);
    collection_updatedindexes();
    manager_g->items_list[SLOT_INDEXING].isactive=0;
    if(driverpack_handle.items<=1&&(flags&FLAG_DPINSTMODE)==0)
        itembar_settext(manager_g,SLOT_NODRIVERS,L"",0);
    driverpack_list[0].driverpack_genhashes();
    time_indexes=GetTickCount()-time_indexes;
    flags&=~COLLECTION_FORCE_REINDEXING;

//{thread
    driverpack_indexinf_async(0,this,L"",L"",0,0);
    WaitForSingleObject(thr,INFINITE);
    CloseHandle_log(thr,L"driverpack_genindex",L"thr");
    for(i=0;i<LSTCNT;i++)
    {
        CloseHandle_log(inflist[i].dataready,L"driverpack_genindex",L"dataready");
        CloseHandle_log(inflist[i].slotvacant,L"driverpack_genindex",L"slotvacant");
    }
    free(inflist);
//}thread
}

void Collection::collection_print()
{
    int i;

    time_indexprint=GetTickCount();

    for(i=0;i<driverpack_handle.items;i++)
        driverpack_list[i].driverpack_print();

    time_indexprint=GetTickCount()-time_indexprint;
}

void Collection::collection_printstates()
{
    int i,sum=0;
    Driverpack *drp;
    WCHAR *s;

    if((log_verbose&LOG_VERBOSE_DRP)==0)return;
    log_file("Driverpacks\n");
    for(i=0;i<driverpack_handle.items;i++)
    {
        drp=&driverpack_list[i];
        s=(WCHAR *)drp->text;
        log_file("  %6d  %ws\\%ws\n",drp->HWID_list_handle.items,s+drp->drppath,(WCHAR *)(drp->text+drp->drpfilename));
        sum+=drp->HWID_list_handle.items;
    }
    log_file("  Sum: %d\n\n",sum);
}

WCHAR *Collection::collection_finddrp(WCHAR *fnd)
{
    int i,j,num;
    Driverpack *drp;
    WCHAR *s,*d,*n_s;

    num=driverpack_handle.items;
    j=0;
    n_s=0;
    for(i=0;i<num;i++)
    {
        drp=&driverpack_list[i];
        s=(WCHAR *)(drp->text+drp->drpfilename);
        if(StrStrIW(s,fnd)&&drp->type!=DRIVERPACK_TYPE_UPDATE)
        {
            d=s;
            while(*d)
            {
                if(*d==L'_'&&d[1]>=L'0'&&d[1]<=L'9')
                {
                    if(j<_wtoi(d+1))
                    {
                        j=_wtoi(d+1);
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

void Collection::collection_scanfolder(const WCHAR *path)
{
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA FindFileData;
    WCHAR buf[1024];
    Driverpack *drp;

    wsprintf(buf,L"%s\\*.*",path);
    hFind=FindFirstFile(buf,&FindFileData);

    while(FindNextFile(hFind,&FindFileData)!=0)
    {
        if(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
        {
            if(lstrcmp(FindFileData.cFileName,L"..")==0)continue;
            wsprintf(buf,L"%s\\%s",path,FindFileData.cFileName);
            collection_scanfolder(buf);
        } else
        {
            int len=lstrlen(FindFileData.cFileName);
            if(_wcsicmp(FindFileData.cFileName+len-3,L".7z")==0)
            {
                //drp=(driverpack_t *)heap_allocitem_ptr(&driverpack_handle);
                int index=heap_allocitem_i(&driverpack_handle);
                drp=&driverpack_list[index];
                drp->driverpack_init(path,FindFileData.cFileName,this);
                if(flags&COLLECTION_FORCE_REINDEXING||!drp->driverpack_loadindex())
                {
                    WCHAR bufw1[BUFLEN];
                    WCHAR bufw2[BUFLEN];
                    if(!drp_count)drp_count=1;
                    wsprintf(bufw1,L"Indexing %d/%d",drp_cur,drp_count);
                    wsprintf(bufw2,L"%s\\%s",path,FindFileData.cFileName);
                    manager_g->items_list[SLOT_INDEXING].isactive=1;
                    manager_g->items_list[SLOT_INDEXING].val1=drp_cur;
                    manager_g->items_list[SLOT_INDEXING].val2=drp_count;
                    itembar_settext(manager_g,SLOT_INDEXING,bufw2,(drp_cur)*1000/drp_count);
                    manager_setpos(manager_g);
                    drp=&driverpack_list[index];
                    drp->driverpack_genindex();
                    drp_cur++;
                }
            }else
            if(_wcsicmp(FindFileData.cFileName+len-4,L".inf")==0&&loaded_unpacked==0)
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
                driverpack_list[0].driverpack_indexinf(buf,FindFileData.cFileName,buft,len);
                free(buft);
            }else
            if(_wcsicmp(FindFileData.cFileName+len-4,L".cat")==0&&loaded_unpacked==0)
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
//                driverpack_indexinf(&driverpack_list[0],buf,FindFileData.cFileName,buft,len);
                driverpack_list[0].driverpack_parsecat(buf,FindFileData.cFileName,buft,len);
                free(buft);
            }
        }
    }
    FindClose(hFind);
}
//}

//{ Driverpack
void Driverpack::driverpack_init(WCHAR const *driverpack_path,WCHAR const *driverpack_filename,Collection *col_v)
{
    char buf[BUFLEN];

    col=col_v;

    heap_init(&inffile_handle,ID_DRIVERPACK_inffile,(void **)&inffile,0,sizeof(data_inffile_t));
    heap_init(&manufacturer_handle,ID_DRIVERPACK_manufacturer,(void **)&manufacturer_list,0,sizeof(data_manufacturer_t));
    heap_init(&desc_list_handle,ID_DRIVERPACK_desc_list,(void **)&desc_list,0,sizeof(data_desc_t));
    heap_init(&HWID_list_handle,ID_DRIVERPACK_HWID_list,(void **)&HWID_list,0,sizeof(data_HWID_t));
    heap_init(&text_handle,ID_DRIVERPACK_text,(void **)&text,0,1);

    hash_init(&string_list,ID_STRINGS,1024,HASH_FLAG_KEYS_ARE_POINTERS|HASH_FLAG_STR_TO_LOWER);
    hash_init(&section_list,ID_SECTIONS,16,HASH_FLAG_KEYS_ARE_POINTERS|HASH_FLAG_STR_TO_LOWER);
    hash_init(&cat_list,ID_CAT_LIST,512*8,HASH_FLAG_STR_TO_LOWER);

    sprintf(buf,"%ws",driverpack_path);
    drppath=heap_memcpy(&text_handle,driverpack_path,wcslen(driverpack_path)*2+2);

    sprintf(buf,"%ws",driverpack_filename);
    drpfilename=heap_memcpy(&text_handle,driverpack_filename,wcslen(driverpack_filename)*2+2);
    indexes.size=0;
}

void Driverpack::driverpack_free()
{
    heap_free(&inffile_handle);
    heap_free(&manufacturer_handle);
    heap_free(&desc_list_handle);
    heap_free(&HWID_list_handle);
    heap_free(&text_handle);

    hash_free(&string_list);
    hash_free(&section_list);
    hash_free(&cat_list);

    if(indexes.size)hash_free(&indexes);
}

void Driverpack::driverpack_saveindex()
{
    WCHAR filename[BUFLEN];
    FILE *f;
    int sz;
    int version=VER_INDEX;
    char *mem,*p,*mem_pack;

    driverpack_getindexfilename(col->getIndex_bin_dir(),L"bin",filename);
    if(!canWrite(filename))
    {
        log_err("ERROR in driverpack_saveindex(): Write-protected,'%ws'\n",filename);
        return;
    }
    f=_wfopen(filename,L"wb");

    sz=
        inffile_handle.used+
        manufacturer_handle.used+
        desc_list_handle.used+
        HWID_list_handle.used+
        text_handle.used+
        indexes.items_handle.used+sizeof(int)+
        6*sizeof(int)*2;

    p=mem=(char *)malloc(sz);

    fwrite("SDW",3,1,f);
    fwrite(&version,sizeof(int),1,f);

    p=heap_save(&inffile_handle,p);
    p=heap_save(&manufacturer_handle,p);
    p=heap_save(&desc_list_handle,p);
    p=heap_save(&HWID_list_handle,p);
    p=heap_save(&text_handle,p);
    p=hash_save(&indexes,p);
    /*log_con("Sz:(%d,%d,%d,%d,%d,%d)=%d\n",
            inffile_handle.used,
            manufacturer_handle.used,
            desc_list_handle.used,
            HWID_list_handle.used,
            text_handle.used,
            indexes.items_handle.used,
            sz);*/

    if(col->getFlags()&COLLECTION_USE_LZMA)
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

int Driverpack::driverpack_checkindex()
{
    WCHAR filename[BUFLEN];
    CHAR buf[3];
    FILE *f;
    int sz;
    int version;

    driverpack_getindexfilename(col->getIndex_bin_dir(),L"bin",filename);
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

int Driverpack::driverpack_loadindex()
{
    WCHAR filename[BUFLEN];
    CHAR buf[3];
    FILE *f;
    int sz;
    int version;
    char *mem,*p,*mem_unpack=0;

    driverpack_getindexfilename(col->getIndex_bin_dir(),L"bin",filename);
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

    if(col->getFlags()&COLLECTION_USE_LZMA)
    {
        UInt64 sz_unpack;

        Lzma86_GetUnpackSize((Byte *)p,sz,&sz_unpack);
        mem_unpack=(char *)malloc(sz_unpack);
        decode(mem_unpack,sz_unpack,mem,sz);
        p=mem_unpack;
    }

    p=heap_load(&inffile_handle,p);
    p=heap_load(&manufacturer_handle,p);
    p=heap_load(&desc_list_handle,p);
    p=heap_load(&HWID_list_handle,p);
    p=heap_load(&text_handle,p);
    p=hash_load(&indexes,p);

    free(mem);
    if(mem_unpack)free(mem_unpack);
    fclose(f);

    type=StrStrIW(filename,L"\\_")?DRIVERPACK_TYPE_UPDATE:DRIVERPACK_TYPE_INDEXED;
    return 1;
}

void Driverpack::driverpack_getindexfilename(const WCHAR *dir,const WCHAR *ext,WCHAR *indfile)
{
    WCHAR *p;
    WCHAR buf[BUFLEN];
    int len=wcslen((WCHAR *)(text+drpfilename));

    wsprintf(buf,L"%s",text+drpfilename);

    if(*(text+drppath))
        wsprintf(buf+(len-3)*1,L"%s.%s",(WCHAR *)(text+drppath)+lstrlen(col->getDriverpack_dir()),ext);
    else
        wsprintf(buf+(len-3)*1,L".%s",ext);

    p=buf;
    while(*p){if(*p==L'\\'||*p==L' ')*p=L'_';p++;}
    wsprintf(indfile,L"%s\\%s",dir,buf);
}

void Driverpack::driverpack_print()
{
    int inffile_index,manuf_index,pos,desc_index,HWID_index;
    int n=inffile_handle.items;
    version_t *t;
    data_inffile_t *d_i;
    hwidmatch_t hwidmatch;
    char buf[BUFLEN];
    WCHAR filename[BUFLEN];
    FILE *f;
    int cnts[NUM_DECS],plain;
    int HWID_index_last=0;
    int manuf_index_last=0;
    int i;

    hwidmatch.drp=this;
    driverpack_getindexfilename(col->getIndex_linear_dir(),L"txt",filename);
    f=_wfopen(filename,L"wt");

    fprintf(f,"%ws\\%ws (%d inf files)\n",text+drppath,text+drpfilename,n);
    for(inffile_index=0;inffile_index<n;inffile_index++)
    {
        d_i=&inffile[inffile_index];
        fprintf(f,"  %s%s (%d bytes)\n",text+d_i->infpath,text+d_i->inffilename,d_i->infsize);
    for(i=0;i<n;i++)if(i!=inffile_index&&d_i->infcrc==inffile[i].infcrc)
    fprintf(f,"**%s%s\n",text+inffile[i].infpath,text+inffile[i].inffilename);
        t=&d_i->version;
        fprintf(f,"    date\t\t\t%d/%d/%d\n",t->d,t->m,t->y);
        fprintf(f,"    version\t\t\t%d.%d.%d.%d\n",t->v1,t->v2,t->v3,t->v4);
        for(i=0;i<NUM_VER_NAMES;i++)
            if(d_i->fields[i])
            {
                fprintf(f,"    %-28s%s\n",table_version[i].s,text+d_i->fields[i]);
                if(d_i->cats[i])fprintf(f,"      %s\n",text+d_i->cats[i]);

            }

        memset(cnts,-1,sizeof(cnts));plain=0;
        for(manuf_index=manuf_index_last;manuf_index<manufacturer_handle.items;manuf_index++)
            if(manufacturer_list[manuf_index].inffile_index==inffile_index)
        {
            manuf_index_last=manuf_index;
            //hwidmatch.HWID_index=HWID_index_last;
            if(manufacturer_list[manuf_index].manufacturer)
                fprintf(f,"      {%s}\n",text+manufacturer_list[manuf_index].manufacturer);
            for(pos=0;pos<manufacturer_list[manuf_index].sections_n;pos++)
            {
                getdrp_drvsectionAtPos(this,buf,pos,manuf_index);
                i=calc_secttype(buf);
                if(i>=0&&cnts[i]<0)cnts[i]=0;
                if(i<0&&pos>0)fprintf(f,"!!![%s]\n",buf);
                fprintf(f,"        [%s]\n",buf);
//                strcpy(buf+1000,buf);
                for(desc_index=0;desc_index<desc_list_handle.items;desc_index++)
                    if(desc_list[desc_index].manufacturer_index==manuf_index&&
                       desc_list[desc_index].sect_pos==pos)
                {
                    for(HWID_index=HWID_index_last;HWID_index<HWID_list_handle.items;HWID_index++)
                        if(HWID_list[HWID_index].desc_index==desc_index)
                    {
                        if(HWID_index_last+1!=HWID_index&&HWID_index)fprintf(f,"Skip:%d,%d\n",HWID_index_last,HWID_index);
                        HWID_index_last=HWID_index;
                        hwidmatch.HWID_index=HWID_index_last;

                //if(text+manufacturer_list[manuf_index].manufacturer!=get_manufacturer(&hwidmatch))
                //fprintf(f,"*%s\n",get_manufacturer(&hwidmatch));
                //get_section(&hwidmatch,buf+500);
                //if(strcmp(buf+1000,buf+500))
                //fprintf(f,"*%s,%s\n",buf+1000,buf+500);
                        if(i>=0)cnts[i]++;
                        if(pos==0&&i<0)plain++;

                        if(getdrp_drvinfpos(&hwidmatch))
                            sprintf(buf,"%-2d",getdrp_drvinfpos(&hwidmatch));
                        else
                            sprintf(buf,"  ");

                        fprintf(f,"       %s %-50s%-20s\t%s\n",buf,
                            getdrp_drvHWID(&hwidmatch),
                            getdrp_drvinstall(&hwidmatch),
                            getdrp_drvdesc(&hwidmatch));
                        fprintf(f,"          feature:%-42hX%-20s\n\n",
                            getdrp_drvfeature(&hwidmatch)&0xFF,
                            getdrp_drvinstallPicked(&hwidmatch));
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
    fprintf(f,"  HWIDS:%d/%d\n",HWID_index_last+1,HWID_list_handle.items);

    //hash_stats(&indexes);
/*    for(i=0;i<indexes.items_handle.items;i++)
    {
        fprintf(f,"%d,%d,%d\n",i,indexes.items[i].key,indexes.items[i].next);
    }*/
    fclose(f);
}

void Driverpack::driverpack_genhashes()
{
    char filename[BUFLEN];
    int i,j,r;


    hash_init(&indexes,ID_INDEXES,HWID_list_handle.items/2,HASH_FLAG_STRS_ARE_INTS);
    //heap_expand(&t->indexes.strs_handle,64*1024);
    //log_file("Items: %d\n",pack->HWID_list_handle.items);
    for(i=0;i<inffile_handle.items;i++)
    {
        sprintf(filename,"%s%s",text+inffile[i].infpath,text+inffile[i].inffilename);
        strtolower(filename,strlen(filename));
        //log_con("%s\n",filename);
        for(j=CatalogFile;j<=CatalogFile_ntamd64;j++)
        {
            if(inffile[i].fields[j])
            {
                sprintf(filename,"%s%s",text+inffile[i].infpath,text+inffile[i].fields[j]);
                strtolower(filename,strlen(filename));
                //log_con("%d: (%s)\n",j,filename);
                r=hash_find_str(&cat_list,filename);
                if(r)
                {
                    //log_con("^^%d: %s\n",r,text+r);
                    inffile[i].cats[j]=r;
                }
                //else log_con("Not found\n");

            }
        }
    }
    //hash_stats(&cat_list);

    for(i=0;i<HWID_list_handle.items;i++)
    {
        int val=0;
        char *vv=text+HWID_list[i].HWID;

        val=hash_getcode(vv,strlen(vv));
        hash_add(&indexes,(char *)&val,sizeof(int),i,HASH_MODE_ADD);
    }
}

unsigned int __stdcall thread_indexinf(void *arg)
{
    Collection *col=(Collection *)arg;
    inflist_t *t;

    while(1)
    {
        t=&col->inflist[col->pos_out];
        if(++col->pos_out>=LSTCNT)col->pos_out=0;

        WaitForSingleObject(t->dataready,INFINITE);
        if(!t->drp)break;
        if(!*t->inffile)
        {
            t->drp->driverpack_genhashes();
            free(t->adr);
            SetEvent(t->slotvacant);
            continue;
        }

        t->drp->driverpack_indexinf_ansi(t->pathinf,t->inffile,t->adr,t->len);
        free(t->adr);
        SetEvent(t->slotvacant);
    }
    return 0;
}

void driverpack_indexinf_async(Driverpack *drp,Collection *colv,WCHAR const *pathinf,WCHAR const *inffile,char *adr,int len)
{
    Collection *col=colv;
    inflist_t *t=&col->inflist[col->pos_in];
    if(++col->pos_in>=LSTCNT)col->pos_in=0;

    WaitForSingleObject(t->slotvacant,INFINITE);
    if(len>4&&((adr[0]==-1&&adr[3]==0)||adr[0]==0))
    {
        t->adr=(char *)malloc(len+2);
        if(!t->adr)
        {
            log_err("ERROR in driverpack_indexinf: malloc(%d)\n",len+2);
            return;
        }
        len=unicode2ansi(adr,t->adr,len);
    }
    else
    {
        t->adr=(char *)malloc(len);
        memmove(t->adr,adr,len);
    }

    t->drp=drp;
    wcscpy(t->pathinf,pathinf);
    wcscpy(t->inffile,inffile);
    t->len=len;
    SetEvent(t->dataready);
}

void Driverpack::driverpack_parsecat(WCHAR const *pathinf,WCHAR const *inffilename,char *adr,int len)
{
    CHAR filename[BUFLEN];
    CHAR bufa[BUFLEN];
    unsigned bufal=0;
    char *p=adr;

    *bufa=0;
    sprintf(filename,"%ws%ws",pathinf,inffilename);
    while(p+11<adr+len)
    {
        if(*p=='O'&&!memcmp(p,L"OSAttr",11))
        {
            //sprintf(bufb,"%ws",p+19);
            //if(*bufa&&strcmp(bufa,bufb))log_con("^^");
            if(!*bufa||bufal<wcslen((WCHAR *)(p+19)))
            {
                sprintf(bufa,"%ws",p+19);
                bufal=strlen(bufa);
                //log_con("Found '%s'\n",bufa);
                //break;
            }
        }
        p++;
    }
    if(*bufa)
    {
        strtolower(filename,strlen(filename));
        hash_add(&cat_list,filename,strlen(filename),heap_memcpyz_dup(&text_handle,bufa,strlen(bufa)),HASH_MODE_INTACT);
        //log_con("(%s)\n##%s\n",filename,bufa);
    }
}

#ifdef MERGE_FINDER
int checkfolders(WCHAR *folder1,WCHAR *folder2,hashtable_t *filename2path,hashtable_t *path2filename,int sub)
{
    filedata_t *file1,*file2;
    char bufa[BUFLEN];
    char bufa1[BUFLEN];
    int size=0,sizedif=0,sizeuniq=0;
    int isfound;
    int ismergeable=1;

    //log_con("\n%ws\n%ws\n",folder1,folder2);

    sprintf(bufa,"%ws",folder1);
    file1=(filedata_t *)hash_find(path2filename,bufa,strlen(bufa),&isfound);
    while(file1)
    {

        sprintf(bufa1,"%ws",file1->str);
        file2=(filedata_t *)hash_find(filename2path,bufa1,strlen(bufa1),&isfound);
        //log_con("  [%ws]\n",file1->str);

        sizeuniq+=file1->size;
        while(file2)
        {
            if(!wcscmp(folder2,file2->str))
            {
                sizeuniq-=file1->size;
                //if(/*sub&&*/file1->crc!=file2->crc)
                  //  log_con("rem diff %c%ws\t%d\n",file1->crc==file2->crc?'+':'-',file1->str,file1->size);

                if(file1->crc==file2->crc)
                    size+=file1->size;
                else
                {
                    sizedif+=file1->size;
                    ismergeable=0;
                }
            }
            file2=(filedata_t *)hash_findnext(filename2path);
        }

        file1=(filedata_t *)hash_findnext(path2filename);
    }
    if(ismergeable&&sub==0&&size>=1024*1024)
    {
        WCHAR folder1d[BUFLEN],folder2d[BUFLEN],folder3[BUFLEN];
        WCHAR *folder1a=folder1d,*folder2a=folder2d;
        wcscpy(folder1a,folder1);
        wcscpy(folder2a,folder2);
        while(wcschr(folder1a,L'/'))folder1a=wcschr(folder1a,L'/')+1;
        while(wcschr(folder2a,L'/'))folder2a=wcschr(folder2a,L'/')+1;
        folder1a[-1]=0;folder2a[-1]=0;


        log_con("\nrem %ws\nrem %ws\n",folder1,folder2);
        log_con("rem %s (%d,%d,%d)\n",ismergeable?"++++":"----",size/1024,sizedif/1024,sizeuniq/1024);
        int val=checkfolders(folder1d,folder2d,filename2path,path2filename,1);
        log_con("rem subfolders(%ws,%ws):%d\n",folder1d,folder2d,val);

        if(ismergeable&&sub==0)
        {
            //WCHAR rep[BUFLEN],*f1="",*f2="";

            folder1a=folder1d;folder2a=folder2d;
            while(wcschr(folder1a,L'/'))*wcschr(folder1a,L'/')=L'\\';
            while(wcschr(folder2a,L'/'))*wcschr(folder2a,L'/')=L'\\';
            wsprintf(folder3,L"%ws",folder1);

            //printf(rep,L"_merge");
            //if(StrStrIW(folder1,L"6x64")f1="6x64";
            //strsub(folder3,L"6x64",L"merge");
            //strsub(folder3,L"7x64",L"merge");

            log_con("xcopy /S /I /Y /H %ws %ws\n",folder1,folder3);
            log_con("xcopy /S /I /Y /H %ws %ws\n",folder2,folder3);
            log_con("rd /S /Q %ws\nrd /S /Q %ws\n",folder1,folder2);
        }
    }
    if(ismergeable&&!size)return 1;
    return ismergeable?size:0;
}

void hash_clearfiles(hashtable_t *t)
{
    hashitem_t *cur;
    int i=0;

    while(i<t->size)
    {
        cur=&t->items[i++];
        while(1)
        {
            if(t->flags&HASH_FLAG_KEYS_ARE_POINTERS&&cur->key)
            {
                //if(t->id==ID_STRINGS)printf("'%s'\n",cur->key);
                filedata_t *file1;
                file1=(filedata_t *)cur->key;
                //log_con("%x\n",file1);
                //log_con("%ws\n",file1->str);
                free(file1->str);
            }

            if(cur->next<=0)break;
            cur=&t->items[cur->next];
        }
    }
}
#endif

int Driverpack::driverpack_genindex()
{
    CFileInStream archiveStream;
    CLookToRead lookStream;
    CSzArEx db;
    SRes res;
    ISzAlloc allocImp;
    ISzAlloc allocTempImp;
    UInt16 *temp=NULL;
    size_t tempSize=0;
    unsigned i;

#ifdef MERGE_FINDER
    hashtable_t filename2path;
    hashtable_t path2filename;
    hashtable_t foldercmps;
    filedata_t *filedata;
#endif

    WCHAR name[BUFLEN];
    WCHAR pathinf[BUFLEN];
    WCHAR *iinfdilename;

    log_con("Indexing %ws\\%ws\n",text+drppath,text+drpfilename);
    wsprintf(name,L"%ws\\%ws",text+drppath,text+drpfilename);
    //log_file("Scanning '%s'\n",name);
    allocImp.Alloc=SzAlloc;
    allocImp.Free=SzFree;
    allocTempImp.Alloc=SzAllocTemp;
    allocTempImp.Free=SzFreeTemp;

    if(InFile_OpenW(&archiveStream.file,name))return 1;

    FileInStream_CreateVTable(&archiveStream);
    LookToRead_CreateVTable(&lookStream,False);
    lookStream.realStream=&archiveStream.s;
    LookToRead_Init(&lookStream);
    CrcGenerateTable();
    SzArEx_Init(&db);

#ifdef MERGE_FINDER
    hash_init(&filename2path,ID_FILES,1024,HASH_FLAG_KEYS_ARE_POINTERS);
    hash_init(&path2filename,ID_FILES,1024,HASH_FLAG_KEYS_ARE_POINTERS);
    hash_init(&foldercmps,ID_FILES,1024,0);
#endif

    res=SzArEx_Open(&db,&lookStream.s,&allocImp,&allocTempImp);
    if(res==SZ_OK)
    {
      /*
      if you need cache, use these 3 variables.
      if you use external function, you can make these variable as static.
      */
        UInt32 blockIndex=0xFFFFFFFF; /* it can have any value before first call (if outBuffer = 0) */
        Byte *outBuffer=0; /* it must be 0 before first call for each new archive. */
        size_t outBufferSize=0;  /* it can have any value before first call (if outBuffer = 0) */

        for(i=0;i<db.NumFiles;i++)
        {
            size_t offset=0;
            size_t outSizeProcessed=0;
            size_t len;
            if(SzArEx_IsDir(&db,i))continue;

            len=SzArEx_GetFileNameUtf16(&db,i,NULL);
            if(len>tempSize)
            {
                SzFree(NULL,temp);
                tempSize=len;
                temp=(UInt16 *)SzAlloc(NULL,tempSize *sizeof(temp[0]));
                if(temp==0)
                {
                    res=SZ_ERROR_MEM;
                    log_err("ERROR mem(%d)\n",tempSize *sizeof(temp[0]));
                    break;
                }
            }
            SzArEx_GetFileNameUtf16(&db,i,temp);

#ifdef MERGE_FINDER
            {
                char bufa[BUFLEN];
                WCHAR *filename=(WCHAR *)temp;
                while(wcschr(filename,L'/'))filename=wcschr(filename,L'/')+1;
                filename[-1]=0;
                //log_con("%8d,%ws\n",f->Size,temp);

                filedata=malloc(sizeof(filedata_t));
                filedata->crc=f->Crc;
                filedata->size=f->Size;
                filedata->str=malloc(wcslen(temp)*2+2);
                wcscpy(filedata->str,temp);
                sprintf(bufa,"%ws",filename);
                hash_add(&filename2path,bufa,strlen(bufa),(int)filedata,HASH_MODE_ADD);
                //log_con("%8d,%08X,[%s],%ws\n",filedata->size,filedata->crc,bufa,filedata->str);

                filedata=malloc(sizeof(filedata_t));
                filedata->crc=f->Crc;
                filedata->size=f->Size;
                filedata->str=malloc(wcslen(filename)*2+2);
                wcscpy(filedata->str,filename);
                sprintf(bufa,"%ws",temp);
                hash_add(&path2filename,bufa,strlen(bufa),(int)filedata,HASH_MODE_ADD);
                //log_con("%8d,%08X,%ws,[%s]\n",filedata->size,filedata->crc,filedata->str,bufa);
            }
#endif

            if(_wcsicmp((WCHAR *)temp+wcslen((WCHAR *)temp)-4,L".inf")==0)
            {
                WCHAR *ii=(WCHAR *)temp;
                while(*ii)
                {
                    if(*ii=='/')*ii='\\';
                    ii++;
                }
                res = SzArEx_Extract(&db,&lookStream.s,i,
                    &blockIndex,&outBuffer,&outBufferSize,
                    &offset,&outSizeProcessed,
                    &allocImp,&allocTempImp);
                if(res!=SZ_OK)continue;


                iinfdilename=(WCHAR *)temp;
                while(*iinfdilename++);iinfdilename--;
                while(iinfdilename!=(WCHAR *)temp&&*iinfdilename!='\\')iinfdilename--;
                if(*iinfdilename=='\\'){*iinfdilename++=0;}
                wsprintf(pathinf,L"%ws\\",temp);
//                log_file("%10ld, %10ld, Openning '%ws%ws'\n",offset,outSizeProcessed,pathinf,iinfdilename);

//                driverpack_indexinf(drp,pathinf,iinfdilename,(char *)(outBuffer+offset),f->Size);
                driverpack_indexinf_async(this,col,pathinf,iinfdilename,(char *)(outBuffer+offset),outSizeProcessed);
            }
            if(_wcsicmp((WCHAR *)temp+wcslen((WCHAR *)temp)-4,L".cat")==0)
            {
                WCHAR *ii=(WCHAR *)temp;
                while(*ii)
                {
                    if(*ii=='/')*ii='\\';
                    ii++;
                }
                res = SzArEx_Extract(&db,&lookStream.s,i,
                    &blockIndex,&outBuffer,&outBufferSize,
                    &offset,&outSizeProcessed,
                    &allocImp,&allocTempImp);
                if(res!=SZ_OK)continue;

                iinfdilename=(WCHAR *)temp;
                while(*iinfdilename++);iinfdilename--;
                while(iinfdilename!=(WCHAR *)temp&&*iinfdilename!='\\')iinfdilename--;
                if(*iinfdilename=='\\'){*iinfdilename++=0;}
                wsprintf(pathinf,L"%ws\\",temp);
                driverpack_parsecat(pathinf,iinfdilename,(char *)(outBuffer+offset),outSizeProcessed);
            }
        }
#ifdef MERGE_FINDER
        for(i=0;i<db.db.NumFiles;i++)
        {
            char bufa[BUFLEN];
            char bufa1[BUFLEN];
            int isfound;
            SzArEx_GetFileNameUtf16(&db,i,temp);
            const CSzFileItem *f=db.db.Files+i;

            WCHAR *filename=(WCHAR *)temp;
            while(wcschr(filename,L'/'))filename=wcschr(filename,L'/')+1;
            filename[-1]=0;

            //log_con("%ws,%ws\n",temp,filename);
            sprintf(bufa,"%ws",filename);
            filedata=(filedata_t *)hash_find(&filename2path,bufa,strlen(bufa),&isfound);
            while(filedata)
            {
                sprintf(bufa1,"%ws - %ws",temp,filedata->str);
                hash_find(&foldercmps,bufa1,strlen(bufa1),&isfound);

                if(f->Crc==filedata->crc&&wcscmp(temp,filedata->str)&&!isfound)
                {
                    checkfolders(temp,filedata->str,&filename2path,&path2filename,0);
                    //log_con("  %ws\n",filedata->str);
                }

                sprintf(bufa1,"%ws - %ws",temp,filedata->str);
                hash_add(&foldercmps,bufa1,strlen(bufa1),1,HASH_MODE_INTACT);

                sprintf(bufa1,"%ws - %ws",filedata->str,temp);
                hash_add(&foldercmps,bufa1,strlen(bufa1),1,HASH_MODE_INTACT);

                filedata=(filedata_t *)hash_findnext(&filename2path);
            }
        }
#endif

        IAlloc_Free(&allocImp,outBuffer);
    }
    SzArEx_Free(&db,&allocImp);
    SzFree(NULL,temp);
    File_Close(&archiveStream.file);

#ifdef MERGE_FINDER
    hash_clearfiles(&filename2path);
    hash_clearfiles(&path2filename);
    hash_free(&filename2path);
    hash_free(&path2filename);
    hash_free(&foldercmps);
#endif

    driverpack_indexinf_async(this,col,L"",L"",0,0);
    return 1;
}

void Driverpack::driverpack_indexinf(WCHAR const *drpdir,WCHAR const *iinfdilename,char *bb,int inf_len)
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
        driverpack_indexinf_ansi(drpdir,iinfdilename,buf_out,size);
        free(buf_out);
    }
    else
    {
        driverpack_indexinf_ansi(drpdir,iinfdilename,bb,inf_len);
    }
}
// http://msdn.microsoft.com/en-us/library/ff547485(v=VS.85).aspx
void Driverpack::driverpack_indexinf_ansi(WCHAR const *drpdir,WCHAR const *inffilename,char *inf_base,int inf_len)
{
    version_t *cur_ver;
    sect_data_t strlink;

    int cur_inffile_index;
    data_inffile_t *cur_inffile;
    int cur_manuf_index;
    data_manufacturer_t *cur_manuf;
    int cur_desc_index;
    data_desc_t * cur_desc;
    int cur_HWID_index;
    data_HWID_t *cur_HWID;

    char date_s[256];
    char build_s[256];
    char secttry[256];
    char line[2048];
    int  strs[16];

    Parser_str parse_info,parse_info2,parse_info3;
    sect_data_t *lnk,*lnk2;

    //hashtable_t *string_list=&string_list;
    //hashtable_t *section_list=&section_list;

    parse_info.init(this);
    parse_info2.init(this);
    parse_info3.init(this);

    char *p=inf_base,*strend=inf_base+inf_len;
    char *p2,*sectnmend;
    char *lnk_s=0;

    cur_inffile_index=heap_allocitem_i(&inffile_handle);
    cur_inffile=&inffile[cur_inffile_index];
    sprintf(line,"%ws",drpdir);
    cur_inffile->infpath=heap_strcpy(&text_handle,line);
    sprintf(line,"%ws",inffilename);
    cur_inffile->inffilename=heap_strcpy(&text_handle,line);
    cur_inffile->infsize=inf_len;
    cur_inffile->infcrc=0;
    //log_file("%ws%ws\n",drpdir,inffile);

    // Populate sections
    //char *b;
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
                if(lnk_s)
                    ((sect_data_t *)lnk_s)->len=p-inf_base;
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

                strlink.ofs=p2-inf_base;
                strlink.len=inf_len;
                {
                    lnk_s=memcpy_alloc((char *)(&strlink),sizeof(sect_data_t));
                    hash_add(&section_list,p,sectnmend-p,(intptr_t)lnk_s,HASH_MODE_ADD);
                }
                p=p2;
                break;

            default:
                //b=p;
                //while(*p!='\n'&&*p!='\r'&&*p!='['&&p<strend)p++;
                //if(*p=='['&&p<strend)log_file("ERROR in %ws%ws:\t\t\t'%.*s'(%d/%d)\n",drpdir,inffile,p-b+20,b,p,strend);
                while(p<strend&&*p!='\n'&&*p!='\r'/*&&*p!='['*/)cur_inffile->infcrc+=*p++;
        }
    }

    // Find [strings]
    lnk=(sect_data_t *)hash_find_str(&section_list,"strings");
    if(!lnk)log_index("ERROR: missing [strings] in %ws%ws\n",drpdir,inffilename);
    while(lnk)
    {
        char *s1b,*s1e,*s2b,*s2e;

        parse_info.setRange(inf_base,lnk);
        while(parse_info.parseItem())
        {
            parse_info.readStr(&s1b,&s1e);
            parse_info.parseField();
            parse_info.readStr(&s2b,&s2e);
            hash_add(&string_list,s1b,s1e-s1b,(intptr_t)memcpy_alloc(s2b,s2e-s2b),HASH_MODE_INTACT);
        }
        lnk=(sect_data_t *)hash_findnext(&section_list);
        if(lnk)log_index("NOTE: multiple [strings] in %ws%ws\n",drpdir,inffilename);
    }

    // Find [version]
    date_s[0]=0;
    build_s[0]=0;
    cur_ver=&cur_inffile->version;
    cur_ver->v1=-1;
    cur_ver->y=-1;

    lnk=(sect_data_t *)hash_find_str(&section_list,"version");
    if(!lnk)log_index("ERROR: missing [version] in %ws%ws\n",drpdir,inffilename);
    while(lnk)
    {
        char *s1b,*s1e;

        parse_info.setRange(inf_base,lnk);
        while(parse_info.parseItem())
        {
            parse_info.readStr(&s1b,&s1e);
            strtolower(s1b,s1e-s1b);

            int i,sz=s1e-s1b;
            for(i=0;i<NUM_VER_NAMES;i++)
            if(table_version[i].sz==sz&&!memcmp(s1b,table_version[i].s,sz))
            {
                if(i==DriverVer)
                {
                        // date
                        parse_info.parseField();
                        i=parse_info.readDate(cur_ver);
                        if(i)log_index("ERROR: invalid date(%d.%d.%d)[%d] in %ws%ws\n",
                                 cur_ver->d,cur_ver->m,cur_ver->y,i,drpdir,inffilename);

                        sprintf(date_s,"%02d/%02d/%04d",cur_ver->m,cur_ver->d,cur_ver->y);

                        // version
                        if(parse_info.parseField())
                        {
                            parse_info.readVersion(cur_ver);
                        }
                        sprintf(build_s,"%d.%d.%d.%d",cur_ver->v1,cur_ver->v2,cur_ver->v3,cur_ver->v4);

                }else
                {
                    parse_info.parseField();
                    parse_info.readStr(&s1b,&s1e);
                    cur_inffile->fields[i]=heap_memcpyz(&text_handle,s1b,s1e-s1b);
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
        lnk=(sect_data_t *)hash_findnext(&section_list);
        if(lnk)log_index("NOTE:  multiple [version] in %ws%ws\n",drpdir,inffilename);
        if(!lnk)
        {
            if(cur_ver->y==-1) log_index("ERROR: missing date in %ws%ws\n",drpdir,inffilename);
            if(cur_ver->v1==-1)log_index("ERROR: missing build number in %ws%ws\n",drpdir,inffilename);
        }
    }

    // Find [manufacturer] section
    lnk=(sect_data_t *)hash_find_str(&section_list,"manufacturer");
    if(!lnk)log_index("ERROR: missing [manufacturer] in %ws%ws\n",drpdir,inffilename);
    while(lnk)
    {
        parse_info.setRange(inf_base,lnk);
        while(parse_info.parseItem())
        {
            char *s1b,*s1e;
            parse_info.readStr(&s1b,&s1e);

            cur_manuf_index=heap_allocitem_i(&manufacturer_handle);
            cur_manuf=&manufacturer_list[cur_manuf_index];
            cur_manuf->inffile_index=cur_inffile_index;
            cur_manuf->manufacturer=heap_memcpyz(&text_handle,s1b,s1e-s1b);
            cur_manuf->sections_n=0;

            if(parse_info.parseField())
            {
                parse_info.readStr(&s1b,&s1e);
                strtolower(s1b,s1e-s1b);
                strs[cur_manuf->sections_n++]=heap_memcpyz(&text_handle,s1b,s1e-s1b);
                while(1)
                {
                    find_t savedfind0;
                    if(cur_manuf->sections_n>1)
                        sprintf(secttry,"%s.%s",
                                text+strs[0],
                                text+strs[cur_manuf->sections_n-1]);
                    else
                        sprintf(secttry,"%s",text+strs[0]);

                    memcpy(&savedfind0,&section_list.finddata,sizeof(find_t));
                    lnk2=(sect_data_t *)hash_find_str(&section_list,secttry);
                    if(!lnk2&&cur_manuf->sections_n>1)log_index("ERROR: missing [%s] in %ws%ws\n",secttry,drpdir,inffilename);
                    while(lnk2)
                    {
                        parse_info2.setRange(inf_base,lnk2);
                        while(parse_info2.parseItem())
                        {
                            parse_info2.readStr(&s1b,&s1e);

                            cur_desc_index=heap_allocitem_i(&desc_list_handle);
                            cur_desc=&desc_list[cur_desc_index];
                            cur_desc->manufacturer_index=cur_manuf_index;
                            cur_desc->sect_pos=manufacturer_list[cur_manuf_index].sections_n-1;
                            cur_desc->desc=heap_memcpyz_dup(&text_handle,s1b,s1e-s1b);

                            //{ featurescore
                            cur_desc->feature=0xFF;
                            char installsection[BUFLEN];
                            sect_data_t *lnk3;
                            //parse_info3.pack=drp;
                            find_t savedfind;

                            parse_info2.parseField();
                            parse_info2.readStr(&s1b,&s1e);
                            cur_desc->install=heap_memcpyz_dup(&text_handle,s1b,s1e-s1b);

                            memcpy(&savedfind,&section_list.finddata,sizeof(find_t));

                            sprintf(installsection,"%.*s.nt",s1e-s1b,s1b);
                            strtolower(installsection,strlen(installsection));
                            lnk3=(sect_data_t *)hash_find_str(&section_list,installsection);

                            if(!lnk3)
                            {
                                sprintf(installsection,"%.*s",s1e-s1b,s1b);
                                strtolower(installsection,strlen(installsection));
                                lnk3=(sect_data_t *)hash_find_str(&section_list,installsection);
                            }
                            if(!lnk3)
                            {
                                if(cur_manuf->sections_n>1)
                                    sprintf(installsection,"%.*s.%s",s1e-s1b,s1b,text+strs[cur_manuf->sections_n-1]);
                                else
                                    sprintf(installsection,"%.*s",s1e-s1b,s1b);

                                strtolower(installsection,strlen(installsection));
                                while(strlen(installsection)>=(unsigned)(s1e-s1b))
                                {
                                    lnk3=(sect_data_t *)hash_find_str(&section_list,installsection);
                                    if(lnk3)break;
                                    //log_file("Tried '%s'\n",installsection);
                                    installsection[strlen(installsection)-1]=0;
                                }
                            }
                            char iii[BUFLEN];
                            *iii=0;
                            //int cnt=0;
                            sect_data_t *tlnk;
                            if(!lnk3)
                            {
                                int i;
                                for(i=0;i<NUM_DECS;i++)
                                {
                                    sprintf(installsection,"%.*s.%s",s1e-s1b,s1b,nts[i]);
                                    strtolower(installsection,strlen(installsection));
                                    tlnk=(sect_data_t *)hash_find_str(&section_list,installsection);
                                    if(tlnk&&!lnk3)lnk3=tlnk;
                                    if(tlnk)
                                    {
                                        strcat(iii,installsection);
                                        strcat(iii,",");
                                    }
                                    //if(lnk3){log_file("Found '%s'\n",installsection);cnt++;}
                                }
                            }
                            //if(cnt>1)log_file("@num: %d\n",cnt);
                            //if(cnt>1&&!lnk3)log_file("ERROR in %ws%ws:\t\t\tMissing [%s]\n",drpdir,inffilename,iii);
                            if(lnk3)
                            {
                                if(*iii)sprintf(installsection,"$%s",iii);
                                cur_desc->install_picked=heap_memcpyz_dup(&text_handle,installsection,strlen(installsection));
                            }
                            else
                            {
                                cur_desc->install_picked=heap_memcpyz_dup(&text_handle,"{missing}",9);
                            }

                            while(lnk3)
                            {
                                parse_info3.setRange(inf_base,lnk3);
                                if(!strcmp(secttry,installsection))
                                {
                                    log_index("ERROR: [%s] refers to itself in %ws%ws\n",installsection,drpdir,inffilename);
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
                                        cur_desc->feature=parse_info3.readHex();
                                    }
                                    while(parse_info3.parseField());
                                }
                                lnk3=(sect_data_t *)hash_findnext(&section_list);
                            }
                            memcpy(&section_list.finddata,&savedfind,sizeof(find_t));
                            //} feature

                            int hwid_pos=0;
                            while(parse_info2.parseField())
                            {
                                parse_info2.readStr(&s1b,&s1e);
                                if(s1b>=s1e)continue;
                                strtoupper(s1b,s1e-s1b);

                                cur_HWID_index=heap_allocitem_i(&HWID_list_handle);
                                cur_HWID=&HWID_list[cur_HWID_index];
                                cur_HWID->desc_index=cur_desc_index;
                                cur_HWID->HWID=heap_memcpyz_dup(&text_handle,s1b,s1e-s1b);
                                cur_HWID->inf_pos=hwid_pos++;

                                /*sprintf(line,"%s%s",text+cur_HWID->HWID,text+cur_desc->desc);
                                if(fi&&!hash_find_str(dup_list,line))
                                {
                                    hash_add(dup_list,line,strlen(line),1,HASH_MODE_INTACT);
                                    trap_mode=0;
                                }*/
                            }
                        }
                        lnk2=(sect_data_t *)hash_findnext(&section_list);
                        if(lnk2)log_index("NOTE:  multiple [%s] in %ws%ws\n",secttry,drpdir,inffilename);
                    }
                    memcpy(&section_list.finddata,&savedfind0,sizeof(find_t));

                    if(!parse_info.parseField())break;
                    parse_info.readStr(&s1b,&s1e);
                    if(s1b>s1e)break;
                    strtolower(s1b,s1e-s1b);
                    strs[cur_manuf->sections_n++]=heap_memcpyz(&text_handle,s1b,s1e-s1b);
                }
            }
            cur_manuf->sections=heap_memcpyz(&text_handle,strs,sizeof(int)*cur_manuf->sections_n);

        }
        lnk=(sect_data_t *)hash_findnext(&section_list);
        if(lnk)log_index("NOTE:  multiple [manufacturer] in %ws%ws\n",drpdir,inffilename);
    }
    //hash_stats(string_list);
    //hash_stats(section_list);
    //hash_stats(dup_list);

    parse_info.release();
    parse_info2.release();
    parse_info3.release();

    hash_clear(&string_list,1);
    hash_clear(&section_list,1);
}
//}
