@echo off
rd /q /s logs

rd /q /s indexes_169
..\SDI_R169.exe -license -preservecfg -drp_dir:..\drivers -index_dir:indexes_169 -output_dir:indexes_169\txt -index_hr -nosnapshot -nostamp -nogui -nologfile

rd /q /s indexes_233
..\SDI_R233.exe -license -preservecfg -drp_dir:..\drivers -index_dir:indexes_233 -output_dir:indexes_233\txt -index_hr -nosnapshot -nostamp -nogui -nologfile

rd /q /s indexes_R
..\SDI_R.exe    -license -preservecfg -drp_dir:..\drivers -index_dir:indexes_R   -output_dir:indexes_R\txt   -index_hr -nosnapshot -nostamp -nogui -nologfile

rd /q /s logs
