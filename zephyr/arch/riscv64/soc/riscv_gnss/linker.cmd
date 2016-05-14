/*
 * Copyright (c) 2014-2015 Wind River Systems, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * generate a symbol to mark the start of the device initialization objects for
 * the specified level, then link all of those objects (sorted by priority);
 * ensure the objects aren't discarded if there is no direct reference to them
 */

#define DEVICE_INIT_LEVEL(level)				\
		__device_##level##_start = .;			\
		KEEP(*(SORT(.init_##level[0-9])));		\
		KEEP(*(SORT(.init_##level[1-9][0-9])));	\

/*
 * link in device initialization objects for all devices that are automatically
 * initialized by the kernel; the objects are sorted in the order they will be
 * initialized (i.e. ordered by level, sorted by priority within a level)
 */

#define	DEVICE_INIT_SECTIONS()			\
		__device_init_start = .;		\
		DEVICE_INIT_LEVEL(PRIMARY)		\
		DEVICE_INIT_LEVEL(SECONDARY)	\
		DEVICE_INIT_LEVEL(NANOKERNEL)	\
		DEVICE_INIT_LEVEL(MICROKERNEL)	\
		DEVICE_INIT_LEVEL(APPLICATION)	\
		__device_init_end = .;


/**
 * @brief Common parts of the linker scripts for the Risc-V targets.
 */

SECTIONS
{

  /* text: test code section */
  . = 0x10000000;
  .text :
  {
    	*(.text)
		*(".text.*")
		*(.gnu.linkonce.t.*)
  }

  .devconfig :
  {
	__devconfig_start = .;
	*(".devconfig.*")
	KEEP(*(SORT_BY_NAME(".devconfig*")))
	__devconfig_end = .;
  }

  .rodata :
  {
	*(.rodata)
	*(".rodata.*")
	*(.gnu.linkonce.r.*)
  }

  /* data segment */
  .data : { *(.data) }

  .initlevel :
  {
		DEVICE_INIT_SECTIONS()
  }

  .sdata : {
    *(.srodata.cst16) *(.srodata.cst8) *(.srodata.cst4) *(.srodata.cst2) *(.srodata*)
    *(.sdata .sdata.* .gnu.linkonce.s.*)
  }

  /* bss segment */
  .sbss : {
    *(.sbss .sbss.* .gnu.linkonce.sb.*)
    *(.scommon)
  }
  .bss : { *(.bss) }

  /* thread-local data segment */
  .tdata :
  {
    _tls_data = .;
    *(.tdata)
  }
  .tbss :
  {
    *(.tbss)
  }

  /* End of uninitalized data segement */
  _end = .;
}
