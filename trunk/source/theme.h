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

#include "themelist.h"
#include "langlist.h"

// Theme/lang
#define STR(A) (language[A].valstr?language[A].valstr:L"")
#define D(A) theme[A].val
#define D_STR(A) theme[A].valstr

class Filemon;
class Vault;
struct entry_t;

//{ Global vars
extern entry_t language[STR_NM];
extern entry_t theme[THEME_NM];
extern Filemon *mon_lang,*mon_theme;
extern Vault vLang,vTheme;
extern int monitor_pause;
//}

struct entry_t
{
    const wchar_t *name;
    union
    {
        int val;
        wchar_t *valstr;
    };
    int init;
};

typedef std::unordered_map <std::wstring,int> lookuptbl_t;

class Vault
{
public:
    entry_t *entry;
    int num;
    wchar_t namelist[64][250];
    std::unique_ptr<wchar_t []> data_ptr,odata_ptr,datav_ptr;

    lookuptbl_t lookuptbl;
    int res;

private:
    int  findvar(wchar_t *str);
    wchar_t *findstr(wchar_t *str);
    int  readvalue(const wchar_t *str);
    void parse();
    bool loadFromEncodedFile(const wchar_t *filename);
    void loadFromFile(wchar_t *filename);
    void loadFromRes(int id);

public:
    Vault(const Vault&)=delete;
    Vault &operator=(const Vault&)=delete;
    Vault();
    void init1(entry_t *entry,int num,int res);
    void load(int i);

    friend void lang_enum(HWND hwnd,const wchar_t *path,int locale);
    friend void theme_enum(HWND hwnd,const wchar_t *path);
};

// Lang/theme
void lang_set(int i);
void theme_set(int i);
void lang_enum(HWND hwnd,const wchar_t *path,int locale);
void theme_enum(HWND hwnd,const wchar_t *path);

void vault_startmonitors();
void CALLBACK lang_callback(const wchar_t *szFile,DWORD action,LPARAM lParam);
void CALLBACK theme_callback(const wchar_t *szFile,DWORD action,LPARAM lParam);
