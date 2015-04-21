@echo off
rd /q /s logs

..\SDI_R169.exe -license -preservecfg -drp_dir:..\drivers -index_dir:indexes_169 -output_dir:indexes_169\txt -nosnapshot -nostamp -nogui
rename logs\log.txt log_169.txt

..\SDI_R233.exe -license -preservecfg -drp_dir:..\drivers -index_dir:indexes_233 -output_dir:indexes_233\txt -nosnapshot -nostamp -nogui
rename logs\log.txt log_233.txt

..\SDI_R.exe    -license -preservecfg -drp_dir:..\drivers -index_dir:indexes_R   -output_dir:indexes_R\txt   -nosnapshot -nostamp -nogui
rename logs\log.txt log_R.txt

explorer logs