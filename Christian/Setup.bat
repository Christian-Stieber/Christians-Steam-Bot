@echo off
c: || exit /b %errorlevel%
cd c:\Users\Christian\Source\Repos\Christians-Steam-Bot || exit /b %errorlevel%
if exist out/ rmdir out /s /q
call "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build/vcvarsall.bat" x86_amd64  || exit /b %errorlevel%
cmake --preset windows-x64-debug  || exit /b %errorlevel%
