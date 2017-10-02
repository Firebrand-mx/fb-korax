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
cd..
cd..
ECHO Building common progs basepak.pk3...
vlumpy E:\korax\vavoom\source\basev\common\basepak.ls
ECHO Copying compiled  common basepak.pk3 file to binary directory...
move basepak.pk3 "E:\Korax\KoraxRPG\basev\common\"
cd koraxrpg
ECHO Building koraxrpg progs basepak.pk3...
vlumpy basepak.ls
ECHO Copying compiled koraxrpg basepak.pk3 file to binary directory...
move basepak.pk3 "E:\Korax\KoraxRPG\koraxrpg\"
cd progs
del cgame.dat
del cgame.txt
del game.dat
del game.txt
del linespec.dat
del linespec.txt
ECHO Done building progs.
pause
cls
ECHO Beginning procces to build resource file lists...
cd pk3s
call pk3s
ECHO Finished building resource file lists...
pause
cls
cd..
ECHO Beginning process to build resource file PK3s...
for /f %%a IN ('dir %CD%\pk3s\*.ls /B /A-d') do vlumpy %CD%\pk3s\%%a
for /f %%a IN ('dir %CD%\pk3s\*.ls /B /A-d') do del %CD%\pk3s\%%a
ECHO Copying resource files to binary directory...
for /f %%a IN ('dir %CD%\*.pk3 /B /A-d') do move %%a E:\Korax\KoraxRPG\koraxrpg
ECHO Finished, bye.
pause