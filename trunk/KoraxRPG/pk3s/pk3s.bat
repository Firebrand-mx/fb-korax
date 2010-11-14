@ECHO OFF
REM This batch file calls another batch file that handles directory names from the current directory...
REM (See directory.bat for details on what it does...)
for /d %%a IN (%CD%\*) do call directory.bat %%a %%~na

@ECHO ON
REM We have finished building most of the resource files, the remaining ones (the ones that require a special handling
REM have to be built after we finish with the first ones...

REM BUILD SOUNDS PACK
cd sounds
call pk3s.bat
@ECHO ON
cd..
del sounds.ls
move %CD%\sounds\sounds.ls %CD%
pause
@ECHO OFF

REM BUILD MODEL PACK
cd models
call pk3s.bat
@ECHO ON
cd..
del models.ls
move %CD%\models\models.ls %CD%

REM BUILD SKYBOX PACK
cd skyboxespack
call pk3s.bat
@ECHO ON
cd..
del skyboxespack.ls
move %CD%\skyboxespack\skyboxespack.ls %CD%
