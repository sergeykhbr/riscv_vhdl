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
#include "iboardsim.h"
#ifdef USE_VERILATED_CORE
#include "verilated/Vtop.h"
#endif

namespace debugger {

class BoardSim : public IService, 
                 public IThread,
                 public IBoardSim,
                 public IRawListener {
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

protected:
    /** IThread interface */
    virtual void busyLoop();

private:
    void write32(uint8_t *buf, uint32_t v);
    uint32_t read32(uint8_t *buf);

private:
    AttributeType isEnable_;
    AttributeType transport_;
    IUdp *itransport_;
    uint8_t rxbuf_[1<<12];
    uint8_t txbuf_[1<<12];
    uint8_t SRAM_[1<<20];
    uint32_t seq_cnt_;

#ifdef USE_VERILATED_CORE
    Vtop *core;
#endif
};


class BoardSimClass : public IClass {
public:
    BoardSimClass() : IClass("BoardSimClass") {}

    virtual IService *createService(const char *obj_name) { 
        BoardSim *serv = new BoardSim(obj_name);
        AttributeType item(static_cast<IService *>(serv));
        listInstances_.add_to_list(&item);
        return serv;
    }
};

}  // namespace debugger

#endif  // __DEBUGGER_BOARDSIM_H__
