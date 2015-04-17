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
typedef struct _hashtable_t hashtable_t;

enum // heap_t
{
    ID_HASH_ITEMS=0,    // 0
    ID_HASH_STR,        // 1
    NUM_HEAPS
};

//{ Structs
typedef struct _heap_t
{
    int id;
    void **membck;
    void *base;
    int used;
    int allocated;
    int itemsize;
    int items;

    //hashtable_t *dup;
}heap_t;

typedef struct hashitem_type
{
    int key;
    int value;
    int next;
    int valuelen;
}hashitem_t;

typedef struct _hashtable_t
{
//    int id;
//    int flags;
    int findnext;
    int findstr;
    int size;

    hashitem_t *items;
    heap_t items_handle;
    char *strs;
    heap_t strs_handle;

}hashtable_t;
//}

// Heap
void heap_refresh(heap_t *t);
void heap_expand(heap_t *t,int sz);
void heap_init(heap_t *t,int id,void **mem,int initsize,int itemsize);
void heap_free(heap_t *t);
void heap_reset(heap_t *t,int sz);

int  heap_alloc(heap_t *t,int sz);
int  heap_allocitem_i(heap_t *t);
void *heap_allocitem_ptr(heap_t *t);
void heap_freelastitem(heap_t *t);
int  heap_memcpy(heap_t *t,const void *mem,int sz);
int  heap_strcpy(heap_t *t,const char *s);
int  heap_memcpyz(heap_t *t,const void *mem,int sz);
//int  heap_memcpyz_dup(heap_t *t,const void *mem,int sz);
int  heap_strtolowerz(heap_t *t,const char *s,int sz);
char *heap_save(heap_t *t,char *p);
char *heap_load(heap_t *t,char *p);

// Strings
void strsub(wchar_t *str,const wchar_t *pattern,const wchar_t *rep);
void strtoupper(char *s,int len);
void strtolower(char *s,int len);

// 7-zip
namespace NArchive{
namespace N7z{
extern void register7z();
}}
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

// Hash
unsigned hash_getcode(const char *s,int sz);
inline char *getstr(hashtable_t *t,hashitem_t *cur);
void hash_init(hashtable_t *t,int size);
void hash_clear(hashtable_t *t,int zero);
void hash_free(hashtable_t *t);
char *hash_save(hashtable_t *t,char *p);
char *hash_load(hashtable_t *t,char *p);
void hash_add(hashtable_t *t,int key,int value);
int  hash_find(hashtable_t *t,int vl,int *isfound);
int  hash_findnext(hashtable_t *t,int *isfound);
