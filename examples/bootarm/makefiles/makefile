include util.mak

TOP_DIR=../
OBJ_DIR = $(TOP_DIR)makefiles/obj
ELF_DIR = $(TOP_DIR)makefiles/bin


#-----------------------------------------------------------------------------
.SILENT:
  TEA = 2>&1 | tee _$@-comp.err

all: boot
	$(ECHO) "    All done.\n"

boot:
	$(ECHO) "    Boot rom image building started:"
	$(MKDIR) ./$(OBJ_DIR)
	$(MKDIR) ./$(ELF_DIR)
	make -f make_boot TOP_DIR=$(TOP_DIR) OBJ_DIR=$(OBJ_DIR) ELF_DIR=$(ELF_DIR) $@ $(TEA)
