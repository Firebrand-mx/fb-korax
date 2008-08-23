vcc -P../../common/ ../../common/engine/classes.vc ../../common/engine.dat
vcc -P../../common/ ../../common/linespec/classes.vc ../../common/linespec.dat
vcc -P../../common/ ../../common/uibase/classes.vc ../../common/uibase.dat
@ECHO OFF
cd..
cd..
cd..
cd basev
cd common
vlumpy basepak.ls
copy basepak.pk3 "D:\Korax\KArena\basev\common\"
cd..
cd..
cd progs
cd ka_source
cd progs
@ECHO ON

vcc -I../../common/linespec -P../../common/progs linespec/classes.vc linespec.dat
vcc -P../../common/ game/classes.vc svprogs.dat
vcc -P../../common/ cgame/classes.vc clprogs.dat
@ECHO OFF
pause
cd..
vlumpy basepak.ls
copy basepak.pk3 "D:\Korax\KArena\karena\"
cd progs
