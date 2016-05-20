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

static const int AXI4_SYSTEM_CLOCK = 60000000;  /// 60 MHz

#define VENDOR_GNSSSENSOR        0x00F1

#define GNSSSENSOR_EMPTY         0x5577     /// Dummy device
#define GNSSSENSOR_BOOTROM       0x0071     /// Boot ROM Device ID
#define GNSSSENSOR_FWIMAGE       0x0072     /// FW ROM image Device ID
#define GNSSSENSOR_SRAM          0x0073     /// Internal SRAM block Device ID
#define GNSSSENSOR_PNP           0x0074     /// Configuration Registers Module Device ID provided by gnsslib
#define GNSSSENSOR_SPI_FLASH     0x0075     /// SD-card controller Device ID provided by gnsslib
#define GNSSSENSOR_GPIO          0x0076     /// General purpose IOs Device ID provided by gnsslib
#define GNSSSENSOR_RF_CONTROL    0x0077     /// RF front-end controller Device ID provided by gnsslib
#define GNSSSENSOR_ENGINE        0x0078     /// GNSS Engine Device ID provided by gnsslib
#define GNSSSENSOR_ENGINE_STUB   0x0068     /// GNSS Engine stub
#define GNSSSENSOR_FSE_V2        0x0079     /// Fast Search Engines Device ID provided by gnsslib
#define GNSSSENSOR_UART          0x007a     /// rs-232 UART Device ID
#define GNSSSENSOR_ACCELEROMETER 0x007b     /// Accelerometer Device ID provided by gnsslib
#define GNSSSENSOR_GYROSCOPE     0x007c     /// Gyroscope Device ID provided by gnsslib
#define GNSSSENSOR_IRQCTRL       0x007d     /// Interrupt controller
#define GNSSSENSOR_ETHMAC        0x007f
#define GNSSSENSOR_DSU           0x0080
#define GNSSSENSOR_GPTIMERS      0x0081


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