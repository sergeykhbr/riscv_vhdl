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
#include "coreservices/iconsolelistener.h"
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
    virtual void registerConsoleListener(IFace *iface);

    /** IHap */
    virtual void hapTriggered(EHapType type);

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
    int getData();
    void clearLine();
    bool convertToWinKey(uint8_t symb);
    void addToCommandLine(int val);
    void processCommandLine();
    void addToHistory(const char *cmd);

private:
    AttributeType isEnable_;
    AttributeType stepQueue_;
    AttributeType signals_;
    AttributeType consoleListeners_;
    AttributeType logFile_;
    AttributeType serial_;
    AttributeType history_;
    AttributeType history_size_;

    event_def config_done_;
    mutex_def mutexConsoleOutput_;
    IClock *iclk_;
    char tmpbuf_[4096];
    std::string cmdLine_;
    std::string serial_input_;

    uint32_t symb_seq_;         // symbol sequence
    uint32_t symb_seq_msk_;
    // History switching
    std::string unfinshedLine_; // store the latest whe we look through history
    unsigned history_idx_;

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
