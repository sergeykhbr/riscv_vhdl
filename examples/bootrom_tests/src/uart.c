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

#include <inttypes.h>
#include <string.h>
#include <stdarg.h>
#include <axi_maps.h>
#include "fw_api.h"

//#define UART_BUF_ENABLE

static const char UART0_NAME[8] = "uart0";
#ifdef UART_BUF_ENABLE
static const int UART_BUF_SZ = 4096;
#endif

typedef void (*f_putch)(int, void**);


typedef struct uart_data_type {
    char *fifo;
    unsigned digs[32];

    volatile int wrcnt;
    volatile int rdcnt;
} uart_data_type;

#ifdef UART_BUF_ENABLE
static int buf_total(uart_data_type *p) {
    int total = p->wrcnt - p->rdcnt;
    if (total <= 0) {
        total += UART_BUF_SZ;
    }
    return total - 1;
}

static uint8_t buf_get(uart_data_type *p) {
    if (++p->rdcnt >= UART_BUF_SZ) {
        p->rdcnt = 0;
    }
    return (uint8_t)p->fifo[p->rdcnt];
}

void buf_put(uart_data_type *p, char s) {
    if (p->wrcnt == p->rdcnt) {
        return;
    }
    p->fifo[p->wrcnt] = s;
    if (++p->wrcnt >= UART_BUF_SZ) {
        p->wrcnt = 0;
    }
}
#endif

void isr_uart0_tx() {
    uart_data_type *pdata = fw_get_ram_data(UART0_NAME);
    uart_map *uart = (uart_map *)ADDR_BUS0_XSLV_UART0;
#ifdef UART_BUF_ENABLE    
    uart_txdata_type txdata;

    while (buf_total(pdata)) {
        txdata.v = uart->txdata;
        if (txdata.b.full) {
             break;
        }
        uart->txdata = buf_get(pdata);
    }
#endif
}

#ifdef UART_BUF_ENABLE    
int uart_tx_nempty() {
    uart_data_type *pdata = fw_get_ram_data(UART0_NAME);
    return buf_total(pdata);
}
#endif

void uart_isr_init(void) {
    uart_map *uart = (uart_map *)ADDR_BUS0_XSLV_UART0;
    uart_data_type *pdata;

    register_ext_interrupt_handler(CFG_IRQ_UART0, isr_uart0_tx);

    pdata = fw_malloc(sizeof(uart_data_type));
#ifdef UART_BUF_ENABLE    
    pdata->fifo = fw_malloc(UART_BUF_SZ);
    pdata->wrcnt = 1;
    pdata->rdcnt = 0;
#else
    pdata->fifo = 0;
    pdata->wrcnt = 0;
    pdata->rdcnt = 0;
#endif
    fw_register_ram_data(UART0_NAME, pdata);

    // scaler is enabled in SRAM self test, duplicate it here
    uart->scaler = SYS_HZ / 115200 / 2;
    uart->txctrl = 0x1;  // txen=1
    uart->rxctrl = 0x1;  // rxen=1
    uart_irq_type ie;
    ie.v = 0;
#ifdef UART_BUF_ENABLE    
    ie.b.txwm = 1;
    ie.b.rxwm = 0;
#endif
    uart->ie = ie.v;

    fw_enable_plic_irq(CTX_CPU0_M_MODE, CFG_IRQ_UART0);
}


static void printnum(f_putch putch,
                     void **putdat,
                     uint64_t num,
                     unsigned base,
                     int width,
                     int padc) {
    uart_data_type *p = fw_get_ram_data(UART0_NAME);
    int pos = 0;

    while (1) {
        p->digs[pos++] = num % base;
        if (num < base) {
            break;
        }
        num /= base;
    }

    while (width-- > pos) {
        putch(padc, putdat);
    }

    while (pos-- > 0) {
        putch(p->digs[pos] + (p->digs[pos] >= 10 ? 'a' - 10 : '0'), putdat);
    }
}

static uint64_t getuint(va_list *ap, int lflag) {
    if (lflag >= 2) {
        return va_arg(*ap, unsigned long long);
    } else if (lflag) {
        return va_arg(*ap, unsigned long);
    }
    return va_arg(*ap, unsigned int);
}

static int64_t getint(va_list *ap, int lflag) {
    if (lflag >= 2) {
        return va_arg(*ap, long long);
    } else if (lflag) {
        return va_arg(*ap, long);
    }
    return va_arg(*ap, int);
}

void vprintfmt_lib(f_putch putch,
                   void **putdat,
                   const char *fmt,
                   va_list ap) {
    register const char* p;
    const char* last_fmt;
    register int ch;
    unsigned long long num;
    int base, lflag, width, precision;// altflag;
    char padc;

    while (1) {
        while ((ch = *(unsigned char *) fmt) != '%') {
            if (ch == '\0') {
                return;
            }
            fmt++;
            putch(ch, putdat);
        }
        fmt++;

        // Process a %-escape sequence
        last_fmt = fmt;
        padc = ' ';
        width = -1;
        precision = -1;
        lflag = 0;
        //altflag = 0;
reswitch:
        switch (ch = *(unsigned char *) fmt++) {
        // flag to pad on the right
        case '-':
            padc = '-';
            goto reswitch;
      
        // flag to pad with 0's instead of spaces
        case '0':
            padc = '0';
            goto reswitch;

        // width field
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            for (precision = 0; ; ++fmt) {
                precision = precision * 10 + ch - '0';
                ch = *fmt;
                if (ch < '0' || ch > '9') {
                    break;
                }
            }
            goto process_precision;

        case '*':
            precision = va_arg(ap, int);
            goto process_precision;

        case '.':
            if (width < 0) {
                width = 0;
            }
            goto reswitch;

        case '#':
            //altflag = 1;
            goto reswitch;

process_precision:
            if (width < 0) {
                width = precision, precision = -1;
            }
            goto reswitch;

        // long flag (doubled for long long)
        case 'l':
            lflag++;
            goto reswitch;

        // character
        case 'c':
            putch(va_arg(ap, int), putdat);
            break;

        // string
        case 's':
            if ((p = va_arg(ap, char *)) == 0) {
                p = "(null)";
            }
            if (width > 0 && padc != '-') {
                for (width -= (int)strnlen(p, precision); width > 0; width--) {
                    putch(padc, putdat);
                }
            }
            for (; (ch = *p) != '\0' && (precision < 0 || --precision >= 0); width--) {
                putch(ch, putdat);
                p++;
            }
            for (; width > 0; width--) {
                putch(' ', putdat);
            }
            break;

        // (signed) decimal
        case 'd':
            num = getint(&ap, lflag);
            if ((long long) num < 0) {
                putch('-', putdat);
                num = -(long long) num;
            }
            base = 10;
            goto signed_number;

        // unsigned decimal
        case 'u':
            base = 10;
            goto unsigned_number;

        // (unsigned) octal
        case 'o':
            // should do something with padding so it's always 3 octits
            base = 8;
            goto unsigned_number;

        // pointer
        case 'p':
            //static_assert(sizeof(long) == sizeof(void*));
            lflag = 1;
            putch('0', putdat);
            putch('x', putdat);
            /* fall through to 'x' */

        // (unsigned) hexadecimal
        case 'x':
            base = 16;
unsigned_number:
            num = getuint(&ap, lflag);
signed_number:
            printnum(putch, putdat, num, base, width, padc);
            break;

        // escaped '%' character
        case '%':
            putch(ch, putdat);
            break;
      
        // unrecognized escape sequence - just print it literally
        default:
            putch('%', putdat);
            fmt = last_fmt;
            break;
        }
    }
}

void print_uart(const char *buf, int sz) {
    uart_map *uart = (uart_map *)ADDR_BUS0_XSLV_UART0;
    uart_txdata_type txdata;
    for (int i = 0; i < sz; i++) {
        do {
            txdata.v = uart->txdata;
        } while (txdata.b.full);
        uart->txdata = buf[i];
    }
}

void print_uart_hex(uint64_t val) {
    unsigned char t, s;
    uart_map *uart = (uart_map *)ADDR_BUS0_XSLV_UART0;
    uart_txdata_type txdata;

    for (int i = 0; i < 16; i++) {
        do {
            txdata.v = uart->txdata;
        } while (txdata.b.full);
        
        t = (unsigned char)((val >> ((15 - i) * 4)) & 0xf);
        if (t < 10) {
            s = t + '0';
        } else {
            s = (t - 10) + 'a';
        }
        uart->txdata = s;
    }
}

#undef putchar
int putchar(int ch) {
#ifdef UART_BUF_ENABLE    
    uart_data_type *p = fw_get_ram_data(UART0_NAME);
    buf_put(p, ch);
#else
    uart_map *uart = (uart_map *)ADDR_BUS0_XSLV_UART0;
    uart_txdata_type txdata;

    // No need to lock UART because we transmit only one symbol
    do {
        txdata.v = uart->txdata;
    } while (txdata.b.full);
    uart->txdata = ch;
#endif
    return 0;
}

void printf_uart(const char *fmt, ... ) {
    uart_data_type *pdata = fw_get_ram_data(UART0_NAME);
    uart_map *uart = (uart_map *)ADDR_BUS0_XSLV_UART0;
    int id = fw_get_cpuid() + 1;

    // lock UART to current CPU
    while (uart->fwcpuid != id) {
        if (uart->fwcpuid != 0) {
            continue;
        }
        uart->fwcpuid = id;
    }

    va_list arg;
    va_start(arg, fmt);
    vprintfmt_lib((f_putch)putchar, 0, fmt, arg);

    va_end(arg);

#ifdef UART_BUF_ENABLE    
    uart_txdata_type txdata;
    txdata.v = uart->txdata;
    while (!txdata.b.full && buf_total(pdata)){
        uart->txdata = buf_get(pdata);
        txdata.v = uart->txdata;
    }
#endif
    // free UART lock
    uart->fwcpuid = 0;
}
