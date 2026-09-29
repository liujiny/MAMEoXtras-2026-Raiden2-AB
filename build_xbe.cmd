@echo off
setlocal
cd /d "%~dp0"
if defined MAMEOX_PYTHON goto run
if exist "C:\Program Files\LibreOffice\program\python.exe" set "MAMEOX_PYTHON=C:\Program Files\LibreOffice\program\python.exe"
if defined MAMEOX_PYTHON goto run
if exist "C:\Program Files (x86)\Microsoft SDKs\Azure\CLI2\python.exe" set "MAMEOX_PYTHON=C:\Program Files (x86)\Microsoft SDKs\Azure\CLI2\python.exe"
if defined MAMEOX_PYTHON goto run
echo Python 3 was not found. Set MAMEOX_PYTHON to the full path of python.exe. 1>&2
exit /b 2
:run
"%MAMEOX_PYTHON%" "%~dp0build_xbe.py" %*
exit /b %ERRORLEVEL%
