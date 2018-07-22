#ifndef __SOCONFIG_H__
#define __SOCONFIG_H__

#include "stdtypes.h"

static const uint32 ADR_APBUART_BASE = 0x80000100;
static const uint32 ADR_IRQMP_BASE   = 0x80000200;
static const uint32 ADR_GPTIMER_BASE = 0x80000300;


static const int32 TIMERS_TOTAL    = 2;

// IRQ numbers:
static const int32 IRQ_IRQ_CONTROL = 0;// irqmp, has to be 0
static const int32 IRQ_DSU         = IRQ_IRQ_CONTROL+1;
static const int32 IRQ_GNSS_ENGINE = IRQ_DSU+1;
static const int32 IRQ_UART_CFG    = IRQ_GNSS_ENGINE+1;
static const int32 IRQ_TIMER       = IRQ_UART_CFG+1;
static const int32 IRQ_TOTAL       = IRQ_TIMER+1+(TIMERS_TOTAL-1);


static const uint32 UART_STATUS_DREADY        = (1 << 0);
static const uint32 UART_STATUS_TSEMPTY       = (1 << 1);
static const uint32 UART_STATUS_THEMPTY       = (1 << 2);
static const uint32 UART_STATUS_BREAK         = (1 << 3);
static const uint32 UART_STATUS_OVERFLOW      = (1 << 4);
static const uint32 UART_STATUS_PARERR        = (1 << 5);
static const uint32 UART_STATUS_FRAME         = (1 << 6);
static const uint32 UART_STATUS_THALFFULL     = (1 << 7); // half of fifo is full
static const uint32 UART_STATUS_RHALFFULL     = (1 << 8);
static const uint32 UART_STATUS_TFULL         = (1 << 9);
static const uint32 UART_STATUS_RFULL         = (1 << 10);

static const uint32 UART_CTRL_DISABLE       = 0x0;
static const uint32 UART_CTRL_ENABLE_RX     = (1 << 0);
static const uint32 UART_CTRL_ENABLE_TX     = (1 << 1);
static const uint32 UART_CTRL_RX_INT        = (1 << 2);
static const uint32 UART_CTRL_TX_INT        = (1 << 3);
static const uint32 UART_CTRL_PARSEL        = (1 << 4);
//static const uint32 UART_CTRL_EVEN_PARITY   = (1 << 5);
static const uint32 UART_CTRL_PAREN         = (1 << 5);
//static const uint32 UART_CTRL_ODD_PARITY    = (1 << 6);
static const uint32 UART_CTRL_FLOW_CONTROL  = (1 << 6);
static const uint32 UART_CTRL_LOOP_BACK     = (1 << 7);
static const uint32 UART_CTRL_EXTCLKENA     = (1 << 8);
static const uint32 UART_CTRL_FIFO_TX_INT   = (1 << 9);
static const uint32 UART_CTRL_FIFO_RX_INT   = (1 << 10);
static const uint32 UART_CTRL_DEBUG         = (1 << 11);
static const uint32 UART_CTRL_BREAK_IRQEN   = (1 << 12);
static const uint32 UART_CTRL_DELAY_IRQEN   = (1 << 13);
static const uint32 UART_CTRL_TSEMPTY_IRQEN = (1 << 14);


struct uart_fields
{
   volatile int32 data;
   volatile int32 status;
   volatile int32 control;
   volatile int32 scaler;
};

#endif
