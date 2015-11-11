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
#include "common.h"
#include "logging.h"
#include "settings.h"
#include "theme.h"

#include <windows.h>

#include "draw.h"
#include "main.h"
#include "system.h"

#include <memory>
#include <string.h>

//{ Global vars
VaultInt *vLang;
VaultInt *vTheme;
wchar_t lang_ids[64][128];
//}

//{ Vault (imp)
class Vault:public VaultInt
{
    Vault(const Vault&)=delete;
    Vault &operator=(const Vault&)=delete;

protected:
    entry_t *entry;
    size_t num;
    std::unique_ptr<wchar_t []> data_ptr,odata_ptr,datav_ptr;

    std::unordered_map <std::wstring,int> lookuptbl;
    int res;

    Filemon *mon;
    int elem_id;
    const wchar_t *folder;

    wchar_t namelist[64][128];

protected:
    int  findvar(wchar_t *str);
    wchar_t *findstr(wchar_t *str)const;
    int  readvalue(const wchar_t *str);
    void parse();
    bool loadFromEncodedFile(const wchar_t *filename);
    void loadFromFile(const wchar_t *filename);
    void loadFromRes(int id);

public:
    Vault(entry_t *entry,size_t num,int res,int elem_id_,const wchar_t *folder_);
    virtual ~Vault(){}
    void load(int i);
    int PickLang();
    int PickTheme();

    virtual void SwitchData(int i)=0;
    virtual void EnumFiles(Combobox *lst,const wchar_t *path,int arg=0)=0;
    virtual void StartMonitor()=0;
    virtual void StopMonitor(){delete mon;};
    void updateCallback(const wchar_t *szFile,int action,int lParam);
};
//}

//{ Vault (lang/theme)
class VaultLang:public Vault
{
public:
    VaultLang(entry_t *entry,size_t num,int res,int elem_id_,const wchar_t *folder_);
    void SwitchData(int i);
    void EnumFiles(Combobox *lst,const wchar_t *path,int arg=0);
    void StartMonitor();
    Image *GetIcon(int){return nullptr;}
    Image *GetImage(int){return nullptr;}
    static void updateCallback(const wchar_t *szFile,int action,int lParam);
};

class VaultTheme:public Vault
{
    ImageStorange *Images;
    ImageStorange *Icons;

public:
    void SwitchData(int i);
    void EnumFiles(Combobox *lst,const wchar_t *path,int arg=0);
    void StartMonitor();
    Image *GetIcon(int i){return Icons->GetImage(i);}
    Image *GetImage(int i){return Images->GetImage(i);}

    VaultTheme(entry_t *entryv,size_t numv,int resv,int elem_id_,const wchar_t *folder_):
        Vault{entryv,numv,resv,elem_id_,folder_}
    {
        Images=CreateImageStorange(BOX_NUM,boxindex,4);
        Icons=CreateImageStorange(ICON_NUM,iconindex);
    }
    ~VaultTheme()
    {
        delete Images;
        delete Icons;
    }
    static void updateCallback(const wchar_t *szFile,int action,int lParam);
};
VaultInt *CreateVaultLang(entry_t *entry,size_t num,int res)
{
    return new VaultLang(entry,num,res,WM_UPDATELANG,L"langs");
}
VaultInt *CreateVaultTheme(entry_t *entry,size_t num,int res)
{
    return new VaultTheme(entry,num,res,WM_UPDATETHEME,L"themes");
}
//}

//{ Vault definitions
int Vault::findvar(wchar_t *str)
{
    int i;
    wchar_t *p;
    wchar_t c;

    while(*str&&(*str==L' '||*str==L'\t'))str++;
    p=str;
    while(*p&&(*p!=L' '&&*p!=L'\t'))p++;
    c=*p;
    *p=0;

    auto got=lookuptbl.find(std::wstring(str));
    i=(got!=lookuptbl.end())?got->second:0;
    i--;
    *p=c;
    return i;
}

wchar_t *Vault::findstr(wchar_t *str)const
{
    wchar_t *b,*e;

    b=wcschr(str,L'\"');
    if(!b)return nullptr;
    b++;
    e=wcschr(b,L'\"');
    if(!e)return nullptr;
    *e=0;
    return b;
}

int Vault::readvalue(const wchar_t *str)
{
    const wchar_t *p;

    p=wcsstr(str,L"0x");
    return p?wcstol(str,nullptr,16):_wtoi_my(str);
}

void Vault::parse()
{
    wchar_t *lhs,*rhs,*le;
    le=lhs=datav_ptr.get();

    while(le)
    {
        // Split lines
        le=wcschr(lhs,L'\n');
        if(le)*le=0;

        // Comments
        if(wcsstr(lhs,L"//"))*wcsstr(lhs,L"//")=0;

        // Split LHS and RHS
        rhs=wcschr(lhs,L'=');
        if(!rhs)
        {
            lhs=le+1;// next line
            continue;
        }
        *rhs=0;rhs++;

        // Parse LHS
        int r=findvar(lhs);
        if(r<0)
        {
            Log.print_err("ERROR: unknown var '%S'\n",lhs);
        }else
        {
            wchar_t *r1;
            int r2;
            size_t ri=static_cast<size_t>(r);
            r1=findstr(rhs);
            r2=findvar(rhs);

            if(r1)               // String
            {
                while(wcsstr(r1,L"\\n"))
                {
                    wchar_t *yy=wcsstr(r1,L"\\n");
                    wcscpy(yy,yy+1);
                    *yy=L'\n';
                }
                while(wcsstr(r1,L"\\0"))
                {
                    wchar_t *yy=wcsstr(r1,L"\\0");
                    wcscpy(yy,yy+1);
                    *yy=1;
                }
                {
                    size_t l=wcslen(r1);
                    for(size_t i=0;i<l;i++)if(r1[i]==1)r1[i]=0;
                }
                entry[ri].valstr=r1;
                entry[ri].init=1;
            }
            else if(r2>=0)      // Var
            {
                entry[ri].val=entry[r2].val;
                entry[ri].init=10+r2;
            }
            else                // Number
            {
                int val=readvalue(rhs);
                //if(!entry[r].init&&entry[r].val==val)Log.print_err("WARNNING: double definition for '%S'\n",lhs);
                entry[ri].val=val;
                entry[ri].init=2;
            }
        }
        lhs=le+1; // next line
    }
    odata_ptr=std::move(data_ptr);
    //data_ptr.reset(datav);
    data_ptr=std::move(datav_ptr);
}

static void myswab(const char *s,char *d,int sz)
{
    while(sz--)
    {
        d[0]=s[1];
        d[1]=s[0];
        d+=2;s+=2;
    }
}

bool Vault::loadFromEncodedFile(const wchar_t *filename)
{
    FILE *f=_wfopen(filename,L"rb");
    if(!f)
    {
        Log.print_err("ERROR in loadfile(): failed _wfopen(%S)\n",filename);
        return false;
    }

    fseek(f,0,SEEK_END);
    int sz=ftell(f);
    fseek(f,0,SEEK_SET);
    if(sz<10)
    {
        Log.print_err("ERROR in loadfile(): '%S' has only %d bytes\n",filename,sz);
        fclose(f);
        return false;
    }
    datav_ptr.reset(new wchar_t[sz+1]);
    wchar_t *datav=datav_ptr.get();
    //Log.print_con("Read '%S':%d\n",filename,sz);

    fread(datav,2,1,f);
    if(!memcmp(datav,"\xEF\xBB",2))// UTF-8
    {
        int szo;
        fread(datav,1,1,f);
        sz-=3;
        size_t q=fread(datav,1,sz,f);
        szo=MultiByteToWideChar(CP_UTF8,0,(LPCSTR)datav,(int)q,nullptr,0);
        wchar_t *dataloc1=new wchar_t[szo+1];
        sz=MultiByteToWideChar(CP_UTF8,0,(LPCSTR)datav,(int)q,dataloc1,szo);
        fclose(f);
        dataloc1[sz]=0;
        datav_ptr.reset(dataloc1);
        return true;
    }else
    if(!memcmp(datav,"\xFF\xFE",2))// UTF-16 LE
    {
        fread(datav,sz,1,f);
        sz>>=1;(sz)--;
        fclose(f);
        datav[sz]=0;
        return true;
    }else
    if(!memcmp(datav,"\xFE\xFF",2))// UTF-16 BE
    {
        fread(datav,sz,1,f);
        myswab((char *)datav,(char *)datav,sz);
        sz>>=1;(sz)--;
        fclose(f);
        datav[sz]=0;
        return true;
    }else                         // ANSI
    {
        fclose(f);
        f=_wfopen(filename,L"rt");
        if(!f)
        {
            Log.print_err("ERROR in loadfile(): failed _wfopen(%S)\n",filename);
            return false;
        }
        wchar_t *p=datav;(sz)--;
        while(!feof(f))
        {
            fgetws(p,sz,f);
            p+=wcslen(p);
        }
        fclose(f);
        datav[sz]=0;
        return true;
    }
}

void Vault::loadFromFile(const wchar_t *filename)
{
    if(!filename[0])return;
    if(!loadFromEncodedFile(filename))
    {
        Log.print_err("ERROR in vault_loadfromfile(): failed to load '%S'\n",filename);
        return;
    }
    parse();

    for(size_t i=0;i<num;i++)
        if(entry[i].init>=10)entry[i].val=entry[entry[i].init-10].val;
}

void Vault::loadFromRes(int id)
{
    char *data1;
    int sz;

    get_resource(id,(void **)&data1,&sz);
    datav_ptr.reset(new wchar_t[sz+1]);
    wchar_t *datav=datav_ptr.get();
    for(int i=0;i<sz;i++)
    {
        if(data1[i]==L'\r')datav[i]=L' ';else
        datav[i]=data1[i];
    }
    datav[sz]=0;
    parse();
    for(size_t i=0;i<num;i++)
        if(entry[i].init<1)Log.print_err("ERROR in vault_loadfromres: not initialized '%S'\n",entry[i].name);
}

Vault::Vault(entry_t *entryv,size_t numv,int resv,int elem_id_,const wchar_t *folder_):
    entry(entryv),
    num(numv),
    res(resv),
    elem_id(elem_id_),
    folder(folder_)
{
    for(size_t i=0;i<num;i++)
        lookuptbl.insert({std::wstring(entry[i].name),static_cast<int>(i)+1});
}

VaultLang::VaultLang(entry_t *entryv,size_t numv,int resv,int elem_id_,const wchar_t *folder_):
    Vault{entryv,numv,resv,elem_id_,folder_}
{
    load(-1);
}


void Vault::load(int i)
{
    //Log.print_con("vault %d,'%S'\n",i,namelist[i]);
    loadFromRes(res);
    if(i<0)return;
    loadFromFile(namelist[i]);
}

int Vault::PickLang()
{
    int f=0;
    int j=MainWindow.hLang->GetNumItems();
    for(int i=0;i<j;i++)
        if(StrStrIW(lang_ids[i],Settings.curlang))
           {f=i;break;}

    return f;
}

int Vault::PickTheme()
{
    int f=0;
    int j=MainWindow.hTheme->GetNumItems();
    for(int i=0;i<j;i++)
        if(StrStrIW(namelist[i],D_STR(THEME_NAME))&&
            StrStrIW(namelist[i],L"big")==nullptr){f=i;break;}

    return f;
}
//}

//{ Lang/theme
void VaultLang::SwitchData(int i)
{
    if(Settings.flags&FLAG_NOGUI)return;
    load(i);
}

void VaultTheme::SwitchData(int i)
{
    if(Settings.flags&FLAG_NOGUI)return;
    load(i);
    Images->LoadAll();
    Icons->LoadAll();
}

void VaultLang::EnumFiles(Combobox *lst,const wchar_t *path,int locale)
{
    WStringShort buf;
    HANDLE hFind;
    WIN32_FIND_DATA FindFileData;
    int i=0;

    if(Settings.flags&FLAG_NOGUI)return;

    int lang_auto=-1;
    WStringShort lang_auto_str;
    lang_auto_str.sprintf(L"Auto (English)");

    buf.sprintf(L"%s\\%s\\*.txt",Settings.data_dir,path);
    hFind=FindFirstFile(buf.Get(),&FindFileData);
    if(hFind!=INVALID_HANDLE_VALUE)
    do
    if(!(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
    {
        buf.sprintf(L"%s\\%s\\%s",Settings.data_dir,path,FindFileData.cFileName);
        loadFromFile(buf.Get());
        if(language[STR_LANG_CODE].val==(locale&0xFF))
        {
            lang_auto_str.sprintf(L"Auto (%s)",STR(STR_LANG_NAME));
            lang_auto=i;
        }
        lst->AddItem(STR(STR_LANG_NAME));
        wcscpy(namelist[i],buf.Get());
        wcscpy(lang_ids[i],STR(STR_LANG_ID));
        i++;
    }
    while(FindNextFile(hFind,&FindFileData)!=0);
    FindClose(hFind);

    if(!i)
    {
        lst->AddItem(L"English");
        namelist[i][0]=0;
    }else
    {
        lst->AddItem(lang_auto_str.Get());
        wcscpy(namelist[i],(lang_auto>=0)?namelist[lang_auto]:L"");
    }
}

void VaultTheme::EnumFiles(Combobox *lst,const wchar_t *path,int arg)
{
	UNREFERENCED_PARAMETER(arg);

    WStringShort buf;
    HANDLE hFind;
    WIN32_FIND_DATA FindFileData;
    int i=0;

    if(Settings.flags&FLAG_NOGUI)return;
    buf.sprintf(L"%s\\%s\\*.txt",Settings.data_dir,path);
    hFind=FindFirstFile(buf.Get(),&FindFileData);
    if(hFind!=INVALID_HANDLE_VALUE)
    do
    if(!(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
    {
        buf.sprintf(L"%s\\%s\\%s",Settings.data_dir,path,FindFileData.cFileName);
        loadFromFile(buf.Get());
        lst->AddItem(D_STR(THEME_NAME));
        wcscpy(namelist[i],buf.Get());
        i++;
    }
    while(FindNextFile(hFind,&FindFileData)!=0);
    FindClose(hFind);

    if(!i)
    {
        lst->AddItem(L"(default)");
        namelist[i][0]=0;
    }
    load(-1);
}

void VaultLang::StartMonitor()
{
    WStringShort buf;
    buf.sprintf(L"%s\\%s",Settings.data_dir,folder);
    mon=CreateFilemon(buf.Get(),1,updateCallback);
}

void VaultTheme::StartMonitor()
{
    WStringShort buf;
    buf.sprintf(L"%s\\%s",Settings.data_dir,folder);
    mon=CreateFilemon(buf.Get(),1,updateCallback);
}

void VaultLang::updateCallback(const wchar_t *szFile,int action,int lParam)
{
    UNREFERENCED_PARAMETER(szFile);
    UNREFERENCED_PARAMETER(action);
    UNREFERENCED_PARAMETER(lParam);

    PostMessage(MainWindow.hMain,WM_UPDATELANG,0,0);
}

void VaultTheme::updateCallback(const wchar_t *szFile,int action,int lParam)
{
    UNREFERENCED_PARAMETER(szFile);
    UNREFERENCED_PARAMETER(action);
    UNREFERENCED_PARAMETER(lParam);

    PostMessage(MainWindow.hMain,WM_UPDATETHEME,0,0);
}
//}
