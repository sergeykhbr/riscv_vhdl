/*
 *  Copyright 2023 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

OUTPUT_ARCH( "riscv" )

ENTRY(__VECTOR_TABLE)

HEAP_SIZE  = DEFINED(__heap_size__)  ? __heap_size__  : 0x00003000;
STACK_SIZE = DEFINED(__stack_size__) ? __stack_size__ : 0x00000400;

/* Specify the memory areas */
MEMORY
{
  BOOTROM               (RX)  : ORIGIN = 0x0000000010000, LENGTH = 0x0000000010000
  SRAM                  (RW)  : ORIGIN = 0x0000008000000, LENGTH = 0x0000000200000
}

REGION_ALIAS("REGION_TEXT", BOOTROM);
REGION_ALIAS("REGION_DATA", SRAM);
REGION_ALIAS("REGION_BSS", SRAM);


/*----------------------------------------------------------------------*/
/* Sections                                                             */
/*----------------------------------------------------------------------*/
SECTIONS
{
  /* text: test code section */
  .text :
  {
    __VECTOR_TABLE = .;
    . = ALIGN(8);
    KEEP(*(.isr_vector))

    . = ALIGN(8);
    KEEP(*(.hard_reset))
    KEEP(*(.after_vectors .after_vectors.*))   /* Startup code */
    *(.text)                 /* .text sections (code) */
    *(.text*)                /* .text* sections (code) */
    *(.init)                 /* section used in crti.o files */
    *(.fini)                 /* section used in crti.o files */
    . = ALIGN(8);

    /*
    * The preinit code, i.e. an array of pointers to initialisation 
    * functions to be performed before constructors.
    */
    PROVIDE_HIDDEN (__preinit_array_start = .);
        
    /** Used to run the SystemInit() before anything else. */
    KEEP(*(.preinit_array_sysinit .preinit_array_sysinit.*))
        
    /** Used for other platform inits. */
    KEEP(*(.preinit_array_platform .preinit_array_platform.*))
        
    /*
    * The application inits. If you need to enforce some order in 
    * execution, create new sections, as before.
    */
    KEEP(*(.preinit_array .preinit_array.*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
    . = ALIGN(8);

    /** The init code, i.e. an array of pointers to static constructors. */
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP(*(SORT(.init_array.*)))
    KEEP(*(.init_array))
    PROVIDE_HIDDEN (__init_array_end = .);
    . = ALIGN(8);

    /** The fini code, i.e. an array of pointers to static destructors. */
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP(*(SORT(.fini_array.*)))
    KEEP(*(.fini_array))
    PROVIDE_HIDDEN (__fini_array_end = .);
    . = ALIGN(8);

    *(vtable)                /* C++ virtual tables */
    *(.eh_frame)             /* section used in crtbegin.o files */
    . = ALIGN(8);

    *(.rodata)               /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)              /* .rodata* sections (constants, strings, etc.) */
    *(.srodata.cst16) *(.srodata.cst8) *(.srodata.cst4) *(.srodata.cst2) *(.srodata*)
    . = ALIGN(8);
     __text_end = .;   	
  } > REGION_TEXT


  . = ALIGN(8);
  .data : AT(__text_end)
  {
    __DATA_START = .;
    *(.got)      /* Global Offset Table is used for relocation of ELF symbols */
    *(.got*)
    *(.data)
    *(.data*)
    *(.tdata)
    *(.sdata .sdata.* .gnu.linkonce.s.*)
    *(.dtb)
  } > REGION_DATA
  __DATA_SIZE = SIZEOF(.data);
  __DATA_LOAD_START = LOADADDR(.data);


  /* Uninitialized data section. */
  .bss :
  {
    . = ALIGN(8);
    __bss_start__ = .;
    *(.sbss .sbss.* .gnu.linkonce.sb.*)
    *(.tbss)
    *(.scommon)
    *(COMMON)
    . = ALIGN(8);
    __bss_end__ = .;
  } > REGION_BSS


   /* Put heap section after the program data */
  .heap :
  {
    . = ALIGN(8);
    PROVIDE(__heap_start__ = .);
    . += HEAP_SIZE;
    PROVIDE(__heap_end__ = .);
    __stack_limit = ORIGIN(REGION_DATA) + LENGTH(REGION_DATA) - HEAP_SIZE;
  } > REGION_DATA

  __stack_base = ORIGIN(REGION_DATA) + LENGTH(REGION_DATA);
}

