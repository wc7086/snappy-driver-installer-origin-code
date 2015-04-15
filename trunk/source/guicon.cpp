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

//{ Global variables
FILE *logfile=0;
int error_count=0;
int log_console=0;
WCHAR timestamp[BUFLEN];
monitor_t *mon_vir;

int log_verbose=
    LOG_VERBOSE_ARGS|
    LOG_VERBOSE_SYSINFO|
    LOG_VERBOSE_DEVICES|
    LOG_VERBOSE_MATCHER|
    LOG_VERBOSE_MANAGER|
    LOG_VERBOSE_DRP|
    LOG_VERBOSE_TIMES|
    LOG_VERBOSE_LOG_ERR|
    LOG_VERBOSE_LOG_CON|
    //LOG_VERBOSE_LAGCOUNTER|
    //LOG_VERBOSE_DEVSYNC|
    0;

long
    time_total=0,
    time_startup=0,
    time_indexes=0,
    time_devicescan=0,
    time_chkupdate=0,
    time_indexsave=0,
    time_indexprint=0,
    time_sysinfo=0,
    time_matcher=0,
    time_test=0;
//}

//{ Logging
void log_times()
{
    if((log_verbose&LOG_VERBOSE_TIMES)==0)return;
    log_file("Times\n");
    log_file("  devicescan: %7ld (%d errors)\n",time_devicescan,error_count);
    log_file("  indexes:    %7ld\n",time_indexes);
    log_file("  sysinfo:    %7ld\n",time_sysinfo);
    log_file("  matcher:    %7ld\n",time_matcher);
    log_file("  chkupdate:  %7ld\n",time_chkupdate);
    log_file("  startup:    %7ld (%ld)\n",time_startup,time_startup-time_devicescan-time_indexes-time_matcher-time_sysinfo);
    log_file("  indexsave:  %7ld\n",time_indexsave);
    log_file("  indexprint: %7ld\n",time_indexprint);
    log_file("  total:      %7ld\n",time_total);
    log_file("  test:       %7ld\n",time_test);
}

void gen_timestamp()
{
    WCHAR pcname[BUFLEN];
    time_t rawtime;
    struct tm *ti;
    DWORD sz=BUFLEN;

    GetComputerName(pcname,&sz);
    time(&rawtime);
    ti=localtime(&rawtime);
    if(flags&FLAG_NOSTAMP)
        *timestamp=0;
    else
        wsprintf(timestamp,L"%4d_%02d_%02d__%02d_%02d_%02d__%s_",
             1900+ti->tm_year,ti->tm_mon+1,ti->tm_mday,
             ti->tm_hour,ti->tm_min,ti->tm_sec,pcname);
}

void log_start(WCHAR *logdir)
{
    WCHAR filename[BUFLEN];

    if(flags&FLAG_NOLOGFILE)return;
    setlocale(LC_ALL,"");
    //system("chcp 1251");

    gen_timestamp();

    wsprintf(filename,L"%s\\%slog.txt",logdir,timestamp);
    if(!canWrite(filename))
    {
        log_err("ERROR in log_start(): Write-protected,'%S'\n",filename);
        GetEnvironmentVariable(L"TEMP",logdir,BUFLEN);
        wcscat(logdir,L"\\SDI_logs");
        wsprintf(filename,L"%s\\%slog.txt",logdir,timestamp);
    }

    mkdir_r(logdir);
    logfile=_wfopen(filename,L"wt");
    if(!logfile)
    {
        log_err("ERROR in log_start(): Write-protected,'%S'\n",filename);
        GetEnvironmentVariable(L"TEMP",logdir,BUFLEN);
        wcscat(logdir,L"\\SDI_logs");
        wsprintf(filename,L"%s\\%slog.txt",logdir,timestamp);
        mkdir_r(logdir);
        logfile=_wfopen(filename,L"wb");
    }
    if((log_verbose&LOG_VERBOSE_BATCH)==0)
        log_file("{start logging\n%s\n\n",SVN_REV_STR);
}

void log_save()
{
    if(!logfile)return;
    fflush(logfile);
}

void log_stop()
{
    if(!logfile)return;
    if((log_verbose&LOG_VERBOSE_BATCH)==0)
        log_file("}stop logging");
    fclose(logfile);
}

void log_file(CHAR const *format,...)
{
    CHAR buffer[1024*16];

    if(!logfile)return;
    va_list args;
    va_start(args,format);
    vsprintf(buffer,format,args);
    fputs(buffer,logfile);
    if(log_console)fputs(buffer,stdout);
    va_end(args);
}

void log_err(CHAR const *format,...)
{
    CHAR buffer[1024*16];

    if((log_verbose&LOG_VERBOSE_LOG_ERR)==0)return;
    va_list args;
    va_start(args,format);
    vsprintf(buffer,format,args);
    if(logfile)fputs(buffer,logfile);
    fputs(buffer,stdout);
    va_end(args);
}

void log_con(CHAR const *format,...)
{
    CHAR buffer[1024*16];

    if((log_verbose&LOG_VERBOSE_LOG_CON)==0)return;
    va_list args;
    va_start(args,format);
    wvsprintfA(buffer,format,args);
    if(logfile)fputs(buffer,logfile);
    fputs(buffer,stdout);
    va_end(args);
}

void log_nul(CHAR const *format,...)
{
    UNREFERENCED_PARAMETER(format);
}
//}

//{ Error handling
const WCHAR *errno_str()
{
    switch(errno)
    {
        case EPERM:               return L"Operation not permitted";
        case ENOENT:              return L"No such file or directory";
        case ESRCH:               return L"No such process";
        case EINTR:               return L"Interrupted function";
        case EIO:                 return L"I/O error";
        case ENXIO:               return L"No such device or address";
        case E2BIG:               return L"Argument list too long";
        case ENOEXEC:             return L"Exec format error";
        case EBADF:               return L"Bad file number";
        case ECHILD:              return L"No spawned processes";
        case EAGAIN:              return L"No more processes or not enough memory or maximum nesting level reached";
        case ENOMEM:              return L"Not enough memory";
        case EACCES:              return L"Permission denied";
        case EFAULT:              return L"Bad address";
        case EBUSY:               return L"Device or resource busy";
        case EEXIST:              return L"File exists";
        case EXDEV:               return L"Cross-device link";
        case ENODEV:              return L"No such device";
        case ENOTDIR:             return L"Not a directory";
        case EISDIR:              return L"Is a directory";
        case EINVAL:              return L"Invalid argument";
        case ENFILE:              return L"Too many files open in system";
        case EMFILE:              return L"Too many open files";
        case ENOTTY:              return L"Inappropriate I/O control operation";
        case EFBIG:               return L"File too large";
        case ENOSPC:              return L"No space left on device";
        case ESPIPE:              return L"Invalid seek";
        case EROFS:               return L"Read-only file system";
        case EMLINK:              return L"Too many links";
        case EPIPE:               return L"Broken pipe";
        case EDOM:                return L"Math argument";
        case ERANGE:              return L"Result too large";
        case EDEADLK:             return L"Resource deadlock would occur";
        case ENAMETOOLONG:        return L"Filename too long";
        case ENOLCK:              return L"No locks available";
        case ENOSYS:              return L"Function not supported";
        case ENOTEMPTY:           return L"Directory not empty";
        case EILSEQ:              return L"Illegal byte sequence";
        default: return L"Unknown error";
    }
}

void print_error(int r,const WCHAR *s)
{
    WCHAR buf[BUFLEN];
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,r,MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),(LPWSTR)&buf,BUFLEN,NULL);
    log_err("ERROR with %S:[%d]'%S'\n",s,r,buf);
    error_count++;
}

void CloseHandle_log(HANDLE h,const WCHAR *func,const WCHAR *obj)
{
    if(!CloseHandle(h))
        log_err("ERROR in %S(): failed CloseHandle(%S)\n",func,obj);
}

void UnregisterClass_log(LPCTSTR lpClassName,HINSTANCE hInstance,const WCHAR *func,const WCHAR *obj)
{
    if(!UnregisterClass(lpClassName,hInstance))
        log_err("ERROR in %S(): failed UnregisterClass(%S)\n",func,obj);
}

static void myterminate()
{
    WCHAR buf[BUFLEN];

    std::exception_ptr p;
    p=std::current_exception();

    try
    {
        std::rethrow_exception (p);
    }
    catch(const std::exception& e)
    {
        wsprintfW(buf,L"Exception: %s\n",e.what());
    }
    catch(int i)
    {
        wsprintfW(buf,L"Exception: %d\n",i);
    }
    catch(char const*str)
    {
        wsprintfW(buf,L"Exception: %S\n",str);
    }
    catch(WCHAR const*str)
    {
        wsprintfW(buf,L"Exception: %s\n",str);
    }
    catch(...)
    {
        wsprintfW(buf,L"Exception: unknown");
    }
    log_err("ERROR: %S\n",buf);
    log_save();
    StrCatW(buf,L"\n\nThe program will self terminate now.");
    MessageBox(hMain,buf,L"Exception",MB_ICONERROR);

    abort();
}

static void myunexpected()
{
    log_err("ERROR: myunexpected()\n");
    myterminate();
}

void start_exception_hadnlers()
{
    std::set_unexpected(myunexpected);
    std::set_terminate(myterminate);
}
//}

//{ Virus detection
void CALLBACK viruscheck(const WCHAR *szFile,DWORD action,LPARAM lParam)
{
    UNREFERENCED_PARAMETER(szFile);
    UNREFERENCED_PARAMETER(action);
    UNREFERENCED_PARAMETER(lParam);

    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA FindFileData;
    int type;
    int update=0;

    if(flags&FLAG_NOVIRUSALERTS)return;
    type=GetDriveType(0);

    // autorun.inf
    if(type!=DRIVE_CDROM)
    {
        if(PathFileExists(L"\\autorun.inf"))
        {
            FILE *f;
            f=_wfopen(L"\\autorun.inf",L"rb");
            if(f)
            {
                char buf[BUFLEN];
                fread(buf,BUFLEN,1,f);
                fclose(f);
                buf[BUFLEN-1]=0;
                if(!strstr(buf,"[NOT_A_VIRUS]")&&StrStrIA(buf,"open"))
                    manager_g->items_list[SLOT_VIRUS_AUTORUN].isactive=update=1;
            }
            else
                log_con("NOTE: cannot open autorun.inf [error: %d]\n",errno);
        }
    }

    // RECYCLER
    if(type==DRIVE_REMOVABLE)
        if(PathFileExists(L"\\RECYCLER")&&!PathFileExists(L"\\RECYCLER\\not_a_virus.txt"))
            manager_g->items_list[SLOT_VIRUS_RECYCLER].isactive=update=1;

    // Hidden folders
    hFind=FindFirstFile(L"\\*.*",&FindFileData);
    if(type==DRIVE_REMOVABLE)
    while(FindNextFile(hFind,&FindFileData)!=0)
    {
        if(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
        {
            if(lstrcmp(FindFileData.cFileName,L"..")==0)continue;
            if(lstrcmpi(FindFileData.cFileName,L"System Volume Information")==0)continue;

            if(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN)
            {
                WCHAR bufw[BUFLEN];
                wsprintf(bufw,L"\\%ws\\not_a_virus.txt",FindFileData.cFileName);
                if(PathFileExists(bufw))continue;
                log_con("VIRUS_WARNING: hidden folder '%S'\n",FindFileData.cFileName);
                manager_g->items_list[SLOT_VIRUS_HIDDEN].isactive=update=1;
            }
        }
    }
    FindClose(hFind);
    if(update)
    {
        manager_g->setpos();
        SetTimer(hMain,1,1000/60,0);
    }
}

void virusmonitor_start()
{
    mon_vir=monitor_start(L"\\",FILE_NOTIFY_CHANGE_LAST_WRITE|FILE_NOTIFY_CHANGE_FILE_NAME,0,viruscheck);
}

void virusmonitor_stop()
{
    if(mon_vir)mon_vir->monitor_stop();
}
//}

//{ Misc
int canWrite(const WCHAR *path)
{
    DWORD flagsv;
    WCHAR drive[4];

    wcscpy(drive,L"C:\\");

    if(path&&wcslen(path)>1&&path[1]==':')
    {
        drive[0]=path[0];
        GetVolumeInformation(drive,0,0,0,0,&flagsv,0,0);
    }
    else
        GetVolumeInformation(0,0,0,0,0,&flagsv,0,0);

    return (flagsv&FILE_READ_ONLY_VOLUME)?0:1;
}

DWORD run_command(const WCHAR* file,const WCHAR* cmd,int show,int wait)
{
    DWORD ret;

    SHELLEXECUTEINFO ShExecInfo;
    memset(&ShExecInfo,0,sizeof(SHELLEXECUTEINFO));
    ShExecInfo.cbSize=sizeof(SHELLEXECUTEINFO);
    ShExecInfo.fMask=SEE_MASK_NOCLOSEPROCESS;
    ShExecInfo.lpFile=file;
    ShExecInfo.lpParameters=cmd;
    ShExecInfo.nShow=show;

    log_con("Run(%S,%S,%d,%d)\n",file,cmd,show,wait);
    if(!wcscmp(file,L"open"))
        ShellExecute(0,L"open",cmd,0,0,SW_SHOWNORMAL);
    else
        ShellExecuteEx(&ShExecInfo);

    if(!wait)return 0;
    WaitForSingleObject(ShExecInfo.hProcess,INFINITE);
    GetExitCodeProcess(ShExecInfo.hProcess,&ret);
    return ret;
}
//}
