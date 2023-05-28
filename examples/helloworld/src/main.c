/*****************************************************************************
 * @file
 * @author   Sergey Khabarov
 * @brief    Main entry function for real Firmware. 
 ****************************************************************************/

#include <axi_maps.h>

extern void helloWorld();

void exception_instr_load_fault_c() {}
void exception_load_fault_c() {}
void exception_store_fault_c() {}
void exception_stack_overflow_c() {}
void exception_stack_underflow_c() {}
void exception_handler_c() {}
void interrupt_s_software_c() {}
void interrupt_m_software_c() {}
void interrupt_s_timer_c() {}
void interrupt_m_timer_c() {}
void interrupt_s_external_c() {}
void interrupt_m_external_c() {}
long interrupt_handler_c(long cause, long epc, long long regs[32]) {}

void HwInit() {
    uart_map *uart = (uart_map *)ADDR_BUS0_XSLV_UART0;

    // scaler is enabled in SRAM self test, duplicate it here
    uart->scaler = SYS_HZ / 115200 / 2;
    uart->txctrl = 0x1;  // txen=1
    uart->rxctrl = 0x1;  // rxen=1
    uart_irq_type ie;
    ie.v = 0;
#ifdef UART_BUF_ENABLE    
    ie.b.txwm = 1;
    ie.b.rxwm = 0;
#endif
    uart->ie = ie.v;

}

int main() {
    HwInit();
    helloWorld();
    return 0;
}

