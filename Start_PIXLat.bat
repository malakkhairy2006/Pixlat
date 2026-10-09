@echo off
setlocal
cd /d "%~dp0"

if exist "PIXLat.exe" (
    start "" "%~dp0PIXLat.exe"
    exit /b 0
)

set "TOOLBIN=C:\msys64\ucrt64\bin"
if exist "%TOOLBIN%\g++.exe" set "PATH=%TOOLBIN%;%PATH%"
set "CXX=%TOOLBIN%\g++.exe"
set "WINDRES=%TOOLBIN%\windres.exe"
if not exist "%CXX%" set "CXX=g++"
if not exist "%WINDRES%" set "WINDRES=windres"

if /i "%CXX%"=="g++" (
    where g++ >nul 2>nul
) else (
    if exist "%CXX%" (
        rem Compiler exists at the configured path.
    ) else (
        where g++ >nul 2>nul
    )
)
if errorlevel 1 (
    echo PIXLat.exe is not included in this source package yet, and g++ was not found.
    echo Ask the project developer for the prebuilt PIXLat.exe, or install MSYS2 UCRT64.
    pause
    exit /b 1
)
if /i "%WINDRES%"=="windres" (
    where windres >nul 2>nul
) else (
    if exist "%WINDRES%" (
        rem Resource compiler exists at the configured path.
    ) else (
        where windres >nul 2>nul
    )
)
if errorlevel 1 (
    echo The Windows resource compiler windres was not found.
    echo Install the MSYS2 UCRT64 toolchain or ask for the prebuilt PIXLat.exe.
    pause
    exit /b 1
)

echo Building PIXLat.exe with the embedded logo...
"%WINDRES%" "PIXLat.rc" -O coff -o "PIXLat_resources.o"
if errorlevel 1 goto build_failed
"%CXX%" -std=c++17 "GUI\GUI.cpp" "PIXLat_resources.o" -o "PIXLat.exe" -lgdiplus -lcomctl32 -lcomdlg32 -lgdi32 -luser32 -mwindows
if errorlevel 1 goto build_failed
start "" "%~dp0PIXLat.exe"
endlocal
exit /b 0

:build_failed
echo.
echo Build failed. Check that MSYS2 UCRT64 g++ and windres are installed.
pause
endlocal
exit /b 1
