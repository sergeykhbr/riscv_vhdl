###
## @file
## @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
## @author     Sergey Khabarov - sergeykhbr@gmail.com
##
include util.mak

TOP_DIR=../
OBJ_DIR = $(TOP_DIR)linuxbuild/obj
ELF_DIR = $(TOP_DIR)linuxbuild/bin

#-----------------------------------------------------------------------------
.SILENT:
  TEA = 2>&1 | tee _$@-comp.err

all: libdpiwrapper
	$(ECHO) "    All done.\n"


clean:
	$(RM) $(TOP_DIR)linuxbuild
	$(RM) *.err

libdpiwrapper:
	$(MKDIR) ./$(ELF_DIR)
	$(MKDIR) ./$(OBJ_DIR)
	$(ECHO) "    RISC-V debugger Shared Library building started:"
	make -f make_libdpiwrapper TOP_DIR=$(TOP_DIR) OBJ_DIR=$(OBJ_DIR)/lib ELF_DIR=$(ELF_DIR) $(TEA)
