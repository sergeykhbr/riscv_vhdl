/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
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

#include "api_core.h"
#include "fpu_func.h"
#include "fpu_func_tests.h"

namespace debugger {

int FpuCmdType::isValid(AttributeType *args) {
    if (!(*args)[0u].is_equal(parent_->getObjName())) {
        return CMD_INVALID;
    }
    if (args->size() < 3) {
        return CMD_WRONG_ARGS;
    }
    return CMD_VALID;
}

void FpuCmdType::exec(AttributeType *args, AttributeType *res) {
    FpuFunctional *p = static_cast<FpuFunctional *>(parent_);
    res->make_nil();
    if ((*args)[1].is_equal("test")) {
        if ((*args)[2].is_equal("fdiv.d")) {
            p->test_FDIV_D(res);
        }
    }
}

FpuFunctional::FpuFunctional(const char *name) : IService(name) {
    registerAttribute("CmdExecutor", &cmdexec_);
    pcmd_ = 0;
}

void FpuFunctional::postinitService() {
    icmdexec_ = static_cast<ICmdExecutor *>(
       RISCV_get_service_iface(cmdexec_.to_string(), IFACE_CMD_EXECUTOR));
    if (!icmdexec_) {
        RISCV_error("ICmdExecutor interface '%s' not found", 
                    cmdexec_.to_string());
    } else {
        pcmd_ = new FpuCmdType(static_cast<IService *>(this), getObjName());
        icmdexec_->registerCommand(static_cast<ICommand *>(pcmd_));
    }
}

void FpuFunctional::predeleteService() {
    if (icmdexec_ && pcmd_) {
        icmdexec_->unregisterCommand(static_cast<ICommand *>(pcmd_));
        delete pcmd_;
    }
}

void FpuFunctional::div_stage(int inMuxEna,
                              int inMuxInd[],   // 7 bits value (8 values)
                              int64_t inDivident,
                              int64_t inDivisor,
                              int64_t &outDif,
                              int &outBits,
                              int &outMuxInd,
                              int &outMuxIndRdy) {
    int64_t tmpThresh[16];
    int64_t tmpDif[4];

    inDivident &= MSK61;

    // 1 of 4
    tmpThresh[15] = inDivident - ( (inDivisor<<7) + (inDivisor<<6) );
    tmpThresh[14] = inDivident - ( (inDivisor<<7) );
    tmpThresh[13] = inDivident - ( (inDivisor<<6) );
    tmpThresh[12] = inDivident;

    outBits =  !(tmpThresh[15]&BIT62) ? 0xC0 :
               !(tmpThresh[14]&BIT62) ? 0x80 :
               !(tmpThresh[13]&BIT62) ? 0x40 : 0x0;

    tmpDif[0] = !(tmpThresh[15]&BIT62) ? (tmpThresh[15]&MSK61) :
                !(tmpThresh[14]&BIT62) ? (tmpThresh[14]&MSK61) :
                !(tmpThresh[13]&BIT62) ? (tmpThresh[13]&MSK61) : (tmpThresh[12]&MSK61);


    // 2 of 4
    tmpThresh[11] = tmpDif[0] - ( (inDivisor<<5) + (inDivisor<<4) );
    tmpThresh[10] = tmpDif[0] - ( (inDivisor<<5) );
    tmpThresh[9]  = tmpDif[0] - ( (inDivisor<<4) );
    tmpThresh[8]  = tmpDif[0];

    outBits =  !(tmpThresh[11]&BIT62) ? (outBits|0x30) :
               !(tmpThresh[10]&BIT62) ? (outBits|0x20) :
               !(tmpThresh[9]&BIT62)  ? (outBits|0x10) : (outBits|0x0);

    tmpDif[1] = !(tmpThresh[11]&BIT62) ? (tmpThresh[11]&MSK61) :
                !(tmpThresh[10]&BIT62) ? (tmpThresh[10]&MSK61) :
                !(tmpThresh[9]&BIT62)  ? (tmpThresh[9]&MSK61) : (tmpThresh[8]&MSK61);


    // 3 of 4
    tmpThresh[7] = tmpDif[1] - ( (inDivisor<<3) + (inDivisor<<2) );
    tmpThresh[6] = tmpDif[1] - ( (inDivisor<<3) );
    tmpThresh[5] = tmpDif[1] - ( (inDivisor<<2) );
    tmpThresh[4] = tmpDif[1];

    outBits =  !(tmpThresh[7]&BIT62) ? (outBits|0x0C) :
               !(tmpThresh[6]&BIT62) ? (outBits|0x08) :
               !(tmpThresh[5]&BIT62) ? (outBits|0x04) : (outBits|0x0);

    tmpDif[2] = !(tmpThresh[7]&BIT62) ? (tmpThresh[7]&MSK61) :
                !(tmpThresh[6]&BIT62) ? (tmpThresh[6]&MSK61) :
                !(tmpThresh[5]&BIT62) ? (tmpThresh[5]&MSK61) : (tmpThresh[4]&MSK61);

    // 4 of 4
    tmpThresh[3] = tmpDif[2] - ( (inDivisor<<1) + (inDivisor) );
    tmpThresh[2] = tmpDif[2] - ( (inDivisor<<1) );
    tmpThresh[1] = tmpDif[2] - ( (inDivisor) );
    tmpThresh[0] = tmpDif[2];

    outBits =  !(tmpThresh[3] & BIT62) ? (outBits|0x03) :
               !(tmpThresh[2] & BIT62) ? (outBits|0x02) :
               !(tmpThresh[1] & BIT62) ? (outBits|0x01) : (outBits|0x0);

    tmpDif[3] = !(tmpThresh[3]&BIT62) ? (tmpThresh[3]&MSK61) :
                !(tmpThresh[2]&BIT62) ? (tmpThresh[2]&MSK61) :
                !(tmpThresh[1]&BIT62) ? (tmpThresh[1]&MSK61) : (tmpThresh[0]&MSK61);

    outMuxInd = (inMuxEna&!(tmpThresh[15]&BIT62)) ? inMuxInd[7] :
                (inMuxEna&!(tmpThresh[14]&BIT62)) ? inMuxInd[7] :
                (inMuxEna&!(tmpThresh[13]&BIT62)) ? inMuxInd[6] :
            
                (inMuxEna&!(tmpThresh[11]&BIT62)) ? inMuxInd[5] :
                (inMuxEna&!(tmpThresh[10]&BIT62)) ? inMuxInd[5] :
                (inMuxEna&!(tmpThresh[9]&BIT62))  ? inMuxInd[4] :

                (inMuxEna&!(tmpThresh[7]&BIT62)) ? inMuxInd[3] :
                (inMuxEna&!(tmpThresh[6]&BIT62)) ? inMuxInd[3] :
                (inMuxEna&!(tmpThresh[5]&BIT62)) ? inMuxInd[2] :
            
                (inMuxEna&!(tmpThresh[3]&BIT62)) ? inMuxInd[1] :
                (inMuxEna&!(tmpThresh[2]&BIT62)) ? inMuxInd[1] :
                (inMuxEna&!(tmpThresh[1]&BIT62)) ? inMuxInd[0] : inMuxInd[0];

    outDif = tmpDif[3];
    outMuxIndRdy = inMuxEna & (outBits != 0 ? 1: 0);
}


void FpuFunctional::idiv53(int64_t inDivident,
                           int64_t inDivisor,
                           int *outResult,         // 106 bits value
                           int &outShift,          // 7 bits value
                           int &outOverBit,
                           int &outZeroResid) {
    // Model with removed pipeline logic:
    int64_t wbDif[14];
    int wbBits[14];
    int wbOutIndex[14];
    int wIndexRdy[14];

    //int wbIndex00[8];
    int wbIndex01[8];
    int wbIndex02[8];
    int wbIndex03[8];
    int wbIndex04[8];
    int wbIndex05[8];
    int wbIndex06[8];
    int wbIndex07[8];
    int wbIndex08[8];
    int wbIndex09[8];
    int wbIndex10[8];
    int wbIndex11[8];
    int wbIndex12[8];
    int wbIndex13[8];
    int rLShiftRdy[14];
    int rbLShift[14];  // 8 bits value

    // 0 of 13  
    int64_t wbDifStart = inDivident - inDivisor;
#if 1
    wbOutIndex[0] = 0;
    if (wbDifStart < 0) {
        wbDif[0]      = inDivident;
        wbBits[0]     = 0;
        wIndexRdy[0]  = 0;
    } else {
        wbDif[0]      = wbDifStart;
        wbBits[0]     = 1;
        wIndexRdy[0]  = 1;
    }
#else
    wbIndex00[7] = wbIndex00[6] =   wbIndex00[5] = wbIndex00[4] = 
    wbIndex00[3] = wbIndex00[2] =   wbIndex00[1] = wbIndex00[0] = 0;
    div_stage(1, wbIndex00, inDivident, inDivisor,
                        wbDif[0], wbBits[0], wbOutIndex[0], wIndexRdy[0]);
#endif
    rbLShift[0] = wIndexRdy[0] ? wbOutIndex[0] : 0;
    rLShiftRdy[0] = wIndexRdy[0];

    // 1 of 13
    wbIndex01[7] = 1; wbIndex01[6] = 2; wbIndex01[5] = 3; wbIndex01[4] = 4;
    wbIndex01[3] = 5; wbIndex01[2] = 6; wbIndex01[1] = 7; wbIndex01[0] = 8;
    div_stage(!rLShiftRdy[0], wbIndex01, wbDif[0] << 8, inDivisor,
                        wbDif[1], wbBits[1], wbOutIndex[1], wIndexRdy[1]);
    rbLShift[1] = wIndexRdy[1] ? wbOutIndex[1] : rbLShift[0];
    rLShiftRdy[1] = wIndexRdy[1] | rLShiftRdy[0];

    // 2 of 13
    wbIndex02[7] = 9;  wbIndex02[6] = 10; wbIndex02[5] = 11; wbIndex02[4] = 12;
    wbIndex02[3] = 13; wbIndex02[2] = 14; wbIndex02[1] = 15; wbIndex02[0] = 16;
    div_stage(!rLShiftRdy[1], wbIndex02, wbDif[1] << 8, inDivisor,
                        wbDif[2], wbBits[2], wbOutIndex[2], wIndexRdy[2]);
    rbLShift[2] = wIndexRdy[2] ? wbOutIndex[2] : rbLShift[1];
    rLShiftRdy[2] = wIndexRdy[2] | rLShiftRdy[1];

    // 3 of 13  
    wbIndex03[7] = 17; wbIndex03[6] = 18; wbIndex03[5] = 19; wbIndex03[4] = 20;
    wbIndex03[3] = 21; wbIndex03[2] = 22; wbIndex03[1] = 23; wbIndex03[0] = 24;
    div_stage(!rLShiftRdy[2], wbIndex03, wbDif[2] << 8, inDivisor,
                        wbDif[3], wbBits[3], wbOutIndex[3], wIndexRdy[3]);
    rbLShift[3] = wIndexRdy[3] ? wbOutIndex[3] : rbLShift[2];
    rLShiftRdy[3] = wIndexRdy[3] | rLShiftRdy[2];

    // 4 of 13
    wbIndex04[7] = 25; wbIndex04[6] = 26; wbIndex04[5] = 27; wbIndex04[4] = 28;
    wbIndex04[3] = 29; wbIndex04[2] = 30; wbIndex04[1] = 31; wbIndex04[0] = 32;
    div_stage(!rLShiftRdy[3], wbIndex04, wbDif[3] << 8, inDivisor,
                        wbDif[4], wbBits[4], wbOutIndex[4], wIndexRdy[4]);
    rbLShift[4] = wIndexRdy[4] ? wbOutIndex[4] : rbLShift[3];
    rLShiftRdy[4] = wIndexRdy[4] | rLShiftRdy[3];

    // 5 of 13
    wbIndex05[7] = 33; wbIndex05[6] = 34; wbIndex05[5] = 35; wbIndex05[4] = 36;
    wbIndex05[3] = 37; wbIndex05[2] = 38; wbIndex05[1] = 39; wbIndex05[0] = 40;
    div_stage(!rLShiftRdy[4], wbIndex05, wbDif[4] << 8, inDivisor,
                        wbDif[5], wbBits[5], wbOutIndex[5], wIndexRdy[5]);
    rbLShift[5] = wIndexRdy[5] ? wbOutIndex[5] : rbLShift[4];
    rLShiftRdy[5] = wIndexRdy[5] | rLShiftRdy[4];

    // 6 of 13
    wbIndex06[7] = 41; wbIndex06[6] = 42; wbIndex06[5] = 43; wbIndex06[4] = 44;
    wbIndex06[3] = 45; wbIndex06[2] = 46; wbIndex06[1] = 47; wbIndex06[0] = 48;
    div_stage(!rLShiftRdy[5], wbIndex06, wbDif[5] << 8, inDivisor,
                        wbDif[6], wbBits[6], wbOutIndex[6], wIndexRdy[6]);
    rbLShift[6] = wIndexRdy[6] ? wbOutIndex[6] : rbLShift[5];
    rLShiftRdy[6] = wIndexRdy[6] | rLShiftRdy[5];

    // 7 of 13
    wbIndex07[7] = 49; wbIndex07[6] = 50; wbIndex07[5] = 51; wbIndex07[4] = 52;
    wbIndex07[3] = 53; wbIndex07[2] = 54; wbIndex07[1] = 55; wbIndex07[0] = 56;
    div_stage(!rLShiftRdy[6], wbIndex07, wbDif[6] << 8, inDivisor,
                        wbDif[7], wbBits[7], wbOutIndex[7], wIndexRdy[7]);
    rbLShift[7] = wIndexRdy[7] ? wbOutIndex[7] : rbLShift[6];
    rLShiftRdy[7] = wIndexRdy[7] | rLShiftRdy[6];

    // 8 of 13
    wbIndex08[7] = 57; wbIndex08[6] = 58; wbIndex08[5] = 59; wbIndex08[4] = 60;
    wbIndex08[3] = 61; wbIndex08[2] = 62; wbIndex08[1] = 63; wbIndex08[0] = 64;
    div_stage(!rLShiftRdy[7], wbIndex08, wbDif[7] << 8, inDivisor,
                        wbDif[8], wbBits[8], wbOutIndex[8], wIndexRdy[8]);
    rbLShift[8] = wIndexRdy[8] ? wbOutIndex[8] : rbLShift[7];
    rLShiftRdy[8] = wIndexRdy[8] | rLShiftRdy[7];

    // 9 of 13
    wbIndex09[7] = 65; wbIndex09[6] = 66; wbIndex09[5] = 67; wbIndex09[4] = 68;
    wbIndex09[3] = 69; wbIndex09[2] = 70; wbIndex09[1] = 71; wbIndex09[0] = 72;
    div_stage(!rLShiftRdy[8], wbIndex09, wbDif[8] << 8, inDivisor,
                        wbDif[9], wbBits[9], wbOutIndex[9], wIndexRdy[9]);
    rbLShift[9] = wIndexRdy[9] ? wbOutIndex[9] : rbLShift[8];
    rLShiftRdy[9] = wIndexRdy[9] | rLShiftRdy[8];

    // 10 of 13
    wbIndex10[7] = 73; wbIndex10[6] = 74; wbIndex10[5] = 75; wbIndex10[4] = 76;
    wbIndex10[3] = 77; wbIndex10[2] = 78; wbIndex10[1] = 79; wbIndex10[0] = 80;
    div_stage(!rLShiftRdy[9], wbIndex10, wbDif[9] << 8, inDivisor,
                        wbDif[10], wbBits[10], wbOutIndex[10], wIndexRdy[10]);
    rbLShift[10] = wIndexRdy[10] ? wbOutIndex[10] : rbLShift[9];
    rLShiftRdy[10] = wIndexRdy[10] | rLShiftRdy[9];

    // 11 of 13
    wbIndex11[7] = 81; wbIndex11[6] = 82; wbIndex11[5] = 83; wbIndex11[4] = 84;
    wbIndex11[3] = 85; wbIndex11[2] = 86; wbIndex11[1] = 87; wbIndex11[0] = 88;
    div_stage(!rLShiftRdy[10], wbIndex11, wbDif[10] << 8, inDivisor,
                        wbDif[11], wbBits[11], wbOutIndex[11], wIndexRdy[11]);
    rbLShift[11] = wIndexRdy[11] ? wbOutIndex[11] : rbLShift[10];
    rLShiftRdy[11] = wIndexRdy[11] | rLShiftRdy[10];

    // 12 of 13
    wbIndex12[7] = 89; wbIndex12[6] = 90; wbIndex12[5] = 91; wbIndex12[4] = 92;
    wbIndex12[3] = 93; wbIndex12[2] = 94; wbIndex12[1] = 95; wbIndex12[0] = 96;
    div_stage(!rLShiftRdy[11], wbIndex12, wbDif[11] << 8, inDivisor,
                        wbDif[12], wbBits[12], wbOutIndex[12], wIndexRdy[12]);
    rbLShift[12] = wIndexRdy[12] ? wbOutIndex[12] : rbLShift[11];
    rLShiftRdy[12] = wIndexRdy[12] | rLShiftRdy[11];

    // 13 of 13
    wbIndex13[7] = 97; wbIndex13[6] = 98; wbIndex13[5] = 99; wbIndex13[4] = 100;
    wbIndex13[3] = 101; wbIndex13[2] = 102; wbIndex13[1] = 103; wbIndex13[0] = 104;
    div_stage(!rLShiftRdy[12], wbIndex13, wbDif[12] << 8, inDivisor,
                        wbDif[13], wbBits[13], wbOutIndex[13], wIndexRdy[13]);
    rbLShift[13] = wIndexRdy[13] ? wbOutIndex[13] : rbLShift[12];
    rLShiftRdy[13] = wIndexRdy[13] | rLShiftRdy[12];

    for (int i = 0; i < 8; i++) {
        outResult[i] = (wbBits[13] >> i) & 0x1;
        outResult[8 + i] = (wbBits[12] >> i) & 0x1;
        outResult[16 + i] = (wbBits[11] >> i) & 0x1;
        outResult[24 + i] = (wbBits[10] >> i) & 0x1;
    
        outResult[32 + i] = (wbBits[9] >> i) & 0x1;
        outResult[40 + i] = (wbBits[8] >> i) & 0x1;
        outResult[48 + i] = (wbBits[7] >> i) & 0x1;
        outResult[56 + i] = (wbBits[6] >> i) & 0x1;
    
        outResult[64+i] = (wbBits[5] >> i) & 0x1;
        outResult[72+i] = (wbBits[4] >> i) & 0x1;
        outResult[80+i] = (wbBits[3] >> i) & 0x1;
        outResult[88+i] = (wbBits[2] >> i) & 0x1;
        outResult[96+i] = (wbBits[1] >> i) & 0x1;
    }
    outResult[104] = (wbBits[0] >> 0) & 0x1;

    outShift = rbLShift[13];
    outOverBit = rbLShift[13] == 0x7F ? 1: 0;
    outZeroResid = wbDif[13] ? 0 : 1;
}

int FpuFunctional::FDIV_D(Reg64Type A, Reg64Type B, Reg64Type *fres) {
    uint64_t zeroA = !A.f64bits.sign && !A.f64bits.exp ? 1: 0;
    uint64_t zeroB = !B.f64bits.sign && !B.f64bits.exp ? 1: 0;

    int64_t mantA = A.f64bits.mant;
    mantA |= A.f64bits.exp ? 0x0010000000000000ull: 0;

    int64_t mantB = B.f64bits.mant;
    mantB |= B.f64bits.exp ? 0x0010000000000000ull: 0;

    // multiplexer for operation with zero expanent
    int preShift = 0;
    while (preShift < 52 && ((mantB >> (52 - preShift)) & 0x1) == 0) {
        preShift++;
    }
    int64_t divisor = mantB << preShift;

    // IDiv53 module:
    int idivResult[106], idivLShift, idivOverBit, idivZeroResid;
    idiv53(mantA, divisor, idivResult, idivLShift,
            idivOverBit, idivZeroResid);

    // easy in HDL
    int mantAlign[105];
    for (int i = 0; i < 105; i++) {
        if ((i - idivLShift) >= 0) {
            mantAlign[i] = idivResult[i - idivLShift];
        } else {
            mantAlign[i] = 0;
        }
    }

    int64_t expAB = A.f64bits.exp - B.f64bits.exp + 1023;
    int expShift;
    if (B.f64bits.exp == 0 && A.f64bits.exp != 0) {
        expShift = preShift - idivLShift - 1;
    } else {
        expShift = preShift - idivLShift;
    }

    int64_t expAlign = expAB + expShift;
    int64_t postShift = 0;
    if (expAlign <= 0) {
        postShift = -expAlign;
        if (B.f64bits.exp != 0 && A.f64bits.exp != 0) {
            postShift += 1;
        }
    }

    int mantPostScale[105];
    for (int i = 0; i < 105; i++) {
        if ((i + postShift) < 105) {
            mantPostScale[i] = mantAlign[i + postShift];
        } else {
            mantPostScale[i] = 0;
        }
    }

    int64_t mantShort = 0;
    int64_t tmpMant05 = 0;
    for (int i = 0; i < 53; i++) {
        mantShort |= static_cast<int64_t>(mantPostScale[52 + i]) << i;
    }
    for (int i = 0; i < 52; i++) {
        tmpMant05 |= static_cast<int64_t>(mantPostScale[i]) << i;
    }

    int64_t mantOnes = mantShort == 0x001fffffffffffff ? 1: 0;

    // rounding bit
    int mantEven = mantPostScale[52];
    int mant05 = tmpMant05 == 0x0008000000000000 ? 1: 0;
    int64_t rndBit = mantPostScale[51] & !(mant05 & !mantEven);

    // Exceptions:
    int64_t nanRes = expAlign == 0x7ff ? 1: 0;
    int64_t overflow = !((expAlign >> 12) & 0x1) & ((expAlign >> 11) & 0x1);
    int64_t underflow = ((expAlign >> 12) & 0x1) & ((expAlign >> 11) & 0x1);

    // Check borders:
    int nanA = A.f64bits.exp == 0x7ff ? 1: 0;
    int nanB = B.f64bits.exp == 0x7ff ? 1: 0;
    int mantZeroA = A.f64bits.mant ? 0: 1;
    int mantZeroB = B.f64bits.mant ? 0: 1;
    int divOnZero = zeroB || (mantB == 0) ? 1: 0;

    // Result multiplexers:
    if ((nanA & mantZeroA) & (nanB & mantZeroB)) {
        fres->f64bits.sign = 1;
    } else if (nanA & !mantZeroA) {
        fres->f64bits.sign = A.f64bits.sign;
    } else if (nanB & !mantZeroB) {
        fres->f64bits.sign = B.f64bits.sign;
    } else if (divOnZero && zeroA) {
        fres->f64bits.sign = 1;
    } else {
        fres->f64bits.sign = A.f64bits.sign ^ B.f64bits.sign; 
    }

    if (nanB & !mantZeroB) {
        fres->f64bits.exp = B.f64bits.exp;
    } else if ((underflow | zeroA | zeroB) & !divOnZero) {
        fres->f64bits.exp = 0x0;
    } else if (overflow | divOnZero) {
        fres->f64bits.exp = 0x7FF;
    } else if (nanA) {
        fres->f64bits.exp = A.f64bits.exp;
    } else if ((nanB & mantZeroB) || expAlign < 0) {
        fres->f64bits.exp = 0x0;
    } else {
        fres->f64bits.exp = expAlign + (mantOnes & rndBit & !overflow);
    }

    if ((zeroA & zeroB) | (nanA & mantZeroA & nanB & mantZeroB)) {
        fres->f64bits.mant = 0x8000000000000;
    } else if (nanA & !mantZeroA) {
        fres->f64bits.mant = A.f64bits.mant | 0x8000000000000;
    } else if (nanB & !mantZeroB) {
        fres->f64bits.mant = B.f64bits.mant | 0x8000000000000;
    } else if (overflow | nanRes | (nanA & mantZeroA) | (nanB & mantZeroB)) {
        fres->f64bits.mant = 0x0;
    } else {
        fres->f64bits.mant = mantShort + rndBit;
    }
    return 0;
}

void FpuFunctional::test_FDIV_D(AttributeType *res) {
    Reg64Type A, B, fres, fref;;
    A.f64 = 0.123;
    B.f64 = 10.45;
    fref.f64 = A.f64 / B.f64;
    FDIV_D(A, B, &fres);

    if (fres.f64 != fref.f64) {
        res->make_string("FAIL");
        RISCV_error("FDIF.D %016" RV_PRI64 "x != %016" RV_PRI64 "x",
                    fres.val, fref.val);
    } else {
        res->make_string("PASS");
        RISCV_debug("passed: FDIF.D res = %016" RV_PRI64 "x", fres.val);
    }
}

}  // namespace debugger
