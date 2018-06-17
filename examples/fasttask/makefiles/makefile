include util.mak

TOP_DIR=../../
OBJ_DIR = $(TOP_DIR)fasttask/makefiles/obj
ELF_DIR = $(TOP_DIR)fasttask/makefiles/bin
#LD_LIBRARY_PATH=$(TOP_DIR)linuxbuild/bin


#-----------------------------------------------------------------------------
.SILENT:
  TEA = 2>&1 | tee _$@-comp.err

all: fw
	$(ECHO) "    All done.\n"

fw:
	$(ECHO) "    Test application building started:"
	$(MKDIR) ./$(OBJ_DIR)
	$(MKDIR) ./$(ELF_DIR)
	make -f make_gnssfw TOP_DIR=$(TOP_DIR) OBJ_DIR=$(OBJ_DIR) ELF_DIR=$(ELF_DIR) $@ $(TEA)
