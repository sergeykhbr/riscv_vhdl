OUTPUT_ARCH( "arm" )

/*----------------------------------------------------------------------*/
/* Sections                                                             */
/*----------------------------------------------------------------------*/

SECTIONS
{

  /* text: test code section */
  . = 0x0000;
  .text : 
  {
    obj/crt.o(.text)
  }

  /* data segment */
  .data : { *(.data) }


  /* bss segment */
  __bss_start__ = .;
  .bss : { *(.bss) }
  __bss_end__ = .;

    __exidx_start = .;
    .ARM.exidx   : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) }
    __exidx_end = .;

  /* End of uninitalized data segement */
  _end = .;
}

