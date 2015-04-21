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

extern int trap_mode;

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
private:
    std::unordered_map<std::string,int> dub;
    std::vector<char> text;

public:
    unsigned getSize()const{return text.size();}
    std::vector<char> *getVector(){return &text;}
    char *get(int offset){return (char *)(&text[offset]);}
    wchar_t *getw(int offset){return (wchar_t *)(&text[offset]);}

    int strcpy(const char *mem);
    int strcpyw(const wchar_t *mem);
    int t_memcpy(const char *mem,int sz);
    int t_memcpyz(const char *mem,int sz);
    int memcpyz_dup(const char *mem,int sz);

    Txt();
    ~Txt();
    int alloc(int sz);
    void reset(int sz);
    void shrink();
};

// Vector templates
template <class T>
char *vector_save(std::vector<T> *v,char *p)
{
    int used=v->size()*sizeof(T);
    int val;

    memcpy(p,&used,sizeof(int));p+=sizeof(int);

    val=v->size();
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
struct hashitem_t
{
    int key;
    int value;
    int next;
    int valuelen;
    hashitem_t():key(0),value(0),next(0),valuelen(0){}
};

struct hashtable_t
{
    int findnext;
    int findstr;
    int size;
    std::vector<hashitem_t> items_new;
};
unsigned hash_getcode(const char *s,int sz);
void hash_init(hashtable_t *t,int size);
char *hash_save(hashtable_t *t,char *p);
char *hash_load(hashtable_t *t,char *p);
void hash_add(hashtable_t *t,int key,int value);
int  hash_find(hashtable_t *t,int vl,int *isfound);
int  hash_findnext(hashtable_t *t,int *isfound);

// Strings
void strsub(wchar_t *str,const wchar_t *pattern,const wchar_t *rep);
void strtoupper(char *s,int len);
void strtolower(char *s,int len);

// 7-zip
namespace NArchive
{
    namespace N7z
    {
        extern void register7z();
    }
}
void registerall();
extern int  Extract7z(wchar_t *str);
extern void registercrc();
extern void registerBCJ();
extern void registerBCJ2();
extern void registerBranch();
extern void registerCopy();
extern void registerLZMA();
extern void registerLZMA2();
extern void registerPPMD();
extern void registerDelta();
extern void registerByteSwap();
