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

#include "common.h"
#include "indexing.h"
#include "enum.h"
#include "main.h"

#include "model.h"
#include "matcher.h"
#include "guicon.h"
#include "manager.h"
#include "update.h"
#include "settings.h"

extern HANDLE deviceupdate_event;
extern volatile int deviceupdate_exitflag;
extern int bundle_display;
extern int bundle_shadow;

//{ Bundle
unsigned int __stdcall Bundle::thread_scandevices(void *arg)
{
    State *state=(State *)arg;

    if((invaidate_set&INVALIDATE_DEVICES)==0)return 0;

    if(Settings.statemode==STATEMODE_REAL)state->scanDevices();
    if(Settings.statemode==STATEMODE_EMUL)state->load(Settings.state_file);

    return 0;
}

unsigned int __stdcall Bundle::thread_loadindexes(void *arg)
{
    Collection *collection=(Collection *)arg;

    if(invaidate_set&INVALIDATE_INDEXES)collection->updatedir();
    return 0;
}

unsigned int __stdcall Bundle::thread_getsysinfo(void *arg)
{
    State *state=(State *)arg;

    if(Settings.statemode==STATEMODE_REAL&&invaidate_set&INVALIDATE_SYSINFO)
        state->getsysinfo_slow();
    return 0;
}

Bundle::Bundle()
{
    matcher=CreateMatcher();
    bundle_init();
}

Bundle::~Bundle()
{
    delete matcher;
}

unsigned int __stdcall Bundle::thread_loadall(void *arg)
{
    Bundle *bundle=(Bundle *)arg;

    InitializeCriticalSection(&sync);
    while(1)
    {
        // Wait for an update request
        WaitForSingleObject(deviceupdate_event,INFINITE);
        if(deviceupdate_exitflag)break;
        bundle[bundle_shadow].bundle_init();
            /*static long long prmem;
            Log.print_con("Total mem:%ld KB(%ld)\n",nvwa::total_mem_alloc/1024,nvwa::total_mem_alloc-prmem);
            prmem=nvwa::total_mem_alloc;*/

        // Update bundle
        Log.print_con("*** START *** %d,%d [%d]\n",bundle_display,bundle_shadow,invaidate_set);
        bundle[bundle_shadow].bundle_prep();
        bundle[bundle_shadow].bundle_load(&bundle[bundle_display]);

        // Check if the state has been udated during scanning
        int cancel_update=0;
        if(!(Settings.flags&FLAG_NOGUI))
        if(WaitForSingleObject(deviceupdate_event,0)==WAIT_OBJECT_0)cancel_update=1;

        if(cancel_update)
        {
            Log.print_con("*** CANCEL ***\n\n");
            SetEvent(deviceupdate_event);
        }
        else
        {
            Log.print_con("*** FINISH primary ***\n\n");
            invaidate_set&=~(INVALIDATE_DEVICES|INVALIDATE_INDEXES|INVALIDATE_SYSINFO);

            if((Settings.flags&FLAG_NOGUI)&&(Settings.flags&FLAG_AUTOINSTALL)==0)
            {
                // NOGUI mode
                manager_g->matcher=bundle[bundle_shadow].matcher;
                manager_g->populate();
                manager_g->filter(Settings.filters);
                bundle[bundle_shadow].bundle_lowprioirity();
                break;
            }
            else // GUI mode
            {
                if(MainWindow.hMain)SendMessage(MainWindow.hMain,WM_BUNDLEREADY,(WPARAM)&bundle[bundle_shadow],(LPARAM)&bundle[bundle_display]);
            }

            // Save indexes, write info, etc
            Log.print_con("{2Sync\n");
            EnterCriticalSection(&sync);

            bundle[bundle_shadow].bundle_lowprioirity();
            Log.print_con("*** FINISH secondary ***\n\n");

            // Swap display and shadow bundle
            bundle_display^=1;
            bundle_shadow^=1;
            Log.print_con("}2Sync\n");
            bundle[bundle_shadow].bundle_init();
            LeaveCriticalSection(&sync);
        }
    }

    DeleteCriticalSection(&sync);
    return 0;
}

void Bundle::bundle_init()
{
    state.init();
    collection.init(Settings.drp_dir,Settings.index_dir,Settings.output_dir);
    matcher->init(&state,&collection);
}

void Bundle::bundle_prep()
{
    state.getsysinfo_fast();
}
void Bundle::bundle_load(Bundle *pbundle)
{
    HANDLE thandle[3];

    Timers.start(time_test);

    // Copy data from shadow if it's not updated
    if((invaidate_set&INVALIDATE_DEVICES)==0)
    {
        state=pbundle->state;
        Timers.reset(time_devicescan);
        if(invaidate_set&INVALIDATE_SYSINFO)state.getsysinfo_fast();
    }
    if((invaidate_set&INVALIDATE_SYSINFO)==0)state.getsysinfo_slow(&pbundle->state);
    if((invaidate_set&INVALIDATE_INDEXES)==0){collection=pbundle->collection;Timers.reset(time_indexes);}


    thandle[0]=(HANDLE)_beginthreadex(nullptr,0,&thread_scandevices,&state,0,nullptr);
    thandle[1]=(HANDLE)_beginthreadex(nullptr,0,&thread_loadindexes,&collection,0,nullptr);
    thandle[2]=(HANDLE)_beginthreadex(nullptr,0,&thread_getsysinfo,&state,0,nullptr);
    WaitForMultipleObjects(3,thandle,1,INFINITE);
    CloseHandle_log(thandle[0],L"bundle_load",L"0");
    CloseHandle_log(thandle[1],L"bundle_load",L"1");
    CloseHandle_log(thandle[2],L"bundle_load",L"2");

    /*if((invaidate_set&INVALIDATE_DEVICES)==0)
    {
        state=pbundle->state;time_devicescan=0;}*/

    state.isnotebook_a();
    state.genmarker();
    matcher->getState()->textas.shrink();
    matcher->populate();
    Timers.stop(time_test);
}

void Bundle::bundle_lowprioirity()
{
    wchar_t filename[BUFLEN];
    Timers.stoponce(time_startup,time_total);
    Timers.print();

    MainWindow.redrawmainwnd();

    collection.printstats();
    state.print();
    matcher->print();
    manager_g->print_hr();

    #ifdef USE_TORRENT
    if(Settings.flags&FLAG_CHECKUPDATES&&!Timers.get(time_chkupdate))Updater.checkUpdates();
    #endif

    collection.save();
    Log.gen_timestamp();
    wsprintf(filename,L"%s\\%sstate.snp",Settings.log_dir,Log.getTimestamp());
    state.save(filename);

    if(Settings.flags&COLLECTION_PRINT_INDEX)
    {
        Log.print_con("Saving humanreadable indexes...");
        collection.print_index_hr();
        Settings.flags&=~COLLECTION_PRINT_INDEX;
        Log.print_con("DONE\n");
    }
}
//}
