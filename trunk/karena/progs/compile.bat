c:
cd \vavoom_src\progs\korax_arena
..\..\utils\bin\vcc -I../common -Icommon client/clprogs.vc clprogs.dat
..\..\utils\bin\vcc -I../common -Icommon server/svprogs.vc svprogs.dat
copy *.dat C:\va_115_2\basev\karena\progs
cd \va_115_2\basev\karena
vlumpy wad0.ls
copy wad0.wad c:\vavoom\basev\karena
copy wad0.wad c:\vavoom\vavoom\basev\karena
cd \vavoom_src\progs\korax_arena