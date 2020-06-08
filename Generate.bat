@echo off
call "Programs/Sharpmake/Sharpmake.Application.exe" /sources(@"VCLang.Solution.cs")
if NOT errorlevel 0  (
    pause
)