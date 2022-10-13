/*----------------------------------------------------------------------*/
/* Setup                                                                */
/*----------------------------------------------------------------------*/

/* The OUTPUT_ARCH command specifies the machine architecture where the
   argument is one of the names used in the BFD library. More
   specifically one of the entires in bfd/cpu-mips.c */

OUTPUT_ARCH( "riscv" )

/*----------------------------------------------------------------------*/
/* Sections                                                             */
/*----------------------------------------------------------------------*/

SECTIONS
{

  /* text: test code section */
  . = 0x10000;
  .text : 
  {
    KEEP(*(.isr_vector))
    *(.text*)
  }

  /* data segment */
  .data : {
    *(.data)
    *(.dtb)
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

  /* End of uninitalized data segement */
  _end = .;
}

