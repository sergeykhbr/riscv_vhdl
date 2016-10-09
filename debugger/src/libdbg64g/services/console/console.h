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
#include "ihap.h"
#include "coreservices/ithread.h"
#include "coreservices/iconsole.h"
#include "coreservices/iserial.h"
#include "coreservices/iclock.h"
#include "coreservices/iautocomplete.h"
#include "coreservices/icmdexec.h"
#include "coreservices/irawlistener.h"
#include "coreservices/isignallistener.h"
#include <string>
//#define DBG_ZEPHYR


namespace debugger {

class ConsoleService : public IService,
                       public IThread,
                       public IConsole,
                       public IHap,
                       public IRawListener,
                       public ISignalListener,
                       public IClockListener {
public:
    explicit ConsoleService(const char *name);
    virtual ~ConsoleService();

    /** IService interface */
    virtual void postinitService();
    virtual void predeleteService();

    /** IConsole interface */
    virtual void writeBuffer(const char *buf);
    virtual void enableLogFile(const char *filename);

    /** IHap */
    virtual void hapTriggered(IFace *isrc, EHapType type, const char *descr);

    /** IRawListener (serial) */
    virtual void updateData(const char *buf, int buflen);

    /** IClockListener */
    virtual void stepCallback(uint64_t t);

    /** ISignalListener */
    virtual void updateSignal(int start, int width, uint64_t value);

protected:
    /** IThread interface */
    virtual void busyLoop();

private:
    void processScriptFile();
    bool isData();
    uint8_t getData();
    void clearLine(int num);
    void processCommandLine();

    class RawPortType : public IRawListener {
    public:
        RawPortType(ConsoleService *parent, const char *name) {
            parent_ = parent;
            name_.make_string(name);
        }

        // Fake IService method:
        virtual IFace *getInterface(const char *name) {
            if (strcmp(name, IFACE_RAW_LISTENER) == 0) {
                return static_cast<IRawListener *>(this);
            } else {
                return parent_->getInterface(name);
            }
        }
        // IRawListener
        virtual void updateData(const char *buf, int buflen) {
            std::string outdata;
            if (name_.size()) {
                outdata = "<";
                outdata += std::string(name_.to_string());
                outdata = "> ";
            }
            outdata += std::string(buf);
            parent_->writeBuffer(outdata.c_str());
        }

    private:
        ConsoleService *parent_;
        AttributeType name_;
    };

private:
    AttributeType isEnable_;
    AttributeType stepQueue_;
    AttributeType autoComplete_;
    AttributeType commandExecutor_;
    AttributeType signals_;
    AttributeType logFile_;
    AttributeType inPort_;

    event_def config_done_;
    mutex_def mutexConsoleOutput_;
    IClock *iclk_;
    IAutoComplete *iautocmd_;
    ICmdExecutor *iexec_;

    RawPortType portExecutor;

    char tmpbuf_[4096];
    std::string cmdLine_;
    std::string serial_input_;
    unsigned cmdSizePrev_;  // used to clear symbols if string shorter 
                            // than previous

    FILE *logfile_;
#ifdef DBG_ZEPHYR
    int tst_cnt_;
#endif
#if defined(_WIN32) || defined(__CYGWIN__)
#else
    struct termios original_settings_;
    int term_fd_;
#endif
};

DECLARE_CLASS(ConsoleService)

}  // namespace debugger

#endif  // __DEBUGGER_CONSOLE_H__
