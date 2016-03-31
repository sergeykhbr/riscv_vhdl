/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Simulator of the Hardware interface.
 */

#ifndef __DEBUGGER_BOARDSIM_H__
#define __DEBUGGER_BOARDSIM_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/irawlistener.h"
#include "coreservices/iudp.h"
#include "coreservices/ithread.h"
#include "coreservices/imemop.h"
#include "coreservices/iclock.h"
#include "coreservices/iclklistener.h"
#include "iboardsim.h"
#include "ringbuf.h"
#include "fifo.h"

namespace debugger {

class BoardSim : public IService, 
                 public IThread,
                 public IBoardSim,
                 public IRawListener,
                 public IMemoryOperation,
                 public IClockListener {
public:
    BoardSim(const char *name);
    ~BoardSim();

    /** IService interface */
    virtual void postinitService();
    virtual void predeleteService();

    /** @name IBoardSim interface */
    virtual void getInfo(AttributeType *attr) {}

    /** IRawListener interface */
    virtual void updateData(const char *buf, int buflen) {}

    /** IMemoryOperation */
    virtual void transaction(Axi4TransactionType *payload);
    virtual uint64_t getBaseAddress() { return 0; }
    virtual uint64_t getLength() { return 1LL << 32; }

    /** IClockListener */
    virtual void stepCallback(uint64_t t);

    /** IThread status overloading */
    virtual bool isEnabled() { return isEnable_.to_bool(); }

protected:
    /** IThread interface */
    virtual void busyLoop();

private:
    void write32(uint8_t *buf, uint32_t v);
    uint32_t read32(uint8_t *buf);

private:
    AttributeType isEnable_;
    AttributeType transport_;
    AttributeType clkSource_;
    AttributeType map_;
    AttributeType imap_;
    IUdp *itransport_;
    IClock *iclk_;
    uint8_t rxbuf_[1<<12];
    uint8_t txbuf_[1<<12];
    uint32_t seq_cnt_;

    struct FifoMessageType {
        uint64_t addr;
        uint8_t rw;
        uint8_t *buf;
        uint32_t sz;
    };
    TFifo<FifoMessageType> *fifo_to_;
    TFifo<FifoMessageType> *fifo_from_;
    RingBufferType *ring_;
};

DECLARE_CLASS(BoardSim)

}  // namespace debugger

#endif  // __DEBUGGER_BOARDSIM_H__
