@ECHO OFF
REM I don't like this, but it seems it must be done... skip .svn directory...
if %4 == .svn goto :End else goto :Print

:Print
echo skies\%5\%1%2		%4\%1%2>>%CD%\skyboxespack.ls

:End

