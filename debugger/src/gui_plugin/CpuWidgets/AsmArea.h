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
    // Ports:
    class BreakpointListPort : public IGuiCmdHandler {
    public:
        BreakpointListPort(IGui *gui, AttributeType *list)
            : igui_(gui), p_brList_(list) {
            cmdBr_.make_string("br");
        }
        /** IGuiCmdHandler */
        virtual void handleResponse(AttributeType *req, AttributeType *resp);

        void update() {
            igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                                    &cmdBr_, true);
        }
    private:
        IGui *igui_;
        AttributeType *p_brList_;
        AttributeType cmdBr_;
    };

    class BreakpointInjectPort : public IGuiCmdHandler {
    public:
        BreakpointInjectPort(IGui *gui, AttributeType *list) {
            igui_ = gui;
            p_brList_ = list;
            readNpc_.make_string("reg npc");
            dsu_sw_br_ = ~0;
            dsu_hw_br_ = ~0;
        }

        void setBrAddressFetch(uint64_t addr) { dsu_sw_br_ = addr; }
        void setHwRemoveBreakpoint(uint64_t addr) { dsu_hw_br_ = addr; }

        /** IGuiCmdHandler */
        virtual void handleResponse(AttributeType *req, AttributeType *resp);

        /** Write address and instruction into fetcher to skip EBREAK once */
        void injectFetch() {
            igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                                    &readNpc_, true);
        }

    private:
        AttributeType readNpc_;
        AttributeType *p_brList_;
        IGui *igui_;
        uint64_t dsu_sw_br_;
        uint64_t dsu_hw_br_;
    }; 

private:
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

    AttributeType cmdReadMem_;
    AttributeType cmdRegs_;
    AttributeType asmLines_;
    QString name_;
    IGui *igui_;
    ISourceCode *isrc_;

    AttributeType data_;
    AttributeType tmpBuf_;
    AttributeType dataText_;
    AttributeType brList_;

    int lineHeight_;
    ECmdState state_;
    LinesViewModel rangeModel_;
    BreakpointListPort *portBrList_;
    BreakpointInjectPort *portBreak_;
};

}  // namespace debugger
