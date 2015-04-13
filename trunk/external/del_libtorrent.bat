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

rd /S /Q %GCC_PATH%\include\libtorrent
rd /S /Q %GCC_PATH%\include\webp
rd /S /Q %GCC64_PATH%\include\webp

del /F /S /Q %GCC_PATH%\lib\libtorrent.a
del /F /S /Q %GCC_PATH%\lib\libweb*.*

pause