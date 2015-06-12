@echo off
rd /q /s logs
rd /q /s logs_matcher

bin\SDI_R199.exe -license -preservecfg -drp_dir:..\drivers -index_dir:indexes_199 -output_dir:indexes_199\txt -nosnapshot -nostamp -nogui
bin\SDI_R199.exe -license -preservecfg -drp_dir:..\drivers -index_dir:indexes_199 -output_dir:indexes_199\txt -nosnapshot -nostamp -nogui
rename logs\log.txt log_199.txt

..\SDI_R.exe -license -preservecfg -drp_dir:..\drivers -index_dir:indexes_R -output_dir:indexes_R\txt -nosnapshot -nostamp -nogui
..\SDI_R.exe -license -preservecfg -drp_dir:..\drivers -index_dir:indexes_R -output_dir:indexes_R\txt -nosnapshot -nostamp -nogui
rename logs\log.txt log_R.txt

rename logs logs_matcher
explorer logs_matcher