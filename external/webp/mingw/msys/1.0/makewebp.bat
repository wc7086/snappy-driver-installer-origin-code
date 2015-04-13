set HOME=%CD%\home

if NOT "x%WD%" == "x" set WD=

if NOT "x%WD%" == "x" set WD=

rem ember command.com only uses the first eight characters of the label.
goto _WindowsNT

rem ember that we only execute here if we are in command.com.
:_Windows

if "x%COMSPEC%" == "x" set COMSPEC=command.com
start /min %COMSPEC% /e:4096 /c %0 GOTO: _Resume %0 %1 %2 %3 %4 %5 %6 %7 %8 %9
goto EOF

rem ember that we execute here if we recursed.
:_Resume
for %%F in (1 2 3) do shift
if NOT EXIST %WD%msys-1.0.dll set WD=.\bin\

rem ember that we get here even in command.com.
:_WindowsNT

rem Hopefully a temporary workaround for getting MSYS shell to run on x64
rem (WoW64 cmd prompt sets PROCESSOR_ARCHITECTURE to x86)
if not "x%PROCESSOR_ARCHITECTURE%" == "xAMD64" goto _NotX64
set COMSPEC=%WINDIR%\SysWOW64\cmd.exe
%COMSPEC% /c %0 %1 %2 %3 %4 %5 %6 %7 %8 %9
goto EOF
:_NotX64

if NOT EXIST %WD%msys-1.0.dll set WD=%~dp0\bin\

%WD%sh --login -i %~dp0\home\makewebp.bat
