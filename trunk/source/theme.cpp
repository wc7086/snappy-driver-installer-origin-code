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

#include <windows.h>
#include <shlwapi.h>        // for StrStrIW

#include "matcher.h"
#include "common.h"
#include "draw.h"
#include "indexing.h"
#include "enum.h"
#include "main.h"

#include "guicon.h"
#include "theme.h"

//{ Global vars
Filemon *mon_lang,*mon_theme;
Vault vLang,vTheme;
int monitor_pause=0;
//}

//{ Vault
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

wchar_t *Vault::findstr(wchar_t *str)
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
    wchar_t *p;

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
                    int l=wcslen(r1);
                    for(int i=0;i<l;i++)if(r1[i]==1)r1[i]=0;
                }
                entry[r].valstr=r1;
                entry[r].init=1;
            }
            else if(r2>=0)      // Var
            {
                entry[r].val=entry[r2].val;
                entry[r].init=10+r2;
            }
            else                // Number
            {
                entry[r].val=readvalue(rhs);
                entry[r].init=2;
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
        int q=fread(datav,1,sz,f);
        szo=MultiByteToWideChar(CP_UTF8,0,(LPCSTR)datav,q,nullptr,0);
        wchar_t *dataloc1=new wchar_t[szo+1];
        sz=MultiByteToWideChar(CP_UTF8,0,(LPCSTR)datav,q,dataloc1,szo);
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

void Vault::loadFromFile(wchar_t *filename)
{
    if(!filename[0])return;
    if(!loadFromEncodedFile(filename))
    {
        Log.print_err("ERROR in vault_loadfromfile(): failed to load '%S'\n",filename);
        return;
    }
    parse();

    for(int i=0;i<num;i++)
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
    for(int i=0;i<num;i++)
        if(entry[i].init<1)Log.print_err("ERROR in vault_loadfromres: not initialized '%S'\n",entry[i].name);
}

Vault::Vault(){}

void Vault::init1(entry_t *entryv,int numv,int resv)
{
    entry=entryv;
    num=numv;
    res=resv;

    for(int i=0;i<num;i++)
        lookuptbl.insert({std::wstring(entry[i].name),i+1});
}

void Vault::load(int i)
{
    //Log.print_con("vault %d,'%S'\n",i,namelist[i]);
    loadFromRes(res);
    if(i<0)return;
    loadFromFile(namelist[i]);
}

int Vault::pickTheme()
{
    int f=0;
    int j=SendMessage(MainWindow.hTheme,CB_GETCOUNT,0,0);
    for(int i=0;i<j;i++)
        if(StrStrI(namelist[i],D_STR(THEME_NAME))&&
            StrStrI(namelist[i],L"big")==nullptr){f=i;break;}

    return f;
}

//}

//{ Lang/theme
void lang_set(int i)
{
    if(Settings.flags&FLAG_NOGUI)return;
    vLang.load(i);
}

void theme_set(int i)
{
    if(Settings.flags&FLAG_NOGUI)return;
    vTheme.load(i);

    for(i=0;i<BOX_NUM;i++)
    {
        wchar_t *str=D_STR(boxindex[i]+4);
        int j;
        for(j=0;j<i;j++)
            if(!wcscmp(str,D_STR(boxindex[j]+4)))
        {
            box[i].makecopy(box[j]);
            //Log.print_con("%d Copy %S %d\n",i,str,j);
            break;
        }
        if(i==j)
        {
            box[i].load(boxindex[i]+4);
            //Log.print_con("%d New  %S\n",i,str);
        }
    }
    for(i=0;i<ICON_NUM;i++)
    {
        wchar_t *str=D_STR(iconindex[i]);
        int j;
        for(j=0;j<i;j++)
            if(!wcscmp(str,D_STR(iconindex[j])))
        {
            icon[i].makecopy(icon[j]);
            //Log.print_con("%d Copy %S %d\n",i,str,j);
            break;
        }
        if(i==j)
        {
            icon[i].load(iconindex[i]);
            //Log.print_con("%d New  %S\n",i,str);
        }
    }
}

void lang_enum(HWND hwnd,const wchar_t *path,int locale)
{
    wchar_t buf[BUFLEN];
    HANDLE hFind;
    WIN32_FIND_DATA FindFileData;
    int i=0;

    if(Settings.flags&FLAG_NOGUI)return;

    int lang_auto=-1;
    wchar_t lang_auto_str[BUFLEN];
    wcscpy(lang_auto_str,L"Auto (English)");

    wsprintf(buf,L"%s\\%s\\*.txt",Settings.data_dir,path);
    hFind=FindFirstFile(buf,&FindFileData);
    if(hFind!=INVALID_HANDLE_VALUE)
    do
    if(!(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
    {
        wsprintf(buf,L"%s\\%s\\%s",Settings.data_dir,path,FindFileData.cFileName);
        vLang.loadFromFile(buf);
        if(language[STR_LANG_CODE].val==(locale&0xFF))
        {
            wsprintf(lang_auto_str,L"Auto (%s)",STR(STR_LANG_NAME));
            lang_auto=i;
        }
        SendMessage(hwnd,CB_ADDSTRING,0,(LPARAM)STR(STR_LANG_NAME));
        wcscpy(vLang.namelist[i],buf);
        i++;
    }
    while(FindNextFile(hFind,&FindFileData)!=0);
    FindClose(hFind);

    if(!i)
    {
        SendMessage(hwnd,CB_ADDSTRING,0,(LPARAM)L"English");
        vLang.namelist[i][0]=0;
    }else
    {
        SendMessage(hwnd,CB_ADDSTRING,0,(LPARAM)lang_auto_str);
        wcscpy(vLang.namelist[i],(lang_auto>=0)?vLang.namelist[lang_auto]:L"");
    }
}

void theme_enum(HWND hwnd,const wchar_t *path)
{
    wchar_t buf[BUFLEN];
    HANDLE hFind;
    WIN32_FIND_DATA FindFileData;
    int i=0;

    if(Settings.flags&FLAG_NOGUI)return;
    wsprintf(buf,L"%s\\%s\\*.txt",Settings.data_dir,path);
    hFind=FindFirstFile(buf,&FindFileData);
    if(hFind!=INVALID_HANDLE_VALUE)
    do
    if(!(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
    {
        wsprintf(buf,L"%s\\%s\\%s",Settings.data_dir,path,FindFileData.cFileName);
        vTheme.loadFromFile(buf);
        SendMessage(hwnd,CB_ADDSTRING,0,(LPARAM)D(THEME_NAME));
        wcscpy(vTheme.namelist[i],buf);
        i++;
    }
    while(FindNextFile(hFind,&FindFileData)!=0);
    FindClose(hFind);

    if(!i)
    {
        SendMessage(hwnd,CB_ADDSTRING,0,(LPARAM)L"(default)");
        vTheme.namelist[i][0]=0;
    }
    vTheme.load(-1);
}

void vault_startmonitors()
{
    wchar_t buf[BUFLEN];

    wsprintf(buf,L"%s\\langs",Settings.data_dir);
    mon_lang=Filemon::start(buf,FILE_NOTIFY_CHANGE_LAST_WRITE|FILE_NOTIFY_CHANGE_FILE_NAME,1,lang_callback);
    wsprintf(buf,L"%s\\themes",Settings.data_dir);
    mon_theme=Filemon::start(buf,FILE_NOTIFY_CHANGE_LAST_WRITE|FILE_NOTIFY_CHANGE_FILE_NAME,1,theme_callback);
}

void CALLBACK lang_callback(const wchar_t *szFile,DWORD action,LPARAM lParam)
{
    UNREFERENCED_PARAMETER(szFile);
    UNREFERENCED_PARAMETER(action);
    UNREFERENCED_PARAMETER(lParam);

    PostMessage(MainWindow.hMain,WM_UPDATELANG,0,0);
}

void CALLBACK theme_callback(const wchar_t *szFile,DWORD action,LPARAM lParam)
{
    UNREFERENCED_PARAMETER(szFile);
    UNREFERENCED_PARAMETER(action);
    UNREFERENCED_PARAMETER(lParam);

    PostMessage(MainWindow.hMain,WM_UPDATETHEME,0,0);
}
//}
