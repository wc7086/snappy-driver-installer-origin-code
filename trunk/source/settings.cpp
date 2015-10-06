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
#include <wchar.h>
#include "common.h"
#include "settings.h"
#include "cli.h"
#include "guicon.h"
#include "update.h"
#include "install.h"

#include <windows.h>
#include <setupapi.h>       // for CommandLineToArgvW

#include "system.h"
#include "main.h"
#include "draw.h"

int volatile installmode=MODE_NONE;
int invaidate_set;
int num_cores;
int ret_global=0;

Font CLIHelp_Font;

static BOOL CALLBACK ShowHelpProcedure(HWND hwnd,UINT Message,WPARAM wParam,LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    HWND hEditBox;
    LPCSTR s;
    int sz;

    switch(Message)
    {
    case WM_INITDIALOG:
        SetWindowTextW(hwnd,L"Command Line options help");
        hEditBox=GetDlgItem(hwnd,0);
        SetWindowTextW(hEditBox,L"Command Line options");
        //ShowWindow(hEditBox,SW_HIDE);
        hEditBox=GetDlgItem(hwnd,IDOK);
        ShowWindow(hEditBox,SW_HIDE);
        hEditBox=GetDlgItem(hwnd,IDCANCEL);
        SetWindowTextW(hEditBox,L"Close");

        get_resource(IDR_CLI_HELP,(void **)&s,&sz);
        hEditBox=GetDlgItem(hwnd,IDC_EDIT1);

        SendMessage(hEditBox,WM_SETFONT,(WPARAM)CLIHelp_Font.get(),0);

        SetWindowTextA(hEditBox,s);
        SendMessage(hEditBox,EM_SETREADONLY,1,0);

        return TRUE;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            EndDialog(hwnd,IDOK);
            return TRUE;

        case IDCANCEL:
            EndDialog(hwnd,IDCANCEL);
            return TRUE;

        default:
            break;
        }
        break;

    default:
        break;
    }
    return false;
}

static void ShowHelp(HINSTANCE AhInst)
{
    CLIHelp_Font.SetFont(L"Consolas",12);

    DialogBox(AhInst,MAKEINTRESOURCE(IDD_DIALOG1),0,(DLGPROC)ShowHelpProcedure);
}

Settings_t::Settings_t()
{
    *curlang=0;
    wcscpy(curtheme,  L"(default)");
    wcscpy(logO_dir,  L"logs");
    license=0;

    wcscpy(drp_dir,   L"drivers");
    wcscpy(output_dir,L"indexes\\SDI\\txt");
    *drpext_dir=0;
    wcscpy(index_dir, L"indexes\\SDI");
    wcscpy(data_dir,  L"tools\\SDI");
    *log_dir=0;

    wcscpy(state_file,L"untitled.snp");
    *finish=0;
    *finish_upd=0;
    *finish_rb=0;

    flags=COLLECTION_USE_LZMA;
    statemode=STATEMODE_REAL;
    expertmode=0;
    hintdelay=500;
    wndwx=0,wndwy=0;
    filters=
        (1<<ID_SHOW_MISSING)+
        (1<<ID_SHOW_NEWER)+
        (1<<ID_SHOW_BETTER)+
        (1<<ID_SHOW_NF_MISSING)+
        (1<<ID_SHOW_ONE);
    virtual_os_version=0;
    virtual_arch_type=0;
}

bool Settings_t::argstr(const wchar_t *s,const wchar_t *cmp,wchar_t *d)
{
    if(StrStrIW(s,cmp)){wcscpy(d,s+wcslen(cmp));return true;}
    return false;
}

bool Settings_t::argint(const wchar_t *s,const wchar_t *cmp,int *d)
{
    if(StrStrIW(s,cmp)){*d=_wtoi_my(s+wcslen(cmp));return true;}
    return false;
}

bool Settings_t::argopt(const wchar_t *s,const wchar_t *cmp,int *d)
{
    if(StrStrIW(s,cmp)){*d=1;return true;}
    return false;
}

bool Settings_t::argflg(const wchar_t *s,const wchar_t *cmp,int f)
{
    if(!StrCmpIW(s,cmp)){flags|=f;return true;}
    return false;
}

void Settings_t::parse(const wchar_t *str,int ind)
{
    Log.print_con("Args:[%S]\n",str);
    int argc;
    wchar_t **argv=CommandLineToArgvW(str,&argc);
    for(int i=ind;i<argc;i++)
    {
        wchar_t *pr=argv[i];
        if(pr[0]=='/')pr[0]='-';

        if(argstr(pr,L"-drp_dir:",drp_dir))continue;
        if(argstr(pr,L"-index_dir:",index_dir))continue;
        if(argstr(pr,L"-output_dir:",output_dir))continue;
        if(argstr(pr,L"-data_dir:",data_dir))continue;
        if(argstr(pr,L"-log_dir:",logO_dir))continue;

        if(argstr(pr,L"-finish_cmd:",finish))continue;
        if(argstr(pr,L"-finishrb_cmd:",finish_rb))continue;
        if(argstr(pr,L"-finish_upd_cmd:",finish_upd))continue;

        if(argstr(pr,L"-lang:",curlang))continue;
        if(argstr(pr,L"-theme:",curtheme))continue;
        if(argint(pr,L"-hintdelay:",&hintdelay))continue;
        if(argint(pr,L"-wndwx:",&wndwx))continue;
        if(argint(pr,L"-wndwy:",&wndwy))continue;
        if(argint(pr,L"-filters:",&filters))continue;

        if(argint(pr,L"-port:",&Updater->torrentport))continue;
        if(argint(pr,L"-downlimit:",&Updater->downlimit))continue;
        if(argint(pr,L"-uplimit:",&Updater->uplimit))continue;
        if(argint(pr,L"-connections:",&Updater->connections))continue;

        if(argopt(pr,L"-license",&license))continue;
        if(argopt(pr,L"-expertmode",&expertmode))continue;
        if(argflg(pr,L"-showconsole",FLAG_SHOWCONSOLE))continue;
        if(argflg(pr,L"-norestorepnt",FLAG_NORESTOREPOINT))continue;
        if(argflg(pr,L"-novirusalerts",FLAG_NOVIRUSALERTS))continue;
        if(argflg(pr,L"-preservecfg",FLAG_PRESERVECFG))continue;

        if(argflg(pr,L"-showdrpnames1",FLAG_SHOWDRPNAMES1))continue;
        if(argflg(pr,L"-showdrpnames2",FLAG_SHOWDRPNAMES2))continue;
        if(argflg(pr,L"-oldstyle",FLAG_OLDSTYLE))continue;

        if(argflg(pr,L"-checkupdates",FLAG_CHECKUPDATES))continue;
        if(argflg(pr,L"-onlyupdates",FLAG_ONLYUPDATES))continue;
        if(!StrCmpIW(pr,L"-7z"))
        {
            wchar_t cmd[BUFLEN];
            wsprintf(cmd,L"7za.exe %s",StrStrIW(str,L"-7z")+4);
            Log.print_con("Executing '%S'\n",cmd);
            registerall();
            ret_global=Extract7z(cmd);
            Log.print_con("Ret: %d\n",ret_global);
            statemode=STATEMODE_EXIT;
            break;
        }
        else
        if(!wcscmp(pr,L"-PATH"))
        {
            wcscpy(drpext_dir,argv[++i]);
            flags|=FLAG_AUTOCLOSE|
                FLAG_AUTOINSTALL|FLAG_NORESTOREPOINT|FLAG_DPINSTMODE|//FLAG_DISABLEINSTALL|
                FLAG_PRESERVECFG;
        }
        else
        if(!wcscmp(pr,L"-install")&&argc-i==3)
        {
            wchar_t buf[BUFLEN];
            Log.print_con("Install '%S' '%s'\n",argv[i+1],argv[i+2]);
            GetEnvironmentVariable(L"TEMP",buf,BUFLEN);
            wsprintf(extractdir,L"%s\\SDI",buf);
            installmode=MODE_INSTALLING;
            driver_install(argv[i+1],argv[i+2],&ret_global,&needreboot);
            Log.print_con("Ret: %X,%d\n",ret_global,needreboot);
            if(needreboot)ret_global|=0x80000000;
            wsprintf(buf,L" /c rd /s /q \"%s\"",extractdir);
            System.run_command(L"cmd",buf,SW_HIDE,1);
            statemode=STATEMODE_EXIT;
            break;
        }else
        if(!StrCmpIW(pr,L"-?"))
        {
            ShowHelp(ghInst);
            //Settings.flags|=FLAG_AUTOCLOSE|FLAG_NOGUI;
            Settings.statemode=STATEMODE_EXIT;
            break;
        }else
        if(!StrCmpIW(pr,L"-filtersp"))     {flags|=FLAG_FILTERSP;flags&=~COLLECTION_USE_LZMA;}else
        if(!StrCmpIW(pr,L"-reindex"))      flags|=COLLECTION_FORCE_REINDEXING;else
        if(!StrCmpIW(pr,L"-index_hr"))     flags|=COLLECTION_PRINT_INDEX;else
        if(!StrCmpIW(pr,L"-nogui"))        flags|=FLAG_NOGUI|FLAG_AUTOCLOSE;else
        if(!StrCmpIW(pr,L"-autoinstall"))  flags|=FLAG_AUTOINSTALL;else
        if(!StrCmpIW(pr,L"-autoclose"))    flags|=FLAG_AUTOCLOSE;else
        if(!StrCmpIW(pr,L"-autoupdate"))   flags|=FLAG_AUTOUPDATE;else

        if( StrStrIW(pr,L"-extractdir:"))  {flags|=FLAG_EXTRACTONLY;wcscpy(extractdir,pr+12);}else
        if(!StrCmpIW(pr,L"-keepunpackedindex"))flags|=FLAG_KEEPUNPACKINDEX;else
        if(!StrCmpIW(pr,L"-keeptempfiles"))flags|=FLAG_KEEPTEMPFILES;else
        if(!StrCmpIW(pr,L"-disableinstall"))flags|=FLAG_DISABLEINSTALL;else
        if(!StrCmpIW(pr,L"-failsafe"))     flags|=FLAG_FAILSAFE;else
        if(!StrCmpIW(pr,L"-delextrainfs")) flags|=FLAG_DELEXTRAINFS;else

        if( StrStrIW(pr,L"-ls:"))          {wcscpy(state_file,pr+4);statemode=STATEMODE_EMUL;}else
        if( StrStrIW(pr,L"-verbose:"))     Log.set_verbose(_wtoi_my(pr+9));else
        if(!StrCmpIW(pr,L"-nologfile"))    flags|=FLAG_NOLOGFILE;else
        if(!StrCmpIW(pr,L"-nosnapshot"))   flags|=FLAG_NOSNAPSHOT;else
        if(!StrCmpIW(pr,L"-nostamp"))      flags|=FLAG_NOSTAMP;else

        if(!StrCmpIW(pr,L"-a:32"))         virtual_arch_type=32;else
        if(!StrCmpIW(pr,L"-a:64"))         virtual_arch_type=64;else
        if( StrStrIW(pr,L"-v:"))           virtual_os_version=_wtoi_my(pr+3);else

        if( StrStrIW(pr,SAVE_INSTALLED_ID_DEF))Parse_save_installed_id_swith(pr);else
        if( StrStrIW(pr,HWIDINSTALLED_DEF))    Parse_HWID_installed_swith(pr); else
        if( StrStrIW(pr,GFG_DEF))              continue;
        else
            Log.print_err("Unknown argument '%S'\n",pr);
        if(statemode==STATEMODE_EXIT)break;
    }
    ExpandEnvironmentStrings(logO_dir,log_dir,BUFLEN);
    LocalFree(argv);
    if(statemode==STATEMODE_EXIT)return;
}

void Settings_t::save()
{
    if(flags&FLAG_PRESERVECFG)return;
    if(!System.canWrite(L"sdi.cfg"))
    {
        Log.print_err("ERROR in settings_save(): Write-protected,'sdi.cfg'\n");
        return;
    }
    FILE *f=_wfopen(L"sdi.cfg",L"wt");
    if(!f)return;
    fwprintf(f,L"\"-drp_dir:%s\"\n\"-index_dir:%s\"\n\"-output_dir:%s\"\n"
              "\"-data_dir:%s\"\n\"-log_dir:%s\"\n\n"
              "\"-finish_cmd:%s\"\n\"-finishrb_cmd:%s\"\n\"-finish_upd_cmd:%s\"\n\n"
              "\"-lang:%s\"\n\"-theme:%s\"\n-hintdelay:%d\n-wndwx:%d\n-wndwy:%d\n-filters:%d\n\n"
              "-port:%d\n-downlimit:%d\n-uplimit:%d\n-connections:%d\n\n",
            drp_dir,index_dir,output_dir,
            data_dir,logO_dir,
            finish,finish_rb,finish_upd,
            curlang,curtheme,hintdelay,wndwx,wndwy,filters,
            Updater->torrentport,Updater->downlimit,Updater->uplimit,Updater->connections);

    if(license)fwprintf(f,L"-license ");
    if(expertmode)fwprintf(f,L"-expertmode ");
    if(flags&FLAG_SHOWCONSOLE)fwprintf(f,L"-showconsole ");
    if(flags&FLAG_NORESTOREPOINT)fwprintf(f,L"-norestorepnt ");
    if(flags&FLAG_NOVIRUSALERTS)fwprintf(f,L"-novirusalerts ");

    if(flags&FLAG_SHOWDRPNAMES1)fwprintf(f,L"-showdrpnames1 ");
    if(flags&FLAG_SHOWDRPNAMES2)fwprintf(f,L"-showdrpnames2 ");
    if(flags&FLAG_OLDSTYLE)fwprintf(f,L"-oldstyle ");

    if(flags&FLAG_CHECKUPDATES)fwprintf(f,L"-checkupdates ");
    if(flags&FLAG_ONLYUPDATES)fwprintf(f,L"-onlyupdates ");
    fclose(f);
}

void Settings_t::loginfo()
{
    if(Log.isAllowed(LOG_VERBOSE_ARGS))
    {
        Log.print_con("Settings\n");
        Log.print_con("  drp_dir='%S'\n",drp_dir);
        Log.print_con("  index_dir='%S'\n",index_dir);
        Log.print_con("  output_dir='%S'\n",output_dir);
        Log.print_con("  data_dir='%S'\n",data_dir);
        Log.print_con("  log_dir='%S'\n",log_dir);
        Log.print_con("  extractdir='%S'\n",extractdir);
        Log.print_con("  lang=%S\n",curlang);
        Log.print_con("  theme=%S\n",curtheme);
        Log.print_con("  expertmode=%d\n",expertmode);
        Log.print_con("  filters=%d\n",filters);
        Log.print_con("  autoinstall=%d\n",flags&FLAG_AUTOINSTALL?1:0);
        Log.print_con("  autoclose=%d\n",flags&FLAG_AUTOCLOSE?1:0);
        Log.print_con("  failsafe=%d\n",flags&FLAG_FAILSAFE?1:0);
        Log.print_con("  delextrainfs=%d\n",flags&FLAG_DELEXTRAINFS?1:0);
        Log.print_con("  checkupdates=%d\n",flags&FLAG_CHECKUPDATES?1:0);
        Log.print_con("  norestorepnt=%d\n",flags&FLAG_NORESTOREPOINT?1:0);
        Log.print_con("  disableinstall=%d\n",flags&FLAG_DISABLEINSTALL?1:0);
        Log.print_con("\n");

        if(statemode==STATEMODE_EMUL)Log.print_con("Virtual system system config '%S'\n",state_file);
        if(virtual_arch_type)Log.print_con("Virtual Windows version: %d-bit\n",virtual_arch_type);
        if(virtual_os_version)Log.print_con("Virtual Windows version: %d.%d\n",virtual_os_version/10,virtual_os_version%10);
        Log.print_con("\n");
    }
}

bool Settings_t::load(const wchar_t *filename)
{
    wchar_t buf[BUFLEN];

    if(!loadCFGFile(filename,buf))return false;
    parse(buf,0);
    return true;
}

wchar_t *Settings_t::ltrim(wchar_t *s)
{
    while(iswspace(*s)) s++;
    return s;
}

bool Settings_t::loadCFGFile(const wchar_t *FileName,wchar_t *DestStr)
{
    FILE *f;
    wchar_t Buff[BUFLEN];

    *DestStr=0;

    ExpandEnvironmentStringsW(FileName,Buff,BUFLEN);
    Log.print_con("Opening '%S'\n",Buff);
    f=_wfopen(Buff,L"rt");
    if(!f)
    {
        Log.print_err("Failed to open '%S'\n",Buff);
        return false;
    }

    while(fgetws(Buff,sizeof(Buff),f))
    {
        wcscpy(Buff,ltrim(Buff));       //  trim spaces
        if(*Buff=='#')continue;         // comments
        if(*Buff==';')continue;         // comments
        if(*Buff=='/')*Buff='-';         // replace / with -
        if(wcsstr(Buff,L"-?"))continue; // ignore -?
        if(Buff[wcslen(Buff)-1]=='\n')Buff[wcslen(Buff)-1]='\0';
        if(wcslen(Buff)==0)continue;

        wcscat(wcscat(DestStr,Buff),L" ");
    }
    fclose(f);
    return true;
}

bool Settings_t::load_cfg_switch(const wchar_t *cmdParams)
{
    wchar_t **argv;
    int argc;

    argv=CommandLineToArgvW(cmdParams,&argc);
    for(int i=1;i<argc;i++)
    {
        wchar_t *pr=argv[i];
        if(pr[0]=='/')pr[0]='-';
        if(StrStrIW(pr,GFG_DEF))
        {
            if(load(pr+wcslen(GFG_DEF)))
            {
                flags|=FLAG_PRESERVECFG;
                return true;
            }
        }
    }
    return false;
}
