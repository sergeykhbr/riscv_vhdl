/*****************************************************************************
 * @file
 * @author   Sergey Khabarov
 * @brief    Firmware example. 
 ****************************************************************************/

#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include "axi_maps.h"

extern char _end;
extern void init_mtvec(void);
extern void init_mtvec();

volatile int was_interrupt = 0;

typedef union csr_mcause_type {
    struct bits_type {
        uint64_t code   : 63;   // 11 - Machine external interrupt
        uint64_t irq    : 1;
    } bits;
    uint64_t value;
} csr_mcause_type;


/**
 * @name sbrk
 * @brief Increase program data space.
 * @details Malloc and related functions depend on this.
 */
char *sbrk(int incr) {
    return &_end;
}

void print_uart(const char *buf, int sz) {
    uart_map *uart = (uart_map *)ADDR_BUS0_XSLV_UART1;
    for (int i = 0; i < sz; i++) {
        while (uart->status & UART_STATUS_TX_FULL) {}
        uart->data = buf[i];
    }
}

void print_uart_hex(long val) {
    unsigned char t, s;
    uart_map *uart = (uart_map *)ADDR_BUS0_XSLV_UART1;
    for (int i = 0; i < 16; i++) {
        while (uart->status & UART_STATUS_TX_FULL) {}
        
        t = (unsigned char)((val >> ((15 - i) * 4)) & 0xf);
        if (t < 10) {
            s = t + '0';
        } else {
            s = (t - 10) + 'a';
        }
        uart->data = s;
    }
}


void isr_example_c(long cause, long epc, long long regs[32]) {
    irqctrl_map *p_irqctrl = (irqctrl_map *)ADDR_BUS0_XSLV_IRQCTRL;
    gptimers_map *p_timer = (gptimers_map *)ADDR_BUS0_XSLV_GPTIMERS;
    uint32_t pending;

    csr_mcause_type mcause;
    mcause.value = cause;

    print_uart("mcause:", 7);
    print_uart_hex(cause);
    print_uart(",mepc:", 6);
    print_uart_hex(epc);
    print_uart("\r\n", 2);


    p_timer->pending = 0;

    p_irqctrl->irq_clear = ~0;

    print_uart("IRQ\n", 4);
    was_interrupt = 1;
}


void helloWorld() {
    char ss[256];
    int ss_len;
    int i = 0;
    uint32_t tech;
    pnp_map *pnp = (pnp_map *)ADDR_BUS0_XSLV_PNP;
    tech = pnp->tech & 0xff;

    init_mtvec();

    gptimers_map *p_timer = (gptimers_map *)ADDR_BUS0_XSLV_GPTIMERS;
    if (tech == 0) {
        p_timer->timer[0].init_value = 20000ull;
    } else {
        p_timer->timer[0].init_value = 40000000ull;
    }
    p_timer->timer[0].control = 0x3;  // count_ena | irq_ena

    // enable GpTimer interrupts
    irqctrl_map *p_irqctrl = (irqctrl_map *)ADDR_BUS0_XSLV_IRQCTRL;
    uint32_t bit = p_irqctrl->irq_mask;
    bit &= ~(1u << CFG_IRQ_GPTIMERS);
    p_irqctrl->irq_mask = bit;
    p_irqctrl->irq_lock = 0;


    while (1) {
        if (!was_interrupt) {
            continue;
        } 
        was_interrupt = 0;
        ss_len = sprintf(ss, "RISC-V River CPU demo: %d!!!!\n", i++);
        print_uart(ss, ss_len);
    }
}

