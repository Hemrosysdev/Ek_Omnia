echo off

set /p port_id="Bitte die COM Port Nummer eingeben: (0=cancel) "

if [%port_id%]==[] goto ende
if [%port_id%]==[0] goto ende

.\McuSimulator.exe -d \\.\COM%port_id% -D 2
exit

:ende
echo Anwendung beendet ...
pause
