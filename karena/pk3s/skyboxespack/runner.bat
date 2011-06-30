@ECHO OFF
REM This batch file receives 5 parameters, a filename, a file extension, the path of the file stripped from drive letter
REM and a full directory name, it appends these parameters in a special sequence to generate a .ls file for a single directory
REM directories are handled by pk3s.bat and directory.bat (if you want to know more open those files...)

REM Now, depending on the type of files we are using, we need to use different directories like this...
if %5 == root goto :Root
if %5 == skies goto :End

:Root
echo %1%2          %4\%1%2>>%CD%\skyboxespack.ls
goto :End

REM echo skies\%1%2          %4\%1%2>>%CD%\skyboxespack.ls
REM goto :End

:End
