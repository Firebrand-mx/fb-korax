@ECHO OFF
cd..
cd..
vlumpy d:\korax\vavoom\source\basev\common\basepak.ls
copy "d:\korax\vavoom\source\basev\common\basepak.pk3" "D:\Korax\Vavoom\basev\common\"
cd koraxrpg
vlumpy basepak.ls
pause
copy basepak.pk3 "D:\Korax\KoraxRPG\koraxrpg\"

