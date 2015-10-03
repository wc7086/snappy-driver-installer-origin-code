@echo off
for /F "tokens=1,2 delims=#" %%a in ('"prompt #$H#$E# & echo on & for %%b in (1) do rem"') do (set "DEL=%%a")

rem Detect GCC (32-bit)
set GCC_VERSION=4.8.1
set GCC_VERSION2=48
set GCC_PATH=c:\MinGW
if /I exist c:\MinGW_481 (set GCC_PATH=c:\MinGW_481& goto foundgcc)
if /I exist c:\MinGW (set GCC_PATH=c:\MinGW& goto foundgcc)
:foundgcc

rem Detect GCC (64-bit)
set GCC64_VERSION=4.9.2
set GCC64_VERSION2=49
set GCC64_PATH=TDM-GCC-64
if /I exist c:\TDM-GCC-64_492 (set GCC64_PATH=c:\TDM-GCC-64_492& goto foundgcc64)
if /I exist c:\TDM-GCC-64 (set GCC64_PATH=c:\TDM-GCC-64& goto foundgcc64)
:foundgcc64

rem versions
set BOOST_VER2=1.59.0
set BOOST_VER=1_59_0
set LIBTORRENT_VER2=1.0.6
set LIBTORRENT_VER=1_0_6
set LIBWEBP_VER=0.4.3

rem configure paths
set BOOST_ROOT=%CD%\boost_%BOOST_VER%
set MSYS_PATH=%GCC_PATH%\msys\1.0
set MSYS_BIN=%MSYS_PATH%\bin
set BOOST_BUILD_PATH=%BOOST_ROOT%
set LIBTORRENT_PATH=%CD%\libtorrent-libtorrent-%LIBTORRENT_VER%
set WEBP_PATH=%CD%\webp

rem GCC 5.1.0
rem goto skip510
set GCC_PATH=c:\mingw_510\mingw32
set GCC_VERSION=5.1.0
set GCC_VERSION2=51
set GCC_PREFIX1=/i686-w64-mingw32
set GCC_PREFIX=\i686-w64-mingw32
set GCC64_PATH=c:\mingw_510\mingw64
set GCC64_VERSION=5.1.0
set GCC64_VERSION2=51
set GCC64_PREFIX1=/x86_64-w64-mingw32
set GCC64_PREFIX=\x86_64-w64-mingw32
set MSYS_PATH=C:\msys32
set MSYS_BIN=%MSYS_PATH%\usr\bin
set ADR64=\adrs-mdl-64
:skip510

set BOOST_INSTALL_PATH=C:\BOOST32_%GCC_VERSION2%
set BOOST64_INSTALL_PATH=C:\BOOST64_%GCC64_VERSION2%

set path=%GCC_PATH%\bin;%BOOST_ROOT%;%MSYS_BIN%;%path%

rem Check for MinGW
if /I not exist "%GCC_PATH%\bin" (color 0C&echo ERROR: MinGW not found in %GCC_PATH% & goto fatalError)

rem Check for MinGW64
if /I not exist "%GCC64_PATH%\bin" (color 0C&echo ERROR: MinGW_64 not found in %GCC64_PATH% & goto fatalError)

rem Check for MSYS
if /I not exist "%MSYS_PATH%" (color 0C&echo ERROR: MSYS not found in %MSYS_PATH% & goto fatalError)

:mainmenu
cls
color 03
echo.
echo   ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
echo   º MAIN MENU               º
echo   ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
echo   º A - Install all         º
echo   º C - Check all           º
echo   º D - Delete all          º
echo   º T - Rebuild libttorret  º
echo   º W - Rebuild WebP        º
echo   º Q - Quit                º
echo   ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼
echo.
set /p MENU=Enter command:
if /I "%menu%"=="A" call :installall
if /I "%menu%"=="D" call :delall
if /I "%menu%"=="C" (color 7&cls&call :checkall)
if /I "%menu%"=="T" (color 7&cls&goto installtorrent)
if /I "%menu%"=="W" (color 7&cls&goto installwebp)
if /I "%menu%"=="Q" (color 7&cls&exit)
goto mainmenu

:delall
cls
color 7
echo.
echo|set /p=Deleteing...

rem del libtorrent
rd /S /Q "%GCC_PATH%%GCC_PREFIX%\include\libtorrent" 2>nul
echo|set /p=.
rd /S /Q "%GCC64_PATH%%GCC64_PREFIX%\include\libtorrent" 2>nul
echo|set /p=.
del "%GCC_PATH%%GCC_PREFIX%\lib\libtorrent.a" 2>nul
echo|set /p=.
del "%GCC_PATH%%GCC_PREFIX%\lib\libtorrent_dbg.a" 2>nul
echo|set /p=.
del "%GCC64_PATH%%GCC64_PREFIX%\lib\libtorrent.a" 2>nul
echo|set /p=.
del "%GCC64_PATH%%GCC64_PREFIX%\lib\libtorrent_dbg.a" 2>nul
echo|set /p=.
del "%GCC_PATH%%GCC_PREFIX%\lib\libboost_system_tr.a" 2>nul
echo|set /p=.
del "%GCC64_PATH%%GCC64_PREFIX%\lib\libboost_system_tr.a" 2>nul
echo|set /p=.
rd /S /Q "%LIBTORRENT_PATH%\bin" 2>nul
echo|set /p=.
rd /S /Q "%LIBTORRENT_PATH%\examples\bin" 2>nul
echo|set /p=.

rem del webp
rd /S /Q "%GCC_PATH%%GCC_PREFIX%\include\webp" 2>nul
echo|set /p=.
rd /S /Q "%GCC64_PATH%%GCC64_PREFIX%\include\webp" 2>nul
echo|set /p=.
del "%GCC_PATH%%GCC_PREFIX%\lib\libwebp.*" 2>nul
echo|set /p=.
del "%GCC64_PATH%%GCC64_PREFIX%\lib\libwebp.*" 2>nul
echo|set /p=.
rd /S /Q "%MSYS_PATH%\home\libwebp-%LIBWEBP_VER%" 2>nul
echo|set /p=.
del "%MSYS_PATH%\makewebp.bat" 2>nul
echo|set /p=.
del "%MSYS_PATH%\home\makewebp.bat" 2>nul
echo|set /p=.
del "%MSYS_PATH%\home\libwebp-%LIBWEBP_VER%.tar.gz" 2>nul
echo|set /p=.

rem del BOOST
rd /S /Q "%BOOST_ROOT%\bin.v2" 2>nul
echo|set /p=.
rd /S /Q "%BOOST_ROOT%\libs\config\checks\architecture\bin" 2>nul
echo|set /p=.
rd /S /Q "%BOOST_ROOT%\tools\build\src\engine\bin.ntx86" 2>nul
echo|set /p=.
del "%BOOST_ROOT%\bjam.exe" 2>nul
echo|set /p=.
del "%BOOST_ROOT%\b2.exe" 2>nul
echo|set /p=.

call :ColorText 0A "DONE"
echo.
echo.
pause
goto :eof

:checkall
echo.
echo GCC (32 bit):  %GCC_PATH%
echo GCC (64 bit):  %GCC64_PATH%
echo MSYS:          %MSYS_PATH%
echo WebP:          %WEBP_PATH%
echo libtorrent:    %LIBTORRENT_PATH%
echo BOOST_scr:     %BOOST_ROOT%
rem echo BOOST_dest:    %BOOST_INSTALL_PATH%
rem echo BOOST64_dest:  %BOOST64_INSTALL_PATH%
echo.

echo|set /p=Checking wget...................
if /I exist "%MSYS_BIN%\wget.exe" (call :ColorText 0A "OK") else (call :ColorText 0C "FAIL")
echo.

echo|set /p=Checking tar....................
if /I exist "%MSYS_BIN%\tar.exe" (call :ColorText 0A "OK") else (call :ColorText 0C "FAIL")
echo.

echo|set /p=Checking make...................
if /I exist "%MSYS_BIN%\make.exe" (call :ColorText 0A "OK") else (call :ColorText 0C "FAIL")
echo.

echo|set /p=Checking libwebp-%LIBWEBP_VER%.tar.gz...
if /I exist "%WEBP_PATH%\mingw\msys\1.0\home\libwebp-%LIBWEBP_VER%.tar.gz" (call :ColorText 0A "OK") else (call :ColorText 0C "FAIL")
echo.

echo|set /p=Checking boost_%BOOST_VER%.tar.gz....
if /I exist "%LIBTORRENT_PATH%\..\boost_%BOOST_VER%.tar.gz" (call :ColorText 0A "OK") else (call :ColorText 0C "FAIL")
echo.

echo|set /p=Checking libtorrent.tar.gz......
if /I exist "%LIBTORRENT_PATH%\..\libtorrent-rasterbar-%LIBTORRENT_VER%.tar.gz" (call :ColorText 0A "OK") else (call :ColorText 0C "FAIL")
echo.

if /I "%GCC_VERSION2%"=="51" goto skipheaders
echo|set /p=Checking wspiapi.h..............
if /I exist "%GCC_PATH%\include\wspiapi.h" (call :ColorText 0A "OK") else (call :ColorText 0C "FAIL")
echo.

echo|set /p=Checking shobjidl.h.............
if /I exist "%GCC_PATH%\include\shobjidl.h" (call :ColorText 0A "OK") else (call :ColorText 0C "FAIL")
echo.
:skipheaders

echo|set /p=Checking BOOST(source)..........
if /I exist "%BOOST_ROOT%\boost.png" (call :ColorText 0A "OK") else (call :ColorText 0C "FAIL")
echo.

echo|set /p=Checking BOOST(binaries32)......
if /I exist "%GCC_PATH%%GCC_PREFIX%\lib\libboost_system_tr.a" (call :ColorText 0A "OK") else (call :ColorText 0C "FAIL")
echo.

echo|set /p=Checking BOOST(binaries64)......
if /I exist "%GCC64_PATH%%GCC64_PREFIX%\lib\libboost_system_tr.a" (call :ColorText 0A "OK") else (call :ColorText 0C "FAIL")
echo.

echo|set /p=Checking BJAM...................
if /I exist "%BOOST_ROOT%\bjam.exe" (call :ColorText 0A "OK") else (call :ColorText 0C "FAIL")
echo.

echo|set /p=Checking WebP...................
if /I exist "%GCC_PATH%%GCC_PREFIX%\lib\libwebp.a" (call :ColorText 0A "OK") else (call :ColorText 0C "FAIL")
echo.

echo|set /p=Checking libtorrent(source).....
if /I exist "%LIBTORRENT_PATH%\examples\client_test.cpp" (call :ColorText 0A "OK") else (call :ColorText 0C "FAIL")
echo.

echo|set /p=Checking libtorrent(binaries32).
if /I exist "%GCC_PATH%%GCC_PREFIX%\lib\libtorrent.a" (call :ColorText 0A "OK") else (call :ColorText 0C "FAIL")
echo.

echo|set /p=Checking libtorrent(binaries64).
if /I exist "%GCC64_PATH%%GCC64_PREFIX%\lib\libtorrent.a" (call :ColorText 0A "OK") else (call :ColorText 0C "FAIL")
echo.

echo.
pause
goto :eof

:installall
Color 7
cls
echo.

rem download wget
if /I exist "%MSYS_BIN%\wget.exe" (echo Skipping downloading wget & goto skipwget)
call :ColorText 9F "Downloading wget"&echo.
mingw-get install msys-wget
%MSYS_BIN%\pacman.exe -S wget --noconfirm
:skipwget

rem download tar
if /I exist "%MSYS_BIN%\tar.exe" (echo Skipping downloading tar & goto skiptar)
call :ColorText 9F "Downloading tar"&echo.
%MSYS_BIN%\pacman.exe -S tar --noconfirm
:skiptar

rem download make
if /I exist "%MSYS_BIN%\make.exe" (echo Skipping downloading make & goto skipmake)
call :ColorText 9F "Downloading make"&echo.
%MSYS_BIN%\pacman.exe -S make --noconfirm
:skipmake

rem update toolchain
call :ColorText 9F "Updating toolchain"&echo.
%MSYS_BIN%\pacman.exe -Syu --noconfirm
copy %GCC64_PATH%\bin\libwinpthread-1.dll %GCC64_PATH%\libexec\gcc\x86_64-w64-mingw32\5.1.0

rem download WebP
if /I exist "%LIBTORRENT_PATH%\..\webp\mingw\msys\1.0\home\libwebp-%LIBWEBP_VER%.tar.gz" (echo Skipping downloading WebP & goto skipdownloadwebp)
call :ColorText 9F "Downloading WebP"&echo.
%MSYS_BIN%\wget http://downloads.webmproject.org/releases/webp/libwebp-%LIBWEBP_VER%.tar.gz -Owebp\mingw\msys\1.0\home\libwebp-%LIBWEBP_VER%.tar.gz
:skipdownloadwebp

rem download BOOST
if /I exist "boost_%BOOST_VER%.tar.gz" (echo Skipping downloading BOOST & goto skipdownloadboost)
call :ColorText 9F "Downloading BOOST"&echo.
%MSYS_BIN%\wget http://sourceforge.net/projects/boost/files/boost/%BOOST_VER2%/boost_%BOOST_VER%.tar.gz/download -Oboost_%BOOST_VER%.tar.gz
:skipdownloadboost
if /I not exist "%BOOST_ROOT%\boost.png" (%MSYS_BIN%\tar -xf "boost_%BOOST_VER%.tar.gz" -v)

rem download libtorrent
if /I exist "libtorrent-rasterbar-%LIBTORRENT_VER%.tar.gz" (echo Skipping downloading libtorrent & goto skipdownloadlibtorrent)
call :ColorText 9F "Downloading libtorrent"&echo.
%MSYS_BIN%\wget https://github.com/arvidn/libtorrent/archive/libtorrent-%LIBTORRENT_VER%.tar.gz -Olibtorrent-rasterbar-%LIBTORRENT_VER%.tar.gz --no-check-certificate
:skipdownloadlibtorrent
if /I not exist "%LIBTORRENT_PATH%\examples\client_test.cpp" (%MSYS_BIN%\tar -xf "libtorrent-rasterbar-%LIBTORRENT_VER%.tar.gz" -v)

rem Install webp
if /I exist "%GCC_PATH%%GCC_PREFIX%\lib\libwebp.a" (echo Skipping installing WebP & goto skipprepwebp)
:installwebp
call :ColorText 9F "Installing WebP"&echo.
xcopy webp\mingw\msys\1.0 %MSYS_PATH% /E /I /Y
echo %GCC_PATH% /mingw32> %MSYS_PATH%\etc\fstab
echo %GCC64_PATH% /mingw64>> %MSYS_PATH%\etc\fstab
pushd %MSYS_PATH%
call makewebp.bat %MSYS_BIN% /mingw32%GCC_PREFIX1% /mingw64%GCC64_PREFIX1%
popd
if /I "%GCC_VERSION2%"=="51" goto skipprepwebp
xcopy "%GCC_PATH%\include\webp" "%GCC64_PATH%\include\webp"  /E /I /Y
if /I "%menu%"=="W" (echo.&pause&goto mainmenu)
:skipprepwebp

rem Add missing headers to GCC
if /I "%GCC_VERSION2%"=="51" goto skipcopyheaders
if /I not exist "%GCC_PATH%\include\wspiapi.h" (copy "gcc_patch\wspiapi.h" "%GCC_PATH%\include\wspiapi.h" /Y)
if /I not exist "%GCC_PATH%\include\shobjidl.h" (copy "gcc_patch\shobjidl.h" "%GCC_PATH%\include\shobjidl.h" /Y)
:skipcopyheaders

rem Build bjam.exe
if /I exist "%BOOST_ROOT%\bjam.exe" (echo Skipping building bjam.exe & goto skipbuildbjam)
call :ColorText 9F "Building BJAM"&echo.
pushd %BOOST_ROOT%
call bootstrap.bat mingw
popd
:skipbuildbjam

rem Install BOOST (32-bit)
if /I exist "%BOOST_INSTALL_PATH%\include\boost\version.hpp" (echo Skipping installing BOOST32 & goto skipinstallboost32)
copy "libtorrent_patch\socket_types.hpp" "%BOOST_ROOT%\boost\asio\detail\socket_types.hpp" /Y >nul
pushd %BOOST_ROOT%
call :ColorText 9F "Installing BOOST32"&echo.
rem bjam.exe install toolset=gcc release --layout=tagged -j%NUMBER_OF_PROCESSORS% --prefix=%BOOST_INSTALL_PATH%
popd
:skipinstallboost32

rem Install BOOST (64-bit)
if /I exist "%BOOST64_INSTALL_PATH%\include\boost\version.hpp" (echo Skipping installing BOOST64 & goto skipinstallboost64)
pushd %BOOST_ROOT%
set oldpath=%path%
set path=%GCC64_PATH%\bin;%BOOST_ROOT%;%MSYS_BIN%;%path%
call :ColorText 9F "Installing BOOST64"&echo.
rem bjam.exe install toolset=gcc release --layout=tagged -j%NUMBER_OF_PROCESSORS% --prefix=%BOOST64_INSTALL_PATH% address-model=64
set path=%oldpath%
popd
:skipinstallboost64

rem Rebuild libtorrent
goto skiprebuild
:installtorrent
rd /S /Q %BOOST_ROOT%\bin.v2\libs\system\build\gcc-mngw-%GCC_VERSION% 2>nul
rd /S /Q %BOOST_ROOT%\bin.v2\libs\system\build\gcc-mngw-%GCC64_PATH% 2>nul
rd /S /Q "%GCC_PATH%%GCC_PREFIX%\include\libtorrent" 2>nul
rd /S /Q "%GCC64_PATH%%GCC64_PREFIX%\include\libtorrent" 2>nul
rd /S /Q "%LIBTORRENT_PATH%\bin" 2>nul
rd /S /Q "%LIBTORRENT_PATH%\examples\bin" 2>nul
del "%GCC_PATH%%GCC_PREFIX%\lib\libtorrent.a" 2>nul
del "%GCC64_PATH%%GCC64_PREFIX%\lib\libtorrent.a" 2>nul
:skiprebuild

rem Copy libtorrent headers
if /I exist "%GCC_PATH%%GCC_PREFIX%\include\libtorrent" (echo Skipping copying headers for libtorrent & goto skipcopylibtorrentinc)
call :ColorText 9F "Copying libtorrent headers"&echo.
xcopy %LIBTORRENT_PATH%\include %GCC_PATH%%GCC_PREFIX%\include /E /I /Y > nul
xcopy %LIBTORRENT_PATH%\include %GCC64_PATH%%GCC64_PREFIX%\include /E /I /Y >nul
:skipcopylibtorrentinc

rem Build libtorrent.a (32-bit)
if /I not exist "%GCC_PATH%%GCC_PREFIX%\lib\libtorrent.a" goto buildtorrent32
if /I not exist "%GCC_PATH%%GCC_PREFIX%\lib\libboost_system_tr.a" goto buildtorrent32
echo Skipping building libtorrent[32-bit]
goto skipbuildlibtorrent
:buildtorrent32
call :ColorText 9F "Building libtorrent32"&echo.
copy "libtorrent_patch\Jamfile_fixed" "%LIBTORRENT_PATH%\examples\Jamfile" /Y
pushd "%LIBTORRENT_PATH%\examples"
bjam --abbreviate-paths client_test -j%NUMBER_OF_PROCESSORS% toolset=gcc myrelease exception-handling=on "cxxflags=-fexpensive-optimizations -fomit-frame-pointer -D IPV6_TCLASS=30"
rem bjam --abbreviate-paths client_test -j%NUMBER_OF_PROCESSORS% toolset=gcc mydebug exception-handling=on "cxxflags=-fexpensive-optimizations -fomit-frame-pointer -D IPV6_TCLASS=30"
copy ..\bin\gcc-mngw-%GCC_VERSION%\myrls\libtorrent.a %GCC_PATH%%GCC_PREFIX%\lib /Y
copy ..\bin\gcc-mngw-%GCC_VERSION%\mydbg\libtorrent.a %GCC_PATH%%GCC_PREFIX%\lib\libtorrent_dbg.a /Y
copy %BOOST_ROOT%\bin.v2\libs\system\build\gcc-mngw-%GCC_VERSION%\myrls\libboost_system-mgw%GCC_VERSION2%-mt-s-1_59.a %GCC_PATH%%GCC_PREFIX%\lib\libboost_system_tr.a /Y
popd
:skipbuildlibtorrent

rem Build libtorrent.a (64-bit)
if /I not exist "%GCC64_PATH%%GCC64_PREFIX%\lib\libtorrent.a" goto buildtorrent64
if /I not exist "%GCC64_PATH%%GCC64_PREFIX%\lib\libboost_system_tr.a" goto buildtorrent64
echo Skipping building libtorrent[64-bit]
goto skipbuildlibtorrent64
:buildtorrent64
call :ColorText 9F "Building libtorrent64"&echo.
copy "libtorrent_patch\Jamfile_fixed" "%LIBTORRENT_PATH%\examples\Jamfile" /Y
set oldpath=%path%
set path=%GCC64_PATH%\bin;%BOOST_ROOT%;%MSYS_BIN%;%path%
pushd "%LIBTORRENT_PATH%\examples"
bjam --abbreviate-paths client_test -j%NUMBER_OF_PROCESSORS% address-model=64 toolset=gcc myrelease64 exception-handling=on "cxxflags=-fexpensive-optimizations -fomit-frame-pointer -D IPV6_TCLASS=30"
rem bjam --abbreviate-paths client_test -j%NUMBER_OF_PROCESSORS% address-model=64 toolset=gcc mydebug64 exception-handling=on "cxxflags=-fexpensive-optimizations -fomit-frame-pointer -D IPV6_TCLASS=30"
copy ..\bin\gcc-mngw-%GCC64_VERSION%\myrls\adrs-mdl-64\libtorrent.a  %GCC64_PATH%%GCC64_PREFIX%\lib /Y
copy ..\bin\gcc-mngw-%GCC64_VERSION%\mydbg\adrs-mdl-64\libtorrent.a  %GCC64_PATH%%GCC64_PREFIX%\lib\libtorrent_dbg.a /Y
copy %BOOST_ROOT%\bin.v2\libs\system\build\gcc-mngw-%GCC64_VERSION%\myrls%ADR64%\libboost_system-mgw%GCC64_VERSION2%-mt-s-1_59.a %GCC64_PATH%%GCC64_PREFIX%\lib\libboost_system_tr.a /Y
set path=%oldpath%
popd
:skipbuildlibtorrent64

call :checkall
goto :eof

:fatalError
echo.
call :ColorText 7 "Press any key to continue"
echo.
pause>nul
goto :eof

:ColorText
echo off
<nul set /p ".=%DEL%" > "%~2"
findstr /v /a:%1 /R "^$" "%~2" nul
del "%~2" > nul 2>&1
goto :eof