/******************************************************************************
 * @file
 * @author    Sergey Khabarov - sergeykhbr@gmail.com
 * @brief     UART register mapping definition.
******************************************************************************/

#pragma once

#include <inttypes.h>

static const uint32_t UART_TX_FULL            = 0x80000000;
static const uint32_t UART_RX_EMPTY           = 0x80000000;


typedef struct uart_txdata_bits_type {
    uint32_t data : 8;           // [7:0] [RW] transmit data
    uint32_t rsrv : 23;          // [30:8]
    uint32_t full : 1;           // [31] [RO] transmit FIFO full
} uart_txdata_bits_type;

typedef union uart_txdata_type {
    uart_txdata_bits_type b;
    volatile uint32_t v;
} uart_txdata_type;


typedef struct uart_rxdata_bits_type {
    uint32_t data : 8;           // [7:0] [RO] receieved data
    uint32_t rsrv : 23;          // [30:8]
    uint32_t empty : 1;          // [31] [RO] receive FIFO empty
} uart_rxdata_bits_type;

typedef union uart_rxdata_type {
    uart_rxdata_bits_type b;
    volatile uint32_t v;
} uart_rxdata_type;


typedef struct uart_txctrl_bits_type {
    uint32_t txen : 1;           // [0] [RW] Transmit enable
    uint32_t nstop : 1;          // [1] [RW] Number of stop bits
    uint32_t parity : 1;         // [2] [RW] Number of stop bits
    uint32_t rsrv15_3 : 13;      // [15:3]
    uint32_t txcnt : 3;          // [18:16] [RW] Transmit watermark level
    uint32_t rsrv31_19 : 13;     // [31:19]
} uart_txctrl_bits_type;

typedef union uart_txctrl_type {
    uart_txctrl_bits_type b;
    volatile uint32_t v;
} uart_txctrl_type;


typedef struct uart_rxctrl_bits_type {
    uint32_t rxen : 1;           // [0] [RW] Receive enable
    uint32_t rsrv15_1 : 15;      // [15:2]
    uint32_t rxcnt : 3;          // [18:16] [RW] Receive watermark level
    uint32_t rsrv31_19 : 13;     // [31:19]
} uart_rxctrl_bits_type;

typedef union uart_rxctrl_type {
    uart_rxctrl_bits_type b;
    volatile uint32_t v;
} uart_rxctrl_type;


typedef struct uart_irq_bits_type {
    uint32_t txwm : 1;           // [0] [RW/RO] tx IRQ enable / pending
    uint32_t rxwm : 1;           // [1] [RW/RO] rx IRQ enable / pending
    uint32_t rsrv31_2 : 30;      // [31:2]
} uart_irq_bits_type;

typedef union uart_irq_type {
    uart_irq_bits_type b;
    volatile uint32_t v;
} uart_irq_type;


typedef struct uart_map {
    volatile uint32_t txdata;    // 0x00 Transmit data register
    volatile uint32_t rxdata;    // 0x04 Receive data register
    volatile uint32_t txctrl;    // 0x08 Transmit control register
    volatile uint32_t rxctrl;    // 0x0C Receive control register
    volatile uint32_t ie;        // 0x10 UART interrupt enable
    volatile uint32_t ip;        // 0x14 UART interrupt pending
    volatile uint32_t scaler;    // 0x18 Baud rate divisor F_baud = Fin / (div + 1)
    volatile uint32_t fwcpuid;   // 0x1c lock flag on multi cpu access
} uart_map;

