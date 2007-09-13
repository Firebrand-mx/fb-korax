vcc -I../common -P../common/progs ../common/engine/classes.vc ../common/progs/engine.dat
vcc -I../common -P../common/progs ../common/linespec/classes.vc ../common/progs/linespec.dat
@ECHO OFF
cd..
cd common
vlumpy basepak.ls
copy basepak.pk3 "<TYPE YOUR PATH HERE>\KoraxRPG\basev\common\"
cd..
cd progs
@ECHO ON

vcc -I../common -P../common/progs shared.vc shared.dat
vcc -I../common -P../common/progs svprogs.vc svprogs.dat
vcc -I../common -P../common/progs clprogs.vc clprogs.dat
@ECHO OFF
pause
cd..
vlumpy basepak.ls
copy basepak.pk3 "<TYPE YOUR PATH HERE>\KoraxRPG\koraxrpg\"
cd progs
pause