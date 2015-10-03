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

#include "com_header.h"
#include "7zip.h"

#include <windows.h>
#include <shlwapi.h>        // for StrStrIW

#include "common.h"
#include "guicon.h"

Test::Test()
{
    Log.print_con("Test(%s) created\n",s.c_str());
}
Test::~Test()
{
    Log.print_con("Test destroyed\n");
}

//{ Txt
int Txt::strcpy(const char *str)
{
    int r=text.size();
    text.insert(text.end(),str,str+strlen(str)+1);
    return r;
}

int Txt::strcpyw(const wchar_t *str)
{
    int r=text.size();
    text.insert(text.end(),(char *)str,(char *)(str+wcslen(str)+1));
    return r;
}

int Txt::t_memcpy(const char *mem,int sz)
{
    int r=text.size();
    text.insert(text.end(),mem,mem+sz);
    return r;
}

int Txt::t_memcpyz(const char *mem,int sz)
{
    int r=text.size();
    text.insert(text.end(),mem,mem+sz);
    text.insert(text.end(),0);
    return r;
}

int Txt::memcpyz_dup(const char *mem,int sz)
{
    std::string str(mem,sz);
    auto it=dub.find(str);

    if(it==dub.end())
    {
        int r=text.size();
        text.insert(text.end(),mem,mem+sz);
        text.insert(text.end(),0);

        dub.insert({{std::move(str),r}});
        return r;
    }
    else
    {
        return it->second;
    }
}

int Txt::alloc(int sz)
{
    int r=text.size();
    text.resize(r+sz);
    return r;
}

Txt::Txt()
{
    reset(2);
    text[0]=text[1]=0;
}

void Txt::reset(int sz)
{
    text.resize(sz);
    text.reserve(1024*1024*2); //TODO
}

void Txt::shrink()
{
    //Log.print_con("Text_usage %d/%d\n",text.size(),text.capacity());
    text.shrink_to_fit();
}
//}

//{ Hashtable
unsigned Hashtable::gethashcode(const char *s,int sz)
{
    int h=5381;

    while(sz--)
    {
        int ch=*s++;
        h=((h<<5)+h)^ch;
    }
    return h;
}

void Hashtable::reset(int size1)
{
    size=size1;
    if(!size)size=1;
    items.resize(size);
    items.reserve(size*4);
    memset(items.data(),0,size*sizeof(Hashitem));
}

char *Hashtable::save(char *p)
{
    memcpy(p,&size,sizeof(int));p+=sizeof(int);
    p=vector_save(&items,p);
    return p;
}

char *Hashtable::load(char *p)
{
    memcpy(&size,p,sizeof(int));p+=sizeof(int);
    items.resize(size);
    p=vector_load(&items,p);
    return p;
}

/*
next
      0: free
     -1: used,next is free
  1..x : used,next is used
*/
void Hashtable::additem(int key,int value)
{
    int curi=gethashcode((char *)&key,sizeof(int))%size;
    Hashitem *cur=&items[curi];

    int previ=-1;
    if(cur->next!=0)
    do
    {
        cur=&items[curi];
        previ=curi;
    }
    while((curi=cur->next)>0);

    if(cur->next==-1)
    {
        items.emplace_back(Hashitem());
        cur=&items.back();
        curi=items.size()-1;
    }

    cur->key=key;
    cur->value=value;
    cur->next=-1;
    if(previ>=0)(&items[previ])->next=curi;
}

int Hashtable::find(int key,int *isfound)
{
    if(!size)
    {
        *isfound=0;
        return 0;
    }

    int curi=gethashcode((char *)&key,sizeof(int))%size;
    Hashitem *cur=&items[curi];

    if(cur->next<0)
    {
        if(key==cur->key)
        {
            findnext_v=cur->next;
            findstr=key;
            *isfound=1;
            return cur->value;
        }
    }

    if(cur->next==0)
    {
        *isfound=0;
        return 0;
    }

    do
    {
        cur=&items[curi];
        if(key==cur->key)
        {
            findnext_v=cur->next;
            findstr=key;
            *isfound=1;
            return cur->value;
        }
    }
    while((curi=cur->next)>0);

    *isfound=0;
    return 0;
}

int Hashtable::findnext(int *isfound)
{
    Hashitem *cur;
    int curi=findnext_v;

    *isfound=0;
    if(curi<=0)return 0;

    cur=&items[curi];
    do
    {
        cur=&items[curi];
        if(cur->key==findstr)
        {
            findnext_v=cur->next;
            *isfound=1;
            return cur->value;
        }
    }
    while((curi=cur->next)>0);
    return 0;
}
//}

//{ Strings
void strsub(wchar_t *str,const wchar_t *pattern,const wchar_t *rep)
{
    wchar_t *s;

    s=StrStrIW(str,pattern);
    if(s)
    {
        wchar_t buf[BUFLEN];
        wcscpy(buf,s);
        wcscpy(s,rep);
        wcscpy(s+wcslen(rep),buf+wcslen(pattern));
    }
}

void strtoupper(char *s,int len)
{
    while(len--)
    {
        *s=toupper(*s);
        s++;
    }
}

void strtolower(char *s,int len)
{
    if(len)
    while(len--)
    {
        *s=tolower(*s);
        s++;
    }
}

int unicode2ansi(char *s,char *out,int size)
{
    int ret,flag;
    size/=2;
    /*if(!out)Log.log_err("Error out:\n");
    if(!s)Log.log_err("Error in:\n");
    if(size<0)Log.log_err("Error size:\n");*/
    ret=WideCharToMultiByte(CP_ACP,0,(wchar_t *)(s+(s[0]==-1?2:0)),size-(s[0]==-1?1:0),out,size,nullptr,&flag);
    if(!ret)Log.print_syserr(GetLastError(),L"unicode2ansi()");
    out[size]=0;
    return ret;
}

int _wtoi_my(const wchar_t *str)
{
    int val;
    swscanf(str,L"%d",&val);
    return val;
}
//}

//{ 7-zip
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

void registerall()
{
#ifndef CONSOLE_MODE
    NArchive::N7z::register7z();
    registerBCJ();
    registerBCJ2();
    registerBranch();
    registerCopy();
    registerLZMA();
    registerLZMA2();

    CrcGenerateTable();
#endif
}
//}
