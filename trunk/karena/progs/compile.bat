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
vlumpy d:\korax\vavoom\source\basev\common\basepak.ls
move basepak.pk3 "D:\Korax\KArena\basev\common\"
vlumpy basepak2.ls
move basepak.pk3 "D:\Korax\KArena\karena\"
cd progs
pause