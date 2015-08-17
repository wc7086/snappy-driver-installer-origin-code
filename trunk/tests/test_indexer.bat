@echo off
rd /q /s logs

rd /q /s indexes_317
bin\SDI_R317.exe -license -preservecfg -drp_dir:..\drivers -index_dir:indexes_317 -output_dir:indexes_317\txt -index_hr -nosnapshot -nostamp -nogui -nologfile

rd /q /s indexes_R
..\SDI_R.exe    -license -preservecfg -drp_dir:..\drivers -index_dir:indexes_R   -output_dir:indexes_R\txt   -index_hr -nosnapshot -nostamp -nogui -nologfile

rd /q /s logs
