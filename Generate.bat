@echo off
call "Programs/Sharpmake/Sharpmake.Application.exe" /sources(@"VCLang.Solution.cs")
if errorlevel 0  (
    exit
)
pause
exit