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

#include <api_core.h>
#include "fpu_func.h"
#include "fpu_func_tests.h"
#include <stdlib.h>

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
        } else if ((*args)[2].is_equal("fmul.d")) {
            p->test_FMUL_D(res);
        } else if ((*args)[2].is_equal("fadd.d")) {
            p->test_FADD_D(res);
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

void FpuFunctional::imul53(int64_t A,              // 53 bits
                           int64_t B,              // 53 bits
                           int *outResult,         // 106 bits value
                           int &outShift,          // 7 bits value
                           int &outOverBit) {
    uint64_t rbMux[16];
    uint64_t rbPart[14];
    int rbLShift;  // 8 bits value

    rbMux[0] = 0;
    rbMux[1] = A;
    rbMux[2] = A << 1;
    rbMux[3] = (A << 1) + A;
    rbMux[4] = (A << 2);
    rbMux[5] = (A << 2) + A;
    rbMux[6] = (A << 2) + (A << 1);
    rbMux[7] = (A << 2) + (A << 1) + A;
    rbMux[8] = (A << 3);
    rbMux[9] = (A << 3) + A;
    rbMux[10] = (A << 3) + (A << 1);
    rbMux[11] = (A << 3) + (A << 1) + A;
    rbMux[12] = (A << 3) + (A << 2);
    rbMux[13] = (A << 3) + (A << 2) + A;
    rbMux[14] = (A << 3) + (A << 2) + (A << 1);
    rbMux[15] = (A << 4) - A;

    // Multiplex 57 bits width values
    rbPart[0] = rbMux[(B >> 0) & 0xF];
    rbPart[1] = rbMux[(B >> 4) & 0xF];
    rbPart[2] = rbMux[(B >> 8) & 0xF];
    rbPart[3] = rbMux[(B >> 12) & 0xF];
    rbPart[4] = rbMux[(B >> 16) & 0xF];
    rbPart[5] = rbMux[(B >> 20) & 0xF];
    rbPart[6] = rbMux[(B >> 24) & 0xF];
    rbPart[7] = rbMux[(B >> 28) & 0xF];
    rbPart[8] = rbMux[(B >> 32) & 0xF];
    rbPart[9] = rbMux[(B >> 36) & 0xF];
    rbPart[10] = rbMux[(B >> 40) & 0xF];
    rbPart[11] = rbMux[(B >> 44) & 0xF];
    rbPart[12] = rbMux[(B >> 48) & 0xF];
    rbPart[13] = rbMux[(B >> 52) & 0xF];

    // Pyramidal adder
    uint64_t lSumLsb = 0;
    uint64_t lCarryLsb = 0;
    for (int i = 0; i < 14; i++) {
        lSumLsb = (lSumLsb & 0x1FFFFFFFFFFFFFFF) 
               + ((rbPart[i] << 4*i) & 0x01FFFFFFFFFFFFFF);
        lCarryLsb = (lSumLsb >> 57) & 0xF;
    }
    uint64_t lSumMsb = lCarryLsb;
    lSumLsb &= 0x01FFFFFFFFFFFFFF;

    for (int i = 0; i < 14; i++) {
        lSumMsb += (rbPart[i] >> (57 - 4*i));
    }
    lSumLsb = lSumLsb | (lSumMsb << (64 - 7));
    lSumMsb >>= 7;

    if ((lSumMsb >> 41) & 0x1)      rbLShift = 0x7F;
    else if ((lSumMsb >> 40) & 0x1) rbLShift = 0;
    else if ((lSumMsb >> 39) & 0x1) rbLShift = 1;
    else if ((lSumMsb >> 38) & 0x1) rbLShift = 2;
    else if ((lSumMsb >> 37) & 0x1) rbLShift = 3;
    else if ((lSumMsb >> 36) & 0x1) rbLShift = 4;
    else if ((lSumMsb >> 35) & 0x1) rbLShift = 5;
    else if ((lSumMsb >> 34) & 0x1) rbLShift = 6;
    else if ((lSumMsb >> 33) & 0x1) rbLShift = 7;
    else if ((lSumMsb >> 32) & 0x1) rbLShift = 8;
    else if ((lSumMsb >> 31) & 0x1) rbLShift = 9;
    else if ((lSumMsb >> 30) & 0x1) rbLShift = 10;
    else if ((lSumMsb >> 29) & 0x1) rbLShift = 11;
    else if ((lSumMsb >> 28) & 0x1) rbLShift = 12;
    else if ((lSumMsb >> 27) & 0x1) rbLShift = 13;
    else if ((lSumMsb >> 26) & 0x1) rbLShift = 14;
    else if ((lSumMsb >> 25) & 0x1) rbLShift = 15;
    else if ((lSumMsb >> 24) & 0x1) rbLShift = 16;
    else if ((lSumMsb >> 23) & 0x1) rbLShift = 17;
    else if ((lSumMsb >> 22) & 0x1) rbLShift = 18;
    else if ((lSumMsb >> 21) & 0x1) rbLShift = 19;
    else if ((lSumMsb >> 20) & 0x1) rbLShift = 20;
    else if ((lSumMsb >> 19) & 0x1) rbLShift = 21;
    else if ((lSumMsb >> 18) & 0x1) rbLShift = 22;
    else if ((lSumMsb >> 17) & 0x1) rbLShift = 23;
    else if ((lSumMsb >> 16) & 0x1) rbLShift = 24;
    else if ((lSumMsb >> 15) & 0x1) rbLShift = 25;
    else if ((lSumMsb >> 14) & 0x1) rbLShift = 26;
    else if ((lSumMsb >> 13) & 0x1) rbLShift = 27;
    else if ((lSumMsb >> 12) & 0x1) rbLShift = 28;
    else if ((lSumMsb >> 11) & 0x1) rbLShift = 29;
    else if ((lSumMsb >> 10) & 0x1) rbLShift = 30;
    else if ((lSumMsb >> 9) & 0x1)  rbLShift = 31;
    else if ((lSumMsb >> 8) & 0x1)  rbLShift = 32;
    else if ((lSumMsb >> 7) & 0x1)  rbLShift = 33;
    else if ((lSumMsb >> 6) & 0x1)  rbLShift = 34;
    else if ((lSumMsb >> 5) & 0x1)  rbLShift = 35;
    else if ((lSumMsb >> 4) & 0x1)  rbLShift = 36;
    else if ((lSumMsb >> 3) & 0x1)  rbLShift = 37;
    else if ((lSumMsb >> 2) & 0x1)  rbLShift = 38;
    else if ((lSumMsb >> 1) & 0x1)  rbLShift = 39;
    else if ((lSumMsb >> 0) & 0x1)  rbLShift = 40;
  
    else if ((lSumLsb >> 63) & 0x1)  rbLShift = 41;
    else if ((lSumLsb >> 62) & 0x1)  rbLShift = 42;
    else if ((lSumLsb >> 61) & 0x1)  rbLShift = 43;
    else if ((lSumLsb >> 60) & 0x1)  rbLShift = 44;
    else if ((lSumLsb >> 59) & 0x1)  rbLShift = 45;
    else if ((lSumLsb >> 58) & 0x1)  rbLShift = 46;
    else if ((lSumLsb >> 57) & 0x1)  rbLShift = 47;
    else if ((lSumLsb >> 56) & 0x1)  rbLShift = 48;
    else if ((lSumLsb >> 55) & 0x1)  rbLShift = 49;
    else if ((lSumLsb >> 54) & 0x1)  rbLShift = 50;
    else if ((lSumLsb >> 53) & 0x1)  rbLShift = 51;
    else if ((lSumLsb >> 52) & 0x1)  rbLShift = 52;
    else if ((lSumLsb >> 51) & 0x1)  rbLShift = 53;
    else if ((lSumLsb >> 50) & 0x1)  rbLShift = 54;
    else if ((lSumLsb >> 49) & 0x1)  rbLShift = 55;
    else if ((lSumLsb >> 48) & 0x1)  rbLShift = 56;
    else if ((lSumLsb >> 47) & 0x1)  rbLShift = 57;
    else if ((lSumLsb >> 46) & 0x1)  rbLShift = 58;
    else if ((lSumLsb >> 45) & 0x1)  rbLShift = 59;
    else if ((lSumLsb >> 44) & 0x1)  rbLShift = 60;
    else if ((lSumLsb >> 43) & 0x1)  rbLShift = 61;
    else if ((lSumLsb >> 42) & 0x1)  rbLShift = 62;
    else if ((lSumLsb >> 41) & 0x1)  rbLShift = 63;
    else if ((lSumLsb >> 40) & 0x1)  rbLShift = 64;
    else if ((lSumLsb >> 39) & 0x1)  rbLShift = 65;
    else if ((lSumLsb >> 38) & 0x1)  rbLShift = 66;
    else if ((lSumLsb >> 37) & 0x1)  rbLShift = 67;
    else if ((lSumLsb >> 36) & 0x1)  rbLShift = 68;
    else if ((lSumLsb >> 35) & 0x1)  rbLShift = 69;
    else if ((lSumLsb >> 34) & 0x1)  rbLShift = 70;
    else if ((lSumLsb >> 33) & 0x1)  rbLShift = 71;
    else if ((lSumLsb >> 32) & 0x1)  rbLShift = 72;
    else if ((lSumLsb >> 31) & 0x1)  rbLShift = 73;
    else if ((lSumLsb >> 30) & 0x1)  rbLShift = 74;
    else if ((lSumLsb >> 29) & 0x1)  rbLShift = 75;
    else if ((lSumLsb >> 28) & 0x1)  rbLShift = 76;
    else if ((lSumLsb >> 27) & 0x1)  rbLShift = 77;
    else if ((lSumLsb >> 26) & 0x1)  rbLShift = 78;
    else if ((lSumLsb >> 25) & 0x1)  rbLShift = 79;
    else if ((lSumLsb >> 24) & 0x1)  rbLShift = 80;
    else if ((lSumLsb >> 23) & 0x1)  rbLShift = 81;
    else if ((lSumLsb >> 22) & 0x1)  rbLShift = 82;
    else if ((lSumLsb >> 21) & 0x1)  rbLShift = 83;
    else if ((lSumLsb >> 20) & 0x1)  rbLShift = 84;
    else if ((lSumLsb >> 19) & 0x1)  rbLShift = 85;
    else if ((lSumLsb >> 18) & 0x1)  rbLShift = 86;
    else if ((lSumLsb >> 17) & 0x1)  rbLShift = 87;
    else if ((lSumLsb >> 16) & 0x1)  rbLShift = 88;
    else if ((lSumLsb >> 15) & 0x1)  rbLShift = 89;
    else if ((lSumLsb >> 14) & 0x1)  rbLShift = 90;
    else if ((lSumLsb >> 13) & 0x1)  rbLShift = 91;
    else if ((lSumLsb >> 12) & 0x1)  rbLShift = 92;
    else if ((lSumLsb >> 11) & 0x1)  rbLShift = 93;
    else if ((lSumLsb >> 10) & 0x1)  rbLShift = 94;
    else if ((lSumLsb >> 9) & 0x1)   rbLShift = 95;
    else if ((lSumLsb >> 8) & 0x1)   rbLShift = 96;
    else if ((lSumLsb >> 7) & 0x1)   rbLShift = 97;
    else if ((lSumLsb >> 6) & 0x1)   rbLShift = 98;
    else if ((lSumLsb >> 5) & 0x1)   rbLShift = 99;
    else if ((lSumLsb >> 4) & 0x1)   rbLShift = 100;
    else if ((lSumLsb >> 3) & 0x1)   rbLShift = 101;
    else if ((lSumLsb >> 2) & 0x1)   rbLShift = 102;
    else if ((lSumLsb >> 1) & 0x1)   rbLShift = 103;
    else                             rbLShift = 104;

    // Form output:
    for (int i = 0; i < 64; i++) {
        outResult[i] = static_cast<int>((lSumLsb >> i) & 0x1);
    }
    for (int i = 0; i < 42; i++) {
        outResult[64 + i] = static_cast<int>((lSumMsb >> i) & 0x1);
    }

    outShift = rbLShift;
    outOverBit = static_cast<int>((lSumMsb >> 41) & 0x1);
}

int FpuFunctional::FDIV_D(Reg64Type A, Reg64Type B, Reg64Type *fres) {
    uint64_t zeroA = !A.f64bits.exp && !A.f64bits.mant ? 1: 0;
    uint64_t zeroB = !B.f64bits.exp && !B.f64bits.mant ? 1: 0;

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
    int idivResult[106];
    int idivLShift;
    int idivOverBit;
    int idivZeroResid;
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
    } else if (B.f64bits.exp != 0 && A.f64bits.exp == 0) {
        expShift = preShift - idivLShift + 1;
    } else {
        expShift = preShift - idivLShift;
    }

    int64_t expAlign = expAB + expShift;
    int64_t postShift = 0;
    if (expAlign <= 0) {
        postShift = -expAlign;
        postShift += 1;
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
    int64_t overflow = (!((expAlign >> 12) & 0x1)) & ((expAlign >> 11) & 0x1);
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

int FpuFunctional::FMUL_D(Reg64Type A, Reg64Type B, Reg64Type *fres,
                          int &except) {
    uint64_t zeroA = !A.f64bits.exp && !A.f64bits.mant ? 1: 0;
    uint64_t zeroB = !B.f64bits.exp && !B.f64bits.mant ? 1: 0;

    int64_t mantA = A.f64bits.mant;
    mantA |= A.f64bits.exp ? 0x0010000000000000ull: 0;

    int64_t mantB = B.f64bits.mant;
    mantB |= B.f64bits.exp ? 0x0010000000000000ull: 0;

    // IMul53 module:
    int imulResult[106];
    int imulShift;
    int imulOverBit;
    imul53(mantA, mantB, imulResult, imulShift, imulOverBit);

    // scaling sum
    int mantAlign[105];
    for (int i = 0; i < 105; i++) {
        if (imulShift == 0x7F) {
            mantAlign[i] = imulResult[i + 1];
        } else if ((i - imulShift) >= 0) {
            mantAlign[i] = imulResult[i - imulShift];
        } else {
            mantAlign[i] = 0;
        }
    }

    int64_t expAB = A.f64bits.exp + B.f64bits.exp - 1023;
    int64_t expAlign;
    if (imulResult[105]) {
        expAlign = expAB + 1;
    } else if (A.f64bits.exp == 0 || B.f64bits.exp == 0) {
        expAlign = expAB - imulShift + 1;
    } else {
        expAlign = expAB - imulShift;
    }

    // IMPORTANT exception! new ZERO value
    int wExpAlignZero = (expAlign <= 0 && imulShift == 0) ? 1 : 0;
    int64_t postShift = 0;
    if (expAlign <= 0) {
        if (wExpAlignZero || imulResult[105]
            || A.f64bits.exp == 0 || B.f64bits.exp == 0) {
            postShift = -expAlign + 1;
        } else {
            postShift = -expAlign;
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
    int nanA = A.f64bits.exp == 0x7ff ? 1: 0;
    int nanB = B.f64bits.exp == 0x7ff ? 1: 0;
    int overflow = expAlign >= 0x7FF ? 1: 0;

    // Check borders:
    int mantZeroA = A.f64bits.mant ? 0: 1;
    int mantZeroB = B.f64bits.mant ? 0: 1;

    // Result multiplexers:
    if ((nanA & mantZeroA & zeroB) | (nanB & mantZeroB & zeroA)) {
        fres->f64bits.sign = 1;
    } else if (nanA & !mantZeroA) {
        fres->f64bits.sign = A.f64bits.sign;
    } else if (nanB & !mantZeroB) {
        fres->f64bits.sign = B.f64bits.sign;
    } else {
        fres->f64bits.sign = A.f64bits.sign ^ B.f64bits.sign; 
    }

    if (nanA) {
        fres->f64bits.exp = A.f64bits.exp;
    } else if (nanB) {
        fres->f64bits.exp = B.f64bits.exp;
    } else if (expAlign < 0 || zeroA || zeroB) {
        fres->f64bits.exp = 0;
    } else if (overflow) {
        fres->f64bits.exp = 0x7FF;
    } else {
        fres->f64bits.exp = expAlign + (mantOnes & rndBit & !overflow);
    }

    if ((nanA & mantZeroA & !mantZeroB) || (nanB & mantZeroB & !mantZeroA)
        || (!nanA & !nanB & overflow)) {
        fres->f64bits.mant = 0;
    } else if (nanA) {
        fres->f64bits.mant = A.f64bits.mant | 0x8000000000000;
    } else if (nanB) {
        fres->f64bits.mant = B.f64bits.mant | 0x8000000000000;
    } else {
        fres->f64bits.mant = mantShort + rndBit;
    }
    except = nanA | nanB | overflow;
    return 0;
}

int FpuFunctional::FADD_D(int addEna, int subEna, int cmpEna, int moreEna,
                          int absEna, Reg64Type A, Reg64Type B,
                            Reg64Type *fres, int &except) {

    int signOp = subEna | cmpEna | moreEna;
    uint64_t signA = A.f64bits.sign;
    uint64_t signB = B.f64bits.sign ^ signOp;

    int64_t mantA = A.f64bits.mant;
    mantA |= A.f64bits.exp ? 0x0010000000000000ull: 0;

    int64_t mantB = B.f64bits.mant;
    mantB |= B.f64bits.exp ? 0x0010000000000000ull: 0;

    int expDif;
    if (A.f64bits.exp && !B.f64bits.exp) {
        expDif = static_cast<int>(A.f64bits.exp - B.f64bits.exp - 1);
    } else if (!A.f64bits.exp && B.f64bits.exp) {
        expDif = static_cast<int>(A.f64bits.exp - B.f64bits.exp + 1);
    } else {
        expDif = static_cast<int>(A.f64bits.exp - B.f64bits.exp);
    }

    int preShift;
    uint64_t expMore;
    uint64_t expLess;
    uint64_t mantMore;
    uint64_t mantLess;
    uint64_t signMore;
    if (expDif > 0) {
        preShift = expDif;
        signMore = signA;
        expMore = A.f64bits.exp;
        expLess = B.f64bits.exp;
        mantMore = mantA;
        mantLess = mantB;
    } else if (expDif == 0) {
        preShift = expDif;
        if (A.f64bits.mant >= B.f64bits.mant) {
            signMore = signA;
            expMore = A.f64bits.exp;
            expLess = B.f64bits.exp;
            mantMore = mantA;
            mantLess = mantB;
        } else {
            signMore = signB;
            expMore = B.f64bits.exp;
            expLess = A.f64bits.exp;
            mantMore = mantB;
            mantLess = mantA;
        }
    } else {
        preShift = -expDif;
        signMore = signB;
        expMore = B.f64bits.exp;
        expLess = A.f64bits.exp;
        mantMore = mantB;
        mantLess = mantA;
    }

    // Convert to 105 bits width bus
    // A = {1'd0, mantA, 52'd0}
    // M = {1'd0, mantM, 52'd0}
    int wbMantMore[106];
    int wbMantLessWidth[106];

    for (int i = 0; i < 52; i++) {
        wbMantMore[i] = 0;
        wbMantLessWidth[i] = 0;
    }
    for (int i = 52; i < 105; i++) {
        wbMantMore[i] = static_cast<int>(mantMore >> (i - 52)) & 0x1;
        wbMantLessWidth[i] = static_cast<int>(mantLess >> (i - 52)) & 0x1;
    }
    wbMantMore[105] = 0;
    wbMantLessWidth[105] = 0;

    int preOverShift = preShift > 105 ? 1: 0;

    // Scale = mantLess >> preShift
    int mantLessScale[106];
    for (int i = 0; i < 106; i++) {
        if (preOverShift) {
            mantLessScale[i] = 0;
        } else if ((i + preShift) < 106) {
            mantLessScale[i] = wbMantLessWidth[i + preShift];
        } else {
            mantLessScale[i] = 0;
        }
    }

    // 106-bits adder/subtractor
    int mantSum[106];
    int mantSumCarryBit[107];
    mantSumCarryBit[0] = 0;
    for (int i = 0; i < 106; i++) {
        if (signA ^ signB) {
            // DIFFERENCE computation: D = A - B
            //      iCar[0]   = 0
            //      iDif[k]   = iCar[k]^iA[k]^iB[k];
            //      iCar[k+1] = (!iA[k]&iB[k]) | (iCar[k]& !(iA[k]^iB[k]));
            mantSum[i] = mantSumCarryBit[i] ^ wbMantMore[i] ^ mantLessScale[i];
            mantSumCarryBit[i+1] = (!wbMantMore[i] & mantLessScale[i])
                | (mantSumCarryBit[i] & !(wbMantMore[i] ^ mantLessScale[i]));
        } else {
            // SUMMATOR computation: S = A + B
            //      iCar[0]   = 0
            //      iSum[k]   = iCar[k]^iA[k]^iB[k];
            //      iCar[k+1] = (iA[k]&iB[k]) | (iCar[k]&(iA[k]|iB[k]));
            //
            mantSum[i] = mantSumCarryBit[i] ^ wbMantMore[i] ^ mantLessScale[i];
            mantSumCarryBit[i+1] = (wbMantMore[i] & mantLessScale[i])
                | (mantSumCarryBit[i] & (wbMantMore[i] | mantLessScale[i]));
        }
    }

    int rbLShift;
    if (mantSum[105])  {
        rbLShift = 0x7F;
    } else {
        rbLShift = 104;
        for (int i = 0; i < 104; i++) {
            if (mantSum[104 - i]) {
                rbLShift = i;
                break;
            }
        }
    }

    int mantAlign[105];
    int expPostScale;
    int mantPostScale[105];
    // scaling sum
    for (int i = 0; i < 105; i++) {
        if (rbLShift == 0x7F) {
            mantAlign[i] = mantSum[i + 1];
        } else if ((i - rbLShift) >= 0) {
            mantAlign[i] = mantSum[i - rbLShift];
        } else {
            mantAlign[i] = 0;
        }
    }
  
    // exponent scaling rate
    if (rbLShift == 0x7F) {
        if (expMore == 0x7FF) {
            expPostScale = static_cast<int>(expMore);
        } else {
            expPostScale = static_cast<int>(expMore) + 1;
        }
    } else {
        if (expMore == 0 && rbLShift == 0) {
            expPostScale = 1;
        } else {
            expPostScale = static_cast<int>(expMore) - rbLShift;
        }
    }

    // Scaled = SumScale>>(-ExpSum) if ExpSum < 0;
    for (int i = 0; i < 105; i++) {
        if (expPostScale >= 0) {
            mantPostScale[i] = mantAlign[i];
        } else if ((i - expPostScale) < 105) {
            mantPostScale[i] = mantAlign[i - expPostScale];
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

    // Check borders:
    int mantZeroA = A.f64bits.mant ? 0: 1;
    int mantZeroB = B.f64bits.mant ? 0: 1;

    // Exceptions:
    int allZero = A.f64bits.exp == 0 && mantZeroA
               && B.f64bits.exp == 0 && mantZeroB ? 1: 0;
    int sumZero = 1;
    for (int i = 0; i < 105; i++) {
        if (mantPostScale[i]) {
            sumZero = 0;
        }
    }
    int nanA = A.f64bits.exp == 0x7ff ? 1: 0;
    int nanB = B.f64bits.exp == 0x7ff ? 1: 0;
    int nanAB = nanA & mantZeroA & nanB && mantZeroB ? 1: 0;
    int overflow = expPostScale == 0x7FF ? 1: 0;

    // Result multiplexers:
    Reg64Type::f64_bits_type resAdd;
    if (nanAB & signOp) {
        resAdd.sign = A.f64bits.sign ^ B.f64bits.sign;
    } else if (nanA) {
        resAdd.sign = A.f64bits.sign;
    } else if (nanB) {
        resAdd.sign = B.f64bits.sign ^ (signOp & !mantZeroB);
    } else if (allZero) {
        resAdd.sign = A.f64bits.sign & B.f64bits.sign;
    } else if (sumZero) {
        resAdd.sign = 0;
    } else {
        resAdd.sign = signMore; 
    }

    if (nanA | nanB) {
        resAdd.exp = 0x7FF;
    } else if (expPostScale < 0 || sumZero) {
        resAdd.exp = 0;
    } else {
        resAdd.exp = expPostScale + (mantOnes & rndBit & !overflow);
    }

    if (nanA & mantZeroA & nanB & mantZeroB) {
        resAdd.mant = signOp ? 0x0008000000000000: 0;
    } else if (nanA) {
        resAdd.mant = A.f64bits.mant | 0x0008000000000000;
    } else if (nanB) {
        resAdd.mant = B.f64bits.mant | 0x0008000000000000;
    } else if (overflow) {
        resAdd.mant = 0;
    } else {
        resAdd.mant = mantShort + rndBit;
    }

    // CMP command exception
    int qnand = (nanA & !mantZeroA) | (nanB & !mantZeroB);
    int qnandCmpX86 = (nanA & mantZeroA) & (nanB & mantZeroB);

    // CMP = {29'd0, flMore, flEqual, flLess}  
    int flEqual = (qnandCmpX86 | 
                  (!qnand && !resAdd.mant && !resAdd.exp)) ? 1 : 0;
    int flLess =  (resAdd.sign | (B.f64bits.exp == 0x7ff)) & !flEqual;
    int flMore = !resAdd.sign & !flEqual & !flLess;
    int resCmp = (flMore << 2) | (flEqual << 1) | (flLess);
  
    // More value
    Reg64Type resMore;
    if (!allZero && (resAdd.sign || B.f64bits.exp == 0x7FF)) {
        resMore.f64bits.sign = signB;
        resMore.f64bits.exp = B.f64bits.exp;
        resMore.f64bits.mant = B.f64bits.mant;
    } else {
        resMore.f64bits.sign = signA;
        resMore.f64bits.exp = A.f64bits.exp;
        resMore.f64bits.mant = A.f64bits.mant;
    }

    // Absolute value                              
    Reg64Type resAbs;
    resAbs.f64bits.sign = 0;
    resAbs.f64bits.exp = A.f64bits.exp;
    resAbs.f64bits.mant = A.f64bits.mant;
    if (A.f64bits.exp == 0 && A.f64bits.mant == 0) {
        resAbs.f64bits.sign = A.f64bits.sign;
    }
  
    if (cmpEna) {
        fres->val = resCmp;
    } else if (moreEna) {
        *fres = resMore;
    } else if (absEna) {
        *fres = resAbs;
    } else {
        fres->f64bits = resAdd;
    }

    except = nanA | nanB | overflow;

    return 0;
}


void FpuFunctional::test_FDIV_D(AttributeType *res) {
    Reg64Type A, B, fres, fref;
    bool passed = true;
    A.f64 = 0.123;
    B.f64 = 10.45;

    fref.f64 = A.f64 / B.f64;
    FDIV_D(A, B, &fres);

    if (fres.val != fref.val) {
        passed = false;
        RISCV_error("FDIF.D %016" RV_PRI64 "x != %016" RV_PRI64 "x",
                    fres.val, fref.val);
    }
    for (size_t i = 0; i < TSTDDIV_LENGTH; i++) {
        A.val = TestCases_FDIV_D[i][0];
        B.val = TestCases_FDIV_D[i][1];
        fref.f64 = A.f64 / B.f64;
        FDIV_D(A, B, &fres);

        if (fres.val != fref.val) {
            passed = false;
            RISCV_error("[%d] FDIF.D %016" RV_PRI64 "x != %016" RV_PRI64 "x",
                        static_cast<int>(i), fres.val, fref.val);
        }
    }

    for (int i = 0; i < 1000000; i++) {
        A.f64bits.sign = rand();
        A.f64bits.exp = rand();
        for (int n = 0; n < 4; n++) {
            A.f64bits.mant = (A.f64bits.mant << 15) | (rand() & 0x7FFF);
        }
        B.f64bits.sign = rand();
        B.f64bits.exp = rand();
        for (int n = 0; n < 4; n++) {
            B.f64bits.mant = (A.f64bits.mant << 15) | (rand() & 0x7FFF);
        }

        fref.f64 = A.f64 / B.f64;
        FDIV_D(A, B, &fres);

        if (fres.val != fref.val) {
            passed = false;
            RISCV_error("[%d] FDIF.D %016" RV_PRI64 "x != %016" RV_PRI64 "x",
                        static_cast<int>(i), fres.val, fref.val);
        }
    }

    if (passed) {
        RISCV_info("%s all tests passed", "FDIV.D");
        res->make_string("FDIV_D: PASS");
    } else {
        res->make_string("FDIV_D: FAIL");
    }
}

void FpuFunctional::test_FMUL_D(AttributeType *res) {
    Reg64Type A, B, fres, fref;
    int exception;
    bool passed = true;
    A.f64 = 0.123;
    B.f64 = 10.45;

    fref.f64 = A.f64 * B.f64;
    FMUL_D(A, B, &fres, exception);

    if (fres.val != fref.val) {
        passed = false;
        RISCV_error("FMUL.D %016" RV_PRI64 "x != %016" RV_PRI64 "x",
                    fres.val, fref.val);
    }

    for (size_t i = 0; i < TSTDMUL_LENGTH; i++) {
        A.val = TestCases_FMUL_D[i][0];
        B.val = TestCases_FMUL_D[i][1];
        fref.f64 = A.f64 * B.f64;
        FMUL_D(A, B, &fres, exception);

        if (fres.val != fref.val) {
            passed = false;
            RISCV_error("[%d] FMUL.D %016" RV_PRI64 "x != %016" RV_PRI64 "x",
                        static_cast<int>(i), fres.val, fref.val);
        }
    }
    for (int i = 0; i < 1000000; i++) {
        A.f64bits.sign = rand();
        A.f64bits.exp = rand();
        for (int n = 0; n < 4; n++) {
            A.f64bits.mant = (A.f64bits.mant << 15) | (rand() & 0x7FFF);
        }
        B.f64bits.sign = rand();
        B.f64bits.exp = rand();
        for (int n = 0; n < 4; n++) {
            B.f64bits.mant = (A.f64bits.mant << 15) | (rand() & 0x7FFF);
        }

        fref.f64 = A.f64 * B.f64;
        FMUL_D(A, B, &fres, exception);

        if (fres.val != fref.val) {
            passed = false;
            RISCV_error("[%d] FMUL.D %016" RV_PRI64 "x != %016" RV_PRI64 "x",
                        static_cast<int>(i), fres.val, fref.val);
        }
    }

    if (passed) {
        RISCV_info("%s all tests passed", "FMUL.D");
        res->make_string("FMUL_D: PASS");
    } else {
        res->make_string("FMUL_D: FAIL");
    }
}

void FpuFunctional::test_FADD_D(AttributeType *res) {
    Reg64Type A, B, fres, fref;
    int exception;
    bool passed = true;
    A.f64 = 0.123;
    B.f64 = 10.45;

    fref.f64 = A.f64 + B.f64;
    FADD_D(1, 0, 0, 0, 0, A, B, &fres, exception);

    if (fres.val != fref.val) {
        passed = false;
        RISCV_error("FADD.D %016" RV_PRI64 "x != %016" RV_PRI64 "x",
                    fres.val, fref.val);
    }

    for (size_t i = 0; i < TSTDADD_LENGTH; i++) {
        A.val = TestCases_FADD_D[i][0];
        B.val = TestCases_FADD_D[i][1];
        fref.f64 = A.f64 + B.f64;
        FADD_D(1, 0, 0, 0, 0, A, B, &fres, exception);

        if (fres.val != fref.val) {
            passed = false;
            RISCV_error("[%d] FADD.D %016" RV_PRI64 "x != %016" RV_PRI64 "x",
                        static_cast<int>(i), fres.val, fref.val);
        }
    }
    for (int i = 0; i < 1000000; i++) {
        A.f64bits.sign = rand();
        A.f64bits.exp = rand();
        for (int n = 0; n < 4; n++) {
            A.f64bits.mant = (A.f64bits.mant << 15) | (rand() & 0x7FFF);
        }
        B.f64bits.sign = rand();
        B.f64bits.exp = rand();
        for (int n = 0; n < 4; n++) {
            B.f64bits.mant = (A.f64bits.mant << 15) | (rand() & 0x7FFF);
        }

        fref.f64 = A.f64 + B.f64;
        FADD_D(1, 0, 0, 0, 0, A, B, &fres, exception);

        if (fres.val != fref.val) {
            passed = false;
            RISCV_error("[%d] FADD.D %016" RV_PRI64 "x != %016" RV_PRI64 "x",
                        static_cast<int>(i), fres.val, fref.val);
        }
    }

    if (passed) {
        RISCV_info("%s all tests passed", "FADD.D");
        res->make_string("FADD_D: PASS");
    } else {
        res->make_string("FADD_D: FAIL");
    }
}

}  // namespace debugger
