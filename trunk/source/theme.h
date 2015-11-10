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

#ifndef THEME_H
#define THEME_H

#include "themelist.h"
#include "langlist.h"

// Theme/lang
#define STR(A) (language[A].valstr?language[A].valstr:L"")
#define D(A) theme[A].val
#define D_C(A) theme[A].val
#define D_1(A) theme[A].val
#define D_X(A) (theme[A].val*256/Settings.scale)
#define D_STR(A) theme[A].valstr

#define TEXT_1(quote) L##quote
#define DEF_VAL(a) {TEXT_1(a),0,0},
#define DEF_STR(a) {TEXT_1(a),0,0},

// Declarations
class Image;
class Combobox;
class VaultInt;
struct entry_t;

// Global vars
extern entry_t language[STR_NM];
extern entry_t theme[THEME_NM];
extern VaultInt *vLang;
extern VaultInt *vTheme;

// Entry
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

// Vault
class VaultInt
{
public:
    virtual ~VaultInt(){}
    virtual void SwitchData(int i)=0;
    virtual void EnumFiles(Combobox *lst,const wchar_t *path,int arg=0)=0;

    virtual int PickTheme()=0;
    virtual Image *GetIcon(int i)=0;
    virtual Image *GetImage(int i)=0;

    virtual void StartMonitor()=0;
    virtual void StopMonitor()=0;
};
VaultInt *CreateVaultLang(entry_t *entry,size_t num,int res);
VaultInt *CreateVaultTheme(entry_t *entry,size_t num,int res);

#endif
