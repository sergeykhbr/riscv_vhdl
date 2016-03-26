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
#include "coreservices/itap.h"
#include "coreservices/ielfloader.h"
#include <string>
#include <stdarg.h>

namespace debugger {

class CmdParserService : public IService,
                         public IKeyListener {
public:
    explicit CmdParserService(const char *name);
    virtual ~CmdParserService();

    /** IService interface */
    virtual void postinitService();

    /** IKeyListener interface */
    virtual int keyDown(int value) { return 0; }
    virtual int keyUp(int value);

private:
    static const uint64_t DSU_BASE_ADDRESS = 0x80080000;

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
        for (unsigned i = 0; i < listCSR_.size(); i++) {
            if (strcmp(id->to_string(), listCSR_[i][0u].to_string()) == 0) {
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
    void regs(AttributeType *listArgs);

    int outf(const char *fmt, ...);
    void addToHistory(const char *cmd);

    bool convertToWinKey(uint8_t symb);

private:
    AttributeType console_;
    AttributeType tap_;
    AttributeType loader_;
    AttributeType history_;
    // Store each CSR as list: ['Name',<address>,[description], others]
    AttributeType listCSR_;

    IConsole *iconsole_;
    ITap *itap_;
    IElfLoader *iloader_;
    std::string cmdLine_;
    std::string unfinshedLine_; // store the latest whe we look through history
    uint32_t symb_seq_;         // symbol sequence
    uint32_t symb_seq_msk_;
    char cmdbuf_[4096];
    char outbuf_[4096];
    int outbuf_cnt_;
    uint8_t tmpbuf_[4096];
    unsigned history_idx_;
};

class CmdParserServiceClass : public IClass {
public:
    CmdParserServiceClass() : IClass("CmdParserServiceClass") {}

    virtual IService *createService(const char *obj_name) { 
        CmdParserService *serv = new CmdParserService(obj_name);
        AttributeType item(static_cast<IService *>(serv));
        listInstances_.add_to_list(&item);
        return serv;
    }
};

}  // namespace debugger

#endif  // __DEBUGGER_CMDPARSER_H__
