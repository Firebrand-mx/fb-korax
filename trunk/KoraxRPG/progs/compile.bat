@ECHO OFF
ECHO Compiling progs...
@ECHO ON
vcc -P../../../basev/common/progs ../../../progs/common/engine/classes.vc ../../../basev/common/progs/engine.dat
vcc -P../../../basev/common/progs ../../../progs/common/linespec/classes.vc ../../../basev/common/progs/linespec.dat
vcc -P../../../basev/common/progs ../../../progs/common/uibase/classes.vc ../../../basev/common/progs/uibase.dat
vcc -I../../../progs/common/linespec -P../../../basev/common/progs linespec/classes.vc linespec.dat
vcc -P../../../basev/common/progs game/classes.vc game.dat
vcc -P../../../basev/common/progs cgame/classes.vc cgame.dat
@ECHO OFF