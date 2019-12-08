include makeutil.mak

TOP_DIR=../../
OBJ_DIR = $(TOP_DIR)makefiles/obj
ELF_DIR = $(TOP_DIR)makefiles/bin


#-----------------------------------------------------------------------------
.SILENT:
  TEA = 2>&1 | tee _$@-comp.err

all: riscv
	$(ECHO) "    All done.\n"

riscv:
	$(ECHO) "    Dhrystonev 2.1 RISC-V application building started:"
	$(MKDIR) ./$(OBJ_DIR)
	$(MKDIR) ./$(ELF_DIR)
	make -f make_riscv TOP_DIR=$(TOP_DIR) OBJ_DIR=$(OBJ_DIR) ELF_DIR=$(ELF_DIR) $@ $(TEA)

arm:
	$(ECHO) "    Dhrystonev 2.1 Cortex-R5 application building started:"
	$(MKDIR) ./$(OBJ_DIR)
	$(MKDIR) ./$(ELF_DIR)
	make -f make_arm TOP_DIR=$(TOP_DIR) OBJ_DIR=$(OBJ_DIR) ELF_DIR=$(ELF_DIR) $@ $(TEA)
