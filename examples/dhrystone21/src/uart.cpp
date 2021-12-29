#include <stdio.h>
#include <stdarg.h>
#ifdef CONFIG_RISCV64
#include <axi_maps.h>
#elif CONFIG_LEON3
#include "leon3_config.h"
#endif

static const int BUF_LENGTH = 2048;
static char tmpUartStr[BUF_LENGTH];

//****************************************************************************
extern "C" void uart_init() {
#ifdef CONFIG_RISCV64
#elif CONFIG_LEON3
    uart_fields *uart = (uart_fields *)ADR_APBUART_BASE;
    // scaler = 66MHz/(8*(1+rate)) = 115200 = 71
    uart->scaler = 71;       // 115200 baudrate
    // Init port for the transmition:
    uart->control = UART_CTRL_ENABLE_TX;//|UART_CTRL_TSEMPTY_IRQEN;
#endif
}

extern "C" int uart_busy() {
#ifdef CONFIG_RISCV64
    uart_map *uart = (uart_map *)ADDR_BUS0_XSLV_UART0;
    return uart->txdata & UART_TX_FULL;

#elif CONFIG_LEON3
    uart_fields *uart = (uart_fields *)ADR_APBUART_BASE;
    return uart->status & UART_STATUS_TFULL;
#endif
}

extern "C" void uart_send(unsigned v) {
#ifdef CONFIG_RISCV64
    uart_map *uart = (uart_map *)ADDR_BUS0_XSLV_UART0;
    uart->txdata = v;
#elif CONFIG_LEON3
    uart_fields *uart = (uart_fields *)ADR_APBUART_BASE;
    uart->data = v;
#endif
}

//****************************************************************************
extern "C" int printf_uart(const char *_format, ... ) {
    int ret = 0;

    va_list arg;
    va_start(arg, _format );
  
    ret = vsprintf(tmpUartStr, _format, arg);
    va_end( arg );

    for (int i = 0; i < ret; i++) {
        while(uart_busy()) {}
        uart_send((unsigned int)tmpUartStr[i]);
    }
    return ret;
}

