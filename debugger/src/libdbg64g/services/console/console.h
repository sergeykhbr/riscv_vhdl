/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Input console class declaration.
 */

#ifndef __DEBUGGER_CONSOLE_H__
#define __DEBUGGER_CONSOLE_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/ithread.h"
#include "coreservices/iconsole.h"
#include "coreservices/ikeylistener.h"
#include <string>

namespace debugger {

class ConsoleService : public IService,
                       public IThread,
                       public IConsole {
public:
    explicit ConsoleService(const char *name);
    virtual ~ConsoleService();

    /** IService interface */
    virtual void postinitService();
    virtual void predeleteService();

    /** IConsole interface */
    virtual void writeBuffer(const char *buf);
    virtual void writeCommand(const char *cmd);
    virtual void setCmdString(const char *buf);
    virtual int registerKeyListener(IFace *iface);

protected:
    /** IThread interface */
    virtual void busyLoop();

private:
    bool isData();
    int getData();
    void update();
    void clearLine();

private:
    AttributeType isEnable_;
    AttributeType consumer_;
    AttributeType keyListeners_;
    IKeyListener *iconsumer_;
    char curline_[4096];
    std::string cmdLine_;
};

class ConsoleServiceClass : public IClass {
public:
    ConsoleServiceClass() : IClass("ConsoleServiceClass") {}

    virtual IService *createService(const char *obj_name) { 
        ConsoleService *serv = new ConsoleService(obj_name);
        AttributeType item(static_cast<IService *>(serv));
        listInstances_.add_to_list(&item);
        return serv;
    }
};

}  // namespace debugger

#endif  // __DEBUGGER_CONSOLE_H__
