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

//{ Global variables
int trap_mode=0;

const char *heaps[NUM_HEAPS]= // heap_t
{
    "ID_HASH_ITEMS",      // 0
    "ID_HASH_STR",        // 1
};

const int heapsz[NUM_HEAPS]=
{
    1024, // auto
    1024, // auto
};
//}

//{ Heap
void heap_refresh(heap_t *t)
{
    void *p;
    p=malloc(t->allocated);
    memcpy(p,t->base,t->used);
    memset(t->base,0,t->used);
    free(t->base);
    t->base=p;
    *t->membck=t->base;
}

void heap_expand(heap_t *t,int sz)
{
    if(t->used+sz>t->allocated)
    {
        //printf("Expand[%-25s] +%d, (%d/%d) -> ",heaps[t->id],sz,t->used,t->allocated);
        while(t->used+sz>t->allocated)
            t->allocated*=2;
        //printf("(%d/%d)\n",t->used+sz,t->allocated);

        heap_refresh(t);
/*        t->base=realloc(t->base,t->allocated);
        if(!t->base)
        {
            heap_refresh(t);
            return;
            //t->base=realloc_wrap(t->base,t->used,t->allocated);
        }
        *t->membck=t->base;*/
    }
}

void heap_init(heap_t *t,int id,void **mem,int sz, int itemsize)
{
    t->id=id;
    if(!sz)sz=heapsz[id];
    //log_file("initsize %s: %d\n",heaps[t->id],sz);
    t->base=malloc(sz);
    if(!t->base)log_err("No mem %d\n",sz);
    t->membck=mem;
    t->used=0;
    t->allocated=sz;
    t->itemsize=itemsize;
    t->items=0;
    *t->membck=t->base;

//    t->dup=0;
}

void heap_free(heap_t *t)
{
    if(t->base)free(t->base);
    t->base=nullptr;
    *t->membck=nullptr;
}

void heap_reset(heap_t *t,int sz)
{
    t->used=sz;
    t->items=0;
}

int heap_alloc(heap_t *t,int sz)
{
    int r=t->used;

    heap_expand(t,sz);
    t->used+=sz;
    return r;
}

int heap_allocitem_i(heap_t *t)
{
    int r=t->used;

    heap_expand(t,t->itemsize);
    memset((char *)t->base+t->used,0,t->itemsize);
    t->used+=t->itemsize;
    t->items++;
    return r/t->itemsize;
}
/*
void *heap_allocitem_ptr(heap_t *t)
{
    int r=t->used;

    heap_expand(t,t->itemsize);
    memset((char *)t->base+t->used,0,t->itemsize);
    t->used+=t->itemsize;
    t->items++;
    return (char *)t->base+r;
}

void heap_freelastitem(heap_t *t)
{
    t->items--;
    t->used-=t->itemsize;
}

int heap_memcpy(heap_t *t,const void *mem,int sz)
{
    int r=t->used;

    heap_expand(t,sz);
    memcpy((char *)t->base+t->used,mem,sz);
    t->used+=sz;
    return r;
}

int heap_strcpy(heap_t *t,const char *s)
{
    int r=t->used;
    int sz=strlen(s)+1;

    heap_expand(t,sz);
    memcpy((char *)t->base+t->used,s,sz);
    t->used+=sz;
    return r;
}

int heap_memcpyz(heap_t *t,const void *mem,int sz)
{
    int r=t->used;

    heap_expand(t,sz+1);
    memcpy((char *)t->base+t->used,mem,sz);
    t->used+=sz+1;
    *(char *)((intptr_t)t->base+r+sz)=0;
    return r;
}*/

/*int heap_memcpyz_dup(heap_t *t,const void *mem,int sz)
{
    int r=t->used;

    heap_expand(t,sz+1);

    if(!t->dup)
    {
        t->dup=(hashtable_t *)malloc(sizeof(hashtable_t));
        hash_init(t->dup,ID_MEMCPYZ_DUP,4096*2,0);
    }
    if(t->dup)
    {
        int y,flag;
        y=hash_find(t->dup,(char*)mem,sz,&flag);
        if(flag)return y;
    }

    char *u=(char *)t->base+t->used;
    memcpy(u,mem,sz);
    t->used+=sz+1;
    *(char *)((intptr_t)t->base+r+sz)=0;

    if(t->dup)hash_add(t->dup,u,sz,r,HASH_MODE_INTACT);
    return r;
}*/
/*
int heap_strtolowerz(heap_t *t,const char *s,int sz)
{
    int r=t->used;

    heap_expand(t,sz+1);
    memcpy((char *)t->base+t->used,s,sz);
    strtolower((char *)t->base+t->used,sz);
    *(char *)((intptr_t)t->base+r+sz)=0;
    t->used+=sz+1;
    return r;
}*/

char *heap_save(heap_t *t,char *p)
{
    memcpy(p,&t->used,sizeof(t->used));p+=sizeof(t->used);
    memcpy(p,&t->items,sizeof(t->items));p+=sizeof(t->items);
    memcpy(p,t->base,t->used);p+=t->used;
    return p;
}

char *heap_load(heap_t *t,char *p)
{
    int sz;

    memcpy(&sz,p,sizeof(t->used));p+=sizeof(t->used);
    memcpy(&t->items,p,sizeof(t->items));p+=sizeof(t->items);
    t->used=0;
    heap_alloc(t,sz);
    memcpy(t->base,p,t->used);p+=t->used;
    return p;
}

//}

//{ Strings
void strsub(wchar_t *str,const wchar_t *pattern,const wchar_t *rep)
{
    wchar_t buf[BUFLEN],*s;

    s=StrStrIW(str,pattern);
    if(s)
    {
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

/*char *strtolower_alloc(const char *s)
{
    char *d,*p;

    p=d=(char *)malloc(strlen(s)+1);
    do
    {
        *p=tolower(*s);
        s++;p++;
    }
    while(*s);
    *p=0;
    return d;
}

char *memcpy_alloc(const char *s,int sz)
{
    char *d;

    d=(char *)malloc(sz+1);
    memcpy(d,s,sz);
    d[sz]=0;
    return d;
}*/
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

//{ Hash
unsigned hash_getcode(const char *s,int sz)
{
    int h=5381;
    int ch;

    while(sz--)
    {
        ch=*s++;
        h=((h<<5)+h)^ch;
    }
    return h;
}

void hash_init(hashtable_t *t,int size)
{
    memset(t,0,sizeof(hashtable_t));
    t->size=size;

    heap_init(&t->items_handle,ID_HASH_ITEMS,(void **)&t->items,t->size*sizeof(hashitem_t)*2,sizeof(hashitem_t));
    heap_alloc(&t->items_handle,t->size*sizeof(hashitem_t));
    t->items_handle.items=t->size;
    memset(t->items,0,t->size*sizeof(hashitem_t));

    heap_init(&t->strs_handle,ID_HASH_STR,(void **)&t->strs,4096,1);
    heap_alloc(&t->strs_handle,1);
}

void hash_clear(hashtable_t *t,int zero)
{
    hashitem_t *cur;
    int i=0;

    while(i<t->size)
    {
        cur=&t->items[i++];
        while(1)
        {
            if(cur->next<=0)break;
            cur=&t->items[cur->next];
        }
    }
    if(zero)memset(t->items,0,t->size*sizeof(hashitem_t));
    heap_reset(&t->items_handle,t->size*sizeof(hashitem_t));
    heap_reset(&t->strs_handle,1);
}

void hash_free(hashtable_t *t)
{
    hash_clear(t,0);
    heap_free(&t->items_handle);
    heap_free(&t->strs_handle);
}

char *hash_save(hashtable_t *t,char *p)
{
    memcpy(p,&t->size,sizeof(int));p+=sizeof(int);
    memcpy(p,&t->items_handle.used,sizeof(int));p+=sizeof(int);
    memcpy(p,&t->items_handle.items,sizeof(int));p+=sizeof(int);
    memcpy(p,t->items,t->items_handle.used);p+=t->items_handle.used;
    return p;
}

char *hash_load(hashtable_t *t,char *p)
{
    memcpy(&t->size,p,sizeof(int));p+=sizeof(int);
    hash_init(t,t->size);
    p=heap_load(&t->items_handle,p);
    return p;
}

/*
next
     -1: used,next is free
      0: free,next is free
  1..x : used,next is used
*/

void hash_add(hashtable_t *t,int key,int value)
{
    hashitem_t *cur;
    int curi;
    int previ=-1;
    int cur_deep=0;

    curi=hash_getcode((char *)&key,sizeof(int))%t->size;
    cur=&t->items[curi];

    if(cur->next!=0)
    do
    {
        cur_deep++;
        cur=&t->items[curi];
        previ=curi;
    }
    while((curi=cur->next)>0);

    if(cur->next==-1)
    {
        curi=heap_allocitem_i(&t->items_handle);
        cur=&t->items[curi];
    }

    cur->key=key;
    cur->value=value;
    cur->next=-1;
    if(previ>=0)(&t->items[previ])->next=curi;
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
    cur=&t->items[curi];

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
        cur=&t->items[curi];
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

    cur=&t->items[curi];
    do
    {
        cur=&t->items[curi];
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
