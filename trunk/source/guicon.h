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

// Timer
enum TIMER
{
    time_total,
    time_startup,
    time_indexes,
    time_devicescan,
    time_chkupdate,
    time_indexsave,
    time_indexprint,
    time_sysinfo,
    time_matcher,
    time_test,
    time_NUM,
};

class Timers_t
{
    long timers[time_NUM];
public:
    void start(int a){timers[a]=GetTickCount();}
    void stop(int a){if(timers[a])timers[a]=GetTickCount()-timers[a];}
    void reset(int a){timers[a]=0;}
    void print();
    void stoponce(int a,int b){if(!timers[a])timers[a]=GetTickCount()-timers[b];}
};
extern Timers_t Timers;

// Logging
enum LOG_VERBOSE
{
    LOG_VERBOSE_ARGS      = 0x0001,
    LOG_VERBOSE_SYSINFO   = 0x0002,
    LOG_VERBOSE_DEVICES   = 0x0004,
    LOG_VERBOSE_MATCHER   = 0x0008,
    LOG_VERBOSE_MANAGER   = 0x0010,
    LOG_VERBOSE_DRP       = 0x0020,
    LOG_VERBOSE_TIMES     = 0x0040,
    LOG_VERBOSE_LOG_ERR   = 0x0080,
    LOG_VERBOSE_LOG_CON   = 0x0100,
    LOG_VERBOSE_LAGCOUNTER= 0x0200,
    LOG_VERBOSE_DEVSYNC   = 0x0400,
    LOG_VERBOSE_BATCH     = 0x0800,
};

#define print_index print_nul

class Log_t
{
private:
    wchar_t timestamp[BUFLEN];
    FILE *logfile=nullptr;
    int error_count=0;
    int log_console=0;
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

public:
    void print_con(char const *format,...);
    void print_file(char const *format,...);
    void print_err(char const *format,...);
    void print_syserr(int r,const wchar_t *s);
    void print_nul(char const *format,...);

    void gen_timestamp();
    void start(wchar_t *log_dir);
    void save();
    void stop();

    bool isAllowed(int a){return (log_verbose&a)==a;}
    bool isHidden(int a){return !(log_verbose&a);}
    void set_verbose(int a){log_verbose=a;}
    void set_mode(int a){log_console=a;}
    int getErrorCount(){return error_count;}
    wchar_t *getTimestamp(){return timestamp;}
};
extern Log_t Log;

// Error handling
const wchar_t *errno_str();
void CloseHandle_log(HANDLE h,const wchar_t *func,const wchar_t *obj);
void UnregisterClass_log(LPCTSTR lpClassName,HINSTANCE hInstance,const wchar_t *func,const wchar_t *obj);
void start_exception_hadnlers();
void SignalHandler(int signum);

// Virus detection
void CALLBACK viruscheck(const wchar_t *szFile,DWORD action,LPARAM lParam);
void virusmonitor_start();
void virusmonitor_stop();

// FileMonitor
class Filemon;
typedef void (CALLBACK *FileChangeCallback)(const wchar_t *,DWORD,LPARAM);
class Filemon
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

private:
    static void CALLBACK monitor_callback(DWORD dwErrorCode,DWORD dwNumberOfBytesTransfered,LPOVERLAPPED lpOverlapped);
    int  refresh();

public:
    static Filemon *start(LPCTSTR szDirectory,DWORD notifyFilter,int subdirs,FileChangeCallback callback);
    void stop();
};

// Misc
int canWrite(const wchar_t *path);
DWORD run_command(const wchar_t* file,const wchar_t* cmd,int show,int wait);
void benchmark();
