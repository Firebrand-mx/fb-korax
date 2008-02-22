vcc -P../common/progs/ ../common/engine/classes.vc ../common/progs/engine.dat
vcc -P../common/progs/ ../common/linespec/classes.vc ../common/progs/linespec.dat
vcc -P../common/progs/ ../common/uibase/classes.vc ../common/progs/uibase.dat
@ECHO OFF
cd..
cd common
vlumpy basepak.ls
copy basepak.pk3 "Z:\KA_DIRECTORY\basev\common\"
cd..
cd progs
@ECHO ON

vcc -I../common/progs/ -P../common/progs/ shared.vc shared.dat
vcc -I../common/progs/ -P../common/progs/ svprogs.vc svprogs.dat
vcc -I../common/progs/ -P../common/progs/ clprogs.vc clprogs.dat
@ECHO OFF
pause
cd..
vlumpy basepak.ls
copy basepak.pk3 "Z:\KA_DIRECTORY\karena\"
cd progs
