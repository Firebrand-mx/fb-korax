vcc -I../common -P../common ../common/common.vc ../common/common.dat
vcc -I../common -P../common ../common/clcommon.vc ../common/clcommon.dat
vcc -I../common -P../common ../common/svcommon.vc ../common/svcommon.dat
@ECHO OFF
cd..
cd common
vlumpy basepak.ls.in
copy basepak.pk3 "D:\Korax\KArena\basev\common\"
cd..
cd progs
@ECHO ON

vcc -I../common -P../common hexndefs.vc shared.dat
vcc -I../common -P../common clprogs.vc clprogs.dat
vcc -I../common -P../common svprogs.vc svprogs.dat
@ECHO OFF
pause
cd..
vlumpy basepak.ls.in
copy basepak.pk3 "D:\Korax\KArena\karena\"
cd progs