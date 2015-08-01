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
#define INSTALLEDVENFILENAMEDEFPATH L"%temp%\\SDI2\\InstalledID.txt"

//{Global variables
HFONT CLIHelp_Font;
CommandLineParam_t CLIParam;
//}

static void ExpandPath(wchar_t *Apath)
{
    #define INFO_BUFFER_SIZE 32767
    wchar_t infoBuf[INFO_BUFFER_SIZE];

    memset(infoBuf,0,sizeof(infoBuf));
    ExpandEnvironmentStringsW(Apath,infoBuf,INFO_BUFFER_SIZE);
    wcscpy(Apath,infoBuf);
    #undef INFO_BUFFER_SIZE
}

static wchar_t *ltrim(wchar_t *s)
{
    while(iswspace(*s)) s++;
    return s;
}

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

        SendMessage(hEditBox,WM_SETFONT,(WPARAM)CLIHelp_Font,0);

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
    CLIHelp_Font=CreateFont(-12,0,0,0,FW_NORMAL,0,0,0,
                         DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
                         DEFAULT_QUALITY,FF_DONTCARE,L"Consolas");

    DialogBox(AhInst,MAKEINTRESOURCE(IDD_DIALOG1),0,(DLGPROC)ShowHelpProcedure);
    DeleteObject(CLIHelp_Font);
}

void SaveHWID(wchar_t *hwid)
{
    if(CLIParam.SaveInstalledHWD)
    {
        FILE *f=_wfopen(CLIParam.SaveInstalledFileName,L"a+");
        if(!f)
          log_err("Failed to create '%S'\n",CLIParam.SaveInstalledFileName);
        fwprintf(f,L"%s",hwid);
        fwprintf(f,L"\n");
        fclose(f);
    }
}

void Parse_save_installed_id_swith(const wchar_t *ParamStr)
{
    unsigned tmpLen=wcslen(SAVE_INSTALLED_ID_DEF);

    if(wcslen(ParamStr)>tmpLen)
    {
        if(ParamStr[tmpLen]==L':')wcscpy(CLIParam.SaveInstalledFileName,ParamStr+tmpLen+1);else
        if(ParamStr[tmpLen]==L' ')wcscpy(CLIParam.SaveInstalledFileName,INSTALLEDVENFILENAMEDEFPATH);
        else return;
    }
    else
        wcscpy(CLIParam.SaveInstalledFileName,INSTALLEDVENFILENAMEDEFPATH);

    CLIParam.SaveInstalledHWD=TRUE;
}

void Parse_HWID_installed_swith(const wchar_t *ParamStr)
{
    unsigned tmpLen=wcslen(HWIDINSTALLED_DEF);
    if(wcslen(ParamStr)<(tmpLen+17)) //-HWIDInstalled:VEN_xxxx&DEV_xxxx
    {
        log_err("invalid parameter %S\n",ParamStr);
        ret_global=ERROR_BAD_LENGTH;
        statemode=STATEMODE_EXIT;
        return;
    }
    else
    {
        wchar_t buf[BUFLEN];
        wcscpy(buf,ParamStr+tmpLen);
        wchar_t *chB;

        chB=wcsrchr (buf,'=');
        if (chB==NULL)
            wcscpy(CLIParam.SaveInstalledFileName,INSTALLEDVENFILENAMEDEFPATH);
        else
        {
            tmpLen=chB-buf+1;
            wcscpy(CLIParam.SaveInstalledFileName,buf+tmpLen);
            buf [tmpLen-1]=0;
        }
        wcscpy(CLIParam.HWIDSTR, buf);
        CLIParam.HWIDInstalled=TRUE;
    }
}

void init_CLIParam()
{
    memset(&CLIParam,0,sizeof(CLIParam));
    CLIParam.ShowHelp=false;
    CLIParam.SaveInstalledHWD=false;
    CLIParam.SaveInstalledFileName[0]=0;
    CLIParam.HWIDInstalled=false;
}

void RUN_CLI(CommandLineParam_t ACLIParam)
{
    wchar_t buf[BUFLEN];
    if(ACLIParam.ShowHelp)
    {
        ShowHelp(ghInst);
        flags|=FLAG_AUTOCLOSE|FLAG_NOGUI;
        statemode=STATEMODE_EXIT;
        return;
    } else
    if(CLIParam.SaveInstalledHWD)
    {
        ExpandPath(CLIParam.SaveInstalledFileName);
        wcscpy(buf, CLIParam.SaveInstalledFileName);
        PathRemoveFileSpec(buf);
        CreateDirectory(buf,nullptr);
        DeleteFileW(CLIParam.SaveInstalledFileName);
    }
    else
    if(CLIParam.HWIDInstalled)
    {
        ExpandPath(CLIParam.SaveInstalledFileName);
        FILE *f;
        f=_wfopen(CLIParam.SaveInstalledFileName,L"rt");
        if(!f)log_err("Failed to open '%S'\n",CLIParam.SaveInstalledFileName);
        else
        {
            while(fgetws(buf,sizeof(buf),f))
            {
                //log_con("'%S'\n", buf);
                if(wcsstr(buf,CLIParam.HWIDSTR)!=NULL)
                {
                    ret_global=1;
                    break;
                }
            }
            fclose(f);
        }
        flags|=FLAG_AUTOCLOSE|FLAG_NOGUI;
        statemode=STATEMODE_EXIT;
    }
}

bool isCfgSwithExist(const wchar_t *cmdParams,wchar_t *cfgPath)
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
            wcscpy(cfgPath,pr+wcslen(GFG_DEF));
            return true;
        }
    }
    return false;
}

bool loadCFGFile(const wchar_t *FileName,wchar_t *DestStr)
{
    FILE *f;
    wchar_t Buff[BUFLEN];

    *DestStr=0;

    ExpandEnvironmentStringsW(FileName,Buff,BUFLEN);
    log_con("Opening '%S'\n",Buff);
    f=_wfopen(Buff,L"rt");
    if(!f)
    {
        log_err("Failed to open '%S'\n",Buff);
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

