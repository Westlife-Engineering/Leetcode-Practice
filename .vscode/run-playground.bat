@echo off
setlocal
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 (
  echo Failed to load MSVC environment.
  exit /b 1
)

if /I "%~1"=="debug" (
  cl /nologo /EHsc /std:c++17 /Zc:__cplusplus /W3 /utf-8 /Zi /Od playground.cpp /Fe:playground.exe /Fd:playground.pdb
  exit /b %ERRORLEVEL%
)

cl /nologo /EHsc /std:c++17 /Zc:__cplusplus /W3 /utf-8 playground.cpp /Fe:playground.exe
if errorlevel 1 exit /b 1
echo.
echo ---------- running ----------
playground.exe
