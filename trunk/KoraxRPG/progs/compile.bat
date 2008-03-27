vcc -I../common/progs -P../common/progs ../common/engine/classes.vc ../common/progs/engine.dat
vcc -I../common/progs -P../common/progs ../common/linespec/classes.vc ../common/progs/linespec.dat
vcc -I../common/progs -P../common/progs ../common/uibase/classes.vc ../common/progs/uibase.dat
@ECHO OFF
cd..
cd common
vlumpy basepak.ls
copy basepak.pk3 "D:\Korax\KoraxRPG\basev\common\"
cd..
cd progs
@ECHO ON

vcc -I../common -P../common/progs game/classes.vc svprogs.dat
vcc -I../common -P../common/progs cgame/classes.vc clprogs.dat
@ECHO OFF
pause
cd..
vlumpy basepak.ls
copy basepak.pk3 "D:\Korax\KoraxRPG\koraxrpg\"
cd progs
pause