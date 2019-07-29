@echo off
call "binaries/game/output/win64/debug editor/game.exe"
if errorlevel 0  (
    pause
    exit
)
pause
exit