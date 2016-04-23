/******************************************************************************
 * @file
 * @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
 * @author    Sergey Khabarov - sergeykhbr@gmail.com
 * @brief     Boot procedure of copying FW image into SRAM with the debug
 *            signals.
******************************************************************************/

#include <string.h>
#include "axi_maps.h"
#include "encoding.h"

static const int FW_IMAGE_SIZE_BYTES = 1 << 18;

void led_set(int output) {
    ((gpio_map *)ADDR_NASTI_SLAVE_GPIO)->led = output;
}

void print_uart(const char *buf, int sz) {
    uart_map *uart = (uart_map *)ADDR_NASTI_SLAVE_UART1;
    for (int i = 0; i < sz; i++) {
        while (uart->status & UART_STATUS_TX_FULL) {}
        uart->data = buf[i];
        led_set(0x10 + i);
    }
}

void copy_image() { 
    uint32_t tech;
    uint64_t *fwrom = (uint64_t *)ADDR_NASTI_SLAVE_FWIMAGE;
    uint64_t *sram = (uint64_t *)ADDR_NASTI_SLAVE_SRAM;
    pnp_map *pnp = (pnp_map *)ADDR_NASTI_SLAVE_PNP;

    /** 
     * Speed-up RTL simulation by skipping coping stage.
     * Or skip this stage to avoid rewritting of externlly loaded image.
     */
    tech = pnp->tech & 0xFF;
    if (tech != TECH_INFERRED && pnp->fwid == 0) {
        memcpy(sram, fwrom, FW_IMAGE_SIZE_BYTES);
    }

#if 0
    /** Just to check access to DSU and read MCPUID via this slave device.
     *  Verification is made on time diagram (ModelSim), no other purposes of 
     *  these operations.
     *        DSU base address = 0x80080000: 
     *        CSR address: Addr[15:4] = 16 bytes alignment
     *  3296 ns - reading (iClkCnt = 409)
     *  3435 ns - writing (iClkCnt = 427)
     */
    uint64_t *arr_csrs = (uint64_t *)0x80080000;
    uint64_t x1 = arr_csrs[CSR_MCPUID<<1]; 
    pnp->fwdbg1 = x1;
    arr_csrs[CSR_MCPUID<<1] = x1;
#endif
}

void _init() {
    uint32_t tech;
    pnp_map *pnp = (pnp_map *)ADDR_NASTI_SLAVE_PNP;
    uart_map *uart = (uart_map *)ADDR_NASTI_SLAVE_UART1;
    gpio_map *gpio = (gpio_map *)ADDR_NASTI_SLAVE_GPIO;
    irqctrl_map *p_irq = (irqctrl_map *)ADDR_NASTI_SLAVE_IRQCTRL;

    // mask all interrupts in interrupt controller to avoid
    // unpredictable behaviour after elf-file reloading via debug port.
    p_irq->irq_mask = 0xFFFFFFFF;

    // Half period of the uart = Fbus / 115200 / 2 = 70 MHz / 115200 / 2:
    //uart->scaler = 304;  // 70 MHz
    uart->scaler = 260;  // 60 MHz

    led_set(0x01);
    print_uart("Boot . . .", 10);
    led_set(0x02);

    copy_image();
    led_set(0x03);
    print_uart("OK\r\n", 4);

    /** Check ADC detector that RF front-end is connected: */
    tech = (pnp->tech >> 24) & 0xff;
    while (tech != 0xFF) {
        print_uart("ADC clock not found. Enable DIP int_rf.\r\n", 41);
        tech = (pnp->tech >> 24) & 0xff;
        led_set(tech);
    }
    led_set(0x04);
}

/** Not used actually */
int main() {
    while (1) {}

    return 0;
}
