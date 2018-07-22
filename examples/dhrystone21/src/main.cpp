//****************************************************************************
//
//****************************************************************************
#include "uart.h"

extern "C" int shell_cmd_soc_dhry(int argc, char *argv[]);

void *__gxx_personality_v0;

//****************************************************************************
int __attribute__((__section__(".text.entrypoint"))) main()
{
  int argc = 0;
  char *argv[1] = {0};

  uart_init();

  shell_cmd_soc_dhry(argc, argv);

  printf_uart("%s[%d]: End of test\n", __FUNCTION__, __LINE__);
  
  return 0;
}

