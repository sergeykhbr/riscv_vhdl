#ifndef __UART_H__
#define __UART_H__

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

void uart_init();
int printf_uart(const char *_format, ... );

#ifdef __cplusplus
}
#endif

#endif
