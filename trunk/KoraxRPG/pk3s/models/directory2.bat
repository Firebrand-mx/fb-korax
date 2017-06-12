@ECHO OFF
REM This batch file gets a full directory (e.g. C:\korax\) name and scans all files on it and calls runner.bat sending
REM 5 parameters, filename (without extension), file extension (separated from filename), path of the file (stripped from drive letter)
REM the directory name received as parameter and the subdirectory name for this batch file
REM (for details on what runner.bat does, please open that file...)

if %2 == artifact goto :Arti
if %2 == items goto :Items
if %2 == keys goto :Keys
if %2 == lights goto :Lights
if %2 == mana goto :Mana
if %2 == monsters goto :Monsters
if %2 == objects goto :Obj
if %2 == puzzle goto :Puzzle
if %2 == root goto :Root
if %2 == weapons goto :Weapons

:Arti
REM Rutine for building by default
for /f %%a IN ('dir %1 /B /A-d') do call artifacts.bat %%~na %%~xa %%~pa %%1 %%2
goto :End

:Items
REM Rutine for building by default
for /f %%a IN ('dir %1 /B /A-d') do call items.bat %%~na %%~xa %%~pa %%1 %%2
goto :End

:Keys
REM Rutine for building by default
for /f %%a IN ('dir %1 /B /A-d') do call keys.bat %%~na %%~xa %%~pa %%1 %%2
goto :End

:Lights
REM Rutine for building by default
for /f %%a IN ('dir %1 /B /A-d') do call lights.bat %%~na %%~xa %%~pa %%1 %%2
goto :End

:Mana
REM Rutine for building by default
for /f %%a IN ('dir %1 /B /A-d') do call mana.bat %%~na %%~xa %%~pa %%1 %%2
goto :End

:Monsters
REM Rutine for building by default
for /d %%a IN (%CD%\models\monsters\*) do for /f %%b IN ('dir %%a /B /A-d-h-r') do call monsters.bat %%~nb %%~xb %%~pb %%b %%~na %%a
goto :End

:Obj
for /d %%a IN (%CD%\models\objects\*) do for /f %%b IN ('dir %%a /B /A-d-h-r') do call objects.bat %%~nb %%~xb %%~pb %%b %%~na %%a
REM 'dir %4 /B /A-d'
REM call directory3.bat %%a %%~na
goto :End

:Puzzle
REM Rutine for building by default
for /f %%a IN ('dir %1 /B /A-d') do call puzzle.bat %%~na %%~xa %%~pa %%1 %%2
goto :End

:Root
for /f %%a IN ('dir %1 /B /A-d') do call models.bat %%~na %%~xa %%~pa %%1 %%2
goto :End

:Weapons
REM Rutine for building by default
for /d %%a IN (%CD%\models\weapons\*) do for /f %%b IN ('dir %%a /B /A-d-h-r') do call weapons.bat %%~nb %%~xb %%~pb %%b %%~na %%a
goto :End

:End
