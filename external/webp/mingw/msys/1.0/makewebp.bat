set HOME=%CD%\home

set MSYSTEM=MINGW32
set PREFIX=%2
%1\sh --login -i %~dp0\home\makewebp.bat

set MSYSTEM=MINGW64
set PREFIX=%3
%1\sh --login -i %~dp0\home\makewebp.bat
