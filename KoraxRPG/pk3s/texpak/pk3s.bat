@ECHO OFF
REM This batch file calls another batch file that handles directory names from the current directory...
REM (See directory.bat for details on what it does...)
echo $dest texpak.pk3>%CD%\texpak.ls
echo.

for /d %%a IN (%CD%\*) do call directory.bat %%a %%~na
