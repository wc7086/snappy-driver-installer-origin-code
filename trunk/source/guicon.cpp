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
#include "guicon.h"
#include "settings.h"

#include <windows.h>
#include <ctime>

// Depend on Win32API
#include "system.h"
#include "main.h"
#include "manager.h"
#include "theme.h"

//{ Global variables
Filemon *mon_vir;
Log_t Log;
Timers_t Timers;
//}

//{ Logging

Timers_t::Timers_t():
    timers{0,0,0,0,0,0,0,0,0,0}
{
}

void Timers_t::start(int a)
{
    timers[a]=GetTickCount();
}
void Timers_t::stop(int a)
{
    if(timers[a])timers[a]=GetTickCount()-timers[a];
}
void Timers_t::stoponce(int a,int b)
{
    if(!timers[a])timers[a]=GetTickCount()-timers[b];
}

void Timers_t::print()
{
    if(Log.isHidden(LOG_VERBOSE_TIMES))return;
    Log.print_con("Times\n");
    Log.print_con("  devicescan: %7ld (%d errors)\n",timers[time_devicescan],Log.getErrorCount());
    Log.print_con("  indexes:    %7ld\n",timers[time_indexes]);
    Log.print_con("  sysinfo:    %7ld\n",timers[time_sysinfo]);
    Log.print_con("  matcher:    %7ld\n",timers[time_matcher]);
    Log.print_con("  chkupdate:  %7ld\n",timers[time_chkupdate]);
    Log.print_con("  startup:    %7ld (%ld)\n",timers[time_startup],timers[time_startup]-timers[time_devicescan]-timers[time_indexes]-timers[time_matcher]-timers[time_sysinfo]);
    Log.print_con("  indexsave:  %7ld\n",timers[time_indexsave]);
    Log.print_con("  indexprint: %7ld\n",timers[time_indexprint]);
    Log.print_con("  total:      %7ld\n",GetTickCount()-timers[time_total]);
    Log.print_con("  test:       %7ld\n",timers[time_test]);
}

void Log_t::gen_timestamp()
{
    wchar_t pcname[BUFLEN];
    time_t rawtime;
    struct tm *ti;
    DWORD sz=BUFLEN;

    GetComputerName(pcname,&sz);
    time(&rawtime);
    ti=localtime(&rawtime);
    if(Settings.flags&FLAG_NOSTAMP)
        *timestamp=0;
    else
        wsprintf(timestamp,L"%4d_%02d_%02d__%02d_%02d_%02d__%s_",
             1900+ti->tm_year,ti->tm_mon+1,ti->tm_mday,
             ti->tm_hour,ti->tm_min,ti->tm_sec,pcname);
}

void Log_t::start(wchar_t *logdir)
{
    wchar_t filename[BUFLEN];

    if(Settings.flags&FLAG_NOLOGFILE)return;
    setlocale(LC_ALL,"");
    //system("chcp 1251");

    gen_timestamp();

    wsprintf(filename,L"%s\\%slog.txt",logdir,timestamp);
    if(!System.canWrite(filename))
    {
        Log.print_err("ERROR in log_start(): Write-protected,'%S'\n",filename);
        GetEnvironmentVariable(L"TEMP",logdir,BUFLEN);
        wcscat(logdir,L"\\SDI_logs");
        wsprintf(filename,L"%s\\%slog.txt",logdir,timestamp);
    }

    mkdir_r(logdir);
    logfile=_wfopen(filename,L"wt");
    if(!logfile)
    {
        Log.print_err("ERROR in log_start(): Write-protected,'%S'\n",filename);
        GetEnvironmentVariable(L"TEMP",logdir,BUFLEN);
        wcscat(logdir,L"\\SDI_logs");
        wsprintf(filename,L"%s\\%slog.txt",logdir,timestamp);
        mkdir_r(logdir);
        logfile=_wfopen(filename,L"wb");
    }
    if((log_verbose&LOG_VERBOSE_BATCH)==0)
        Log.print_file("{start logging\n%s\n\n",SVN_REV_STR);
}

void Log_t::save()
{
    if(!logfile)return;
    fflush(logfile);
}

void Log_t::stop()
{
    if(!logfile)return;
    if((log_verbose&LOG_VERBOSE_BATCH)==0)
        Log.print_file("}stop logging");
    fclose(logfile);
}

void Log_t::print_file(char const *format,...)
{
    char buffer[1024*16];

    if(!logfile)return;
    va_list args;
    va_start(args,format);
    vsprintf(buffer,format,args);
    fputs(buffer,logfile);
    if(log_console)fputs(buffer,stdout);
    va_end(args);
}

void Log_t::print_err(char const *format,...)
{
    char buffer[1024*16];

    if((log_verbose&LOG_VERBOSE_LOG_ERR)==0)return;
    va_list args;
    va_start(args,format);
    vsprintf(buffer,format,args);
    if(logfile)fputs(buffer,logfile);
    fputs(buffer,stdout);
    va_end(args);
}

void Log_t::print_con(char const *format,...)
{
    char buffer[1024*16];

    if((log_verbose&LOG_VERBOSE_LOG_CON)==0)return;
    va_list args;
    va_start(args,format);
    wvsprintfA(buffer,format,args);
    if(logfile)fputs(buffer,logfile);
    fputs(buffer,stdout);
    va_end(args);
}

void Log_t::print_nul(char const *format,...)
{
    UNREFERENCED_PARAMETER(format);
}
//}

//{ Error handling
const wchar_t *errno_str()
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

void Log_t::print_syserr(int r,const wchar_t *s)
{
    wchar_t buf[BUFLEN];
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
                    nullptr,r,MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),(LPWSTR)&buf,BUFLEN,nullptr);
    Log.print_err("ERROR with %S:[%d]'%S'\n",s,r,buf);
    error_count++;
}

static void myterminate()
{
    wchar_t buf[BUFLEN];

    std::exception_ptr p;
    p=std::current_exception();

    try
    {
        std::rethrow_exception (p);
    }
    catch(const std::exception& e)
    {
        wsprintf(buf,L"Exception: %S\n",e.what());
    }
    catch(int i)
    {
        wsprintf(buf,L"Exception: %d\n",i);
    }
    catch(char const*str)
    {
        wsprintf(buf,L"Exception: %S\n",str);
    }
    catch(wchar_t const*str)
    {
        wsprintf(buf,L"Exception: %s\n",str);
    }
    catch(...)
    {
        wsprintf(buf,L"Exception: unknown");
    }
    Log.print_err("ERROR: %S\n",buf);
    Log.save();
    Log.stop();
    wcscat(buf,L"\n\nThe program will self terminate now.");
    MessageBox(MainWindow.hMain,buf,L"Exception",MB_ICONERROR);

    abort();
}

static void myunexpected()
{
    Log.print_err("ERROR: myunexpected()\n");
    myterminate();
}

void start_exception_hadnlers()
{
    std::set_unexpected(myunexpected);
    std::set_terminate(myterminate);
}

void SignalHandler(int signum)
{
    Log.print_err("!!! Crashed %d!!!\n",signum);
    Log.save();
    Log.stop();
}

#undef new
void* operator new(size_t size, const char* file, int line)
{
    try
    {
        //Log.print_con("File '%s',Line %d,Size %d\n",file,line,size);
        return new char[size];
    }
    catch(...)
    {
        Log.print_con("File '%s',Line %d,Size %d\n",file,line,size);
        throw;
    }
}
void* operator new[](size_t size, const char* file, int line)
{
    try
    {
        //Log.print_con("File '%s',Line %d,Size %d\n",file,line,size);
        return new char[size];
    }
    catch(...)
    {
        Log.print_con("File '%s',Line %d,Size %d\n",file,line,size);
        throw;
    }
}
//#define new DEBUG_NEW

//}

//{ Virus detection
void viruscheck(const wchar_t *szFile,int action,int lParam)
{
    UNREFERENCED_PARAMETER(szFile);
    UNREFERENCED_PARAMETER(action);
    UNREFERENCED_PARAMETER(lParam);

    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA FindFileData;
    int type;
    int update=0;

    if(Settings.flags&FLAG_NOVIRUSALERTS)return;
    type=GetDriveType(nullptr);

    // autorun.inf
    if(type!=DRIVE_CDROM)
    {
        if(System.FileExists(L"\\autorun.inf"))
        {
            FILE *f;
            f=_wfopen(L"\\autorun.inf",L"rb");
            if(f)
            {
                char buf[BUFLEN];
                fread(buf,BUFLEN,1,f);
                fclose(f);
                buf[BUFLEN-1]=0;
                if(!StrStrIA(buf,"[NOT_A_VIRUS]")&&StrStrIA(buf,"open"))
                    manager_g->itembar_setactive(SLOT_VIRUS_AUTORUN,update=1);
            }
            else
                Log.print_con("NOTE: cannot open autorun.inf [error: %d]\n",errno);
        }
    }

    // RECYCLER
    if(type==DRIVE_REMOVABLE)
        if(System.FileExists(L"\\RECYCLER")&&!System.FileExists(L"\\RECYCLER\\not_a_virus.txt"))
            manager_g->itembar_setactive(SLOT_VIRUS_RECYCLER,update=1);

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
                wchar_t bufw[BUFLEN];
                wsprintf(bufw,L"\\%ws\\not_a_virus.txt",FindFileData.cFileName);
                if(System.FileExists(bufw))continue;
                Log.print_con("VIRUS_WARNING: hidden folder '%S'\n",FindFileData.cFileName);
                manager_g->itembar_setactive(SLOT_VIRUS_HIDDEN,update=1);
            }
        }
    }
    FindClose(hFind);
    if(update)
    {
        manager_g->setpos();
        SetTimer(MainWindow.hMain,1,1000/60,nullptr);
    }
}

void virusmonitor_start()
{
    mon_vir=CreateFilemon(L"\\",FILE_NOTIFY_CHANGE_LAST_WRITE|FILE_NOTIFY_CHANGE_FILE_NAME,0,viruscheck);
}

void virusmonitor_stop()
{
    delete mon_vir;
}
//}

//{ FileMonitor
struct FilemonDataPOD
{
	OVERLAPPED ol;
	HANDLE     hDir;
	BYTE       buffer[32*1024];
	LPARAM     lParam;
	DWORD      notifyFilter;
	BOOL       fStop;
	wchar_t      dir[BUFLEN];
	int        subdirs;
	FileChangeCallback callback;
};

class FilemonImp:public Filemon
{
    FilemonDataPOD data;

private:
    static void CALLBACK monitor_callback(DWORD dwErrorCode,DWORD dwNumberOfBytesTransfered,LPOVERLAPPED lpOverlapped);
    static int refresh(FilemonDataPOD &data);

public:
    FilemonImp(const wchar_t *szDirectory,int notifyFilter,int subdirs,FileChangeCallback callback);
    ~FilemonImp();
};

Filemon *CreateFilemon(const wchar_t *szDirectory,int notifyFilter,int subdirs,FileChangeCallback callback)
{
    return new FilemonImp(szDirectory,notifyFilter,subdirs,callback);
}

FilemonImp::FilemonImp(const wchar_t *szDirectory, int notifyFilter_, int subdirs_, FileChangeCallback callback_)
{
	wcscpy(data.dir,szDirectory);

	data.hDir=CreateFile(szDirectory,FILE_LIST_DIRECTORY,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
	                            nullptr,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OVERLAPPED,nullptr);

	if(data.hDir!=INVALID_HANDLE_VALUE)
	{
		data.ol.hEvent    = CreateEvent(nullptr,TRUE,FALSE,nullptr);
		data.notifyFilter = notifyFilter_;
		data.callback     = callback_;
		data.subdirs      = subdirs_;

		if(refresh(data))
		{
			return;
		}
		else
		{
			CloseHandle(data.ol.hEvent);
			CloseHandle(data.hDir);
			data.hDir=INVALID_HANDLE_VALUE;
		}
	}
}

int FilemonImp::refresh(FilemonDataPOD &data1)
{
	return ReadDirectoryChangesW(data1.hDir,data1.buffer,sizeof(data1.buffer),data1.subdirs,
	                      data1.notifyFilter,nullptr,&data1.ol,monitor_callback);
}

void CALLBACK FilemonImp::monitor_callback(DWORD dwErrorCode,DWORD dwNumberOfBytesTransfered,LPOVERLAPPED lpOverlapped)
{
    UNREFERENCED_PARAMETER(dwNumberOfBytesTransfered);

	TCHAR szFile[MAX_PATH];
	PFILE_NOTIFY_INFORMATION pNotify;
	FilemonDataPOD *pMonitor=reinterpret_cast<FilemonDataPOD*>(lpOverlapped);

	if(dwErrorCode==ERROR_SUCCESS)
	{
        size_t offset=0;
        do
		{
			pNotify=(PFILE_NOTIFY_INFORMATION)&pMonitor->buffer[offset];
			offset+=pNotify->NextEntryOffset;

            lstrcpynW(szFile,pNotify->FileName,pNotify->FileNameLength/sizeof(wchar_t)+1);

			if(!monitor_pause)
			{
                FILE *f;
                wchar_t buf[BUFLEN];
                int m=0,sz=-1,flag;

                errno=0;
                wsprintf(buf,L"%ws\\%ws",pMonitor->dir,szFile);
                Log.print_con("{\n  changed'%S'\n",buf);
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
                        break;

                    case 5: // Renamed(new name)
                        flag=1;
                        break;

                    default:
                        flag=0;
                }

                Log.print_con("  %c a(%d),m(%d),err(%02d),size(%9d)\n",flag?'+':'-',pNotify->Action,m,errno,sz);

                if(flag)pMonitor->callback(szFile,pNotify->Action,pMonitor->lParam);
                Log.print_con("}\n\n");
			}
		}while(pNotify->NextEntryOffset!=0);
	}
	if(!pMonitor->fStop)refresh(*pMonitor);
}

FilemonImp::~FilemonImp()
{
	if(data.hDir!=INVALID_HANDLE_VALUE)
    {
        data.fStop=TRUE;
        CancelIo(data.hDir);
        if(!HasOverlappedIoCompleted(&data.ol))SleepEx(5,TRUE);
        CloseHandle(data.ol.hEvent);
        CloseHandle(data.hDir);
    }
}
//}
