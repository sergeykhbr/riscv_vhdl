/*
 * Copyright (c) 2016, GNSS Sensor Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <nanokernel.h>
#include <arch/cpu.h>
#include <sections.h>
#include <misc/__assert.h>
#include <stdint.h>
#include <misc/util.h>
#include <string.h>
#include <board.h>
#include <init.h>
#include <uart.h>
#include <irq.h>

#define UART_STATUS_TX_FULL     0x00000001
#define UART_STATUS_TX_EMPTY    0x00000002
#define UART_STATUS_RX_FULL     0x00000010
#define UART_STATUS_RX_EMPTY    0x00000020
#define UART_STATUS_ERR_PARITY  0x00000100
#define UART_STATUS_ERR_STOPBIT 0x00000200
#define UART_CONTROL_RX_IRQ_ENA 0x00002000
#define UART_CONTROL_TX_IRQ_ENA 0x00004000
#define UART_CONTROL_PARITY_ENA 0x00008000


static struct uart_driver_api uart_gnss_driver_api;
/* Device data structure */
struct uart_gnss_dev_data_t {
	uint32_t baud_rate;	/* Baud rate */

#ifdef CONFIG_UART_INTERRUPT_DRIVEN
	uart_irq_callback_t     cb;     /**< Callback function pointer */
#endif
};

static struct uart_gnss_dev_data_t uart_gnss_dev_data_0 = {
	115200,  // .baud_rate
    NULL
};


/**
 * @brief Interrupt service routine.
 *
 * This simply calls the callback function, if one exists.
 *
 * @param arg Argument to ISR.
 *
 * @return N/A
 */
void uart_gnss_isr(void *arg) {
    struct device *dev = (struct device *)arg;
    struct uart_gnss_dev_data_t *data =
        (struct uart_gnss_dev_data_t *)dev->driver_data;
    if (data->cb) {
        data->cb(dev);
    }
}

/*
 * @brief Output a character to serial port
 *
 * @param dev UART device struct
 * @param c character to output
 */
unsigned char uart_gnss_poll_out(struct device *dev, unsigned char c)
{
	/* wait for transmitter to ready to accept a character */
    uint32_t status = READ32(&__UART1->status);
	while (status & UART_STATUS_TX_FULL) {
        status = READ32(&__UART1->status);
	}
	WRITE32(&__UART1->data, c);
	return c;
}

static int uart_gnss_poll_in(struct device *dev, unsigned char *c)
{
	return -ENOTSUP;

}

/**
 * @brief Read data from FIFO
 *
 * @param dev UART device struct
 * @param rx_data Pointer to data container
 * @param size Container size
 *
 * @return Number of bytes read
 */
static int uart_gnss_fifo_read(struct device *dev, uint8_t *rx_data,
				    const int size)
{
	uint32_t status = READ32(&__UART1->status);
	uint8_t num_rx = 0;

	while ((size - num_rx > 0) && ((status & UART_STATUS_RX_EMPTY) == 0)) {
		rx_data[num_rx++] = (uint8_t)READ32(&__UART1->data);
        status = READ32(&__UART1->status);
	}

	return num_rx;
}

void uart_gnss_irq_tx_enable(struct device *dev) {
	uint32_t status = READ32(&__UART1->status);
    status |= UART_CONTROL_TX_IRQ_ENA;
    WRITE32(&__UART1->status, status);
}

void uart_gnss_irq_tx_disable(struct device *dev) {
	uint32_t status = READ32(&__UART1->status);
    status &= ~UART_CONTROL_TX_IRQ_ENA;
    WRITE32(&__UART1->status, status);
}

void uart_gnss_irq_rx_enable(struct device *dev) {
	uint32_t status = READ32(&__UART1->status);
    status |= UART_CONTROL_RX_IRQ_ENA;
    WRITE32(&__UART1->status, status);
}

void uart_gnss_irq_rx_disable(struct device *dev) {
	uint32_t status = READ32(&__UART1->status);
    status &= ~UART_CONTROL_RX_IRQ_ENA;
    WRITE32(&__UART1->status, status);
}

int uart_gnss_irq_tx_empty(struct device *dev) {
	uint32_t status = READ32(&__UART1->status);
	return ((status & UART_STATUS_TX_EMPTY) ? 1: 0);
}

/**
 * @brief Check if Rx IRQ has been raised
 *
 * @param dev UART device struct
 *
 * @return 1 if an IRQ is ready, 0 otherwise
 */
static int uart_gnss_irq_rx_ready(struct device *dev)
{
	uint32_t status = READ32(&__UART1->status);

	return ((status & UART_STATUS_RX_EMPTY) ? 0: 1);
}


/**
 * @brief Check if Tx or Rx IRQ is pending
 *
 * @param dev UART device struct
 *
 * @return 1 if a Tx or Rx IRQ is pending, 0 otherwise
 */
static int uart_gnss_irq_is_pending(struct device *dev)
{
	/* Look only at Tx and Rx data interrupt flags */
	return uart_gnss_irq_rx_ready(dev);
}


/**
 * @brief Update IRQ status
 *
 * @param dev UART device struct
 *
 * @return Always 1
 */
static int uart_gnss_irq_update(struct device *dev)
{
	return 1;
}

/**
 * @brief Set the callback function pointer for IRQ.
 *
 * @param dev UART device struct
 * @param cb Callback function pointer.
 *
 * @return N/A
 */
static void uart_gnss_irq_callback_set(struct device *dev,
                                        uart_irq_callback_t cb)
{
	struct uart_gnss_dev_data_t * const dev_data = dev->driver_data;
	dev_data->cb = cb;
}

static struct uart_driver_api uart_gnss_driver_api = {
	uart_gnss_poll_in,  // poll_in
	uart_gnss_poll_out, // poll_out
	NULL,//int (*err_check)(struct device *dev);

#ifdef CONFIG_UART_INTERRUPT_DRIVEN
	NULL,//int (*fifo_fill)(struct device *dev, const uint8_t *tx_data, int len);
	uart_gnss_fifo_read,
	uart_gnss_irq_tx_enable,
	uart_gnss_irq_tx_disable,
	NULL,//int (*irq_tx_ready)(struct device *dev);
	uart_gnss_irq_rx_enable,
	uart_gnss_irq_rx_disable,
	uart_gnss_irq_tx_empty,
	uart_gnss_irq_rx_ready,
	NULL,//void (*irq_err_enable)(struct device *dev);
	NULL,//void (*irq_err_disable)(struct device *dev);
	uart_gnss_irq_is_pending,
	uart_gnss_irq_update,
	NULL,//int (*irq_input_hook)(struct device *dev, uint8_t byte);
    uart_gnss_irq_callback_set,
#endif

#ifdef CONFIG_UART_LINE_CTRL
	NULL,//int (*line_ctrl_set)(struct device *dev, uint32_t ctrl, uint32_t val);
#endif

#ifdef CONFIG_UART_DRV_CMD
	NULL,//int (*drv_cmd)(struct device *dev, uint32_t cmd, uint32_t p);
#endif
};

/**
 * @brief Initialize fake serial port
 *
 * @param dev UART device struct
 *
 * @return DEV_OK
 */
static int uart_gnss_init(struct device *dev)
{
	dev->driver_api = &uart_gnss_driver_api;
    dev->driver_data = &uart_gnss_dev_data_0;

    // Speed-up RTL simulation avoidig long polling of the status register.
    if (soc_is_rtl_simulation() != 0) {
        WRITE32(&__UART1->scaler, 20);
    } else {
        WRITE32(&__UART1->scaler, CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC/115200/2);
    }

#ifdef CONFIG_UART_INTERRUPT_DRIVEN
    uart_gnss_irq_rx_enable(dev);
	IRQ_CONNECT(CFG_IRQ_UART1, CFG_IRQ_UART1, uart_gnss_isr, dev, UART_IRQ_FLAGS);
	irq_enable(CFG_IRQ_UART1);
#endif
	return 0;
}


DEVICE_INIT(uart_gnss0, "UART_0", &uart_gnss_init,
			NULL, NULL,
			PRIMARY, CONFIG_KERNEL_INIT_PRIORITY_DEVICE);
