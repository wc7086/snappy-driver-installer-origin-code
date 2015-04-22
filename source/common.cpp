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

Test::Test()
{
    log_con("Test(%s) created\n",s.c_str());
}
Test::~Test()
{
    log_con("Test destroyed\n");
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

void Txt::reset(int sz)
{
    text.resize(sz);
    text.reserve(1024*1024*2); //TODO
}

Txt::Txt()
{
    reset(2);
    text[0]=text[1]=0;
}

Txt::~Txt()
{
}

void Txt::shrink()
{
    //log_con("Text_usage %d/%d\n",text.size(),text.capacity());
    text.shrink_to_fit();
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

/*void str_unicode2ansi(char *a)
{
    wchar_t *u=(wchar_t *)a;
    while((*a++=*u++));
}
void str_unicode2ansi(const wchar_t *s,char *d)
{
    while((*d++=*s++));
}
void str_ansi2unicode(const wchar_t *a)
{
    wchar_t *u=(wchar_t *)a;
    while((*a++=*u++));
}*/

int _wtoi_my(const wchar_t *str)
{
    int val;
    swscanf(str,L"%d",&val);
    return val;
}

//}

//{ Hash
unsigned hash_getcode(const char *s,int sz)
{
    int h=5381;

    while(sz--)
    {
        int ch=*s++;
        h=((h<<5)+h)^ch;
    }
    return h;
}

void hash_init(hashtable_t *t,int size)
{
    t->size=size;
    t->items_new.resize(t->size);
    t->items_new.reserve(t->size*4);
    memset(t->items_new.data(),0,t->size*sizeof(hashitem_t));
}

char *hash_save(hashtable_t *t,char *p)
{
    memcpy(p,&t->size,sizeof(int));p+=sizeof(int);
    p=vector_save(&t->items_new,p);
    return p;
}

char *hash_load(hashtable_t *t,char *p)
{
    memcpy(&t->size,p,sizeof(int));p+=sizeof(int);
    t->items_new.resize(t->size);
    p=vector_load(&t->items_new,p);
    return p;
}

/*
next
      0: free
     -1: used,next is free
  1..x : used,next is used
*/
void hash_add(hashtable_t *t,int key,int value)
{
    hashitem_t *cur;
    int curi;
    int previ=-1;

    curi=hash_getcode((char *)&key,sizeof(int))%t->size;
    cur=&t->items_new[curi];

    if(cur->next!=0)
    do
    {
        cur=&t->items_new[curi];
        previ=curi;
    }
    while((curi=cur->next)>0);

    if(cur->next==-1)
    {
        t->items_new.emplace_back(hashitem_t());
        cur=&t->items_new.back();
        curi=t->items_new.size()-1;
    }

    cur->key=key;
    cur->value=value;
    cur->next=-1;
    if(previ>=0)(&t->items_new[previ])->next=curi;
}

int hash_find(hashtable_t *t,int key,int *isfound)
{
    hashitem_t *cur;
    int curi;

    if(!t->size)
    {
        *isfound=0;
        return 0;
    }
    curi=hash_getcode((char *)&key,sizeof(int))%t->size;
    cur=&t->items_new[curi];

    if(cur->next<0)
    {
        if(key==cur->key)
        {
            t->findnext=cur->next;
            t->findstr=key;
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
        cur=&t->items_new[curi];
        if(key==cur->key)
        {
            t->findnext=cur->next;
            t->findstr=key;
            *isfound=1;
            return cur->value;
        }
    }
    while((curi=cur->next)>0);

    *isfound=0;
    return 0;
}

int hash_findnext(hashtable_t *t,int *isfound)
{
    hashitem_t *cur;
    int curi=t->findnext;

    *isfound=0;
    if(curi<=0)return 0;

    cur=&t->items_new[curi];
    do
    {
        cur=&t->items_new[curi];
        if(cur->key==t->findstr)
        {
            t->findnext=cur->next;
            *isfound=1;
            return cur->value;
        }
    }
    while((curi=cur->next)>0);
    return 0;
}
//}

//{ 7-zip
void registerall()
{
#ifndef CONSOLE_MODE
    registercrc();
    NArchive::N7z::register7z();
    registerBCJ();
    registerBCJ2();
    registerBranch();
    registerCopy();
    registerLZMA();
    registerLZMA2();
//  registerPPMD();
    registerDelta();
    registerByteSwap();
#endif
}
//}
