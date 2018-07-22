include makeutil.mak

TOP_DIR=../../
OBJ_DIR = $(TOP_DIR)dhrystone21/makefiles/obj
ELF_DIR = $(TOP_DIR)dhrystone21/makefiles/bin


#-----------------------------------------------------------------------------
.SILENT:
  TEA = 2>&1 | tee _$@-comp.err

all: example
	$(ECHO) "    All done.\n"

example:
	$(ECHO) "    Dhrystonev 2.1 application building started:"
	$(MKDIR) ./$(OBJ_DIR)
	$(MKDIR) ./$(ELF_DIR)
	make -f make_dhry TOP_DIR=$(TOP_DIR) OBJ_DIR=$(OBJ_DIR) ELF_DIR=$(ELF_DIR) $@ $(TEA)
