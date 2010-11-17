@ECHO ON
if %5 == root goto :Root else :Print

:Print
echo models\objects\%5\%1%2		%4>>%CD%\models.ls
goto :End

:Root
echo models\objects\%1%2		%4>>%CD%\models.ls
goto :End

:End

