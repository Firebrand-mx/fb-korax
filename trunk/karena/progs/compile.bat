vcc -P../../../basev/common/progs ../../common/engine/classes.vc ../../../basev/common/progs/engine.dat
vcc -P../../../basev/common/progs ../../common/linespec/classes.vc ../../../basev/common/progs/linespec.dat
vcc -P../../../basev/common/progs ../../common/uibase/classes.vc ../../../basev/common/progs/uibase.dat
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
cd karena
cd progs
@ECHO ON

vcc -I../../common/linespec -P../../../basev/common/progs linespec/classes.vc linespec.dat
vcc -P../../../basev/common/progs game/classes.vc svprogs.dat
vcc -P../../../basev/common/progs cgame/classes.vc clprogs.dat
@ECHO OFF
pause
cd..
vlumpy basepak.ls
copy basepak.pk3 "D:\Korax\KArena\karena\"
cd progs
