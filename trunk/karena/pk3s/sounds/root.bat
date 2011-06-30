@ECHO OFF
REM This batch file receives 5 parameters, a filename, a file extension, the path of the file stripped from drive letter
REM and a full directory name, it appends these parameters in a special sequence to generate a .ls file for a single directory
REM directories are handled by pk3s.bat and directory.bat (if you want to know more open those files...)

echo sounds\%1%2          %4\%1%2>>%CD%\sounds.ls

goto :End

:End
