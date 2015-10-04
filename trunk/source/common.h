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

#ifndef COMMON_H
#define COMMON_H

#include <unordered_map>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#define BOOST_SYSTEM_NO_DEPRECATED
#include <boost/thread/condition_variable.hpp>
#pragma GCC diagnostic pop

// Global vars
extern int trap_mode;

// Test
class Test
{
public:
    std::string s;
    Test();
    ~Test();
};

// Txt
class Txt
{
    std::unordered_map<std::string,int> dub;
    std::vector<char> text;

public:
    unsigned getSize()const{return text.size();}
    std::vector<char> *getVector(){return &text;}
    char *get(int offset){return &text[offset];}
    wchar_t *getw(int offset){return (wchar_t *)(&text[offset]);}
    wchar_t *getw2(int offset){return (wchar_t *)(&text[offset-(text[0]?2:0)]);}

    int strcpy(const char *mem);
    int strcpyw(const wchar_t *mem);
    int t_memcpy(const char *mem,int sz);
    int t_memcpyz(const char *mem,int sz);
    int memcpyz_dup(const char *mem,int sz);
    int alloc(int sz);

    Txt();
    void reset(int sz);
    void shrink();
};

// Vector templates
template <class T>
char *vector_save(std::vector<T> *v,char *p)
{
    int used=v->size()*sizeof(T);
    int val=v->size();

    memcpy(p,&used,sizeof(int));p+=sizeof(int);
    memcpy(p,&val,sizeof(int));p+=sizeof(int);
    memcpy(p,&v->front(),used);p+=used;
    return p;
}

template <class T>
char *vector_load(std::vector<T> *v,char *p)
{
    int sz,num;

    memcpy(&sz,p,sizeof(int));p+=sizeof(int);
    memcpy(&num,p,sizeof(int));p+=sizeof(int);
    if(!num)num=sz;
    v->resize(num);
    memcpy(v->data(),p,sz);p+=sz;
    return p;
}

// Hashtable
class Hashitem
{
    int key;
    int value;
    int next;
    int valuelen;

public:
    Hashitem():key(0),value(0),next(0),valuelen(0){}

    friend class Hashtable;
};

class Hashtable
{
    int findnext_v;
    int findstr;
    int size;
    std::vector<Hashitem> items;

public:
    int getSize(){return items.size();}

    static unsigned gethashcode(const char *s,int sz);
    void reset(int size);
    char *save(char *p);
    char *load(char *p);
    void additem(int key,int value);
    int  find(int vl,int *isfound);
    int  findnext(int *isfound);
};

// Strings
void strsub(wchar_t *str,const wchar_t *pattern,const wchar_t *rep);
void strtoupper(char *s,int len);
void strtolower(char *s,int len);
int  unicode2ansi(char *s,char *out,int size);
int _wtoi_my(const wchar_t *str);

// 7-zip
int  encode(char *dest,int dest_sz,char *src,int src_sz);
int  decode(char *dest,int dest_sz,char *src,int src_sz);
void registerall();

namespace NArchive{
namespace N7z{
        extern void register7z();
}}
extern int  Extract7z(wchar_t *str);
extern void registerBCJ();
extern void registerBCJ2();
extern void registerBranch();
extern void registerCopy();
extern void registerLZMA();
extern void registerLZMA2();

#endif
