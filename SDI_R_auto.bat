rem 32-bit version of SDI works BOTH on 32-bit and 64-bit Windows.
rem 64-bit version of SDI works ONLY on 64-bit Windows.
rem EXECEPTION: 32-bit version of SDI cannot run on Windows PE x64.

rem 64-bit version is faster and doesn't have the 2GB RAM per process limitation.

@echo off
IF %PROCESSOR_ARCHITECTURE% == x86 (IF NOT DEFINED PROCESSOR_ARCHITEW6432 goto bit32)
goto bit64

:bit32
echo 32-bit
sdi_r.exe
exit

:bit64
echo 64-bit
SDI_x64_RD.exe
