@echo off
rd /q /s logs

rd /q /s indexes
..\SDI_R169.exe -license -preservecfg -drp_dir:..\drivers -nosnapshot -nostamp -verbose:64 -nogui
rename logs\log.txt log_169_1.txt

rd /q /s indexes
..\SDI_R169.exe -license -preservecfg -drp_dir:..\drivers -nosnapshot -nostamp -verbose:64 -nogui
rename logs\log.txt log_169_2.txt

rd /q /s indexes
..\SDI_R169.exe -license -preservecfg -drp_dir:..\drivers -nosnapshot -nostamp -verbose:64 -nogui
rename logs\log.txt log_169_3.txt



rd /q /s indexes
..\SDI_R.exe -license -preservecfg -drp_dir:..\drivers -nosnapshot -nostamp -verbose:320 -nogui
rename logs\log.txt log_R_1.txt

rd /q /s indexes
..\SDI_R.exe -license -preservecfg -drp_dir:..\drivers -nosnapshot -nostamp -verbose:320 -nogui
rename logs\log.txt log_R_2.txt

rd /q /s indexes
..\SDI_R.exe -license -preservecfg -drp_dir:..\drivers -nosnapshot -nostamp -verbose:320 -nogui
rename logs\log.txt log_R_3.txt

rd /q /s indexes
rd /q /s logs
