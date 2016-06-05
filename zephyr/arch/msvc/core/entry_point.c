#include <string.h>
#include <device.h>
#include <misc/util.h>
#include <misc/printk.h>


extern void __printk_hook_install(int (*fn)(int));
extern void __stdout_hook_install(int (*fn)(int));
extern void LIBH_create_dispatcher(void *entry_point);
extern int LIBH_uart_output(int v);


struct device *config_levels[6];

// init
// None
// PRIMARY
extern struct device __device_sys_init_riscv_gnss_soc_init;
extern struct device __device_uart_gnss0;
// SECONDARY
extern struct device __device_sys_init_uart_console_init;// CONFIG_EARLY_CONSOLE is disabled
// NANOKERNEL
extern struct device __device_sys_init__sys_clock_driver_init;
extern struct device __device_sys_init__sys_k_event_logger_init;

struct device zephyr_devices[16] = {0
};

extern FUNC_NORETURN void _Cstart(void);

int BaseAddress_0x40000000(int argc, char *argv[]) {

    //__printk_hook_install(LIBH_uart_output);
    //__stdout_hook_install(LIBH_uart_output);

    zephyr_devices[DEVICE_Soc] = __device_sys_init_riscv_gnss_soc_init;
    zephyr_devices[DEVICE_Uart] = __device_uart_gnss0;
    zephyr_devices[DEVICE_SysClock] = __device_sys_init__sys_clock_driver_init;
    zephyr_devices[DEVICE_UartConsole] = __device_sys_init_uart_console_init;
    zephyr_devices[DEVICE_SyskEventLogger] = __device_sys_init__sys_k_event_logger_init;

    LIBH_create_dispatcher(_Cstart);
        

    //printk("Hello %s\n", "World");
   
    return 0;
}