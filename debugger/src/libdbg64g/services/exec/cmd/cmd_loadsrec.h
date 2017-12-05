/**
 * @file
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      SREC-file loader command.
 */

#ifndef __DEBUGGER_CMD_LOADSREC_H__
#define __DEBUGGER_CMD_LOADSREC_H__

#include "api_core.h"
#include "coreservices/itap.h"
#include "coreservices/isocinfo.h"
#include "coreservices/icommand.h"

namespace debugger {

class CmdLoadSrec : public ICommand  {
public:
    explicit CmdLoadSrec(ITap *tap, ISocInfo *info);

    /** ICommand interface */
    virtual bool isValid(AttributeType *args);
    virtual void exec(AttributeType *args, AttributeType *res);

private:
    uint8_t str2byte(uint8_t *pair);
    bool check_crc(uint8_t *str, int sz);
    int check_header(uint8_t *img);
    int readline(uint8_t *img, int off,
                 uint64_t &addr, int &sz, uint8_t *out);

private:
    char header_data_[1024];
};

}  // namespace debugger

#endif  // __DEBUGGER_CMD_LOADSREC_H__
