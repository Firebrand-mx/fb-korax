@ECHO OFF
ECHO Compiling progs...
@ECHO ON
vcc -P../../../basev/common/progs ../../common/engine/classes.vc ../../../basev/common/progs/engine.dat
vcc -P../../../basev/common/progs ../../common/linespec/classes.vc ../../../basev/common/progs/linespec.dat
vcc -P../../../basev/common/progs ../../common/uibase/classes.vc ../../../basev/common/progs/uibase.dat
@ECHO OFF
@ECHO ON

vcc -I../../common/linespec -P../../../basev/common/progs linespec/classes.vc linespec.dat
vcc -P../../../basev/common/progs game/classes.vc game.dat
vcc -P../../../basev/common/progs cgame/classes.vc cgame.dat
@ECHO OFF
pause
cls
cd..
cd..
cd..
ECHO Building common basepak.ls file...
cd basev
cd common
call pk3s
cd..
cd..
set d=%CD%
ECHO Building common basepak.pk3...
vlumpy %CD%\basev\common\basepak.ls
ECHO Copying compiled common basepak.pk3 file to binary directory...
move basepak.pk3 %CD%\bin\basev\common\
cd basev
cd hexen
ECHO Building hexen basepak.ls file...
call pk3s
cd..
cd..
copy %CD%\basev\games.txt %CD%\bin\basev\
ECHO Building hexen progs basepak.pk3...
vlumpy %CD%\basev\hexen\basepak.ls
ECHO Copying compiled hexen basepak.pk3 file to binary directory...
move basepak.pk3 %CD%\bin\basev\hexen\
ECHO Building karena progs basepak.pk3...
cd code
cd karena
vlumpy %CD%\basepak.ls
cd..
cd..
ECHO Copying compiled karena basepak.pk3 file to binary directory...
move %CD%\code\karena\basepak.pk3 %CD%\bin\karena\
ECHO Done building progs.
pause
cls
ECHO Beginning procces to build resource file lists...
cd code
cd karena
cd pk3s
call pk3s
ECHO Finished building resource file lists...
pause
cls
cd..
ECHO Beginning process to build resource file PK3s...
for /f %%a IN ('dir %CD%\pk3s\*.ls /B /A-d') do vlumpy %CD%\pk3s\%%a
ECHO Copying resource files to binary directory...
cd..
cd..
for /f %%a IN ('dir %CD%\code\karena\*.pk3 /B /A-d') do move %CD%\code\karena\%%a %CD%\bin\karena\
ECHO Copying base.txt to binary directory...
copy %CD%\code\karena\base.txt %CD%\bin\karena\
ECHO Finished, bye.
pause