/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Input comport class declaration.
 */

#ifndef __DEBUGGER_COMPORT_H__
#define __DEBUGGER_COMPORT_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/ithread.h"
#include "coreservices/iserial.h"
#include "coreservices/irawlistener.h"
#include <string>
//#define DBG_ZEPHYR


namespace debugger {

class ComPortService : public IService,
                       public IThread,
                       public ISerial,
                       public IRawListener {
public:
    explicit ComPortService(const char *name);
    virtual ~ComPortService();

    /** IService interface */
    virtual void postinitService();

    /** ISerial */
    virtual int writeData(const char *buf, int sz);
    virtual void registerRawListener(IFace *listener);
    virtual void unregisterRawListener(IFace *listener);

    // IRawListener
    virtual void updateData(const char *buf, int buflen);

protected:
    /** IThread interface */
    virtual void busyLoop();

private:
    void getSerialPortList(AttributeType *list);
    int openSerialPort(const char *port, int baud, void *hdl);
    void closeSerialPort(void *hdl);
    int readSerialPort(void *hdl, char *buf, int bufsz);
    int writeSerialPort(void *hdl, char *buf, int bufsz);
    void cleanSerialPort(void *hdl);

private:
    AttributeType isEnable_;
    AttributeType uartSim_;
    AttributeType logFile_;
    AttributeType comPortName_;
    AttributeType comPortSpeed_;
    AttributeType portListeners_;

    FILE *logfile_;
#if defined(_WIN32) || defined(__CYGWIN__)
    HANDLE hPort_;
#else
    int hPort_;
#endif

    bool isSimulation_;
    bool portOpened_;
    ISerial *iuartSim_;

    class SimpleFifoType {
    public:
        SimpleFifoType() {
            wr_ = &arr_[1];
            rd_ = &arr_[0];
        }
        bool isEmpty() {
            return ((wr_ - rd_) == 1) 
                || (wr_ == arr_ && rd_ == &arr_[FIFO_SZ - 1]);
        }
        bool isFull() {
            return wr_ == rd_;
        }
        void put(char v) {
            if (isFull()) {
                return;
            }
            *wr_ = v;
            if ((++wr_) == &arr_[FIFO_SZ]) {
                wr_ = arr_;
            }
        }
        char get() {
            if (isEmpty()) {
                return 0;
            }
            if ((++rd_) == &arr_[FIFO_SZ]) {
                rd_ = arr_;
            }
            return *rd_;
        }
    private:
        static const int FIFO_SZ = 4096;
        char arr_[FIFO_SZ];
        char *rd_, *wr_;
    };
    SimpleFifoType txFifo_;
    SimpleFifoType rxFifo_;
};

DECLARE_CLASS(ComPortService)

}  // namespace debugger

#endif  // __DEBUGGER_COMPORT_H__
