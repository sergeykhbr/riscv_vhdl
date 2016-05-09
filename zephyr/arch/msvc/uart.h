#pragma once

#include "imapdev.h"

class Uart : public IMappedDevice {
public:
    Uart() {
        rx_cnt_ = 0;
        pwr_ = buf_;
        prd_ = buf_;
    }

    virtual bool isAddrValid(uint64_t addr) {
        return (addr >= 0x80001000 && addr < 0x80002000);
    }
    virtual void write(uint64_t addr, uint8_t *buf, int size) {
        uint64_t off = addr - 0x80001000;
        switch (off) {
        case 0x00:
            std::cout << (char)buf[0];
            break;
        default:;
        }

    }
    virtual void read(uint64_t addr, uint8_t *buf, int size) {
        uint64_t off = addr - 0x80001000;
        uint32_t tmp = 0;
        switch (off) {
        case 0x00:
            buf[0] = *prd_;
            if (rx_cnt_) {
                rx_cnt_--;
                if ((++prd_) >= &buf_[FIFO_SIZE]) {
                    prd_ = buf_;
                }
            }
            break;
        case 0x4:
            if (rx_cnt_ == 0) {
                tmp |= UART_STATUS_RX_EMPTY;
            }
            *(uint32_t *)buf = tmp;
            break;
        default:;
        }
    }

    void putRx(char s) {
        *pwr_ = *(pwr_+FIFO_SIZE) = s;
        if ((++pwr_) >= &buf_[FIFO_SIZE]) {
            pwr_ = buf_;
        }
        rx_cnt_++;
    }

private:
    static const uint32_t UART_STATUS_TX_FULL     = 0x00000001;
    static const uint32_t UART_STATUS_TX_EMPTY    = 0x00000002;
    static const uint32_t UART_STATUS_RX_FULL     = 0x00000010;
    static const uint32_t UART_STATUS_RX_EMPTY    = 0x00000020;
    static const uint32_t UART_STATUS_ERR_PARITY  = 0x00000100;
    static const uint32_t UART_STATUS_ERR_STOPBIT = 0x00000200;


    static const int FIFO_SIZE = 64;
    char buf_[2*FIFO_SIZE];
    char *prd_, *pwr_;
    int rx_cnt_;
};

