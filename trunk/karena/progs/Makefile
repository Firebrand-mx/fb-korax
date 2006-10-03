ifndef VCC
VCC=vcc
endif

ifndef OUTDIR
OUTDIR=.
endif

ifndef COMMON_DIR
COMMON_DIR=../../progs/common
endif

ifndef COMMON_PROGS_DIR
COMMON_PROGS_DIR=../../basev/common/progs
endif

all: $(OUTDIR)/shared.dat $(OUTDIR)/svprogs.dat $(OUTDIR)/clprogs.dat
shared: $(OUTDIR)/shared.dat
clprogs: $(OUTDIR)/clprogs.dat
svprogs: $(OUTDIR)/svprogs.dat

del:
	rm $(OUTDIR)/shared.dat $(OUTDIR)/clprogs.dat $(OUTDIR)/svprogs.dat

$(OUTDIR)/shared.dat: *.vc $(COMMON_DIR)/*.vc $(COMMON_PROGS_DIR)/engine.dat
	$(VCC) -I$(COMMON_DIR) -P$(COMMON_PROGS_DIR) -P$(OUTDIR) hexndefs.vc $@

$(OUTDIR)/svprogs.dat: game/*.vc $(COMMON_DIR)/server/*.vc $(OUTDIR)/shared.dat
	$(VCC) -I$(COMMON_DIR) -P$(COMMON_PROGS_DIR) -P$(OUTDIR) svprogs.vc $@

$(OUTDIR)/clprogs.dat: cgame/*.vc $(OUTDIR)/shared.dat $(OUTDIR)/svprogs.dat
	$(VCC) -I$(COMMON_DIR) -P$(COMMON_PROGS_DIR) -P$(OUTDIR) clprogs.vc $@
