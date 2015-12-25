/******************************************************************************
 * @file
 * @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
 * @author    Sergey Khabarov - sergeykhbr@gmail.com
 * @brief     UART register mapping definition.
******************************************************************************/

#ifndef __MAP_UART_H__
#define __MAP_UART_H__

#include <inttypes.h>

static const uint32_t UART_STATUS_TX_FULL     = 0x00000001;
static const uint32_t UART_STATUS_TX_EMPTY    = 0x00000002;
static const uint32_t UART_STATUS_RX_FULL     = 0x00000010;
static const uint32_t UART_STATUS_RX_EMPTY    = 0x00000020;
static const uint32_t UART_STATUS_ERR_PARITY  = 0x00000100;
static const uint32_t UART_STATUS_ERR_STOPBIT = 0x00000200;

typedef struct uart_map {
    volatile uint32_t data;
    volatile uint32_t status;
    volatile uint32_t scaler;
} uart_map;

#endif  // __MAP_UART_H__
