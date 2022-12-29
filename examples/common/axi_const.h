/******************************************************************************
 * @file
 * @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
 * @author    Sergey Khabarov - sergeykhbr@gmail.com
 * @brief     AXI4 system bus constants definition
******************************************************************************/

#ifndef __AXI_CONST_H__
#define __AXI_CONST_H__

#include <inttypes.h>

typedef uint64_t adr_type;

static const int AXI4_SYSTEM_CLOCK = 40000000;  /// 40 MHz

#define VENDOR_GNSSSENSOR        0x00F1
#define VENDOR_OPTIMITECH        0x00F2

#define GNSSSENSOR_EMPTY         0x5577     /// Dummy device
#define GNSS_SUB_SYSTEM          0x0067     /// GNSS system
#define GNSSSENSOR_ENGINE_STUB   0x0068     /// GNSS Engine stub
#define GNSSSENSOR_FSE_V2_GPS    0x0069     /// Fast Search Engines Device ID provided by gnsslib
#define GNSSSENSOR_FSE_V2_GLO    0x006A     ///
#define GNSSSENSOR_FSE_V2_GAL    0x006C     ///
#define GNSSSENSOR_BOOTROM       0x0071     /// Boot ROM Device ID
#define GNSSSENSOR_FWIMAGE       0x0072     /// FW ROM image Device ID
#define GNSSSENSOR_SRAM          0x0073     /// Internal SRAM block Device ID
#define GNSSSENSOR_PNP           0x0074     /// Configuration Registers Module Device ID provided by gnsslib
#define GNSSSENSOR_SPI_FLASH     0x0075     /// SD-card controller Device ID provided by gnsslib
#define GNSSSENSOR_GPIO          0x0076     /// General purpose IOs Device ID provided by gnsslib
#define GNSSSENSOR_RF_CONTROL    0x0077     /// RF front-end controller Device ID provided by gnsslib
#define GNSSSENSOR_ENGINE        0x0078     /// GNSS Engine Device ID provided by gnsslib
#define GNSSSENSOR_UART          0x007a     /// rs-232 UART Device ID
#define GNSSSENSOR_ACCELEROMETER 0x007b     /// Accelerometer Device ID provided by gnsslib
#define GNSSSENSOR_GYROSCOPE     0x007c     /// Gyroscope Device ID provided by gnsslib
#define GNSSSENSOR_IRQCTRL       0x007d     /// Interrupt controller
#define GNSSSENSOR_ETHMAC        0x007f
#define GNSSSENSOR_DSU           0x0080
#define GNSSSENSOR_GPTIMERS      0x0081
#define GNSSSENSOR_RECORDER      0x0082
#define OPTIMITECH_CLINT         0x0083     /// Core Local interrupt controller
#define OPTIMITECH_PLIC          0x0084     /// External interrupt controller
#define OPTIMITECH_AXI2APB_BRIDGE 0x0085    /// AXI to APB bridge
#define OPTIMITECH_AXI_INTERCONNECT 0x0086  /// AXI inteconnect
#define OPTIMITECH_PRCI          0x0087     /// PLL and Reset Control Registers
#define OPTIMITECH_DDRCTRL       0x0088     /// DDR Controller registers
#define OPTIMITECH_SPI           0x0089     /// SPI controller (SD-card in SPI mode)
#define OPTIMITECH_RIVER_DMI     0x008a     /// Workgroup debug interface
#define DID_LAST                 0x008a     /// The last Device ID index of the valid Device


#define CFG_NASTI_MASTER_CACHED     0
#define CFG_NASTI_MASTER_UNCACHED   1
#define CFG_NASTI_MASTER_ETHMAC     2
#define CFG_NASTI_MASTER_TOTAL      3

#define MST_DID_EMPTY             0x7755
#define SLV_DID_EMPTY             0x5577

// Context indexes:
#define CTX_CPU0_M_MODE   0
#define CTX_CPU0_S_MODE   1

// Masters IDs
#define RISCV_CACHED_TILELINK     0x0500
#define RISCV_UNCACHED_TILELINK   0x0501
#define GAISLER_ETH_MAC_MASTER    0x0502
#define GAISLER_ETH_EDCL_MASTER   0x0503
#define RISCV_RIVER_CPU           0x0505
#define RISCV_RIVER_WORKGROUP     0x0506
#define GNSSSENSOR_UART_TAP       0x050a
#define GNSSSENSOR_JTAG_TAP       0x050b

#define PNP_CFG_TYPE_INVALID      0
#define PNP_CFG_TYPE_MASTER       1
#define PNP_CFG_TYPE_SLAVE        2


#define TECH_INFERRED       0
#define TECH_VIRTEX6        36
#define TECH_KINTEX7        49

#ifdef WIN32
#ifdef __cplusplus
extern "C" {
#endif
void WRITE32(const volatile uint32_t *adr, uint32_t val);
void WRITE64(const volatile uint64_t *adr, uint64_t val);
uint8_t READ8(const volatile uint8_t *adr);
uint16_t READ16(const volatile uint16_t *adr);
uint32_t READ32(const volatile uint32_t *adr);
uint64_t READ64(const volatile uint64_t *adr);
#ifdef __cplusplus
}
#endif

#else

static inline void WRITE32(const volatile uint32_t *adr, uint32_t val) {
    *((volatile uint32_t *)adr) = val;
}

static inline void WRITE64(const volatile uint64_t *adr, uint64_t val) {
    *((volatile uint64_t *)adr) = val;
}

static inline uint8_t READ8(const volatile uint8_t *adr) {
    return adr[0];
}

static inline uint16_t READ16(const volatile uint16_t *adr) {
    return adr[0];
}

static inline uint32_t READ32(const volatile uint32_t *adr) {
    return adr[0];
}

static inline uint64_t READ64(const volatile uint64_t *adr) {
    return adr[0];
}

#endif              

#endif  // __AXI_CONST_H__