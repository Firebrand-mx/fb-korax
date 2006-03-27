vcc -I../common -P../common hexndefs.vc shared.dat
vcc -I../common -P../common clprogs.vc clprogs.dat
vcc -I../common -P../common svprogs.vc svprogs.dat
@ECHO OFF
pause
cd..
vlumpy wad0.ls
copy wad0.wad "D:\Korax\KArena\karena\"
cd progs