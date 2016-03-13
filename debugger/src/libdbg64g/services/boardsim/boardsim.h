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
#include "coreservices/iboardsim.h"
#include "coreservices/irawlistener.h"
#include "coreservices/iudp.h"

namespace debugger {

class BoardSim : public IService, 
                 public IBoardSim,
                 public IRawListener {
public:
    BoardSim(const char *name);
    ~BoardSim() {}

    /** IService interface */
    virtual void postinitService();
    virtual void deleteService();

    /** @name IBoardSim interface */
    /// @{
    virtual void runSimulator();
    virtual void stopSimulator();
    virtual AttributeType getConnectionSettings();
    virtual void setTargetSettings(const AttributeType *target);
    /// @}

    /** IRawListener interface */
    virtual void updateData(const char *buf, int buflen) {}

private:
    static thread_return_t runThread(void *arg);
    void busyLoop();

private:
    bool loopEnable_;
    LibThreadType threadInit_;
    IUdp *itransport_;
    char rxbuf[1<<12];
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
