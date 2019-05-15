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

static const char UART1_NAME[8] = "uart1";
static const int UART_BUF_SZ = 4096;

typedef void (*f_putch)(int, void**);


typedef struct uart_data_type {
    char *fifo;
    unsigned digs[32];

    volatile int wrcnt;
    volatile int rdcnt;
} uart_data_type;

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

void isr_uart1_tx(int idx, void *args) {
    uart_data_type *pdata = fw_get_ram_data(UART1_NAME);
    uart_map *uart = (uart_map *)ADDR_NASTI_SLAVE_UART1;

    while (buf_total(pdata)) {
        if (uart->status & UART_STATUS_TX_FULL) {
             break;
        }
        uart->data = buf_get(pdata);
    }
}

int uart_tx_nempty() {
    uart_data_type *pdata = fw_get_ram_data(UART1_NAME);
    return buf_total(pdata);
}

void uart_isr_init(void) {
    uart_map *uart = (uart_map *)ADDR_NASTI_SLAVE_UART1;
    uart_data_type *pdata;

    register_isr_handler(CFG_IRQ_UART1, isr_uart1_tx);

    pdata = fw_malloc(sizeof(uart_data_type));
    pdata->fifo = fw_malloc(UART_BUF_SZ);
    pdata->wrcnt = 1;
    pdata->rdcnt = 0;
    fw_register_ram_data(UART1_NAME, pdata);

    // scaler is enabled in SRAM self test
    uart->status |= UART_CONTROL_TXIRQ_ENA;

    enable_isr(CFG_IRQ_UART1);
}


static void printnum(f_putch putch,
                     void **putdat,
                     uint64_t num,
                     unsigned base,
                     int width,
                     int padc) {
    uart_data_type *p = fw_get_ram_data(UART1_NAME);
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
    uart_map *uart = (uart_map *)ADDR_NASTI_SLAVE_UART1;
    for (int i = 0; i < sz; i++) {
        while (uart->status & UART_STATUS_TX_FULL) {}
        uart->data = buf[i];
    }
}

void print_uart_hex(uint64_t val) {
    unsigned char t, s;
    uart_map *uart = (uart_map *)ADDR_NASTI_SLAVE_UART1;
    for (int i = 0; i < 16; i++) {
        while (uart->status & UART_STATUS_TX_FULL) {}
        
        t = (unsigned char)((val >> ((15 - i) * 4)) & 0xf);
        if (t < 10) {
            s = t + '0';
        } else {
            s = (t - 10) + 'a';
        }
        uart->data = s;
    }
}

#undef putchar
int putchar(int ch) {
    uart_data_type *p = fw_get_ram_data(UART1_NAME);
    buf_put(p, ch);
    return 0;
}

void printf_uart(const char *fmt, ... ) {
    uart_data_type *pdata = fw_get_ram_data(UART1_NAME);
    uart_map *uart = (uart_map *)ADDR_NASTI_SLAVE_UART1;

    va_list arg;
    va_start(arg, fmt);
    vprintfmt_lib((f_putch)putchar, 0, fmt, arg);
    va_end(arg);

    if (uart->status & UART_STATUS_TX_EMPTY) {
        if (buf_total(pdata)) {
            uart->data = buf_get(pdata);
        }
    }
}
