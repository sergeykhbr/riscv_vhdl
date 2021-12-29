OUTPUT_ARCH( "riscv" )

/*----------------------------------------------------------------------*/
/* Sections                                                             */
/*----------------------------------------------------------------------*/
SECTIONS
{

  /* text: test code section */
  . = 0x08000000;
  .text : 
  {
    *(.text.entrypoint*)
    *(.text*)
  }

  /* data segment */
  .data : { *(.data) }

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
    *(.tdata)
  }
  .tbss :
  {
    *(.tbss)
  }

  /* End of uninitalized data segement */
  _end = .;

}

