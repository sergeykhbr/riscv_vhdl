##
##  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
##
##  Licensed under the Apache License, Version 2.0 (the "License");
##  you may not use this file except in compliance with the License.
##  You may obtain a copy of the License at
##
##      http://www.apache.org/licenses/LICENSE-2.0
##
##  Unless required by applicable law or agreed to in writing, software
##  distributed under the License is distributed on an "AS IS" BASIS,
##  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
##  See the License for the specific language governing permissions and
##  limitations under the License.
##
include util.mak

TOP_DIR=../
OBJ_DIR = $(TOP_DIR)linuxbuild/obj
ELF_DIR = $(TOP_DIR)linuxbuild/bin

#-----------------------------------------------------------------------------
.SILENT:
  TEA = 2>&1 | tee _$@-comp.err

all: libdpiwrapper libsv_stub test 
	$(ECHO) "    All done.\n"


clean:
	$(RM) $(TOP_DIR)linuxbuild
	$(RM) *.err

libdpiwrapper:
	$(MKDIR) ./$(ELF_DIR)
	$(MKDIR) ./$(OBJ_DIR)
	$(ECHO) "    DPI wrapper Library building started:"
	make -f make_libdpiwrapper TOP_DIR=$(TOP_DIR) OBJ_DIR=$(OBJ_DIR) ELF_DIR=$(ELF_DIR) $(TEA)

libsv_stub:
	$(MKDIR) ./$(ELF_DIR)
	$(MKDIR) ./$(OBJ_DIR)
	$(ECHO) "    SV stub Library building started:"
	make -f make_libsv_stub TOP_DIR=$(TOP_DIR) OBJ_DIR=$(OBJ_DIR) ELF_DIR=$(ELF_DIR) $(TEA)

test:
	$(MKDIR) ./$(ELF_DIR)
	$(MKDIR) ./$(OBJ_DIR)
	$(ECHO) "    Test application building started:"
	make -f make_test TOP_DIR=$(TOP_DIR) OBJ_DIR=$(OBJ_DIR) ELF_DIR=$(ELF_DIR) $(TEA)
