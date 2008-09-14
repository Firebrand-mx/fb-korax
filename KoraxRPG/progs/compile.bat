vcc -P../common/progs ../common/engine/classes.vc ../common/progs/engine.dat
vcc -P../common/progs ../common/linespec/classes.vc ../common/progs/linespec.dat
vcc -P../common/progs ../common/uibase/classes.vc ../common/progs/uibase.dat
@ECHO OFF
cd..
cd common
vlumpy basepak.ls
copy basepak.pk3 "D:\Korax\KoraxRPG\basev\common\"
cd..
cd progs
@ECHO ON

vcc -I../common/linespec -P../common/progs linespec/classes.vc linespec.dat
vcc -P../common/progs game/classes.vc game.dat
vcc -P../common/progs cgame/classes.vc cgame.dat
@ECHO OFF
pause
cd..
vlumpy basepak.ls
copy basepak.pk3 "D:\Korax\KoraxRPG\koraxrpg\"
cd progs
pause