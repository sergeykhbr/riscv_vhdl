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
#include "coreservices/icpufunctional.h"
#include "coreservices/idport.h"
#include "coreservices/imemop.h"
#include "coreservices/iclock.h"
#include "coreservices/ireset.h"
#include "coreservices/isrccode.h"
#include "coreservices/icmdexec.h"
#include "coreservices/icoveragetracker.h"
#include "generic/mapreg.h"
#include <riscv-isa.h>
#include <fstream>

namespace debugger {

class CpuGeneric : public IService,
                   public IThread,
                   public ICpuFunctional,
                   public IDPort,
                   public IClock,
                   public IPower,
                   public IResetListener,
                   public IHap {
 public:
    explicit CpuGeneric(const char *name);
    virtual ~CpuGeneric();

    /** IService interface */
    virtual void postinitService();

    /** ICpuFunctional */
    virtual uint64_t *getpRegs() { return R; }
    virtual uint64_t getPC() { return *PC_; }
    virtual void setPC(uint64_t v) { *PC_ = v; }
    virtual uint64_t getNPC() { return *NPC_; }
    virtual void setNPC(uint64_t v) { *NPC_ = v; }
    virtual void setReg(int idx, uint64_t val);
    virtual void enterDebugMode(uint64_t v, uint32_t cause) {}
    virtual void setBranch(uint64_t npc);
    virtual void pushStackTrace();
    virtual void popStackTrace();
    virtual uint64_t getPrvLevel() { return cur_prv_level; }
    virtual void setPrvLevel(uint64_t lvl) { cur_prv_level = lvl; }
    virtual ETransStatus dma_memop(Axi4TransactionType *tr, int flags=0);
    virtual void generateException(int e, uint64_t arg) { exceptions_ |= 1ull << e; }
    virtual void generateExceptionLoadInstruction(uint64_t addr) {}
    virtual bool isOn() { return estate_ != CORE_OFF; }
    virtual void resume();
    virtual void halt(uint32_t cause, const char *descr);
    virtual void flush(uint64_t addr);
    virtual void doNotCache(uint64_t addr) { do_not_cache_ = true; }
    virtual bool isMpuEnabled() { return false; }
    virtual bool checkMpu(uint64_t addr, uint32_t sz, const char *rwx) { return true; }
    virtual bool isMmuEnabled() { return false; }
    virtual uint64_t translateMmu(uint64_t addr) { return addr; }
    virtual void flushMmu() {}

    /** IDPort interface */
    virtual int resumereq();
    virtual int haltreq();
    virtual bool isHalted() {
        return estate_ == CORE_OFF || estate_ == CORE_Halted;
    }
    virtual bool isResumeAck() {
        return resumeack_;
    }
    virtual void setHaltOnReset() { resetState_.make_string("Halted"); }
    virtual void clrHaltOnReset() { resetState_.make_string("Run"); }
    virtual int dportReadReg(uint32_t regno, uint64_t *val) { return -1; }
    virtual int dportWriteReg(uint32_t regno, uint64_t val) { return -1; }
    virtual int dportReadMem(uint64_t addr, uint32_t virt, uint32_t sz, uint64_t *payload) { return -1; }
    virtual int dportWriteMem(uint64_t addr, uint32_t virt, uint32_t sz, uint64_t payload) { return -1; }
    virtual bool executeProgbuf(uint32_t *progbuf);
    virtual bool isExecutingProgbuf() { return estate_ == CORE_ProgbufExec; }
    virtual void setResetPin(bool val) {}


 protected:
    virtual uint64_t getResetAddress() { return resetVector_.to_uint64(); }
    virtual EEndianessType endianess() = 0;
    virtual GenericInstruction *decodeInstruction(Reg64Type *cache) = 0;
    virtual void generateIllegalOpcode() = 0;
    virtual void checkStackProtection() {}
    virtual void handleTrap();
    virtual void handleException(int e) = 0;
    virtual void handleInterrupts() = 0;
    virtual void trackContextStart();
    virtual void trackContextEnd();
    virtual void traceRegister(int idx, uint64_t v);
    virtual void traceMemop(uint64_t addr, int we, uint64_t v, uint32_t sz);
    virtual void traceOutput() {}
    virtual bool isStepEnabled() { return false; }
    virtual bool isTriggerICount();
    virtual bool isTriggerInstruction();

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
    virtual void hapTriggered(EHapType type, uint64_t param,
                              const char *descr);

 protected:
    /** IThread interface */
    virtual void busyLoop();

    virtual void updatePipeline();
    virtual bool updateState();
    virtual uint64_t fetchingAddress() { return getPC(); }
    virtual void fetchILine();
    virtual void updateQueue();
    virtual void enterProgbufExec();
    virtual void exitProgbufExec();

 protected:
    AttributeType isEnable_;
    AttributeType freqHz_;
    AttributeType sysBus_;
    AttributeType cmdexec_;
    AttributeType dmibar_;
    AttributeType sysBusWidthBytes_;
    AttributeType sourceCode_;
    AttributeType stackTraceSize_;
    AttributeType generateTraceFile_;
    AttributeType resetVector_;
    AttributeType sysBusMasterID_;
    AttributeType cacheBaseAddr_;
    AttributeType cacheAddrMask_;
    AttributeType coverageTracker_;
    AttributeType resetState_;
    AttributeType triggersTotal_;
    AttributeType mcontrolMaskmax_;

    ISourceCode *isrc_;
    ICoverageTracker *icovtracker_;
    ICmdExecutor *icmdexec_;
    IMemoryOperation *isysbus_;
    GenericInstruction *instr_;

    // DCSR register halt causes:
    static const uint64_t HALT_CAUSE_EBREAK       = 1;  // software breakpoint
    static const uint64_t HALT_CAUSE_TRIGGER      = 2;  // hardware breakpoint
    static const uint64_t HALT_CAUSE_HALTREQ      = 3;  // halt request via debug interface
    static const uint64_t HALT_CAUSE_STEP         = 4;  // step done
    static const uint64_t HALT_CAUSE_RESETHALTREQ = 5;  // not implemented

    enum EContextTypes {
        Ctx_Normal,
        Ctx_ProgbufExec,
        Ctx_Total
    };

    struct ContextRegistersType {
        Reg64Type pc;
        Reg64Type npc;
    } ctxregs_[Ctx_Total];

    enum ETriggerType {
        TriggerType_NoTrigger = 0,
        TriggerType_LegacySiFive = 1,
        TriggerType_AddrDataMatch = 2,
        TriggerType_InstrCountMatch = 3,
        TriggerType_Inetrrupt = 4,
        TriggerType_Exception = 5,
        TriggerType_NotAvailable = 15,
    };

    struct TriggerStorageType {
        TriggerData1Type data1;
        uint64_t data2;
        uint64_t extra;
    } *ptriggers_;

    uint64_t step_cnt_;
    volatile bool resumereq_;
    volatile bool resumeack_;
    volatile bool haltreq_;
    volatile bool procbufexecreq_;
    bool branch_;
    unsigned oplen_;
    uint64_t *R;                            // Pointer to register bank
    uint64_t *PC_;
    uint64_t *NPC_;
    uint32_t *progbuf_;

    GenericReg64Bank portCSR_;      // Not mapped since moved to DMI, just a storage
    GenericReg64Bank portRegs_;
    MappedReg64Type stackTraceCnt_;         // Hardware stack trace buffer
    GenericReg64Bank stackTraceBuf_;        // [[from,to],*]
    
    uint64_t pc_z_;
    uint64_t exceptions_;
    uint64_t interrupt_pending_[2];
    bool do_not_cache_;         // Do not put instruction into ICache

    mutex_def mutex_csr_;
    event_def eventConfigDone_;
    event_def eventDbgRequest_;
    ClockAsyncTQueueType queue_;

    enum ECoreState {
        CORE_OFF,
        CORE_Halted,
        CORE_Normal,
        CORE_ProgbufExec
    } estate_;

    Axi4TransactionType trans_;
    Reg64Type cacheline_[512/4];
    
    // Simple memory cache to avoid access to sysbus and speed-up simulation
    struct ICacheType {
        GenericInstruction *instr;
        uint32_t buf;
    } *icache_;            // parsed instructions storage
    int memcache_sz_;               // allocated size
    uint64_t CACHE_BASE_ADDR_;
    uint64_t CACHE_MASK_;
    uint64_t fetch_addr_;
    uint64_t cache_offset_;         // instruction pointer - CACHE_BASE_ADDR
    bool cachable_pc_;              // fetched_pc hit into cachable region

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
        AttributeType instrbuf;
        AttributeType asmlist;
        // 1 instruction several actions
        trace_action_type action[64];
        int action_cnt;
    } trace_data_;
    std::ofstream *trace_file_;
};

}  // namespace debugger

#endif  // __DEBUGGER_COMMON_CPU_GENERIC_H__
