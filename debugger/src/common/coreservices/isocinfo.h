/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      SOC Information interface.
 */

#ifndef __DEBUGGER_ISOCINFO_H__
#define __DEBUGGER_ISOCINFO_H__

#include "iface.h"
#include "attribute.h"

namespace debugger {

static const char *IFACE_SOC_INFO = "ISocInfo";

static const int CFG_NASTI_MASTER_CACHED    = 0;
static const int CFG_NASTI_MASTER_UNCACHED  = 1;
static const int CFG_NASTI_MASTER_ETHMAC    = 2;
static const int CFG_NASTI_MASTER_TOTAL     = 3;

static const uint16_t MST_DID_EMPTY            = 0x7755;
static const uint16_t SLV_DID_EMPTY            = 0x5577;

static const uint16_t VENDOR_GNSSSENSOR        = 0x00F1;
// Masters IDs
static const uint16_t RISCV_CACHED_TILELINK    = 0x0500;
static const uint16_t RISCV_UNCACHED_TILELINK  = 0x0501;
static const uint16_t GAISLER_ETH_MAC_MASTER   = 0x0502;
static const uint16_t GAISLER_ETH_EDCL_MASTER  = 0x0503;
static const uint16_t RISCV_RIVER_CPU          = 0x0505;

// Slaves IDs
static const uint16_t GNSSSENSOR_BOOTROM       = 0x0071;
static const uint16_t GNSSSENSOR_FWIMAGE       = 0x0072;
static const uint16_t GNSSSENSOR_SRAM          = 0x0073;
static const uint16_t GNSSSENSOR_PNP           = 0x0074;
static const uint16_t GNSSSENSOR_SPI_FLASH     = 0x0075;
static const uint16_t GNSSSENSOR_GPIO          = 0x0076;
static const uint16_t GNSSSENSOR_RF_CONTROL    = 0x0077;
static const uint16_t GNSSSENSOR_ENGINE        = 0x0078;
static const uint16_t GNSSSENSOR_ENGINE_STUB   = 0x0068;
static const uint16_t GNSSSENSOR_FSE_V2        = 0x0079;
static const uint16_t GNSSSENSOR_UART          = 0x007a;
static const uint16_t GNSSSENSOR_ACCELEROMETER = 0x007b;
static const uint16_t GNSSSENSOR_GYROSCOPE     = 0x007c;
static const uint16_t GNSSSENSOR_IRQCTRL       = 0x007d;
static const uint16_t GNSSSENSOR_ETHMAC        = 0x007f;
static const uint16_t GNSSSENSOR_GPTIMERS      = 0x0081;

static const uint32_t PNP_CFG_TYPE_INVALID     = 0;
static const uint32_t PNP_CFG_TYPE_MASTER      = 1;
static const uint32_t PNP_CFG_TYPE_SLAVE       = 2;

static const uint32_t TECH_INFERRED            = 0;
static const uint32_t TECH_VIRTEX6             = 36;
static const uint32_t TECH_KINTEX7             = 49;

typedef struct MasterConfigType {
    union DescrType {
        struct bits_type {
            uint32_t descrsize : 8;
            uint32_t descrtype : 2;
            uint32_t rsrv : 14;
            uint32_t xindex : 8;
        } bits;
        uint32_t val;
    } descr;
    uint16_t did;
    uint16_t vid;
} MasterConfigType;

typedef struct SlaveConfigType {
    union DescrType {
        struct bits_type {
            uint32_t descrsize : 8;
            uint32_t descrtype : 2;
            uint32_t bar_total : 2;
            uint32_t rsrv1 : 4;
            uint32_t irq_idx : 8;
            uint32_t xindex : 8;
        } bits;
        uint32_t val;
    } descr;
    uint16_t did;
    uint16_t vid;
    uint32_t xmask;
    uint32_t xaddr;
} SlaveConfigType;

typedef struct PnpMapType {
    uint32_t hwid;              /// 0xfffff000: RO: HW ID
    uint32_t fwid;              /// 0xfffff004: RW: FW ID
    union TechType {
        struct bits_type {
            uint8_t tech;
            uint8_t slv_total;
            uint8_t mst_total;
            uint8_t adc_detect;
        } bits;
        uint32_t val;
    } tech;                     /// 0xfffff008: RO: technology index
    uint32_t rsrv1;             /// 0xfffff00c: 
    uint64_t idt;               /// 0xfffff010: 
    uint64_t malloc_addr;       /// 0xfffff018: RW: debuggind memalloc pointer 0x18
    uint64_t malloc_size;       /// 0xfffff020: RW: debugging memalloc size 0x20
    uint64_t fwdbg1;            /// 0xfffff028: RW: FW debug register
    uint64_t rsrv[2];           /// 0xfffff030, 0xfffff038
    uint8_t cfg_table[(1 << 12) - 0x40];/// 0xfffff040: RO: PNP configuration
} PnpMapType;

struct GpioType {
    union {
        struct MapType {
            uint32_t led;
            uint32_t dip;
        } map;
        uint64_t val[1];
        uint8_t buf[8];
    } u;
};

struct DsuMapType {
    // Base Address + 0x00000 (Region 0)
    uint64_t csr[1 << 12];
    // Base Address + 0x08000 (Region 1)
    union ureg_type {
        uint8_t buf[1 << (12 + 3)];
        struct regs_type {
            uint64_t iregs[32];     // integer registers
            uint64_t pc;            // index = 32
            uint64_t npc;           // index = 33
            uint64_t rsrv;
        } v;
    } ureg;
    // Base Address + 0x10000 (Region 2)
    union udbg_type {
        uint8_t buf[1 << (12 + 3)];
        struct debug_region_type {
            union control_reg {
                uint64_t val;
                struct {
                    uint64_t halt     : 1;
                    uint64_t stepping : 1;
                    uint64_t breakpoint : 1;
                    uint64_t rsv1     : 1;
                    uint64_t core_id  : 16;
                    uint64_t rsv2     : 44;
                } bits;
            } control;
            uint64_t stepping_mode_steps;
            uint64_t clock_cnt;
            uint64_t executed_cnt;
            union breakpoint_control_reg {
                uint64_t val;
                struct {
                    /** Trap on instruction:
                     *      0 = Halt pipeline on ECALL instruction
                     *      1 = Generate trap on ECALL instruction
                     */
                    uint64_t trap_on_break : 1;
                    uint64_t rsv1          : 63;
                } bits;
            } br_ctrl;
            uint64_t add_breakpoint;
            uint64_t remove_breakpoint;
            /**
             * Don't fetch instruction from this address use specified
             * below instead.
             */
            uint64_t br_address_fetch;
            /**
             * True instruction value instead of injected one. Use this
             * instruction instead of memory.
             */
            uint64_t br_instr_fetch;
        } v;
    } udbg;
    // Base Address + 0x18000 (Region 3)
    union local_regs_type {
        uint8_t buf[1 << (12 + 3)];
        struct local_region_type {
            uint64_t soft_reset;
            uint64_t miss_access_cnt;
            uint64_t miss_access_addr;
            uint64_t rsrv[5];
            // Bus utilization registers
            struct mst_bus_util_type {
                uint64_t w_cnt;
                uint64_t r_cnt;
            } bus_util[CFG_NASTI_MASTER_TOTAL];
        } v;
    } ulocal;
};


const uint64_t REG_ADDR_ERROR = 0xFFFFFFFFFFFFFFFFull;

class ISocInfo : public IFace {
public:
    ISocInfo() : IFace(IFACE_SOC_INFO) {}

    virtual unsigned getMastersTotal() =0;
    virtual unsigned getSlavesTotal() =0;
    virtual unsigned getRegsTotal() =0;
    virtual void getRegsList(AttributeType *lst) =0;
    virtual unsigned getCsrTotal() =0;
    virtual void getCsrList(AttributeType *lst) =0;
    virtual uint64_t csr2addr(const char *name) =0;
    virtual uint64_t reg2addr(const char *name) =0;

    virtual DsuMapType *getpDsu() =0;

    virtual uint64_t addressPlugAndPlay() =0;
    virtual uint64_t addressGpio() =0;
};

}  // namespace debugger

#endif  // __DEBUGGER_ISOCINFO_H__
