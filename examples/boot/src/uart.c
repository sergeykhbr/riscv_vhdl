/*
 *  Copyright 2021 Sergey Khabarov, sergeykhbr@gmail.com
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
#include "axi_maps.h"
#include "uart.h"

int print_char(int ch) {
    uart_map *uart = (uart_map *)ADDR_BUS0_XSLV_UART0;
    uart_txdata_type txdata;
    do {
        txdata.v = uart->txdata;
    } while (txdata.b.full);
    uart->txdata = ch;
    return 0;
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

void print_uart_hex(long val) {
    unsigned char t, s;
    uart_map *uart = (uart_map *)ADDR_BUS0_XSLV_UART0;
    uart_txdata_type txdata;
    for (int i = 0; i < 16; i++) {
        do {
            txdata.v = uart->txdata;
        } while (txdata.b.full);;
        
        t = (unsigned char)((val >> ((15 - i) * 4)) & 0xf);
        if (t < 10) {
            s = t + '0';
        } else {
            s = (t - 10) + 'a';
        }
        uart->txdata = s;
    }
}

void printnum(uint64_t num, int base, int width, int padc) {
    int pos = 0;
    uint8_t digs[64];

    while (1) {
        digs[pos++] = num % base;
        if (num < base) {
            break;
        }
        num /= base;
    }

    while (width-- > pos) {
        print_char(padc);
    }


    while (pos-- > 0) {
        print_char(digs[pos] + (digs[pos] >= 10 ? 'a' - 10 : '0'));
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

void vprintfmt_lib(const char *fmt,
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
            print_char(ch);
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
            print_char(va_arg(ap, int));
            break;

        // string
        case 's':
            if ((p = va_arg(ap, char *)) == 0) {
                p = "(null)";
            }
            if (width > 0 && padc != '-') {
                for (width -= (int)strnlen(p, precision); width > 0; width--) {
                    print_char(padc);
                }
            }
            for (; (ch = *p) != '\0' && (precision < 0 || --precision >= 0); width--) {
                print_char(ch);
                p++;
            }
            for (; width > 0; width--) {
                print_char(' ');
            }
            break;

        // (signed) decimal
        case 'd':
            num = getint(&ap, lflag);
            if ((long long) num < 0) {
                print_char('-');
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
            print_char('0');
            print_char('x');
            /* fall through to 'x' */

        // (unsigned) hexadecimal
        case 'x':
            base = 16;
unsigned_number:
            num = getuint(&ap, lflag);
signed_number:
            printnum(num, base, width, padc);
            break;

        // escaped '%' character
        case '%':
            print_char(ch);
            break;
      
        // unrecognized escape sequence - just print it literally
        default:
            print_char('%');
            fmt = last_fmt;
            break;
        }
    }
}


void printf_uart(const char *fmt, ... ) {
    va_list arg;
    va_start(arg, fmt);
    vprintfmt_lib(fmt, arg);

    va_end(arg);
}
