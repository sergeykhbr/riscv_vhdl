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
#include "iboardsim.h"

namespace debugger {

class BoardSim : public IService, 
                 public IBoardSim {
public:
    BoardSim(const char *name);

    /** IService interface */
    virtual void postinitService();

    /** @name IBoardSim interface */
    virtual void getInfo(AttributeType *attr) {}

private:
};

DECLARE_CLASS(BoardSim)

}  // namespace debugger

#endif  // __DEBUGGER_BOARDSIM_H__
