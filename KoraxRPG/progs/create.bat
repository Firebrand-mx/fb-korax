@ECHO OFF
cd..
cd..
vlumpy d:\korax\vavoom\source\basev\common\basepak.ls
copy "d:\korax\vavoom\basev\common\basepak.pk3" "D:\Korax\KoraxRPG\basev\common\"
cd koraxrpg
vlumpy basepak.ls
move basepak.pk3 "D:\Korax\KoraxRPG\koraxrpg\"
pause

