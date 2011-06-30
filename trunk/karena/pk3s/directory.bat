@ECHO OFF
REM This batch file gets a full directory (e.g. C:\korax\) name and scans all files on it and calls runner.bat sending
REM 5 parameters, filename (without extension), file extension (separated from filename), path of the file (stripped from drive letter)
REM the directory name received as parameter and the subdirectory name for this batch file
REM (for details on what runner.bat does, please open that file...)
echo $dest %2.pk3>%CD%\%2.ls
echo.

ECHO Building %2.ls...
for /f %%a IN ('dir %1 /B /A-d') do call runner.bat %%~na %%~xa %%~pa %%1 %%2