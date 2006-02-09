vcc -I../common -Icommon clprogs.vc clprogs.dat
vcc -I../common -Icommon server/svprogs.vc svprogs.dat
@ECHO OFF
pause
cd..
vlumpy wad0.ls
copy wad0.wad "F:\Korax' Heritage\KArena\karena\"
cd progs