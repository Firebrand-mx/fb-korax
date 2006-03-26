vcc -I../../progs/common -P../../basev/common/progs hexndefs.vc shared.dat
vcc -I../../progs/common -P../../basev/common/progs clprogs.vc clprogs.dat
vcc -I../../progs/common -P../../basev/common/progs svprogs.vc svprogs.dat
@ECHO OFF
pause
cd..
vlumpy wad0.ls
copy wad0.wad "F:\Korax' Heritage\KArena\karena\"
cd progs