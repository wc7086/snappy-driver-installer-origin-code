echo Using %TOOLSET%
set HOME=%CD%\home

set MSYSTEM=MINGW32
set PREFIX=%2
%1\sh --login -i %~dp0\home\makewebp.bat

set MSYSTEM=MINGW64
set PREFIX=%3
%1\sh --login -i %~dp0\home\makewebp.bat

if "%TOOLSET%"=="msvc" goto skip
cd home\libwebp-%LIBWEBP_VER%
call "%MSVC_PATH%\VC\vcvarsall"
nmake /f Makefile.vc CFG=release-static RTLIBCFG=static OBJDIR=output ARCH=x32
nmake /f Makefile.vc CFG=debug-static RTLIBCFG=static OBJDIR=output ARCH=x32
call "%MSVC_PATH%\VC\bin\amd64\vcvars64.bat"
nmake /f Makefile.vc CFG=release-static RTLIBCFG=static OBJDIR=output ARCH=x64
nmake /f Makefile.vc CFG=debug-static RTLIBCFG=static OBJDIR=output ARCH=x64
:skip