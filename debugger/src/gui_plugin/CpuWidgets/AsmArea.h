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
#include <QtGui/QResizeEvent>

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

    void insrease(int delta) {
        if (delta >= 0) {
            if (start_ >= 16) {
                req_start_ = start_ - 16;
                req_size_ = 16;
                start_ -= 16;
            } else if (start_ > 0) {
                req_start_ = 0;
                req_size_ = start_;
                start_ = 0;
            }
        } else {
            req_start_ = end_;
            req_size_ = 16;
            end_ += 16;
        }
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
    void signalNpcChanged(uint64_t npc);
    void signalAsmListChanged();

public slots:
    void slotPostInit(AttributeType *cfg);
    void slotNpcChanged(uint64_t npc);
    void slotAsmListChanged();
    void slotUpdateByTimer();
    void slotBreakpoint();
    void slotCellDoubleClicked(int row, int column);

protected:
    void resizeEvent(QResizeEvent *ev) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent * ev) Q_DECL_OVERRIDE;

private:
    void outLines();
    void outLine(int idx, AttributeType &data);
    void addMemBlock(AttributeType &resp, AttributeType &lines);
    // Ports:

    class BreakpointInjectPort : public IGuiCmdHandler {
    public:
        BreakpointInjectPort(IGui *gui) {
            igui_ = gui;
            readBr_.make_string("br");
            readNpc_.make_string("reg npc");
            dsu_sw_br_ = ~0;
            dsu_hw_br_ = ~0;
            is_waiting_ = false;
        }

        void setBrAddressFetch(uint64_t addr) { dsu_sw_br_ = addr; }
        void setHwRemoveBreakpoint(uint64_t addr) { dsu_hw_br_ = addr; }

        /** IGuiCmdHandler */
        virtual void handleResponse(AttributeType *req, AttributeType *resp);

        /** Write address and instruction into fetcher to skip EBREAK once */
        void injectFetch();

    private:
        AttributeType readBr_;
        AttributeType readNpc_;
        AttributeType brList_;
        IGui *igui_;
        uint64_t dsu_sw_br_;
        uint64_t dsu_hw_br_;
        bool is_waiting_;
    }; 

private:
    enum EColumnNames {
        COL_addrline,
        COL_code,
        COL_label,
        COL_mnemonic,
        COL_comment,
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

    int selRowIdx;
    int hideLineIdx_;
    int lineHeight_;
    ECmdState state_;
    LinesViewModel rangeModel_;
    BreakpointInjectPort *portBreak_;
    mutex_def mutexAsmGaurd_;
};

}  // namespace debugger
