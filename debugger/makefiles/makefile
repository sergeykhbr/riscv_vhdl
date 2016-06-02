###
## @file
## @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
## @author     Sergey Khabarov - sergeykhbr@gmail.com
##
include util.mak

TOP_DIR=../
OBJ_DIR = $(TOP_DIR)linuxbuild/obj
ELF_DIR = $(TOP_DIR)linuxbuild/bin
PLUGINS_OBJ_DIR = $(TOP_DIR)linuxbuild/obj/plugins
PLUGINS_ELF_DIR = $(TOP_DIR)linuxbuild/bin/plugins
#LD_LIBRARY_PATH=$(TOP_DIR)linuxbuild/bin


#-----------------------------------------------------------------------------
.SILENT:
  TEA = 2>&1 | tee _$@-comp.err

all: libdbg64g simple_plugin socsim_plugin cpu_fnc_plugin appdbg64g
	$(RM) $(ELF_DIR)/config.json
	$(ECHO) "    All done.\n"

libdbg64g:
	$(ECHO) "    RISC-V debugger Shared Library building started:"
	$(MKDIR) ./$(OBJ_DIR)
	$(MKDIR) ./$(ELF_DIR)
	$(MKDIR) ./$(PLUGINS_OBJ_DIR)
	$(MKDIR) ./$(PLUGINS_ELF_DIR)
	make -f make_libdbg64g TOP_DIR=$(TOP_DIR) OBJ_DIR=$(OBJ_DIR) ELF_DIR=$(ELF_DIR) $(TEA)

simple_plugin:
	$(ECHO) "    Plugin " $@ " building started:"
	make -f make_simple_plugin TOP_DIR=$(TOP_DIR) PLUGINS_OBJ_DIR=$(PLUGINS_OBJ_DIR) PLUGINS_ELF_DIR=$(PLUGINS_ELF_DIR) $(TEA)

socsim_plugin:
	$(ECHO) "    Plugin " $@ " building started:"
	make -f make_socsim_plugin TOP_DIR=$(TOP_DIR) PLUGINS_OBJ_DIR=$(PLUGINS_OBJ_DIR) PLUGINS_ELF_DIR=$(PLUGINS_ELF_DIR) $(TEA)

cpu_fnc_plugin:
	$(ECHO) "    Plugin " $@ " building started:"
	make -f make_cpu_fnc_plugin TOP_DIR=$(TOP_DIR) PLUGINS_OBJ_DIR=$(PLUGINS_OBJ_DIR) PLUGINS_ELF_DIR=$(PLUGINS_ELF_DIR) $(TEA)

appdbg64g:
	$(ECHO) "    Debugger application building started:"
	make -f make_appdbg64g TOP_DIR=$(TOP_DIR) OBJ_DIR=$(OBJ_DIR) ELF_DIR=$(ELF_DIR) $(TEA)
