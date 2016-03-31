/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      CPU simlator class declaration.
 */

#ifndef __DEBUGGER_SOCSIM_CORE_H__
#define __DEBUGGER_SOCSIM_CORE_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/ithread.h"
#include "coreservices/icpuriscv.h"
#include "coreservices/imemop.h"
#include "coreservices/ihostio.h"
#include "coreservices/iclock.h"
#include "coreservices/iclklistener.h"
#include "types_rtl.h"
#include "verilator/Vtop.h"

namespace debugger {

class CpuRiscV_RTL :  public IService, 
                 public IThread,
                 public ICpuRiscV,
                 public IHostIO,
                 public IClock {
public:
    CpuRiscV_RTL(const char *name);
    ~CpuRiscV_RTL();

    /** IService interface */
    virtual void postinitService();
    virtual void predeleteService();

    /** ICpuRiscV interface */
    virtual void halt() {}

    /** IHostIO */
    virtual uint64_t write(uint16_t adr, uint64_t val);
    virtual uint64_t read(uint16_t adr, uint64_t *val);


    /** IClock */
    virtual uint64_t getStepCounter() { return tick_cnt_/2; }
    virtual void registerStepCallback(IClockListener *cb, uint64_t t);

protected:
    /** IThread interface */
    virtual void busyLoop();

private:
    void tickUpdate();
    void hostBridgeUpdate();
    void ambaBridgeUpdate();

    void procedureDecodeTileAcquire( uint32_t a_type,
                                    uint8_t built_in,
                                    uint32_t u,
                                    uint8_t &write,
                                    uint32_t &wmask,
                                    uint32_t &axi_sz,
                                    uint32_t &byte_addr,
                                    int32_t &beat_cnt);

    void tile_reg(tile_cached_out_type *itile,
                    uint8_t wWrite,
                    uint32_t wbMask,
                    uint32_t wbAxiSize,
                    uint32_t wbByteAddr,
                    int32_t wbBeatCnt);

    void processMemopyOperation(uint32_t *wdata, tile_cached_in_type *otile);

private:
    AttributeType parentThread_;
    AttributeType phys_mem_;

    IMemoryOperation *imemop_;

    Vtop *rtl_;

    uint64_t tick_cnt_;
    uint8_t reset_;
    tile_cached_out_type cto_;
    tile_cached_in_type cti_;
    tile_cached_out_type uto_;
    tile_cached_in_type uti_; 
    host_in_type hosti_;
    host_out_type hosto_;
    Axi4TransactionType memop_;

    // Bridge Data:
    enum EBridgeState {IDLE, UNCACHED, CACHED};
    EBridgeState state_;
    uint64_t r_write_;
    uint64_t r_wr_addr_;
    uint64_t r_wr_addr_incr_;
    uint64_t r_wr_beat_cnt_;
    uint32_t r_wr_xsize_;
    uint32_t r_wr_xact_id_;
    uint32_t r_wr_g_type_;
    uint32_t r_wmask_;
    uint32_t r_wdata_[4];

    uint64_t r_rd_addr_;
    uint64_t r_rd_addr_incr_;
    uint64_t r_rd_beat_cnt_;
    uint32_t r_rd_xsize_;
    uint32_t r_rd_xact_id_;
    uint32_t r_rd_g_type_;

    enum EHostState {HOST_IDLE, HOST_WAIT_REQREADY, HOST_WAIT_RESPVALID};
    EHostState host_state_;

    enum QueueItemNames {
        Queue_Time, 
        Queue_IFace, 
        Queue_Total
    };
    AttributeType stepQueue_;
    unsigned stepQueue_len_;    // to avoid reallocation
};

DECLARE_CLASS(CpuRiscV_RTL)

}  // namespace debugger

#endif  // __DEBUGGER_SOCSIM_CORE_H__
