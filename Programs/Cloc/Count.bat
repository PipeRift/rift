@echo off
cd ../../Source
call "../programs/Cloc/cloc" --exclude-dir=ThirdParty .
pause