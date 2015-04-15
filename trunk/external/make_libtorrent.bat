@echo off
cls

set GCC_PATH=c:\MinGW
if /I exist c:\MinGW_481 (set GCC_PATH=c:\MinGW_481& goto foundgcc)
if /I exist c:\MinGW (set GCC_PATH=c:\MinGW& goto foundgcc)
:foundgcc

set GCC64_PATH=TDM-GCC-64
if /I exist c:\TDM-GCC-64_492 (set GCC64_PATH=c:\TDM-GCC-64_492& goto foundgcc64)
if /I exist c:\TDM-GCC-64 (set GCC64_PATH=c:\TDM-GCC-64_492& goto foundgcc64)
:foundgcc64

cd torrent
set BOOST_ROOT=%CD%\boost_1_57_0
set GCC_VERSION=4.8.1
set BOOST_INSTALL_PATH=C:\BOOST

set path=%GCC_PATH%\bin;%BOOST_ROOT%;%path%
set BOOST_BUILD_PATH=%BOOST_ROOT%
set LIBTORRENT_PATH=%CD%

rem Check for MinGW
if /I not exist %GCC_PATH%\bin (echo ERROR: MinGW not found in %GCC_PATH% & goto EOF)
echo GCC (32 bit): %GCC_PATH%

rem Check for MinGW64
if /I not exist %GCC64_PATH%\bin (echo ERROR: MinGW_64 not found in %GCC64_PATH% & goto EOF)
echo GCC (64 bit): %GCC64_PATH%

rem Check for libtorrent
if /I exist "%LIBTORRENT_PATH%\examples\enum_if.cpp" (echo ERROR: libtorrent was supposed be 1.0.0-RC2 & goto EOF)
if /I not exist "%LIBTORRENT_PATH%\examples\client_test.cpp" (echo ERROR: libtorrent not found in %LIBTORRENT_PATH% & goto EOF)
echo libtorrent:   %LIBTORRENT_PATH%

rem Check for BOOST
if /I not exist "%BOOST_ROOT%\boost.png" (echo ERROR: BOOST not found in %BOOST_ROOT% & goto EOF)
echo BOOST_scr:    %BOOST_ROOT%
echo BOOST_dest:   %BOOST_INSTALL_PATH%
echo.

rem Check for webp
if /I not exist ..\webp\mingw\msys\1.0\home\libwebp-0.4.3.tar.gz  (echo ERROR: libwebp-0.4.3.tar.gz not found in webp\mingw\msys\1.0\home & goto EOF)

rem Install webp
if /I exist %GCC_PATH%\msys\1.0\home\libwebp-0.4.3.tar.gz (echo Skipping installing WebP & goto skipprepwebp)
xcopy ..\webp\mingw %GCC_PATH% /E /I /Y
echo %GCC_PATH% /mingw > %GCC_PATH%\msys\1.0\etc\fstab
pushd %GCC_PATH%\msys\1.0
call %GCC_PATH%\msys\1.0\makewebp.bat
popd
xcopy %GCC_PATH%\include\webp %GCC64_PATH%\include\webp  /E /I /Y
:skipprepwebp

rem Add missing headers to GCC
if /I not exist %GCC_PATH%\include\wspiapi.h (copy ..\gcc_patch\wspiapi.h %GCC_PATH%\include\wspiapi.h /Y)
if /I not exist %GCC_PATH%\include\shobjidl.h (copy ..\gcc_patch\shobjidl.h %GCC_PATH%\include\shobjidl.h /Y)

rem Build bjam.exe
if /I exist "%BOOST_ROOT%\bjam.exe" (echo Skipping building bjam.exe & goto skipbuildbjam)
cd "%BOOST_ROOT%"
call bootstrap.bat mingw
:skipbuildbjam

rem Install BOOST
if /I exist %BOOST_INSTALL_PATH%\include\boost\version.hpp (echo Skipping installing BOOST & goto skipinstallboost)
copy ..\libtorrent_patch\socket_types.hpp "%BOOST_ROOT%\boost\asio\detail\socket_types.hpp" /Y
bjam.exe install toolset=gcc release --layout=tagged -j%NUMBER_OF_PROCESSORS%
:skipinstallboost

rem Build libtorrent.a
cd "%LIBTORRENT_PATH%"
if /I exist "%LIBTORRENT_PATH%\bin\gcc-mngw-%GCC_VERSION%\myrls\excpt-hndl-off\libtorrent.a" (echo Skipping building libtorrent & goto skipbuildlibtorrent)
cd examples
copy "%LIBTORRENT_PATH%\..\libtorrent_patch\Jamfile_fixed" "%LIBTORRENT_PATH%\examples\Jamfile" /Y
bjam --abbreviate-paths client_test -j%NUMBER_OF_PROCESSORS% toolset=gcc myrelease exception-handling=off "-sBUILD=<define>BOOST_NO_EXCEPTIONS" "-sBUILD=<define>BOOST_EXCEPTION_DISABLE" "cxxflags=-fexpensive-optimizations -fomit-frame-pointer -D IPV6_TCLASS=30"
cd ..
copy bin\gcc-mngw-%GCC_VERSION%\myrls\excpt-hndl-off\libtorrent.a %GCC_PATH%\lib /Y
:skipbuildlibtorrent

rem Copy libtorrent headers
if /I exist %GCC_PATH%\include\libtorrent (echo Skipping copying %GCC_PATH%\include\libtorrent & goto skipcopylibtorrentinc)
xcopy include %GCC_PATH%\include /E /I /Y
:skipcopylibtorrentinc
exit

echo.
echo Everything is done

:EOF
pause