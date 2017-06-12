@ECHO OFF

:root
echo models\artifact\%1%2          %4\%1%2>>%CD%\models.ls
goto :End

:End
