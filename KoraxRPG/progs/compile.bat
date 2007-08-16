vcc -I../common -P../common ../common/engine/classes.vc ../common/engine.dat
vcc -I../common -P../common ../common/linespec/classes.vc ../common/linespec.dat
@ECHO OFF
cd..
cd common
vlumpy basepak.ls
copy basepak.pk3 "<PUT YOUR PATH HERE>\KoraxRPG\basev\common\"
cd..
cd progs
@ECHO ON

vcc -I../common -P../common shared.vc shared.dat
vcc -I../common -P../common svprogs.vc svprogs.dat
vcc -I../common -P../common clprogs.vc clprogs.dat
@ECHO OFF
pause
cd..
vlumpy basepak.ls
copy basepak.pk3 "<PUT YOUR PATH HERE>\KoraxRPG\koraxrpg\"
cd progs