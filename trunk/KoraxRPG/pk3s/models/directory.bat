@ECHO ON
REM This batch file gets a full directory (e.g. C:\korax\) name and scans all files on it and calls runner.bat sending
REM 5 parameters, filename (without extension), file extension (separated from filename), path of the file (stripped from drive letter)
REM the directory name received as parameter and the subdirectory name for this batch file
REM (for details on what runner.bat does, please open that file...)

if %2 == models goto :Models else goto :Default

:Default
for /f %%a IN ('dir %1 /B /A-d') do call runner.bat %%~na %%~xa %%~pa %%1 %%2
goto End:

:Models
REM Distinguish between directories and files, so that we can build the file properly
for /d %%a IN (%CD%\models\*) do call directory2.bat %%a %%~na %%~xa %%~pa
goto :End

:End

