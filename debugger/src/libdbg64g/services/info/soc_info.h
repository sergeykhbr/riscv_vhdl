/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      SOC information.
 */

#ifndef __DEBUGGER_SOC_INFO_H__
#define __DEBUGGER_SOC_INFO_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/isocinfo.h"
#include <string>
#include <stdarg.h>

namespace debugger {

class SocInfo : public IService,
                public ISocInfo  {
public:
    explicit SocInfo(const char *name);

    /** IService interface */
    virtual void postinitService();

    /** ISocInfo */
    virtual unsigned getRegsTotal();
    virtual void getRegsList(AttributeType *lst);
    virtual unsigned getCsrTotal();
    virtual void getCsrList(AttributeType *lst);
    virtual uint64_t csr2addr(const char *name);
    virtual uint64_t reg2addr(const char *name);

    virtual DsuMapType *getpDsu() {
        return reinterpret_cast<DsuMapType *>(dsuBase_.to_uint64());
    }

    virtual uint64_t addressPlugAndPlay();
    virtual uint64_t addressGpio();
    virtual uint64_t addressBreakCreate();
    virtual uint64_t addressBreakRemove();
    virtual uint64_t addressRunControl();
    virtual uint64_t addressStepCounter();
    virtual uint64_t valueHalt();
    virtual uint64_t valueRun();
    virtual uint64_t valueRunStepping();

private:
    AttributeType pnpBase_;
    AttributeType gpioBase_;
    AttributeType dsuBase_;
    AttributeType listCSR_;
    AttributeType listRegs_;
};

DECLARE_CLASS(SocInfo)

}  // namespace debugger

#endif  // __DEBUGGER_SOC_INFO_H__
