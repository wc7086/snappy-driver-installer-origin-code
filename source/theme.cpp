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

//{ Global vars
monitor_t *mon_lang,*mon_theme;
Vault vLang,vTheme;
int monitor_pause=0;
//}

//{ Vault
static void myswab(const char *s,char *d,int sz)
{
    while(sz--)
    {
        d[0]=s[1];
        d[1]=s[0];
        d+=2;s+=2;
    }
}

void *Vault::loadFromEncodedFile(const WCHAR *filename,int *sz)
{
    FILE *f;
    void *dataloc;

    f=_wfopen(filename,L"rb");
    if(!f)
    {
        log_err("ERROR in loadfile(): failed _wfopen(%S)\n",filename);
        return 0;
    }

    fseek(f,0,SEEK_END);
    *sz=ftell(f);
    fseek(f,0,SEEK_SET);
    dataloc=malloc(*sz*2+2+1024);
    log_con("Read '%S':%d\n",filename,*sz);

    fread(dataloc,2,1,f);
    if(!memcmp(dataloc,"\xEF\xBB",2))// UTF-8
    {
        void *dataloc1;
        int szo;
        fread(dataloc,1,1,f);
        *sz-=3;
        int q=fread(dataloc,1,*sz,f);
        szo=MultiByteToWideChar(CP_UTF8,0,(LPCSTR)dataloc,q,0,0);
        dataloc1=malloc(szo*2+2);
        *sz=MultiByteToWideChar(CP_UTF8,0,(LPCSTR)dataloc,q,(LPWSTR)dataloc1,szo);
        free(dataloc);
        fclose(f);
        return dataloc1;
    }else
    if(!memcmp(dataloc,"\xFF\xFE",2))// UTF-16 LE
    {
        fread(dataloc,*sz,1,f);
        *sz>>=1;(*sz)--;
        fclose(f);
        return dataloc;
    }else
    if(!memcmp(dataloc,"\xFE\xFF",2))// UTF-16 BE
    {
        fread(dataloc,*sz,1,f);
        myswab((char *)dataloc,(char *)dataloc,*sz);
        *sz>>=1;(*sz)--;
        fclose(f);
        return dataloc;
    }else                         // ANSI
    {
        fclose(f);
        f=_wfopen(filename,L"rt");
        if(!f)
        {
            log_err("ERROR in loadfile(): failed _wfopen(%S)\n",filename);
            free(dataloc);
            return 0;
        }
        WCHAR *p=(WCHAR *)dataloc;(*sz)--;
        while(!feof(f))
        {
            fgetws(p,*sz,f);
            p+=wcslen(p);
        }
        fclose(f);
        return dataloc;
    }
}

void Vault::loadFromFile(WCHAR *filename)
{
    WCHAR *datav;
    int sz,i;

    if(!filename[0])return;
    //printf("{%S\n",filename);
    //if(odata)free(odata);
    //odata=datav;
    datav=(WCHAR *)loadFromEncodedFile(filename,&sz);
    if(!datav)
    {
        log_err("ERROR in vault_loadfromfile(): failed to load '%S'\n",filename);
        return;
    }
    datav[sz]=0;
    parse(datav);

    for(i=0;i<num;i++)
        if(entry[i].init>=10)entry[i].val=entry[entry[i].init-10].val;
}

int Vault::vault_findvar(WCHAR *str)
{
    int i;
    WCHAR *p;
    WCHAR c;
    char buf[BUFLEN];

    while(*str&&(*str==L' '||*str==L'\t'))str++;
    p=str;
    while(*p&&(*p!=L' '&&*p!=L'\t'))p++;
    c=*p;
    *p=0;

    wsprintfA(buf,"%ws",str);
    i=hash_find_str(&strs,buf);
    i--;
    *p=c;
    return i;
}

int Vault::vault_readvalue(const WCHAR *str)
{
    WCHAR *p;

    p=wcsstr(str,L"0x");
    return p?wcstol(str,0,16):_wtoi_my(str);
}

intptr_t Vault::vault_findstr(WCHAR *str)
{
    WCHAR *b,*e;

    b=wcschr(str,L'\"');
    if(!b)return 0;
    b++;
    e=wcschr(b,L'\"');
    if(!e)return 0;
    *e=0;
    return (intptr_t)b;
}

void Vault::init1(entry_t *entryv,int numv,int resv)
{
    char buf[BUFLEN];
    int i;

    memset(this,0,sizeof(Vault));
    entry=entryv;
    num=numv;
    res=resv;

    hash_init(&strs,ID_INF_LIST,num*4,0);
    for(i=0;i<num;i++)
    {
        wsprintfA(buf,"%S",entry[i].name);
        hash_add(&strs,buf,strlen(buf),(int)i+1,HASH_MODE_INTACT);
    }
}

void Vault::free1()
{
    if(odata)free(odata);
    if(data)free(data);
    hash_free(&strs);
}

void Vault::parse(WCHAR *datav)
{
    WCHAR *lhs,*rhs,*le;
    WCHAR *tmp;

    le=lhs=datav;

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
            lhs=le+1;
            continue;
        }
        *rhs=0;rhs++;

        // Parse LHS
        int r;
        r=vault_findvar(lhs);
        if(r<0)
        {
            printf("Error: unknown var '%S'\n",lhs);
        }else
        {
            intptr_t v,r1,r2;
            r1=vault_findstr(rhs);
            r2=vault_findvar(rhs);

            if(r1)               // String
            {
                v=r1;
                tmp=(WCHAR *)v;
                while(wcsstr(tmp,L"\\n"))
                {
                    WCHAR *yy=wcsstr(tmp,L"\\n");
                    wcscpy(yy,yy+1);
                    *yy=L'\n';
                }
                while(wcsstr(tmp,L"\\0"))
                {
                    WCHAR *yy=wcsstr(tmp,L"\\0");
                    wcscpy(yy,yy+1);
                    *yy=1;
                }
                {
                    int l=wcslen(tmp);
                    int i;
                    for(i=0;i<l;i++)if(tmp[i]==1)tmp[i]=0;
                }
                v=(intptr_t)tmp;
                entry[r].init=1;
            }
            else if(r2>=0)      // Var
            {
                v=entry[r2].val;
                entry[r].init=10+r2;
            }
            else                // Number
            {
                v=vault_readvalue(rhs);
                entry[r].init=2;
            }

            //if(r2<0)printf("-RHS:'%S'\n",L"",rhs);
            //if(r2>=0)printf("+RHS:'%S'\n",L"",rhs);
            //log("%d,%d,%X,{%S|%S}\n",r2,v,v,lhs,rhs);
            entry[r].val=v;
        }
        lhs=le+1;
    }
    if(odata)free(odata);
    odata=data;
    data=datav;
}

void Vault::loadFromRes(int id)
{
    WCHAR *datav;
    char *data1;
    int sz,i;

    //if(odata)free(odata);
    //odata=datav;
    get_resource(id,(void **)&data1,&sz);
    datav=(WCHAR*)malloc(sz*2+2);
    int j=0;
    for(i=0;i<sz;i++,j++)
    {
        if(data1[i]==L'\r')datav[i]=L' ';else
        datav[i]=data1[j];
    }
    datav[sz]=0;
    parse(datav);
    for(i=0;i<num;i++)
        if(entry[i].init<1)log_err("ERROR in vault_loadfromres: not initialized '%S'\n",entry[i].name);
}

void Vault::load(int i)
{
    log_con("vault %d,'%S'\n",i,namelist[i]);
    loadFromRes(res);
    loadFromFile(namelist[i]);
}
//}

//{ Lang/theme
void lang_set(int i)
{
    if(flags&FLAG_NOGUI)return;
    vLang.load(i);
}

void theme_set(int i)
{
    if(flags&FLAG_NOGUI)return;
    vTheme.load(i);

    for(i=0;i<BOX_NUM;i++)
    {
        WCHAR *str=(WCHAR *)D(boxindex[i]+4);
        int j;
        for(j=0;j<i;j++)
            if(!wcscmp(str,(WCHAR *)D(boxindex[j]+4)))
        {
            box[i].makecopy(box[j]);
            //log_con("%d Copy %S %d\n",i,str,j);
            break;
        }
        if(i==j)
        {
            box[i].load(boxindex[i]+4);
            //log_con("%d New  %S\n",i,str);
        }
    }
    for(i=0;i<ICON_NUM;i++)
    {
        WCHAR *str=(WCHAR *)D(iconindex[i]);
        int j;
        for(j=0;j<i;j++)
            if(!wcscmp(str,(WCHAR *)D(iconindex[j])))
        {
            icon[i].makecopy(icon[j]);
            //log_con("%d Copy %S %d\n",i,str,j);
            break;
        }
        if(i==j)
        {
            icon[i].load(iconindex[i]);
            //log_con("%d New  %S\n",i,str);
        }
    }
}

void lang_enum(HWND hwnd,const WCHAR *path,int locale)
{
    WCHAR buf[BUFLEN];
    HANDLE hFind;
    WIN32_FIND_DATA FindFileData;
    int i=0;

    if(flags&FLAG_NOGUI)return;

    int lang_auto=-1;
    WCHAR lang_auto_str[BUFLEN];
    wcscpy(lang_auto_str,L"Auto (English)");

    wsprintf(buf,L"%s\\%s\\*.txt",data_dir,path);
    hFind=FindFirstFile(buf,&FindFileData);
    if(hFind!=INVALID_HANDLE_VALUE)
    do
    if(!(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
    {
        wsprintf(buf,L"%s\\%s\\%s",data_dir,path,FindFileData.cFileName);
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

void theme_enum(HWND hwnd,const WCHAR *path)
{
    WCHAR buf[BUFLEN];
    HANDLE hFind;
    WIN32_FIND_DATA FindFileData;
    int i=0;

    if(flags&FLAG_NOGUI)return;
    wsprintf(buf,L"%s\\%s\\*.txt",data_dir,path);
    hFind=FindFirstFile(buf,&FindFileData);
    if(hFind!=INVALID_HANDLE_VALUE)
    do
    if(!(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
    {
        wsprintf(buf,L"%s\\%s\\%s",data_dir,path,FindFileData.cFileName);
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
}

void vault_startmonitors()
{
    WCHAR buf[BUFLEN];

    wsprintf(buf,L"%s\\langs",data_dir);
    mon_lang=monitor_start(buf,FILE_NOTIFY_CHANGE_LAST_WRITE|FILE_NOTIFY_CHANGE_FILE_NAME,1,lang_callback);
    wsprintf(buf,L"%s\\themes",data_dir);
    mon_theme=monitor_start(buf,FILE_NOTIFY_CHANGE_LAST_WRITE|FILE_NOTIFY_CHANGE_FILE_NAME,1,theme_callback);
}

void vault_stopmonitors()
{
    if(mon_lang)monitor_stop(mon_lang);
    if(mon_theme)monitor_stop(mon_theme);
}

void CALLBACK lang_callback(const WCHAR *szFile,DWORD action,LPARAM lParam)
{
    UNREFERENCED_PARAMETER(szFile);
    UNREFERENCED_PARAMETER(action);
    UNREFERENCED_PARAMETER(lParam);

    //log_con("FileMonitor: changed %S\n",szFile);
    PostMessage(hMain,WM_UPDATELANG,0,0);
}

void CALLBACK theme_callback(const WCHAR *szFile,DWORD action,LPARAM lParam)
{
    UNREFERENCED_PARAMETER(szFile);
    UNREFERENCED_PARAMETER(action);
    UNREFERENCED_PARAMETER(lParam);

    //log_con("FileMonitor: changed %S\n",szFile);
    PostMessage(hMain,WM_UPDATETHEME,0,0);
}
//}

//{ FileMonitor
monitor_t *monitor_start(LPCTSTR szDirectory, DWORD notifyFilter, int subdirs, FileChangeCallback callback)
{
	monitor_t *pMonitor = (monitor_t*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(*pMonitor));

	wcscpy(pMonitor->dir,szDirectory);
	pMonitor->hDir=CreateFile(szDirectory,FILE_LIST_DIRECTORY,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
	                            NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OVERLAPPED,NULL);

	if(pMonitor->hDir!=INVALID_HANDLE_VALUE)
	{
		pMonitor->ol.hEvent    = CreateEvent(NULL,TRUE,FALSE,NULL);
		pMonitor->notifyFilter = notifyFilter;
		pMonitor->callback     = callback;
		pMonitor->subdirs      = subdirs;

		if (monitor_refresh(pMonitor))
		{
			return pMonitor;
		}
		else
		{
			CloseHandle(pMonitor->ol.hEvent);
			CloseHandle(pMonitor->hDir);
		}
	}
	HeapFree(GetProcessHeap(),0,pMonitor);
	return NULL;
}

BOOL monitor_refresh(monitor_t *pMonitor)
{
	return ReadDirectoryChangesW(pMonitor->hDir,pMonitor->buffer,sizeof(pMonitor->buffer),pMonitor->subdirs,
	                      pMonitor->notifyFilter,NULL,&pMonitor->ol,monitor_callback);
}

void CALLBACK monitor_callback(DWORD dwErrorCode,DWORD dwNumberOfBytesTransfered,LPOVERLAPPED lpOverlapped)
{
    UNREFERENCED_PARAMETER(dwNumberOfBytesTransfered);

	TCHAR szFile[MAX_PATH];
	PFILE_NOTIFY_INFORMATION pNotify;
	monitor_t *pMonitor=(monitor_t*)lpOverlapped;
	size_t offset=0;

	if(dwErrorCode==ERROR_SUCCESS)
	{
		do
		{
			pNotify=(PFILE_NOTIFY_INFORMATION)&pMonitor->buffer[offset];
			offset+=pNotify->NextEntryOffset;

            lstrcpynW(szFile,pNotify->FileName,pNotify->FileNameLength/sizeof(WCHAR)+1);

			{
                FILE *f;
                WCHAR buf[BUFLEN];
                int m=0,sz=-1,flag;

                errno=0;
                wsprintf(buf,L"%ws\\%ws",pMonitor->dir,szFile);
                log_con("{\n  changed'%S'\n",buf);
                f=_wfsopen(buf,L"rb",0x10); //deny read/write mode
                if(f)m=2;
                if(!f)
                {
                    f=_wfopen(buf,L"rb");
                    if(f)m=1;
                }
                if(f)
                {
                    fseek(f,0,SEEK_END);
                    sz=ftell(f);
                    fclose(f);
                }
                /*
                    m0: failed to open
                    m1: opened(normal)
                    m2: opened(exclusive)
                */
                switch(pNotify->Action)
                {
                    case 1: // Added
                        flag=errno?0:1;
                        break;

                    case 2: // Removed
                        flag=1;
                        break;

                    case 3: // Modifed
                        flag=errno?0:1;
                        break;

                    case 4: // Renamed(old name)
                        flag=0;

                    case 5: // Renamed(new name)
                        flag=1;
                        break;

                    default:
                        flag=0;
                }

                if(!monitor_pause)
                    log_con("  %c a(%d),m(%d),err(%02d),size(%9d)\n",flag?'+':'-',pNotify->Action,m,errno,sz);

                //if(flag&&!monitor_pause)pMonitor->callback(szFile,pNotify->Action,pMonitor->lParam);
                log_con("}\n\n");
			}
		}while(pNotify->NextEntryOffset!=0);
	}
	if(!pMonitor->fStop)monitor_refresh(pMonitor);
}

void monitor_stop(monitor_t *pMonitor)
{
	if(pMonitor)
	{
		pMonitor->fStop=TRUE;
		CancelIo(pMonitor->hDir);
		if(!HasOverlappedIoCompleted(&pMonitor->ol))SleepEx(5,TRUE);
		CloseHandle(pMonitor->ol.hEvent);
		CloseHandle(pMonitor->hDir);
		HeapFree(GetProcessHeap(),0,pMonitor);
	}
}
//}
