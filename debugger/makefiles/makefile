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
#export QT_PATH=../../../../qt/Qt5.7.0/5.7/gcc_64

#-----------------------------------------------------------------------------
.SILENT:
  TEA = 2>&1 | tee _$@-comp.err

all: cleanup base gui_plugin
	$(RM) $(ELF_DIR)/config.json
	$(ECHO) "    All done.\n"

base: libdbg64g simple_plugin socsim_plugin cpu_fnc_plugin appdbg64g

gui: libdbg64g gui_plugin

cleanup:
	$(RM) $(TOP_DIR)linuxbuild

libdbg64g:
	$(MKDIR) ./$(ELF_DIR)
	$(MKDIR) ./$(PLUGINS_ELF_DIR)
	$(MKDIR) ./$(OBJ_DIR)
	$(MKDIR) ./$(OBJ_DIR)/lib
	$(MKDIR) ./$(OBJ_DIR)/app
	$(MKDIR) ./$(PLUGINS_OBJ_DIR)
	$(ECHO) "    RISC-V debugger Shared Library building started:"
	make -f make_libdbg64g TOP_DIR=$(TOP_DIR) OBJ_DIR=$(OBJ_DIR)/lib ELF_DIR=$(ELF_DIR) $(TEA)

simple_plugin:
	$(MKDIR) ./$(PLUGINS_OBJ_DIR)/simple
	$(ECHO) "    Plugin " $@ " building started:"
	make -f make_simple_plugin TOP_DIR=$(TOP_DIR) PLUGINS_OBJ_DIR=$(PLUGINS_OBJ_DIR)/simple PLUGINS_ELF_DIR=$(PLUGINS_ELF_DIR) $(TEA)

socsim_plugin:
	$(MKDIR) ./$(PLUGINS_OBJ_DIR)/socsim
	$(ECHO) "    Plugin " $@ " building started:"
	make -f make_socsim_plugin TOP_DIR=$(TOP_DIR) PLUGINS_OBJ_DIR=$(PLUGINS_OBJ_DIR)/socsim PLUGINS_ELF_DIR=$(PLUGINS_ELF_DIR) $(TEA)

cpu_fnc_plugin:
	$(MKDIR) ./$(PLUGINS_OBJ_DIR)/cpu_fnc
	$(ECHO) "    Plugin " $@ " building started:"
	make -f make_cpu_fnc_plugin TOP_DIR=$(TOP_DIR) PLUGINS_OBJ_DIR=$(PLUGINS_OBJ_DIR)/cpu_fnc PLUGINS_ELF_DIR=$(PLUGINS_ELF_DIR) $(TEA)

gui_plugin:
	$(MKDIR) ./$(PLUGINS_OBJ_DIR)/gui
	$(ECHO) "    Plugin " $@ " building started:"
	make -f make_gui_plugin TOP_DIR=$(TOP_DIR) PLUGINS_OBJ_DIR=$(PLUGINS_OBJ_DIR)/gui PLUGINS_ELF_DIR=$(PLUGINS_ELF_DIR) ELF_DIR=$(ELF_DIR) $(TEA)

appdbg64g:
	$(ECHO) "    Debugger application building started:"
	make -f make_appdbg64g TOP_DIR=$(TOP_DIR) OBJ_DIR=$(OBJ_DIR)/app ELF_DIR=$(ELF_DIR) $(TEA)
