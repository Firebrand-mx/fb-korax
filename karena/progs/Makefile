ifndef VCC
VCC=vcc
endif

ifndef OUTDIR
OUTDIR=.
endif

ifndef COMMON_PROGS_DIR
COMMON_PROGS_DIR=../../debug-sdl/basev/common/progs
endif

all: $(OUTDIR)/shared.dat $(OUTDIR)/svprogs.dat $(OUTDIR)/clprogs.dat
shared: $(OUTDIR)/shared.dat
clprogs: $(OUTDIR)/clprogs.dat
svprogs: $(OUTDIR)/svprogs.dat

del:
	rm $(OUTDIR)/shared.dat $(OUTDIR)/clprogs.dat $(OUTDIR)/svprogs.dat

$(OUTDIR)/shared.dat: *.vc $(COMMON_PROGS_DIR)/engine.dat shared/*.vc
	$(VCC) -P$(COMMON_PROGS_DIR) -P$(OUTDIR) shared.vc $@

$(OUTDIR)/svprogs.dat: game/*.vc $(OUTDIR)/shared.dat $(COMMON_PROGS_DIR)/linespec.dat $(COMMON_PROGS_DIR)/uibase.dat
	$(VCC) -P$(COMMON_PROGS_DIR) -P$(OUTDIR) svprogs.vc $@

$(OUTDIR)/clprogs.dat: cgame/*.vc $(OUTDIR)/shared.dat $(OUTDIR)/svprogs.dat $(COMMON_PROGS_DIR)/uibase.dat
	$(VCC) -P$(COMMON_PROGS_DIR) -P$(OUTDIR) clprogs.vc $@
