###
## @file
## @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
## @author     Sergey Khabarov - sergeykhbr@gmail.com
##
include util.mak

## Enable CentOS 6 pathes to build Qt plugin
# source /opt/rh/devtools-6/enable
CENTOS6=0

TOP_DIR=../
OBJ_DIR = $(TOP_DIR)linuxbuild/obj
ELF_DIR = $(TOP_DIR)linuxbuild/bin
PLUGINS_OBJ_DIR = $(TOP_DIR)linuxbuild/obj/plugins
PLUGINS_ELF_DIR = $(TOP_DIR)linuxbuild/bin/plugins

ifeq ($(QT_PATH), )
$(warning WARNING: Environment variable QT_PATH not defined.)
$(info "    GUI build is not possible.")
$(info "    1. Download Qt-package and install it (checked with version 5.7).")
$(info "    2. export QT_PATH=/home/install_dir/Qt5.7.0/5.7/gcc_64")
endif

ifeq ($(SYSTEMC_PATH), )
$(warning WARNING: Environment variable SYSTEMC_PATH not defined.)
$(info "    Build SystemC model of RIVER CPU not possible. Do the following steps:")
$(info "    1. Download systemc:")
$(info "        http://accellera.org/downloads/standards/systemc")
$(info "    2. Unpack and build sources")
$(info "        tar -xvzf systemc-2.3.1a.tar.gz")
$(info "        cd systemc-2.3.1a")
$(info "        mkdir tmp")
$(info "        cd tmp")
$(info "        ./../configure --prefix=/home/user/systemc-2.3.1a/build")
$(info "       make")
$(info "       make install")
$(info "    3. export SYSTEMC_PATH=/home/user/systemc-2.3.1a/build")
endif
ifeq ($(SYSTEMC_SRC), )
   SYSTEMC_SRC = $(SYSTEMC_PATH)/include
endif
ifeq ($(SYSTEMC_LIB), )
    SYSTEMC_LIB = $(SYSTEMC_PATH)/lib-linux64
endif


#-----------------------------------------------------------------------------
.SILENT:
  TEA = 2>&1 | tee _$@-comp.err

all: base gui_plugin appdbg64g
	$(RM) $(ELF_DIR)/config.json
	$(ECHO) "    All done.\n"

base: libdbg64g simple_plugin socsim_plugin cpu_fnc_plugin cpu_arm_plugin

gui: libdbg64g gui_plugin

sc: libdbg64g cpu_sysc_plugin

clean:
	$(RM) $(TOP_DIR)linuxbuild
	$(RM) *.err

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

cpu_sysc_plugin:
	$(MKDIR) ./$(PLUGINS_OBJ_DIR)/cpu_sysc
	$(ECHO) "    Plugin " $@ " building started:"
	make -f make_cpu_sysc_plugin TOP_DIR=$(TOP_DIR) PLUGINS_OBJ_DIR=$(PLUGINS_OBJ_DIR)/cpu_sysc PLUGINS_ELF_DIR=$(PLUGINS_ELF_DIR) SYSTEMC_SRC=$(SYSTEMC_SRC) SYSTEMC_LIB=$(SYSTEMC_LIB) $(TEA)

cpu_arm_plugin:
	$(MKDIR) ./$(PLUGINS_OBJ_DIR)/cpu_arm
	$(ECHO) "    Plugin " $@ " building started:"
	make -f make_cpu_arm_plugin TOP_DIR=$(TOP_DIR) PLUGINS_OBJ_DIR=$(PLUGINS_OBJ_DIR)/cpu_arm PLUGINS_ELF_DIR=$(PLUGINS_ELF_DIR) $(TEA)

gui_plugin:
	$(MKDIR) ./$(PLUGINS_OBJ_DIR)/gui
	$(ECHO) "    Plugin " $@ " building started:"
	make -f make_gui_plugin TOP_DIR=$(TOP_DIR) PLUGINS_OBJ_DIR=$(PLUGINS_OBJ_DIR)/gui PLUGINS_ELF_DIR=$(PLUGINS_ELF_DIR) ELF_DIR=$(ELF_DIR) CENTOS6=$(CENTOS6) $(TEA)

appdbg64g:
	$(ECHO) "    Debugger application building started:"
	make -f make_appdbg64g TOP_DIR=$(TOP_DIR) OBJ_DIR=$(OBJ_DIR)/app ELF_DIR=$(ELF_DIR) CENTOS6=$(CENTOS6) $(TEA)
