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
vlumpy d:\korax\vavoom\source\basev\common\basepak.ls
move basepak.pk3 "D:\Korax\KoraxRPG\basev\common\"
cd koraxrpg
vlumpy basepak.ls
move basepak.pk3 "D:\Korax\KoraxRPG\koraxrpg\"
pause