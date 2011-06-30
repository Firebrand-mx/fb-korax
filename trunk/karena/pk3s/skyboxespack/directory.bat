@ECHO OFF
REM This batch file gets a full directory (e.g. C:\korax\) name and scans all files on it and calls runner.bat sending
REM 5 parameters, filename (without extension), file extension (separated from filename), path of the file (stripped from drive letter)
REM the directory name received as parameter and the subdirectory name for this batch file
REM (for details on what runner.bat does, please open that file...)

if %2 == skies goto :Skies else goto :Default

:Default
for /f %%a IN ('dir %1 /B /S /A-d-h-r') do call runner.bat %%~na %%~xa %%~pa %%1 %%2
goto :End

:Skies
for /d %%a IN (%CD%\skies\*) do call directory2.bat %%a %%~na
REM for /f %%a IN ('dir %CD%\skies\ /B /S /A-d-h-r') do call skies.bat %%~na %%~xa %%~pa %%a %%~na
goto :End


:End

