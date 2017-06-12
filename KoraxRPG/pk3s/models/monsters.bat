@ECHO OFF
if %5 == root goto :Root else :Print

:Print
echo models\monsters\%5\%1%2		%6\%4>>%CD%\models.ls
goto :End

:Root
echo models\monsters\%1%2		%6\%4>>%CD%\models.ls
goto :End

:End

