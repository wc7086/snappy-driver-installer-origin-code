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

#ifndef UPDATE_H
#define UPDATE_H

// Declarations
class Updater_t;
class Canvas;

// Global variables
extern Updater_t *Updater;

// Updater
class Updater_t
{
public:
    int numfiles=0;
    static int torrentport,downlimit,uplimit,connections;

public:
    virtual ~Updater_t(){}

    virtual void showProgress(wchar_t *buf)=0;
    virtual void showPopup(Canvas &canvas)=0;

    virtual void downloadTorrent()=0;
    virtual void checkUpdates()=0;
    virtual void resumeDownloading()=0;
    virtual void pause()=0;

    virtual bool isTorrentReady()=0;
    virtual bool isPaused()=0;
    virtual bool isUpdateCompleted()=0;

    virtual int  populate(int flags)=0;
    virtual void setFilePriority(const wchar_t *name,int pri)=0;
    virtual void openDialog()=0;
};
Updater_t *CreateUpdater();

#endif
