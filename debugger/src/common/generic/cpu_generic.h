/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
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

#ifndef __DEBUGGER_COMMON_CPU_GENERIC_H__
#define __DEBUGGER_COMMON_CPU_GENERIC_H__

#include <iclass.h>
#include <iservice.h>
#include <ihap.h>
#include <async_tqueue.h>
#include "coreservices/ithread.h"
#include "coreservices/icpugen.h"
#include "coreservices/icpufunctional.h"
#include "coreservices/imemop.h"
#include "coreservices/iclock.h"
#include "coreservices/ireset.h"
#include "coreservices/isrccode.h"
#include "coreservices/icmdexec.h"
#include "coreservices/itap.h"
#include "coreservices/icoveragetracker.h"
#include "generic/mapreg.h"
#include <fstream>

namespace debugger {

class GenericStatusType : public MappedReg64Type {
 public:
    GenericStatusType(IService *parent, const char *name, uint64_t addr) :
        MappedReg64Type(parent, name, addr) {
    }
 protected:
    virtual uint64_t aboutToRead(uint64_t cur_val) override;
    virtual uint64_t aboutToWrite(uint64_t new_val) override;
};

class FetchedBreakpointType : public MappedReg64Type {
 public:
    FetchedBreakpointType(IService *parent, const char *name, uint64_t addr)
        : MappedReg64Type(parent, name, addr) {
    }
 protected:
    virtual uint64_t aboutToWrite(uint64_t new_val) override;
};

class AddBreakpointType : public MappedReg64Type {
 public:
    AddBreakpointType(IService *parent, const char *name, uint64_t addr)
        : MappedReg64Type(parent, name, addr) {
    }
 protected:
    virtual uint64_t aboutToWrite(uint64_t new_val) override;
};

class RemoveBreakpointType : public MappedReg64Type {
 public:
    RemoveBreakpointType(IService *parent, const char *name, uint64_t addr)
        : MappedReg64Type(parent, name, addr) {
    }
 protected:
    virtual uint64_t aboutToWrite(uint64_t new_val) override;
};

class StepCounterType : public MappedReg64Type {
 public:
    StepCounterType(IService *parent, const char *name, uint64_t addr)
        : MappedReg64Type(parent, name, addr) {
    }
 protected:
    virtual uint64_t aboutToRead(uint64_t cur_val) override;
};

class FlushAddressType : public MappedReg64Type {
 public:
    FlushAddressType(IService *parent, const char *name, uint64_t addr)
        : MappedReg64Type(parent, name, addr) {
    }
 protected:
    virtual uint64_t aboutToWrite(uint64_t new_val) override;
};

class CpuGeneric : public IService,
                   public IThread,
                   public ICpuGeneric,
                   public ICpuFunctional,
                   public IClock,
                   public IPower,
                   public IResetListener,
                   public IHap {
 public:
    explicit CpuGeneric(const char *name);
    virtual ~CpuGeneric();

    /** IService interface */
    virtual void postinitService();

    /** ICpuGeneric interface */
    virtual void raiseSignal(int idx) = 0;
    virtual void lowerSignal(int idx) = 0;
    virtual void nb_transport_debug_port(DebugPortTransactionType *trans,
                                         IDbgNbResponse *cb);

    /** ICpuFunctional */
    virtual uint64_t getPC() { return pc_.getValue().val; }
    virtual void setBranch(uint64_t npc);
    virtual void pushStackTrace();
    virtual void popStackTrace();
    virtual uint64_t getPrvLevel() { return cur_prv_level; }
    virtual void setPrvLevel(uint64_t lvl) { cur_prv_level = lvl; }
    virtual ETransStatus dma_memop(Axi4TransactionType *tr);
    virtual void exceptionLoadInstruction(Axi4TransactionType *tr) {}
    virtual void exceptionLoadData(Axi4TransactionType *tr) {}
    virtual void exceptionStoreData(Axi4TransactionType *tr) {}
    virtual bool isOn() { return estate_ != CORE_OFF; }
    virtual bool isHalt() { return estate_ == CORE_Halted; }
    virtual bool isSwBreakpoint() { return sw_breakpoint_; }
    virtual bool isHwBreakpoint() { return hw_breakpoint_; }
    virtual void go();
    virtual void halt(const char *descr);
    virtual void step();
    virtual void addHwBreakpoint(uint64_t addr);
    virtual void removeHwBreakpoint(uint64_t addr);
    virtual void skipBreakpoint();
    virtual void flush(uint64_t addr);
    virtual void doNotCache(uint64_t addr) { do_not_cache_ = true; }
 protected:
    virtual uint64_t getResetAddress() { return resetVector_.to_uint64(); }
    virtual EEndianessType endianess() = 0;
    virtual GenericInstruction *decodeInstruction(Reg64Type *cache) = 0;
    virtual void generateIllegalOpcode() = 0;
    virtual void handleTrap() = 0;
    virtual void trackContextStart();
    virtual void trackContextEnd();
    virtual void traceRegister(int idx, uint64_t v);
    virtual void traceMemop(uint64_t addr, int we, uint64_t v, uint32_t sz);
    virtual void traceOutput() {}

 public:
    /** IClock */
    virtual uint64_t getStepCounter() { return step_cnt_; }
    virtual void registerStepCallback(IClockListener *cb, uint64_t t);
    virtual bool moveStepCallback(IClockListener *cb, uint64_t t);
    virtual double getFreqHz() {
        if (freqHz_.is_floating()) {
            return freqHz_.to_float();
        } else {
            return static_cast<double>(freqHz_.to_int64());
        }
    }

    /** IPower */
    virtual void power(EPowerAction onoff);

    /** IResetListener interface */
    virtual void reset(IFace *isource);

    /** IHap */
    virtual void hapTriggered(IFace *isrc, EHapType type, const char *descr);

 protected:
    /** IThread interface */
    virtual void busyLoop();

    virtual void updatePipeline();
    virtual bool updateState();
    virtual uint64_t fetchingAddress() { return pc_.getValue().val; }
    virtual void fetchILine();
    virtual void updateDebugPort();
    virtual void updateQueue();
    virtual bool checkHwBreakpoint();

 protected:
    AttributeType isEnable_;
    AttributeType freqHz_;
    AttributeType sysBus_;
    AttributeType dbgBus_;
    AttributeType cmdexec_;
    AttributeType tap_;
    AttributeType sysBusWidthBytes_;
    AttributeType sourceCode_;
    AttributeType stackTraceSize_;
    AttributeType generateTraceFile_;
    AttributeType resetVector_;
    AttributeType sysBusMasterID_;
    AttributeType hwBreakpoints_;
    AttributeType cacheBaseAddr_;
    AttributeType cacheAddrMask_;
    AttributeType coverageTracker_;
    AttributeType resetState_;

    ISourceCode *isrc_;
    ICoverageTracker *icovtracker_;
    ICmdExecutor *icmdexec_;
    ITap *itap_;
    IMemoryOperation *isysbus_;
    IMemoryOperation *idbgbus_;
    GenericInstruction *instr_;

    uint64_t step_cnt_;
    uint64_t hw_stepping_break_;
    bool branch_;
    unsigned oplen_;
    MappedReg64Type pc_;
    MappedReg64Type npc_;
    GenericStatusType status_;
    MappedReg64Type stepping_cnt_;
    StepCounterType clock_cnt_;
    StepCounterType executed_cnt_;
    MappedReg64Type stackTraceCnt_;         // Hardware stack trace buffer
    GenericReg64Bank stackTraceBuf_;        // [[from,to],*]
    MappedReg64Type br_control_;            // Enable/disable Trap on break
    MappedReg64Type br_fetch_addr_;         // Skip breakpoint at address
    FetchedBreakpointType br_fetch_instr_;  // Use this instruction on address
    FlushAddressType br_flush_addr_;        // Flush address from ICache
    AddBreakpointType br_hw_add_;
    RemoveBreakpointType br_hw_remove_;

    Reg64Type pc_z_;
    uint64_t interrupt_pending_[2];
    bool sw_breakpoint_;
    bool skip_sw_breakpoint_;
    bool hw_breakpoint_;
    uint64_t hw_break_addr_;    // Last hit breakpoint to skip it on next step
    bool do_not_cache_;         // Do not put instruction into ICache

    event_def eventConfigDone_;
    ClockAsyncTQueueType queue_;

    enum ECoreState {
        CORE_OFF,
        CORE_Halted,
        CORE_Normal,
        CORE_Stepping
    } estate_;

    Axi4TransactionType trans_;
    Reg64Type cacheline_[512/4];
    // Simple memory cache to avoid access to sysbus and speed-up simulation
    uint8_t *memcache_;             // unparsed instructions storage
    uint8_t *memcache_flag_;        // cached instruction length in bytes
    int memcache_sz_;               // allocated size
    uint64_t CACHE_BASE_ADDR_;
    uint64_t CACHE_MASK_;
    uint64_t fetch_addr_;
    uint64_t cache_offset_;         // instruction pointer - CACHE_BASE_ADDR
    bool cachable_pc_;              // fetched_pc hit into cachable region

    struct DebugPortType {
        bool valid;
        DebugPortTransactionType *trans;
        IDbgNbResponse *cb;
    } dport_;

    uint64_t cur_prv_level;

    struct trace_action_type {
        bool memop;             // 0=register; 1=memop
        int waddr;              // register addr
        uint64_t wdata;         // register data
        int memop_write;        // 0=read
        uint64_t memop_addr;
        Reg64Type memop_data;
        int memop_size;
    };

    struct trace_type {
        uint64_t step_cnt;
        uint64_t pc;
        uint32_t instr;
        char disasm[256];
        // 1 instruction several actions
        trace_action_type action[64];
        int action_cnt;
    } trace_data_;
    std::ofstream *trace_file_;
};

}  // namespace debugger

#endif  // __DEBUGGER_COMMON_CPU_GENERIC_H__
