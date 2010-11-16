@ECHO OFF
REM This batch file gets a full directory (e.g. C:\korax\) name and scans all files on it and calls runner.bat sending
REM 5 parameters, filename (without extension), file extension (separated from filename), path of the file (stripped from drive letter)
REM the directory name received as parameter and the subdirectory name for this batch file
REM (for details on what runner.bat does, please open that file...)

if %2 == objects goto :Obj else goto :Default
if %2 == lights goto :Lights
if %2 == root goto :Root

:Lights
REM Rutine for building by default
for /f %%a IN ('dir %1 /B /A-d') do call lights.bat %%~na %%~xa %%~pa %%1 %%2
goto :End

:Obj
for /f %%a IN ('dir %CD%\models\objects\ /B /S /A-d-h-r') do call objects.bat %%~na %%~xa %%~pa %%a %%~na
REM 'dir %4 /B /A-d'
REM call directory3.bat %%a %%~na
goto :End

:Root
for /f %%a IN ('dir %1 /B /A-d') do call models.bat %%~na %%~xa %%~pa %%1 %%2
goto :End

:End
