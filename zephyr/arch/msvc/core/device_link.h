#ifndef __DEVICE_LINK_H__
#define __DEVICE_LINK_H__

#define DEVICE_INIT(dev_name, drv_name, init_fn, data, cfg_info, level, prio) \
	\
	static struct device_config __config_ ## dev_name = \
	{   \
		drv_name,   \
        init_fn,    \
		cfg_info    \
	};  \
	struct device __device_ ## dev_name = {      \
		 &__config_ ## dev_name,   \
         NULL,  \
		 data   \
	}

    typedef enum enum_init_start {
        init_start_Total = 0
    } enum_init_start;

    typedef enum enum_PRIMARY {
        DEVICE_Soc = init_start_Total,
        DEVICE_Uart,
        PRIMARY_Total
    } enum_PRIMARY;

    typedef enum enum_SECONDARY {
        DEVICE_UartConsole = PRIMARY_Total,
        SECONDARY_Total
    } enum_SECONDARY;

    typedef enum enum_NANOKERNEL {
        DEVICE_SysClock = SECONDARY_Total,
        DEVICE_SyskEventLogger,
        NANOKERNEL_Total
    } enum_NANOKERNEL;

    typedef enum enum_MICROKERNEL {
        MICROKERNEL_Total = NANOKERNEL_Total
    } enum_MICROKERNEL;

    typedef enum enum_APPLICATION {
        APPLICATION_Total = MICROKERNEL_Total
    } enum_APPLICATION;

#endif  // __DEVICE_LINK_H__
