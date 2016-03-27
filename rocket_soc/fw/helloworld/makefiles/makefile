include util.mak

TOP_DIR=../../
OBJ_DIR = $(TOP_DIR)helloworld/makefiles/obj
ELF_DIR = $(TOP_DIR)helloworld/makefiles/bin


#-----------------------------------------------------------------------------
.SILENT:
  TEA = 2>&1 | tee _$@-comp.err

all: example
	$(ECHO) "    All done.\n"

example:
	$(ECHO) "    Example application building started:"
	$(MKDIR) ./$(OBJ_DIR)
	$(MKDIR) ./$(ELF_DIR)
	make -f make_example TOP_DIR=$(TOP_DIR) OBJ_DIR=$(OBJ_DIR) ELF_DIR=$(ELF_DIR) $@ $(TEA)
