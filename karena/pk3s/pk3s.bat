@ECHO OFF
REM This batch file calls another batch file that handles directory names from the current directory...
REM (See directory.bat for details on what it does...)
for /d %%a IN (%CD%\*) do call directory.bat %%a %%~na

REM We have finished building most of the resource files, the remaining ones (the ones that require a special handling
REM have to be built after we finish with the first ones...

ECHO Building sounds,ls...
cd sounds
call pk3s.bat
cd..
del sounds.ls
move %CD%\sounds\sounds.ls %CD%

ECHO Building models.ls...
cd models
call pk3s.bat
cd..
del models.ls
move %CD%\models\models.ls %CD%

ECHO Building skyboxespack.ls...
cd skyboxespack
call pk3s.bat
cd..
del skyboxespack.ls
move %CD%\skyboxespack\skyboxespack.ls %CD%
