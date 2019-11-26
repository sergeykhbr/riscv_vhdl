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
    virtual void predeleteService();

    /** ISerial */
    virtual int writeData(const char *buf, int sz);
    virtual void registerRawListener(IFace *listener);
    virtual void unregisterRawListener(IFace *listener);
    virtual void getListOfPorts(AttributeType *list);
    virtual int openPort(const char *port, AttributeType settings);
    virtual void closePort();

    /** IRawListener (simulation only) */
    virtual int updateData(const char *buf, int buflen);

protected:
    /** IThread interface */
    virtual void busyLoop();

private:
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
    HANDLE prtHandler_;
#else
    int prtHandler_;
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
    mutex_def mutexListeners_;
};

DECLARE_CLASS(ComPortService)

}  // namespace debugger

#endif  // __DEBUGGER_COMPORT_H__
