/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Left/Right shifter arithmetic/logic 32/64 bits.
 */

#include "shift.h"
#include "api_utils.h"

namespace debugger {

Shifter::Shifter(sc_module_name name_)
    : sc_module(name_) {
    SC_METHOD(comb);
    sensitive << i_a1;
    sensitive << i_a2;
};

void Shifter::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
}

void Shifter::comb() {
    sc_uint<64> wb_sll;
    sc_uint<64> wb_srl;
    sc_uint<64> wb_sra;
    sc_uint<32> wb_srlw;
    sc_uint<64> wb_sraw;
    sc_uint<64> v64;
    sc_uint<32> v32;
    sc_uint<64> msk64;
    sc_uint<64> msk32;

    v64 = i_a1.read();
    v32 = i_a1.read()(31, 0);

    if (i_a1.read()[63]) {
        msk64 = ~0;
    } else {
        msk64 = 0;
    }
    if (i_a1.read()[31]) {
        msk32 = ~0;
    } else {
        msk32 = 0;
    }

    switch (i_a2.read()) {
    case 0:
        wb_sll = v64;
        wb_srl = v64;
        wb_sra = v64;
        break;
    case 1:
        wb_sll = v64 << 1;
        wb_srl = v64(63, 1);
        wb_sra = (msk64(63, 63), v64(63, 1));
        break;
    case 2:
        wb_sll = v64 << 2;
        wb_srl = v64(63, 2);
        wb_sra = (msk64(63, 62), v64(63, 2));
        break;
    case 3:
        wb_sll = v64 << 3;
        wb_srl = v64(63, 3);
        wb_sra = (msk64(63, 61), v64(63, 3));
        break;
    case 4:
        wb_sll = v64 << 4;
        wb_srl = v64(63, 4);
        wb_sra = (msk64(63, 60), v64(63, 4));
        break;
    case 5:
        wb_sll = v64 << 5;
        wb_srl = v64(63, 5);
        wb_sra = (msk64(63, 59), v64(63, 5));
        break;
    case 6:
        wb_sll = v64 << 6;
        wb_srl = v64(63, 6);
        wb_sra = (msk64(63, 58), v64(63, 6));
        break;
    case 7:
        wb_sll = v64 << 7;
        wb_srl = v64(63, 7);
        wb_sra = (msk64(63, 57), v64(63, 7));
        break;
    case 8:
        wb_sll = v64 << 8;
        wb_srl = v64(63, 8);
        wb_sra = (msk64(63, 56), v64(63, 8));
        break;
    case 9:
        wb_sll = v64 << 9;
        wb_srl = v64(63, 9);
        wb_sra = (msk64(63, 55), v64(63, 9));
        break;
    case 10:
        wb_sll = v64 << 10;
        wb_srl = v64(63, 10);
        wb_sra = (msk64(63, 54), v64(63, 10));
        break;
    case 11:
        wb_sll = v64 << 11;
        wb_srl = v64(63, 11);
        wb_sra = (msk64(63, 53), v64(63, 11));
        break;
    case 12:
        wb_sll = v64 << 12;
        wb_srl = v64(63, 12);
        wb_sra = (msk64(63, 52), v64(63, 12));
        break;
    case 13:
        wb_sll = v64 << 13;
        wb_srl = v64(63, 13);
        wb_sra = (msk64(63, 51), v64(63, 13));
        break;
    case 14:
        wb_sll = v64 << 14;
        wb_srl = v64(63, 14);
        wb_sra = (msk64(63, 50), v64(63, 14));
        break;
    case 15:
        wb_sll = v64 << 15;
        wb_srl = v64(63, 15);
        wb_sra = (msk64(63, 49), v64(63, 15));
        break;
    case 16:
        wb_sll = v64 << 16;
        wb_srl = v64(63, 16);
        wb_sra = (msk64(63, 48), v64(63, 16));
        break;
    case 17:
        wb_sll = v64 << 17;
        wb_srl = v64(63, 17);
        wb_sra = (msk64(63, 47), v64(63, 17));
        break;
    case 18:
        wb_sll = v64 << 18;
        wb_srl = v64(63, 18);
        wb_sra = (msk64(63, 46), v64(63, 18));
        break;
    case 19:
        wb_sll = v64 << 19;
        wb_srl = v64(63, 19);
        wb_sra = (msk64(63, 45), v64(63, 19));
        break;
    case 20:
        wb_sll = v64 << 20;
        wb_srl = v64(63, 20);
        wb_sra = (msk64(63, 44), v64(63, 20));
        break;
    case 21:
        wb_sll = v64 << 21;
        wb_srl = v64(63, 21);
        wb_sra = (msk64(63, 43), v64(63, 21));
        break;
    case 22:
        wb_sll = v64 << 22;
        wb_srl = v64(63, 22);
        wb_sra = (msk64(63, 42), v64(63, 22));
        break;
    case 23:
        wb_sll = v64 << 23;
        wb_srl = v64(63, 23);
        wb_sra = (msk64(63, 41), v64(63, 23));
        break;
    case 24:
        wb_sll = v64 << 24;
        wb_srl = v64(63, 24);
        wb_sra = (msk64(63, 40), v64(63, 24));
        break;
    case 25:
        wb_sll = v64 << 25;
        wb_srl = v64(63, 25);
        wb_sra = (msk64(63, 39), v64(63, 25));
        break;
    case 26:
        wb_sll = v64 << 26;
        wb_srl = v64(63, 26);
        wb_sra = (msk64(63, 38), v64(63, 26));
        break;
    case 27:
        wb_sll = v64 << 27;
        wb_srl = v64(63, 27);
        wb_sra = (msk64(63, 37), v64(63, 27));
        break;
    case 28:
        wb_sll = v64 << 28;
        wb_srl = v64(63, 28);
        wb_sra = (msk64(63, 36), v64(63, 28));
        break;
    case 29:
        wb_sll = v64 << 29;
        wb_srl = v64(63, 29);
        wb_sra = (msk64(63, 35), v64(63, 29));
        break;
    case 30:
        wb_sll = v64 << 30;
        wb_srl = v64(63, 30);
        wb_sra = (msk64(63, 34), v64(63, 30));
        break;
    case 31:
        wb_sll = v64 << 31;
        wb_srl = v64(63, 31);
        wb_sra = (msk64(63, 33), v64(63, 31));
        break;
    case 32:
        wb_sll = v64 << 32;
        wb_srl = v64(63, 32);
        wb_sra = (msk64(63, 32), v64(63, 32));
        break;
    case 33:
        wb_sll = v64 << 33;
        wb_srl = v64(63, 33);
        wb_sra = (msk64(63, 31), v64(63, 33));
        break;
    case 34:
        wb_sll = v64 << 34;
        wb_srl = v64(63, 34);
        wb_sra = (msk64(63, 30), v64(63, 34));
        break;
    case 35:
        wb_sll = v64 << 35;
        wb_srl = v64(63, 35);
        wb_sra = (msk64(63, 29), v64(63, 35));
        break;
    case 36:
        wb_sll = v64 << 36;
        wb_srl = v64(63, 36);
        wb_sra = (msk64(63, 28), v64(63, 36));
        break;
    case 37:
        wb_sll = v64 << 37;
        wb_srl = v64(63, 37);
        wb_sra = (msk64(63, 27), v64(63, 37));
        break;
    case 38:
        wb_sll = v64 << 38;
        wb_srl = v64(63, 38);
        wb_sra = (msk64(63, 26), v64(63, 38));
        break;
    case 39:
        wb_sll = v64 << 39;
        wb_srl = v64(63, 39);
        wb_sra = (msk64(63, 25), v64(63, 39));
        break;
    case 40:
        wb_sll = v64 << 40;
        wb_srl = v64(63, 40);
        wb_sra = (msk64(63, 24), v64(63, 40));
        break;
    case 41:
        wb_sll = v64 << 41;
        wb_srl = v64(63, 41);
        wb_sra = (msk64(63, 23), v64(63, 41));
        break;
    case 42:
        wb_sll = v64 << 42;
        wb_srl = v64(63, 42);
        wb_sra = (msk64(63, 22), v64(63, 42));
        break;
    case 43:
        wb_sll = v64 << 43;
        wb_srl = v64(63, 43);
        wb_sra = (msk64(63, 21), v64(63, 43));
        break;
    case 44:
        wb_sll = v64 << 44;
        wb_srl = v64(63, 44);
        wb_sra = (msk64(63, 20), v64(63, 44));
        break;
    case 45:
        wb_sll = v64 << 45;
        wb_srl = v64(63, 45);
        wb_sra = (msk64(63, 19), v64(63, 45));
        break;
    case 46:
        wb_sll = v64 << 46;
        wb_srl = v64(63, 46);
        wb_sra = (msk64(63, 18), v64(63, 46));
        break;
    case 47:
        wb_sll = v64 << 47;
        wb_srl = v64(63, 47);
        wb_sra = (msk64(63, 17), v64(63, 47));
        break;
    case 48:
        wb_sll = v64 << 48;
        wb_srl = v64(63, 48);
        wb_sra = (msk64(63, 16), v64(63, 48));
        break;
    case 49:
        wb_sll = v64 << 49;
        wb_srl = v64(63, 49);
        wb_sra = (msk64(63, 15), v64(63, 49));
        break;
    case 50:
        wb_sll = v64 << 50;
        wb_srl = v64(63, 50);
        wb_sra = (msk64(63, 14), v64(63, 50));
        break;
    case 51:
        wb_sll = v64 << 51;
        wb_srl = v64(63, 51);
        wb_sra = (msk64(63, 13), v64(63, 51));
        break;
    case 52:
        wb_sll = v64 << 52;
        wb_srl = v64(63, 52);
        wb_sra = (msk64(63, 12), v64(63, 52));
        break;
    case 53:
        wb_sll = v64 << 53;
        wb_srl = v64(63, 53);
        wb_sra = (msk64(63, 11), v64(63, 53));
        break;
    case 54:
        wb_sll = v64 << 54;
        wb_srl = v64(63, 54);
        wb_sra = (msk64(63, 10), v64(63, 54));
        break;
    case 55:
        wb_sll = v64 << 55;
        wb_srl = v64(63, 55);
        wb_sra = (msk64(63, 9), v64(63, 55));
        break;
    case 56:
        wb_sll = v64 << 56;
        wb_srl = v64(63, 56);
        wb_sra = (msk64(63, 8), v64(63, 56));
        break;
    case 57:
        wb_sll = v64 << 57;
        wb_srl = v64(63, 57);
        wb_sra = (msk64(63, 7), v64(63, 57));
        break;
    case 58:
        wb_sll = v64 << 58;
        wb_srl = v64(63, 58);
        wb_sra = (msk64(63, 6), v64(63, 58));
        break;
    case 59:
        wb_sll = v64 << 59;
        wb_srl = v64(63, 59);
        wb_sra = (msk64(63, 5), v64(63, 59));
        break;
    case 60:
        wb_sll = v64 << 60;
        wb_srl = v64(63, 60);
        wb_sra = (msk64(63, 4), v64(63, 60));
        break;
    case 61:
        wb_sll = v64 << 61;
        wb_srl = v64(63, 61);
        wb_sra = (msk64(63, 3), v64(63, 61));
        break;
    case 62:
        wb_sll = v64 << 62;
        wb_srl = v64(63, 62);
        wb_sra = (msk64(63, 2), v64(63, 62));
        break;
    case 63:
        wb_sll = v64 << 63;
        wb_srl = v64(63, 63);
        wb_sra = (msk64(63, 1), v64(63, 63));
        break;
    default:;
    }

    switch (i_a2.read()(4, 0)) {
    case 0:
        wb_srlw = v32;
        wb_sraw = (msk32(63, 32), v32);
        break;
    case 1:
        wb_srlw = v32(31, 1);
        wb_sraw = (msk32(63, 31), v32(31, 1));
        break;
    case 2:
        wb_srlw = v32(31, 2);
        wb_sraw = (msk32(63, 30), v32(31, 2));
        break;
    case 3:
        wb_srlw = v32(31, 3);
        wb_sraw = (msk32(63, 29), v32(31, 3));
        break;
    case 4:
        wb_srlw = v32(31, 4);
        wb_sraw = (msk32(63, 28), v32(31, 4));
        break;
    case 5:
        wb_srlw = v32(31, 5);
        wb_sraw = (msk32(63, 27), v32(31, 5));
        break;
    case 6:
        wb_srlw = v32(31, 6);
        wb_sraw = (msk32(63, 26), v32(31, 6));
        break;
    case 7:
        wb_srlw = v32(31, 7);
        wb_sraw = (msk32(63, 25), v32(31, 7));
        break;
    case 8:
        wb_srlw = v32(31, 8);
        wb_sraw = (msk32(63, 24), v32(31, 8));
        break;
    case 9:
        wb_srlw = v32(31, 9);
        wb_sraw = (msk32(63, 23), v32(31, 9));
        break;
    case 10:
        wb_srlw = v32(31, 10);
        wb_sraw = (msk32(63, 22), v32(31, 10));
        break;
    case 11:
        wb_srlw = v32(31, 11);
        wb_sraw = (msk32(63, 21), v32(31, 11));
        break;
    case 12:
        wb_srlw = v32(31, 12);
        wb_sraw = (msk32(63, 20), v32(31, 12));
        break;
    case 13:
        wb_srlw = v32(31, 13);
        wb_sraw = (msk32(63, 19), v32(31, 13));
        break;
    case 14:
        wb_srlw = v32(31, 14);
        wb_sraw = (msk32(63, 18), v32(31, 14));
        break;
    case 15:
        wb_srlw = v32(31, 15);
        wb_sraw = (msk32(63, 17), v32(31, 15));
        break;
    case 16:
        wb_srlw = v32(31, 16);
        wb_sraw = (msk32(63, 16), v32(31, 16));
        break;
    case 17:
        wb_srlw = v32(31, 17);
        wb_sraw = (msk32(63, 15), v32(31, 17));
        break;
    case 18:
        wb_srlw = v32(31, 18);
        wb_sraw = (msk32(63, 14), v32(31, 18));
        break;
    case 19:
        wb_srlw = v32(31, 19);
        wb_sraw = (msk32(63, 13), v32(31, 19));
        break;
    case 20:
        wb_srlw = v32(31, 20);
        wb_sraw = (msk32(63, 12), v32(31, 20));
        break;
    case 21:
        wb_srlw = v32(31, 21);
        wb_sraw = (msk32(63, 11), v32(31, 21));
        break;
    case 22:
        wb_srlw = v32(31, 22);
        wb_sraw = (msk32(63, 10), v32(31, 22));
        break;
    case 23:
        wb_srlw = v32(31, 23);
        wb_sraw = (msk32(63, 9), v32(31, 23));
        break;
    case 24:
        wb_srlw = v32(31, 24);
        wb_sraw = (msk32(63, 8), v32(31, 24));
        break;
    case 25:
        wb_srlw = v32(31, 25);
        wb_sraw = (msk32(63, 7), v32(31, 25));
        break;
    case 26:
        wb_srlw = v32(31, 26);
        wb_sraw = (msk32(63, 6), v32(31, 26));
        break;
    case 27:
        wb_srlw = v32(31, 27);
        wb_sraw = (msk32(63, 5), v32(31, 27));
        break;
    case 28:
        wb_srlw = v32(31, 28);
        wb_sraw = (msk32(63, 4), v32(31, 28));
        break;
    case 29:
        wb_srlw = v32(31, 29);
        wb_sraw = (msk32(63, 3), v32(31, 29));
        break;
    case 30:
        wb_srlw = v32(31, 30);
        wb_sraw = (msk32(63, 2), v32(31, 30));
        break;
    case 31:
        wb_srlw = v32(31, 31);
        wb_sraw = (msk32(63, 1), v32(31, 31));
        break;
    default:;
    }

    o_sll = wb_sll;
    if (wb_sll[31]) {
        wb_sll(63, 32) = ~0;
    }
    o_sllw = wb_sll;
    o_srl = wb_srl;
    o_sra = wb_sra;
    o_srlw = wb_srlw.to_uint64();
    o_sraw = wb_sraw;
}

}  // namespace debugger

