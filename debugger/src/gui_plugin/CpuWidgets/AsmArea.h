/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Disassembler viewer area.
 */

#pragma once

#include "api_core.h"   // MUST BE BEFORE QtWidgets.h or any other Qt header.
#include "attribute.h"
#include "igui.h"
#include "iservice.h"
#include "coreservices/isocinfo.h"
#include "coreservices/isrccode.h"

#include <QtWidgets/QWidget>
#include <QtWidgets/QTableWidget>

namespace debugger {

/**
 * This class defines what address range need to request to properly show
 * disassembled code.
 */
class LinesViewModel {
public:
    LinesViewModel() : max_lines_(0), start_(0), pc_(0), end_(0),
                       req_start_(0), req_size_(0) {
    }

    void setMaxLines(int v) {
        max_lines_ = v;
    }

    void setPC(uint64_t pc) {
        if (pc == pc_) {
            return;
        }
        if (pc >= start_ && pc < end_) {
            uint64_t center = (start_ + end_) / 2;
            center &= ~0x3;
            if (pc <= center) {
                req_start_ = 0;
                req_size_ = 0;
            } else {
                req_start_ = end_;
                req_size_ = pc - center;
                start_ += req_size_;
                end_ += req_size_;
            }
        } else {
            start_ = pc;
            req_start_ = pc;
            req_size_ = 4 * max_lines_;
            end_ = start_ + req_size_;
        }
        pc_ = pc;
    }

    uint64_t getPC() { return pc_; }

    void updateAddr(uint64_t addr) {
        if (req_size_ != 0 || addr < start_ || addr >= end_) {
            return;
        }
        req_start_ = addr;
        req_size_ = 4;
    }

    void updateRange() {
        req_start_ = 0;
        req_size_ = 0;
    }

    uint64_t getReqAddr() { return req_start_; }
    int getReqSize() { return req_size_; }

private:
    int max_lines_;
    uint64_t start_;
    uint64_t pc_;
    uint64_t end_;
    uint64_t req_start_;
    int req_size_;
};

class AsmArea : public QTableWidget,
                public IGuiCmdHandler {
    Q_OBJECT
public:
    explicit AsmArea(IGui *gui, QWidget *parent);
    virtual ~AsmArea();

    /** IGuiCmdHandler */
    virtual void handleResponse(AttributeType *req, AttributeType *resp);

signals:
    void signalHandleResponse();

public slots:
    void slotPostInit(AttributeType *cfg);
    void slotUpdateByTimer();
    void slotBreakpoint();
    void slotHandleResponse();
    void slotCellDoubleClicked(int row, int column);

private:
    void outLines();
    void outLine(int idx, AttributeType &data);
    void data2lines(uint64_t addr, AttributeType &resp, AttributeType &lines);
    bool addrIsBreakpoint(uint64_t addr, uint8_t *instr, AttributeType *obr);
    int makeAsmAttr(uint64_t addr, uint8_t *data, int offset,
                    AttributeType &out);

    class BreakpointInjectPort : public IGuiCmdHandler {
    public:
        BreakpointInjectPort(IGui *gui, ISourceCode *src) {
            igui_ = gui;
            isrc_ = src;
            readNpc_.make_string("reg npc");
            ISocInfo *isoc = static_cast<ISocInfo *>(igui_->getSocInfo());
            DsuMapType *dsu = isoc->getpDsu();
            dsu_sw_br_ =
                reinterpret_cast<uint64_t>(&dsu->udbg.v.br_address_fetch);
            dsu_hw_br_ =
                reinterpret_cast<uint64_t>(&dsu->udbg.v.remove_breakpoint);
        }

        /** IGuiCmdHandler */
        virtual void handleResponse(AttributeType *req, AttributeType *resp) {
            char tstr[128];
            AttributeType memWrite;
            if (!resp->is_integer()) {
                return;
            }
            uint64_t br_addr = resp->to_uint64();
            uint32_t br_instr = 0;
            bool br_hw;
            isrc_->getBreakpointList(&brList_);
            for (unsigned i = 0; i < brList_.size(); i++) {
                const AttributeType &br = brList_[i];
                if (br_addr == br[0u].to_uint64()) {
                    br_instr = br[1].to_int();
                    br_hw = br[2].to_bool();
                    break;
                }
            }
            if (br_instr == 0) {
                return;
            }
            if (br_hw) {
                RISCV_sprintf(tstr, sizeof(tstr),
                        "write 0x%08" RV_PRI64 "x 8 0x%" RV_PRI64 "x",
                        dsu_hw_br_, br_addr);
            } else {
                RISCV_sprintf(tstr, sizeof(tstr),
                        "write 0x%08" RV_PRI64 "x 16 [0x%" RV_PRI64 "x,0x%x]",
                        dsu_sw_br_, br_addr, br_instr);
            }
            memWrite.make_string(tstr);
            igui_->registerCommand(NULL, &memWrite, true);
        }

        /** Write address and instruction into fetcher to skip EBREAK once */
        void injectFetch() {
            igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                                    &readNpc_, true);
        }

    private:
        AttributeType readNpc_;
        AttributeType brList_;
        IGui *igui_;
        ISourceCode *isrc_;
        uint64_t dsu_sw_br_;
        uint64_t dsu_hw_br_;
    }; 

private:
    AttributeType cmdReadMem_;
    AttributeType cmdRegs_;
    AttributeType asmLines_;
    QString name_;
    IGui *igui_;
    ISourceCode *isrc_;

    enum EColumnNames {
        COL_addrline,
        COL_code,
        COL_label,
        COL_mnemonic,
        COL_comment,
        // not present in view
        COL_codesize,
        COL_breakpoint,
        COL_Total
    };

    enum ECmdState {
        CMD_idle,
        CMD_npc,
        CMD_memdata,
    };

    AttributeType data_;
    AttributeType tmpBuf_;
    AttributeType dataText_;
    AttributeType brList_;
    AttributeType addrBrAddrFetch_;

    int lineHeight_;
    ECmdState state_;
    LinesViewModel rangeModel_;
    BreakpointInjectPort *portBreak_;
};

}  // namespace debugger
