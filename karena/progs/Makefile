ifndef VCC
VCC=vcc
endif

ifndef OUTDIR
OUTDIR=.
endif

ifndef COMMON_DIR
COMMON_DIR=../common
endif

all: $(OUTDIR)/clprogs.dat $(OUTDIR)/svprogs.dat
clprogs: $(OUTDIR)/clprogs.dat
svprogs: $(OUTDIR)/svprogs.dat

del:
	rm $(OUTDIR)/clprogs.dat $(OUTDIR)/svprogs.dat

$(OUTDIR)/clprogs.dat: *.vc cgame/*.vc $(COMMON_DIR)/*.vc $(COMMON_DIR)/shared/*.vc $(COMMON_DIR)/client/*.vc
	$(VCC) -I$(COMMON_DIR) clprogs.vc $@

$(OUTDIR)/svprogs.dat: *.vc game/*.vc $(COMMON_DIR)/*.vc $(COMMON_DIR)/shared/*.vc $(COMMON_DIR)/server/*.vc
	$(VCC) -I$(COMMON_DIR) svprogs.vc $@
