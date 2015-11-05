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
#include <vector>

// Global vars
extern int trap_mode;

// Vector templates
template <class T>
char *vector_save(std::vector<T> *v,char *p)
{
	size_t used = v->size()*sizeof(T);
    size_t val=v->size();

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

template <class T>
class loadable_vector:public std::vector<T>
{
public:
    char *savedata(char *p){return vector_save(this,p);}
    char *loaddata(char *p){return vector_load(this,p);}
};

// Version
class Version
{
    int d,m,y;
    int v1,v2,v3,v4;

public:
    int  setDate(int d_,int m_,int y_);
    void setVersion(int v1_,int v2_,int v3_,int v4_);
    void setInvalid(){y=v1=-1;}
    void str_date(wchar_t *buf);
    void str_version(wchar_t *buf);

    Version():d(0),m(0),y(0),v1(-2),v2(0),v3(0),v4(0){}
    Version(int d1,int m1,int y1):d(d1),m(m1),y(y1),v1(-2),v2(0),v3(0),v4(0){}

    friend class Driverpack;
    friend class Hwidmatch;
    friend class datum;
    friend int cmpdate(Version *t1,Version *t2);
    friend int cmpversion(Version *t1,Version *t2);
};
int cmpdate(Version *t1,Version *t2);
int cmpversion(Version *t1,Version *t2);

// Txt
class Txt
{
    std::unordered_map<std::string,int> dub;
    loadable_vector<char> text;

public:
    size_t getSize()const{return text.size();}
    char *get(int offset){return &text[offset];}
    wchar_t *getw(int offset){return (wchar_t *)(&text[offset]);}
    wchar_t *getw2(int offset){return (wchar_t *)(&text[offset-(text[0]?2:0)]);}

    ofst strcpy(const char *mem);
	ofst strcpyw(const wchar_t *mem);
    ofst t_memcpy(const char *mem,size_t sz);
    ofst t_memcpyz(const char *mem,size_t sz);
    ofst memcpyz_dup(const char *mem,size_t sz);
    ofst alloc(size_t sz);

    char *savedata(char *p){return text.savedata(p);}
    char *loaddata(char *p){return text.loaddata(p);};

    Txt();
    void reset(int sz);
    void shrink();
};

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
    loadable_vector<Hashitem> items;

public:
	size_t getSize(){ return items.size(); }

    static unsigned gethashcode(const char *s,size_t sz);
    void reset(size_t size);
    char *savedata(char *p);
    char *loaddata(char *p);
    void additem(int key,int value);
    int  find(int vl,int *isfound);
    int  findnext(int *isfound);
};

// Strings
void strsub(wchar_t *str,const wchar_t *pattern,const wchar_t *rep);
void strtoupper(char *s,size_t len);
void strtolower(char *s,size_t len);
int  unicode2ansi(char *s,char *out,size_t size);
int _wtoi_my(const wchar_t *str);

class WString
{
private:
    wchar_t buf[BUFLEN];
    wchar_t *buf_dyn=nullptr;
    wchar_t *buf_cur;

public:
    WString():buf_cur(buf){}
    ~WString(){delete buf_dyn;}

    //void sprintf(format,...);
    //void copy(wchar_t *s)

    wchar_t *GetV(){return buf_cur;}
    const wchar_t *Get(){return buf_cur;}
};

// 7-zip
size_t  encode(char *dest,size_t dest_sz,char *src,size_t src_sz);
size_t  decode(char *dest,size_t dest_sz,char *src,size_t src_sz);
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
