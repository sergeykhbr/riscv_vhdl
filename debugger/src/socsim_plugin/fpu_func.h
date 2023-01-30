/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef __DEBUGGER_SRC_SOCSIM_PLUGIN_FPU_FUNC_H__
#define __DEBUGGER_SRC_SOCSIM_PLUGIN_FPU_FUNC_H__

#include <iclass.h>
#include <iservice.h>
#include <attribute.h>
#include "coreservices/icmdexec.h"

namespace debugger {

class FpuCmdType : public ICommand {
 public:
    FpuCmdType(IService *parent, const char *name) : ICommand(parent, name) {
        briefDescr_.make_string("Test environment for FPU model.");
        detailedDescr_.make_string(
            "Test supported:\n"
            "    <obj_name> test fdiv.d\n"
            "Usage:\n"
            "    fpu0 test fdiv.d\n"
            "    fpu0 test all");
    }

    /** ICommand */
    virtual int isValid(AttributeType *args);
    virtual void exec(AttributeType *args, AttributeType *res);
};

class FpuFunctional : public IService {
 public:
    FpuFunctional(const char *name);

    /** IService */
    virtual void postinitService() override;
    virtual void predeleteService() override;

    /** Potential IFpu methods */
    int FDIV_D(Reg64Type A, Reg64Type B, Reg64Type *fres);
    int FMUL_D(Reg64Type A, Reg64Type B, Reg64Type *fres, int &except);
    int FADD_D(int addEna, int subEna, int cmpEna, int moreEna, int lessEna,
               Reg64Type A, Reg64Type B,
               Reg64Type *fres,     // CMP = {29'd0, flMore, flEqual, flLess}
               int &except);
    int L2D_D(int signEna, int w32, Reg64Type A, Reg64Type B, Reg64Type *fres);
    int D2L_D(int signEna, int w32, Reg64Type A, Reg64Type B, Reg64Type *fres,
              int &ovr, int &und);

    /** Common methods */
    void setTestTotal(int v) { randomTestTotal_.make_int64(v); }
    void test_instr(const char *instr, AttributeType *res);

 protected:
    const int64_t BIT62 = 0x2000000000000000;
    const int64_t MSK61 = 0x1FFFFFFFFFFFFFFF;

    void div_stage(int inMuxEna,
                   int inMuxInd[],      // 7 bits (8 values)
                   int64_t inDivident,
                   int64_t inDivisor,
                   int64_t &outDif,
                   int &outBits,
                   int &outMuxInd,
                   int &outMuxIndRdy);

    void idiv53(int64_t inDivident,
                int64_t inDivisor,
                int *outResult,         // 106 bits value
                int &outShift,          // 7 bits value
                int &outOverBit,
                int &outZeroResid);

    void imul53(int64_t inA,            // 53 bits
                int64_t inB,            // 53 bits
                int *outResult,         // 106 bits value
                int &outShift,          // 7 bits value
                int &outOverBit);

 protected:
    AttributeType cmdexec_;
    AttributeType randomTestTotal_;

    ICmdExecutor *icmdexec_;
    FpuCmdType *pcmd_;

};

DECLARE_CLASS(FpuFunctional)

}  // namespace debugger

#endif  // __DEBUGGER_SRC_SOCSIM_PLUGIN_FPU_FUNC_H__
