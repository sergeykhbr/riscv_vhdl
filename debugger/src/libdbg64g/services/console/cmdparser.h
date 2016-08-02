/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Simplified input command string parser.
 */

#ifndef __DEBUGGER_CMDPARSER_H__
#define __DEBUGGER_CMDPARSER_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/iconsole.h"
#include "coreservices/ikeylistener.h"
#include "coreservices/iconsolelistener.h"
#include "coreservices/itap.h"
#include "coreservices/ielfloader.h"
#include <string>
#include <stdarg.h>

namespace debugger {

class CmdParserService : public IService,
                         public IConsoleListener {
public:
    explicit CmdParserService(const char *name);
    virtual ~CmdParserService();

    /** IService interface */
    virtual void postinitService();

    /** IConsoleListener */
    virtual void udpateCommand(const char *line);
    virtual void autocompleteCommand(const char *line) {}

private:
    static const uint64_t DSU_BASE_ADDRESS = 0x80080000;
    // Valid only for simulation.
    static const uint64_t DSU_CTRL_BASE_ADDRESS = 0x80090000;

    void processLine(const char *line);
    void splitLine(char *str, AttributeType *listArgs);

    /**
     * Read CSR MCPUID value
     *      DSU Base address 0x80080000
     *      Each CSR register value has 128-bits alignment.
     */
    uint64_t csr_socaddr(AttributeType *id) {
        if (id->is_integer()) {
            return DSU_BASE_ADDRESS + (id->to_uint64() << 4);
        }
        std::string tmp;
        for (unsigned i = 0; i < listCSR_.size(); i++) {
            if (strcmp(id->to_upper(), listCSR_[i][0u].to_string()) == 0) {
                return DSU_BASE_ADDRESS + (listCSR_[i][1u].to_uint64() << 4);
            }
        }
        return ~0;
    }
    void loadElf(AttributeType *listArgs);
    void readCSR(AttributeType *listArgs);
    void writeCSR(AttributeType *listArgs);
    void readMem(AttributeType *listArgs);
    void writeMem(AttributeType *listArgs);
    void memDump(AttributeType *listArgs);

    // Only simulation platform supports these commands (for now):
    void halt(AttributeType *listArgs);
    void run(AttributeType *listArgs);
    void regs(AttributeType *listArgs);
    void br(AttributeType *listArgs);
    unsigned getRegIDx(const char *name);

    int outf(const char *fmt, ...);

private:
    AttributeType console_;
    AttributeType tap_;
    AttributeType loader_;
    // Store each CSR as list: ['Name',<address>,[description], others]
    AttributeType listCSR_;
    AttributeType regNames_;

    AttributeType iconsoles_;
    ITap *itap_;
    IElfLoader *iloader_;
    std::string cmdLine_;
    char cmdbuf_[4096];
    char *outbuf_;
    int outbuf_size_;
    int outbuf_cnt_;
    uint8_t *tmpbuf_;
    int tmpbuf_size_;
};

DECLARE_CLASS(CmdParserService)

}  // namespace debugger

#endif  // __DEBUGGER_CMDPARSER_H__
