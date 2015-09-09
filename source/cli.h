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

#ifndef CLI_H
#define CLI_H

#define SAVE_INSTALLED_ID_DEF   L"-save-installed-id"
#define HWIDINSTALLED_DEF       L"-HWIDInstalled:"
#define GFG_DEF                 L"-cfg:"

// Structures
struct CommandLineParam_t
{
    bool ShowHelp;
    bool SaveInstalledHWD;
    wchar_t SaveInstalledFileName[BUFLEN];
    bool HWIDInstalled;
    wchar_t HWIDSTR[BUFLEN];
};

// Global vars
extern CommandLineParam_t CLIParam;

// Misc functions
void SaveHWID(wchar_t *hwid);
void Parse_save_installed_id_swith(const wchar_t *ParamStr);
void Parse_HWID_installed_swith(const wchar_t *ParamStr);

void init_CLIParam();
void RUN_CLI(CommandLineParam_t ACLIParam);
bool isCfgSwithExist(const wchar_t *cmdParams,wchar_t *cfgPath);
bool loadCFGFile(const wchar_t *FileName,wchar_t *DestStr);

#endif
