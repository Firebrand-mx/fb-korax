vcc -I../common -P../common ../common/engine/classes.vc ../common/engine.dat
vcc -I../common -P../common ../common/linespec/classes.vc ../common/linespec.dat
vcc -I../common -P../common ../common/server/classes.vc ../common/svcommon.dat
@ECHO OFF
cd..
cd common
vlumpy basepak.ls.in
copy basepak.pk3 "D:\Korax\KArena\basev\common\"
cd..
cd progs
@ECHO ON

vcc -I../common -P../common hexndefs.vc shared.dat
vcc -I../common -P../common svprogs.vc svprogs.dat
vcc -I../common -P../common clprogs.vc clprogs.dat
@ECHO OFF
pause
cd..
vlumpy basepak.ls
copy basepak.pk3 "D:\Korax\KArena\karena\"
cd progs