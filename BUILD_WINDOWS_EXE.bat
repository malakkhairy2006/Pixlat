@echo off
setlocal
cd /d "%~dp0"
set "TOOLBIN=C:\msys64\ucrt64\bin"
if exist "%TOOLBIN%\g++.exe" set "PATH=%TOOLBIN%;%PATH%"
set "CXX=%TOOLBIN%\g++.exe"
set "WINDRES=%TOOLBIN%\windres.exe"
if not exist "%CXX%" set "CXX=g++"
if not exist "%WINDRES%" set "WINDRES=windres"
if /i "%WINDRES%"=="windres" (where windres >nul 2>nul) else (if not exist "%WINDRES%" where windres >nul 2>nul)
if errorlevel 1 goto failed
"%WINDRES%" "PIXLat.rc" -O coff -o "PIXLat_resources.o"
if errorlevel 1 goto failed
"%CXX%" -std=c++17 "GUI\GUI.cpp" "PIXLat_resources.o" -o "PIXLat.exe" -lgdiplus -lcomctl32 -lcomdlg32 -lgdi32 -luser32 -mwindows
if errorlevel 1 goto failed
echo Build successful: PIXLat.exe (logo embedded).
pause
endlocal
exit /b 0
:failed
echo Build failed. Install MSYS2 UCRT64 with g++ and windres.
pause
endlocal
exit /b 1
