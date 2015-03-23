#include "main.h"

const WCHAR INSTALLEDVENFILENAMEDEFPATH[]=L"%temp%\\SDI2\\InstalledID.txt";

HFONT CLIHelp_Font;
CommandLineParam_t CLIParam;

static void ExpandPath(WCHAR *Apath)
{
    #define INFO_BUFFER_SIZE 32767
    WCHAR infoBuf[INFO_BUFFER_SIZE];

    memset(infoBuf,0,sizeof(infoBuf));
    ExpandEnvironmentStringsW(Apath,infoBuf,INFO_BUFFER_SIZE);
    wcscpy(Apath,infoBuf);
    #undef INFO_BUFFER_SIZE
}

static WCHAR *ltrim(WCHAR *s)
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
    return FALSE;
}

static void ShowHelp(HINSTANCE AhInst)
{
    CLIHelp_Font=CreateFontA(-12,0,0,0,FW_NORMAL,0,0,0,
                         DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
                         DEFAULT_QUALITY,FF_DONTCARE,"Consolas");

    DialogBox(AhInst,MAKEINTRESOURCE(IDD_DIALOG1),0,(DLGPROC)ShowHelpProcedure);
    DeleteObject(CLIHelp_Font);
}

void SaveHWID(WCHAR *hwid)
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

void Parse_save_installed_id_swith(const WCHAR *ParamStr)
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

void Parse_HWID_installed_swith(const WCHAR *ParamStr)
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
        WCHAR buf[BUFLEN];
        wcscpy(buf,ParamStr+tmpLen);
        WCHAR *chB;

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
    CLIParam.ShowHelp=FALSE;
    CLIParam.SaveInstalledHWD=FALSE;
    CLIParam.SaveInstalledFileName[0]=0;
    CLIParam.HWIDInstalled=FALSE;
}

void RUN_CLI(CommandLineParam_t ACLIParam)
{
    WCHAR buf[BUFLEN];
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
        CreateDirectory(buf, NULL);
        DeleteFileW(CLIParam.SaveInstalledFileName);
    }
    else
    if(CLIParam.HWIDInstalled)
    {
        ExpandPath(CLIParam.SaveInstalledFileName);
        FILE *f;
        WCHAR *RStr;
        f=_wfopen(CLIParam.SaveInstalledFileName,L"rt");
        if(!f)log_err("Failed to open '%S'\n",CLIParam.SaveInstalledFileName);
        else
        {
            while(fgetws(buf,sizeof(buf),f))
            {
                //log_con("'%S'\n", buf);
                RStr=wcsstr(buf,CLIParam.HWIDSTR);
                if (RStr!=NULL)
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

bool isCfgSwithExist(const WCHAR *cmdParams,WCHAR *cfgPath)
{
    WCHAR **argv,*pr;
    int argc;
    int i;

    argv=CommandLineToArgvW(cmdParams,&argc);
    for(i=1;i<argc;i++)
    {
        pr=argv[i];
        if(pr[0]=='/')pr[0]='-';
        if(StrCmpIW(pr,GFG_DEF)==0)
        {
            wcscpy(cfgPath,pr+wcslen(GFG_DEF));
            return true;
        }
    }
    return false;
}

bool LoadCFGFile(const WCHAR *FileName,WCHAR *DestStr)
{
    FILE *f;
    WCHAR Buff[BUFLEN];

    *DestStr=0;

    ExpandEnvironmentStringsW(FileName,Buff,BUFLEN);
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

