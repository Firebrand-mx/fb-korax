@ECHO OFF
REM This batch file calls another batch file that handles directory names from the current directory...
REM (See directory.bat for details on what it does...)
echo $dest texpak3.pk3>%CD%\texpak3.ls
echo.

for /d %%a IN (%CD%\*) do call directory.bat %%a %%~na
