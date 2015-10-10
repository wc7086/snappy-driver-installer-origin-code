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
#include "logging.h"
#include "settings.h"
#include "cli.h"

#include <windows.h>

// Depend on Win32API
#include "system.h"



#define INSTALLEDVENFILENAMEDEFPATH L"%temp%\\SDI2\\InstalledID.txt"

// Structures
struct CommandLineParam_t
{
    bool ShowHelp;
    bool SaveInstalledHWD;
    wchar_t SaveInstalledFileName[BUFLEN];
    bool HWIDInstalled;
    wchar_t HWIDSTR[BUFLEN];
};
CommandLineParam_t CLIParam;

static void ExpandPath(wchar_t *Apath)
{
    #define INFO_BUFFER_SIZE 32767
    wchar_t infoBuf[INFO_BUFFER_SIZE];

    memset(infoBuf,0,sizeof(infoBuf));
    System.ExpandEnvVar(Apath,infoBuf,INFO_BUFFER_SIZE);
    wcscpy(Apath,infoBuf);
    #undef INFO_BUFFER_SIZE
}

void SaveHWID(wchar_t *hwid)
{
    if(CLIParam.SaveInstalledHWD)
    {
        FILE *f=_wfopen(CLIParam.SaveInstalledFileName,L"a+");
        if(!f)
          Log.print_err("Failed to create '%S'\n",CLIParam.SaveInstalledFileName);
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

    CLIParam.SaveInstalledHWD=true;
}

void Parse_HWID_installed_swith(const wchar_t *ParamStr)
{
    unsigned tmpLen=wcslen(HWIDINSTALLED_DEF);
    if(wcslen(ParamStr)<(tmpLen+17)) //-HWIDInstalled:VEN_xxxx&DEV_xxxx
    {
        Log.print_err("invalid parameter %S\n",ParamStr);
        ret_global=24;//ERROR_BAD_LENGTH;
        Settings.statemode=STATEMODE_EXIT;
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
        CLIParam.HWIDInstalled=true;
    }
}

void init_CLIParam()
{
    memset(&CLIParam,0,sizeof(CLIParam));
}

void RUN_CLI()
{
    wchar_t buf[BUFLEN];

    if(CLIParam.SaveInstalledHWD)
    {
        ExpandPath(CLIParam.SaveInstalledFileName);
        wcscpy(buf, CLIParam.SaveInstalledFileName);
        System.fileDelSpec(buf);
        System.CreateDir(buf);
        System.deletefile(CLIParam.SaveInstalledFileName);
    }
    else
    if(CLIParam.HWIDInstalled)
    {
        ExpandPath(CLIParam.SaveInstalledFileName);
        FILE *f;
        f=_wfopen(CLIParam.SaveInstalledFileName,L"rt");
        if(!f)Log.print_err("Failed to open '%S'\n",CLIParam.SaveInstalledFileName);
        else
        {
            while(fgetws(buf,sizeof(buf),f))
            {
                //Log.print_con("'%S'\n", buf);
                if(wcsstr(buf,CLIParam.HWIDSTR)!=NULL)
                {
                    ret_global=1;
                    break;
                }
            }
            fclose(f);
        }
        Settings.flags|=FLAG_AUTOCLOSE|FLAG_NOGUI;
        Settings.statemode=STATEMODE_EXIT;
    }
}
