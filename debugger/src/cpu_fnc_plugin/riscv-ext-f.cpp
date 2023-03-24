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
 *
 * @brief      RISC-V extension-F (Floating-point Instructions).
 */

#include "api_core.h"
#include "riscv-isa.h"
#include "cpu_riscv_func.h"

namespace debugger {

#define FCVT_WU_D_X86_COMPATIBLE

class FpuInstruction : public RiscvInstruction {
 public:
    FpuInstruction(CpuRiver_Functional *icpu, const char *name, const char *bits)
        : RiscvInstruction(icpu, name, bits) {
    }

 protected:
    const int64_t BIT62 = 0x2000000000000000;
    const int64_t MSK61 = 0x1FFFFFFFFFFFFFFF;

    void div_stage(int inMuxEna,
                   int inMuxInd[],      // 7 bits (8 values)
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

    void idiv53(int64_t inDivident,
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

        int wbIndex00[8];
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
    #if 0
        int64_t wbDifStart = inDivident - inDivisor;
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

    void imul53(int64_t A,              // 53 bits
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

    void AddSubCompare(int addEna, int subEna, int cmpEna, int equalEna,
                        int moreEna, int lessEna, Reg64Type A, Reg64Type B,
                        Reg64Type *fres, int &except) {

        int signOp = subEna | cmpEna;
        uint64_t signA = A.f64bits.sign;
        uint64_t signB = B.f64bits.sign;
        uint64_t signOpB = B.f64bits.sign ^ signOp;

        int64_t mantA = A.f64bits.mant;
        mantA |= A.f64bits.exp ? 0x0010000000000000ull: 0;

        int64_t mantB = B.f64bits.mant;
        mantB |= B.f64bits.exp ? 0x0010000000000000ull: 0;

        int expDif;
        if (A.f64bits.exp && !B.f64bits.exp) {
            expDif = static_cast<int>(A.f64bits.exp - 1);
        } else if (!A.f64bits.exp && B.f64bits.exp) {
            expDif = static_cast<int>(1 - B.f64bits.exp);
        } else {
            expDif = static_cast<int>(A.f64bits.exp - B.f64bits.exp);
        }

        int preShift;
        uint64_t expMore;
        uint64_t mantMore;
        uint64_t mantLess;
        uint64_t signOpMore;
        uint64_t flEqual;
        uint64_t flLess;
        uint64_t flMore;

        if (expDif > 0) {
            flMore = !signA;
            flEqual = 0;
            flLess = signA;

            preShift = expDif;
            signOpMore = signA;
            expMore = A.f64bits.exp;
            mantMore = mantA;
            mantLess = mantB;
        } else if (expDif == 0) {
            preShift = expDif;
            if (mantA == mantB) {
                flMore = (!signA) & (signA ^ signB);
                flEqual = !(signA ^ signB);
                flLess = signA & (signA ^ signB);

                signOpMore = signA;
                expMore = A.f64bits.exp;
                mantMore = mantA;
                mantLess = mantB;
            } else if (mantA > mantB) {
                flMore = !signA;
                flEqual = 0;
                flLess = signA;

                signOpMore = signA;
                expMore = A.f64bits.exp;
                mantMore = mantA;
                mantLess = mantB;
            } else {
                flMore = signB;
                flEqual = 0;
                flLess = !signB;

                signOpMore = signOpB;
                expMore = B.f64bits.exp;
                mantMore = mantB;
                mantLess = mantA;
            }
        } else {
            flMore = signB;
            flEqual = 0;
            flLess = !signB;

            preShift = -expDif;
            signOpMore = signOpB;
            expMore = B.f64bits.exp;
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
            if (signA ^ signOpB) {
                // DIFFERENCE computation: D = A - B
                //      iCar[0]   = 0
                //      iDif[k]   = iCar[k]^iA[k]^iB[k];
                //      iCar[k+1] = (!iA[k]&iB[k]) | (iCar[k]& !(iA[k]^iB[k]));
                mantSum[i] = mantSumCarryBit[i] ^ wbMantMore[i] ^ mantLessScale[i];
                mantSumCarryBit[i+1] = ((!wbMantMore[i]) & mantLessScale[i])
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
        if (signA ^ signOpB) {
            // subtractor only: result value becomes with exp=0
            if (expMore != 0 && expPostScale <= 0) {
                expPostScale -= 1;
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
            resAdd.sign = signA ^ signOpB;
        } else if (nanA) {
            /** when both values are NaN, value B has higher priority if sign=1 */
            resAdd.sign = signA | (nanB & signOpB);
        } else if (nanB) {
            resAdd.sign = signOpB ^ (signOp & !mantZeroB);
        } else if (allZero) {
            resAdd.sign = signA & signOpB;
        } else if (sumZero) {
            resAdd.sign = 0;
        } else {
            resAdd.sign = signOpMore; 
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
        } else if (nanA && !(nanB & signOpB)) {
            /** when both values are NaN, value B has higher priority if sign=1 */
            resAdd.mant = A.f64bits.mant | 0x0008000000000000;
        } else if (nanB) {
            resAdd.mant = B.f64bits.mant | 0x0008000000000000;
        } else if (overflow) {
            resAdd.mant = 0;
        } else {
            resAdd.mant = mantShort + rndBit;
        }

        // More value
        Reg64Type resMore;
        if (nanA | nanB) {
            resMore.val = B.val;
        } else if (flMore) {
            resMore.val = A.val;
        } else {
            resMore.val = B.val;
        }

        // Less value                              
        Reg64Type resLess;
        if (nanA | nanB) {
            resLess.val = B.val;
        } else if (flLess) {
            resLess.val = A.val;
        } else {
            resLess.val = B.val;
        }
  
        if (cmpEna) {
            fres->val = (equalEna & flEqual) | (moreEna & flMore) | (lessEna & flLess);
        } else if (moreEna) {
            *fres = resMore;
        } else if (lessEna) {
            *fres = resLess;
        } else {
            fres->f64bits = resAdd;
        }

        except = nanA | nanB | overflow;
    }

    void Int2Double(int signEna, int w32, Reg64Type A, Reg64Type *fres) {
        Reg64Type tmpA;
        if (w32) {
            tmpA.buf32[0] = A.buf32[0];
            if (signEna && A.bits.b31) {
                tmpA.buf32[1] = ~0u;
            } else {
                tmpA.buf32[1] = 0;
            }
        } else {
            tmpA = A;
        }
        uint64_t absA = signEna & tmpA.f64bits.sign ? ~tmpA.val + 1: tmpA.val;
        // multiplexer
        int rbLShift = 63;
        for (int i = 0; i < 64; i++) {
            if ((absA >> (63 - i)) & 0x1) {
                rbLShift = i;
                break;
            }
        }

        uint64_t mantAlign = absA << rbLShift;
        uint64_t mantShort = mantAlign >> 11;
        uint64_t expAlign;
        if (absA == 0) {
            expAlign = 0;
        } else {
            expAlign = 1023 + 63 - rbLShift;
        }

        // rounding bit
        unsigned mantEven = (mantAlign >> 11) & 0x1;
        uint64_t tmpMant05 = mantAlign & 0x7FF;
        unsigned mant05 = tmpMant05 == 0x0000000000400 ? 1: 0;
        uint64_t rndBit = ((mantAlign >> 10) & 0x1) & !(mant05 & !mantEven);

        uint64_t mantOnes = mantShort == 0x001fffffffffffff ? 1: 0;

        fres->f64bits.sign = signEna & tmpA.f64bits.sign;
        fres->f64bits.exp = expAlign + (mantOnes & rndBit);
        fres->f64bits.mant = mantShort + rndBit;
    }

    void Double2Int(int signEna,
                    int w32,
                    Reg64Type A,
                    Reg64Type *fres,
                    int &ovr,
                    int &und) {
        uint64_t mantA = A.f64bits.mant;
        mantA |= A.f64bits.exp ? 0x0010000000000000ull: 0;

        uint64_t mantPreScale;
        uint64_t expDif;
        uint64_t expMax;
        uint64_t mantShort;
        uint64_t resSign;

        mantPreScale = mantA << 11;
        expDif = (1023u + 63u) - A.f64bits.exp;
        if (w32) {
            if (signEna) {
                expMax = 1023 + 30;
            } else {
    #ifdef FCVT_WU_D_X86_COMPATIBLE
                expMax = 1023 + 62;
    #else
                expMax = 1023 + 31;
    #endif
            }
        } else {
            if (signEna || A.f64bits.sign) {
                expMax = 1023 + 62;
            } else {
                expMax = 1023 + 63;
            }
        }

        if (A.f64bits.exp > expMax) {
            ovr = 1;
            und = 0;
            mantShort = 0;
        } else if (A.f64bits.exp < 1023) {
            ovr = 0;
            und = 1;
            mantShort = 0;
        } else {
            ovr = 0;
            und = 0;
            mantShort = mantPreScale >> expDif;
        }

        resSign = (A.f64bits.sign | ovr) & !und;
        if (signEna) {
            fres->val = A.f64bits.sign ? ~mantShort + 1: mantShort;
            if (resSign) {
                if (w32) {
                    fres->val |= 0xFFFFFFFF80000000ull;
                } else {
                    fres->f64bits.sign = 1;
                }
            }
        } else {
            fres->val = A.f64bits.sign ? ~mantShort + 1: mantShort;
            if (w32) {
                fres->buf32[1] = 0;
    #if!defined FCVT_WU_D_X86_COMPATIBLE
                if (ovr) {
                    fres->bits.b31 = 1;
                }
    #endif
            } else if (ovr) {
                fres->f64bits.sign = 1;
            }
        }
    }

};


/**
 * @brief The FADD.D double precision adder
 */
class FADD_D : public FpuInstruction {
 public:
    FADD_D(CpuRiver_Functional *icpu) : FpuInstruction(icpu,
        "FADD_D", "0000001??????????????????1010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        Reg64Type dest, src1, src2;
        u.value = payload->buf32[0];
        src1.val = RF[u.bits.rs1];
        src2.val = RF[u.bits.rs2];
        int except = 0;
        AddSubCompare(1, 0, 0, 0, 0, 0,
                       src1, src2, &dest, except);
        //dest.f64 = src1.f64 + src2.f64;
        icpu_->setReg(ICpuRiscV::RegFpu_Offset + u.bits.rd, dest.val);
        return 4;
    }
};

/**
 * @brief The FCVT.D.L covert int64_t to double
 */
class FCVT_D_L: public FpuInstruction {
 public:
    FCVT_D_L(CpuRiver_Functional *icpu) : FpuInstruction(icpu,
        "FCVT_D_L", "110100100010?????????????1010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        Reg64Type dest, src1;
        u.value = payload->buf32[0];
        src1.val = R[u.bits.rs1];
        Int2Double(1, 0, src1, &dest);
        //dest.f64 = static_cast<double>(src1.ival);
        icpu_->setReg(ICpuRiscV::RegFpu_Offset + u.bits.rd, dest.val);
        return 4;
    }
};

/**
 * @brief The FCVT.D.LU covert uint64_t to double
 */
class FCVT_D_LU: public FpuInstruction {
 public:
    FCVT_D_LU(CpuRiver_Functional *icpu) : FpuInstruction(icpu,
        "FCVT_D_LU", "110100100011?????????????1010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        Reg64Type dest, src1;
        u.value = payload->buf32[0];
        src1.val = R[u.bits.rs1];
        Int2Double(0, 0, src1, &dest);
        //dest.f64 = static_cast<double>(src1.val);
        icpu_->setReg(ICpuRiscV::RegFpu_Offset + u.bits.rd, dest.val);
        return 4;
    }
};

/**
 * @brief The FCVT.D.W covert int32_t to double
 */
class FCVT_D_W: public FpuInstruction {
 public:
    FCVT_D_W(CpuRiver_Functional *icpu) : FpuInstruction(icpu,
        "FCVT_D_W", "110100100000?????????????1010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        Reg64Type dest, src1;
        u.value = payload->buf32[0];
        src1.val = R[u.bits.rs1];
        Int2Double(1, 1, src1, &dest);
        //dest.f64 = static_cast<double>(static_cast<int>(src1.buf32[0]));
        icpu_->setReg(ICpuRiscV::RegFpu_Offset + u.bits.rd, dest.val);
        return 4;
    }
};

/**
 * @brief The FCVT.D.WU covert uint32_t to double
 */
class FCVT_D_WU: public FpuInstruction {
 public:
    FCVT_D_WU(CpuRiver_Functional *icpu) : FpuInstruction(icpu,
        "FCVT_D_WU", "110100100001?????????????1010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        Reg64Type dest, src1;
        u.value = payload->buf32[0];
        src1.val = R[u.bits.rs1];
        Int2Double(0, 1, src1, &dest);
        //dest.f64 = static_cast<double>(src1.buf32[0]);
        icpu_->setReg(ICpuRiscV::RegFpu_Offset + u.bits.rd, dest.val);
        return 4;
    }
};

/**
 * @brief The FCVT.L.D covert double to int64_t
 */
class FCVT_L_D: public FpuInstruction {
 public:
    FCVT_L_D(CpuRiver_Functional *icpu) : FpuInstruction(icpu,
        "FCVT_L_D", "110000100010?????????????1010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        Reg64Type dest, src1;
        u.value = payload->buf32[0];
        src1.val = RF[u.bits.rs1];
        int ovr, und;
        Double2Int(1, 0, src1, &dest, ovr, und);
        //dest.ival = static_cast<int64_t>(src1.f64);
        icpu_->setReg(u.bits.rd, dest.val);
        return 4;
    }
};

/**
 * @brief The FCVT.LU.D covert double to uint64_t
 */
class FCVT_LU_D : public FpuInstruction {
 public:
    FCVT_LU_D(CpuRiver_Functional *icpu) : FpuInstruction(icpu,
        "FCVT_LU_D", "110000100011?????????????1010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        Reg64Type dest, src1;
        u.value = payload->buf32[0];
        src1.val = RF[u.bits.rs1];
        int ovr, und;
        Double2Int(0, 0, src1, &dest, ovr, und);
        //dest.val = static_cast<uint64_t>(src1.f64);
        icpu_->setReg(u.bits.rd, dest.val);
        return 4;
    }
};

/**
 * @brief The FCVT.W.D covert double to int32_t
 */
class FCVT_W_D : public FpuInstruction {
 public:
    FCVT_W_D(CpuRiver_Functional *icpu) : FpuInstruction(icpu,
        "FCVT_W_D", "110000100000?????????????1010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        Reg64Type dest, src1;
        u.value = payload->buf32[0];
        src1.val = RF[u.bits.rs1];
        int ovr, und;
        Double2Int(1, 1, src1, &dest, ovr, und);
        //dest.ival = static_cast<int32_t>(src1.f64);
        icpu_->setReg(u.bits.rd, dest.val);
        return 4;
    }
};

/**
 * @brief The FCVT.WU.D covert double to uint32_t
 */
class FCVT_WU_D : public FpuInstruction {
 public:
    FCVT_WU_D(CpuRiver_Functional *icpu) : FpuInstruction(icpu,
        "FCVT_WU_D", "110000100001?????????????1010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        Reg64Type dest, src1;
        u.value = payload->buf32[0];
        src1.val = RF[u.bits.rs1];
        int ovr, und;
        Double2Int(0, 1, src1, &dest, ovr, und);
        //dest.val = static_cast<uint32_t>(src1.f64);
        icpu_->setReg(u.bits.rd, dest.val);
        return 4;
    }
};

/**
 * @brief The FDIV.D double precision division
 */
class FDIV_D : public FpuInstruction {
 public:
    FDIV_D(CpuRiver_Functional *icpu) : FpuInstruction(icpu,
        "FDIV_D", "0001101??????????????????1010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        Reg64Type dest, A, B;
        u.value = payload->buf32[0];
        A.val = RF[u.bits.rs1];
        B.val = RF[u.bits.rs2];

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
            dest.f64bits.sign = 1;
        } else if (nanA & !mantZeroA) {
            dest.f64bits.sign = A.f64bits.sign;
        } else if (nanB & !mantZeroB) {
            dest.f64bits.sign = B.f64bits.sign;
        } else if (divOnZero && zeroA) {
            dest.f64bits.sign = 1;
        } else {
            dest.f64bits.sign = A.f64bits.sign ^ B.f64bits.sign; 
        }

        if (nanB & !mantZeroB) {
            dest.f64bits.exp = B.f64bits.exp;
        } else if ((underflow | zeroA | zeroB) & !divOnZero) {
            dest.f64bits.exp = 0x0;
        } else if (overflow | divOnZero) {
            dest.f64bits.exp = 0x7FF;
        } else if (nanA) {
            dest.f64bits.exp = A.f64bits.exp;
        } else if ((nanB & mantZeroB) || expAlign < 0) {
            dest.f64bits.exp = 0x0;
        } else {
            dest.f64bits.exp = expAlign + (mantOnes & rndBit & !overflow);
        }

        if ((zeroA & zeroB) | (nanA & mantZeroA & nanB & mantZeroB)) {
            dest.f64bits.mant = 0x8000000000000;
        } else if (nanA & !mantZeroA) {
            dest.f64bits.mant = A.f64bits.mant | 0x8000000000000;
        } else if (nanB & !mantZeroB) {
            dest.f64bits.mant = B.f64bits.mant | 0x8000000000000;
        } else if (overflow | nanRes | (nanA & mantZeroA) | (nanB & mantZeroB)) {
            dest.f64bits.mant = 0x0;
        } else {
            dest.f64bits.mant = mantShort + rndBit;
        }

        if (RF[u.bits.rs2] == 0) {
            csr_fcsr_type fcsr;
            fcsr.value = icpu_->readCSR(ICpuRiscV::CSR_fcsr);
            fcsr.bits.DZ = 1;
            icpu_->writeCSR(ICpuRiscV::CSR_fcsr, fcsr.value);
        }
        icpu_->setReg(ICpuRiscV::RegFpu_Offset + u.bits.rd, dest.val);
        return 4;
    }
};

/**
 * @brief The FEQ.D quiet comparision
 */
class FEQ_D : public FpuInstruction {
 public:
    FEQ_D(CpuRiver_Functional *icpu) : FpuInstruction(icpu,
        "FEQ_D", "1010001??????????010?????1010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        Reg64Type src1, src2, dest;
        //uint64_t eq = 0;
        int except = 0;
        u.value = payload->buf32[0];
        src1.val = RF[u.bits.rs1];
        src2.val = RF[u.bits.rs2];
        AddSubCompare(0, 0, 1, 1, 0, 0, src1, src2, &dest, except);
        if (src1.f64bits.exp == 0x7FF || src2.f64bits.exp == 0x7FF) {
            /** Do not cause trap, only signal Invalid Operation */
            csr_fcsr_type fcsr;
            fcsr.value = icpu_->readCSR(ICpuRiscV::CSR_fcsr);
            fcsr.bits.NV = 1;
            icpu_->writeCSR(ICpuRiscV::CSR_fcsr, fcsr.value);
        }
        //else {
        //    eq = src1.val == src2.val ? 1ull: 0;
        //}
        icpu_->setReg(u.bits.rd, dest.val);
        return 4;
    }
};

/** @brief The FLD loads a double-precision floating-point value from memory
 *         into floating-point register rd.
 */
class FLD : public FpuInstruction {
public:
    FLD(CpuRiver_Functional *icpu) :
        FpuInstruction(icpu, "FLD", "?????????????????011?????0000111") {}

    virtual int exec(Reg64Type *payload) {
        Axi4TransactionType trans;
        Reg64Type dst;
        ISA_I_type u;
        u.value = payload->buf32[0];
        uint64_t off = u.bits.imm;
        if (off & 0x800) {
            off |= EXT_SIGN_12;
        }
        trans.action = MemAction_Read;
        trans.addr = R[u.bits.rs1] + off;
        trans.xsize = 8;
        if (trans.addr & 0x7) {
            trans.rpayload.b64[0] = 0;
            icpu_->generateException(ICpuRiscV::EXCEPTION_LoadMisalign, icpu_->getPC());
        } else {
            if (icpu_->dma_memop(&trans) == TRANS_ERROR) {
                icpu_->generateException(ICpuRiscV::EXCEPTION_LoadFault, trans.addr);
            }
        }
        dst.val = trans.rpayload.b64[0];
        icpu_->setReg(ICpuRiscV::RegFpu_Offset + u.bits.rd, dst.val);
        return 4;
    }
};

/**
 * @brief The FLE.D quiet comparision less or equal
 */
class FLE_D : public FpuInstruction {
 public:
    FLE_D(CpuRiver_Functional *icpu) : FpuInstruction(icpu,
        "FLE_D", "1010001??????????000?????1010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        Reg64Type src1, src2, dest;
        //uint64_t le = 0;
        int except = 0;
        u.value = payload->buf32[0];
        src1.val = RF[u.bits.rs1];
        src2.val = RF[u.bits.rs2];
        AddSubCompare(0, 0, 1, 1, 0, 1, src1, src2, &dest, except);
        if (src1.f64bits.exp == 0x7FF || src2.f64bits.exp == 0x7FF) {
            /** Do not cause trap, only signal Invalid Operation */
            csr_fcsr_type fcsr;
            fcsr.value = icpu_->readCSR(ICpuRiscV::CSR_fcsr);
            fcsr.bits.NV = 1;
            icpu_->writeCSR(ICpuRiscV::CSR_fcsr, fcsr.value);
        }
        //else {
        //    le = src1.f64 <= src2.f64 ? 1ull: 0;
        //}
        icpu_->setReg(u.bits.rd, dest.val);
        return 4;
    }
};

/**
 * @brief The FLT.D quiet comparision less than
 */
class FLT_D : public FpuInstruction {
 public:
    FLT_D(CpuRiver_Functional *icpu) : FpuInstruction(icpu,
        "FLT_D", "1010001??????????001?????1010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        Reg64Type src1, src2, dest;
        int except = 0;
        //uint64_t le = 0;
        u.value = payload->buf32[0];
        src1.val = RF[u.bits.rs1];
        src2.val = RF[u.bits.rs2];
        AddSubCompare(0, 0, 1, 0, 0, 1, src1, src2, &dest, except);
        if (src1.f64bits.exp == 0x7FF || src2.f64bits.exp == 0x7FF) {
            /** Do not cause trap, only signal Invalid Operation */
            csr_fcsr_type fcsr;
            fcsr.value = icpu_->readCSR(ICpuRiscV::CSR_fcsr);
            fcsr.bits.NV = 1;
            icpu_->writeCSR(ICpuRiscV::CSR_fcsr, fcsr.value);
        }
        //else {
        //    le = src1.f64 < src2.f64 ? 1ull: 0;
        //}
        icpu_->setReg(u.bits.rd, dest.val);
        return 4;
    }
};

/**
 * @brief The FMAX.D select maximum
 */
class FMAX_D : public FpuInstruction {
 public:
    FMAX_D(CpuRiver_Functional *icpu) : FpuInstruction(icpu,
        "FMAX_D", "0010101??????????001?????1010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        Reg64Type dest, src1, src2;
        int except = 0;
        u.value = payload->buf32[0];
        src1.val = RF[u.bits.rs1];
        src2.val = RF[u.bits.rs2];
        AddSubCompare(0, 0, 0, 0, 1, 0, src1, src2, &dest, except);
        //dest.f64 = src1.f64 > src2.f64 ? src1.f64: src2.f64;
        icpu_->setReg(ICpuRiscV::RegFpu_Offset + u.bits.rd, dest.val);
        return 4;
    }
};

/**
 * @brief The FMAX.D select minimum
 */
class FMIN_D : public FpuInstruction {
 public:
    FMIN_D(CpuRiver_Functional *icpu) : FpuInstruction(icpu,
        "FMIN_D", "0010101??????????000?????1010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        Reg64Type dest, src1, src2;
        int except = 0;
        u.value = payload->buf32[0];
        src1.val = RF[u.bits.rs1];
        src2.val = RF[u.bits.rs2];
        AddSubCompare(0, 0, 0, 0, 0, 1, src1, src2, &dest, except);
        //dest.f64 = src1.f64 < src2.f64 ? src1.f64: src2.f64;
        icpu_->setReg(ICpuRiscV::RegFpu_Offset + u.bits.rd, dest.val);
        return 4;
    }
};

/**
 * @brief The FMOV.D.X move value from integer register into fp register
 */
class FMOV_D_X : public FpuInstruction {
 public:
    FMOV_D_X(CpuRiver_Functional *icpu) : FpuInstruction(icpu,
        "FMOV_D_X", "111100100000?????000?????1010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        Reg64Type src1;
        u.value = payload->buf32[0];
        src1.val = R[u.bits.rs1];
        icpu_->setReg(ICpuRiscV::RegFpu_Offset + u.bits.rd, src1.val);
        return 4;
    }
};

/**
 * @brief The FMOV.X.D move fp value into integer register
 */
class FMOV_X_D : public FpuInstruction {
 public:
    FMOV_X_D(CpuRiver_Functional *icpu) : FpuInstruction(icpu,
        "FMOV_X_D", "111000100000?????000?????1010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        Reg64Type src1;
        u.value = payload->buf32[0];
        src1.val = RF[u.bits.rs1];
        icpu_->setReg(u.bits.rd, src1.val);
        return 4;
    }
};

/**
 * @brief The FMUL.D double precision multiplication
 */
class FMUL_D : public FpuInstruction {
 public:
    FMUL_D(CpuRiver_Functional *icpu) : FpuInstruction(icpu,
        "FMUL_D", "0001001??????????????????1010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        Reg64Type dest, A, B;
        u.value = payload->buf32[0];
        A.val = RF[u.bits.rs1];
        B.val = RF[u.bits.rs2];

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
            dest.f64bits.sign = 1;
        } else if (nanA & !mantZeroA) {
            /** when both values are NaN, value B has higher priority if sign=1 */
            dest.f64bits.sign = A.f64bits.sign | (nanB & B.f64bits.sign);
        } else if (nanB & !mantZeroB) {
            dest.f64bits.sign = B.f64bits.sign;
        } else {
            dest.f64bits.sign = A.f64bits.sign ^ B.f64bits.sign; 
        }

        if (nanA) {
            dest.f64bits.exp = A.f64bits.exp;
        } else if (nanB) {
            dest.f64bits.exp = B.f64bits.exp;
        } else if (expAlign < 0 || zeroA || zeroB) {
            dest.f64bits.exp = 0;
        } else if (overflow) {
            dest.f64bits.exp = 0x7FF;
        } else {
            dest.f64bits.exp = expAlign + (mantOnes & rndBit & !overflow);
        }

        if ((nanA & mantZeroA & !mantZeroB) || (nanB & mantZeroB & !mantZeroA)
            || (!nanA & !nanB & overflow)) {
            dest.f64bits.mant = 0;
        } else if (nanA && !(nanB & B.f64bits.sign)) {
            /** when both values are NaN, value B has higher priority if sign=1 */
            dest.f64bits.mant = A.f64bits.mant | 0x8000000000000;
        } else if (nanB) {
            dest.f64bits.mant = B.f64bits.mant | 0x8000000000000;
        } else {
            dest.f64bits.mant = mantShort + rndBit;
        }

        //except = nanA | nanB | overflow;
        //dest.f64 = src1.f64 * src2.f64;
        icpu_->setReg(ICpuRiscV::RegFpu_Offset + u.bits.rd, dest.val);
        return 4;
    }
};

/** @brief The FSD stores a double-precision value from the floating-point registers
           to memory.
 */
class FSD : public FpuInstruction {
public:
    FSD(CpuRiver_Functional *icpu) :
        FpuInstruction(icpu, "FSD", "?????????????????011?????0100111") {}

    virtual int exec(Reg64Type *payload) {
        Axi4TransactionType trans;
        ISA_S_type u;
        u.value = payload->buf32[0];
        uint64_t off = (u.bits.imm11_5 << 5) | u.bits.imm4_0;
        if (off & 0x800) {
            off |= EXT_SIGN_12;
        }
        trans.action = MemAction_Write;
        trans.xsize = 8;
        trans.wstrb = (1 << trans.xsize) - 1;
        trans.addr = R[u.bits.rs1] + off;
        trans.wpayload.b64[0] = RF[u.bits.rs2];
        if (trans.addr & 0x7) {
            icpu_->generateException(ICpuRiscV::EXCEPTION_StoreMisalign, icpu_->getPC());
        } else {
            if (icpu_->dma_memop(&trans) == TRANS_ERROR) {
                icpu_->generateException(ICpuRiscV::EXCEPTION_StoreFault, trans.addr);
            }
        }
        return 4;
    }
};

/**
 * @brief The FSUB.D double precision subtractor
 */
class FSUB_D : public FpuInstruction {
 public:
    FSUB_D(CpuRiver_Functional *icpu) : FpuInstruction(icpu,
        "FSUB_D", "0000101??????????????????1010011") {}

    virtual int exec(Reg64Type *payload) {
        ISA_R_type u;
        Reg64Type dest, src1, src2;
        int except = 0;
        u.value = payload->buf32[0];
        src1.val = RF[u.bits.rs1];
        src2.val = RF[u.bits.rs2];
        AddSubCompare(0, 1, 0, 0, 0, 0, src1, src2, &dest, except);
        //dest.f64 = src1.f64 - src2.f64;
        icpu_->setReg(ICpuRiscV::RegFpu_Offset + u.bits.rd, dest.val);
        return 4;
    }
};

void CpuRiver_Functional::addIsaExtensionD() {
    addSupportedInstruction(new FADD_D(this));
    addSupportedInstruction(new FCVT_D_L(this));
    addSupportedInstruction(new FCVT_D_LU(this));
    addSupportedInstruction(new FCVT_D_W(this));
    addSupportedInstruction(new FCVT_D_WU(this));
    addSupportedInstruction(new FCVT_L_D(this));
    addSupportedInstruction(new FCVT_LU_D(this));
    addSupportedInstruction(new FCVT_W_D(this));
    addSupportedInstruction(new FCVT_WU_D(this));
    addSupportedInstruction(new FDIV_D(this));
    addSupportedInstruction(new FEQ_D(this));
    addSupportedInstruction(new FLD(this));
    addSupportedInstruction(new FLE_D(this));
    addSupportedInstruction(new FLT_D(this));
    addSupportedInstruction(new FMAX_D(this));
    addSupportedInstruction(new FMIN_D(this));
    addSupportedInstruction(new FMOV_D_X(this));
    addSupportedInstruction(new FMOV_X_D(this));
    addSupportedInstruction(new FMUL_D(this));
    addSupportedInstruction(new FSD(this));
    addSupportedInstruction(new FSUB_D(this));

    portCSR_.write(CSR_misa, portCSR_.read(CSR_misa).val | (1LL << ('D' - 'A')));
}

void CpuRiver_Functional::addIsaExtensionF() {
    // TODO
    /*
    addInstr("FADD_S",             "0000000??????????????????1010011", NULL, out);
    addInstr("FSUB_S",             "0000100??????????????????1010011", NULL, out);
    addInstr("FMUL_S",             "0001000??????????????????1010011", NULL, out);
    addInstr("FDIV_S",             "0001100??????????????????1010011", NULL, out);
    addInstr("FSGNJ_S",            "0010000??????????000?????1010011", NULL, out);
    addInstr("FSGNJN_S",           "0010000??????????001?????1010011", NULL, out);
    addInstr("FSGNJX_S",           "0010000??????????010?????1010011", NULL, out);
    addInstr("FMIN_S",             "0010100??????????000?????1010011", NULL, out);
    addInstr("FMAX_S",             "0010100??????????001?????1010011", NULL, out);
    addInstr("FSQRT_S",            "010110000000?????????????1010011", NULL, out);
    addInstr("FLE_S",              "1010000??????????000?????1010011", NULL, out);
    addInstr("FLT_S",              "1010000??????????001?????1010011", NULL, out);
    addInstr("FEQ_S",              "1010000??????????010?????1010011", NULL, out);
    addInstr("FCVT_W_S",           "110000000000?????????????1010011", NULL, out);
    addInstr("FCVT_WU_S",          "110000000001?????????????1010011", NULL, out);
    addInstr("FCVT_L_S",           "110000000010?????????????1010011", NULL, out);
    addInstr("FCVT_LU_S",          "110000000011?????????????1010011", NULL, out);
    addInstr("FMV_X_S",            "111000000000?????000?????1010011", NULL, out);
    addInstr("FCLASS_S",           "111000000000?????001?????1010011", NULL, out);
    addInstr("FCVT_S_W",           "110100000000?????????????1010011", NULL, out);
    addInstr("FCVT_S_WU",          "110100000001?????????????1010011", NULL, out);
    addInstr("FCVT_S_L",           "110100000010?????????????1010011", NULL, out);
    addInstr("FCVT_S_LU",          "110100000011?????????????1010011", NULL, out);
    addInstr("FLW",                "?????????????????010?????0000111", NULL, out);
    addInstr("FSW",                "?????????????????010?????0100111", NULL, out);
    addInstr("FMV_S_X",            "111100000000?????000?????1010011", NULL, out);
    addInstr("FMADD_S",            "?????00??????????????????1000011", NULL, out);
    addInstr("FMSUB_S",            "?????00??????????????????1000111", NULL, out);
    addInstr("FNMSUB_S",           "?????00??????????????????1001011", NULL, out);
    addInstr("FNMADD_S",           "?????00??????????????????1001111", NULL, out);

    addInstr("FCVT_S_D",           "010000000001?????????????1010011", NULL, out);
    addInstr("FCVT_D_S",           "010000100000?????????????1010011", NULL, out);

    addInstr("FADD_D",             "0000001??????????????????1010011", NULL, out);
    addInstr("FSUB_D",             "0000101??????????????????1010011", NULL, out);
    addInstr("FMUL_D",             "0001001??????????????????1010011", NULL, out);
    addInstr("FDIV_D",             "0001101??????????????????1010011", NULL, out);
    addInstr("FSGNJ_D",            "0010001??????????000?????1010011", NULL, out);
    addInstr("FSGNJN_D",           "0010001??????????001?????1010011", NULL, out);
    addInstr("FSGNJX_D",           "0010001??????????010?????1010011", NULL, out);
    addInstr("FMIN_D",             "0010101??????????000?????1010011", NULL, out);
    addInstr("FMAX_D",             "0010101??????????001?????1010011", NULL, out);
    addInstr("FSQRT_D",            "010110100000?????????????1010011", NULL, out);
    addInstr("FLE_D",              "1010001??????????000?????1010011", NULL, out);
    addInstr("FLT_D",              "1010001??????????001?????1010011", NULL, out);
    addInstr("FEQ_D",              "1010001??????????010?????1010011", NULL, out);
    addInstr("FCVT_W_D",           "110000100000?????????????1010011", NULL, out);
    addInstr("FCVT_WU_D",          "110000100001?????????????1010011", NULL, out);
    addInstr("FCVT_L_D",           "110000100010?????????????1010011", NULL, out);
    addInstr("FCVT_LU_D",          "110000100011?????????????1010011", NULL, out);
    addInstr("FMV_X_D",            "111000100000?????000?????1010011", NULL, out);
    addInstr("FCLASS_D",           "111000100000?????001?????1010011", NULL, out);
    addInstr("FCVT_D_W",           "110100100000?????????????1010011", NULL, out);
    addInstr("FCVT_D_WU",          "110100100001?????????????1010011", NULL, out);
    addInstr("FCVT_D_L",           "110100100010?????????????1010011", NULL, out);
    addInstr("FCVT_D_LU",          "110100100011?????????????1010011", NULL, out);
    addInstr("FMV_D_X",            "111100100000?????000?????1010011", NULL, out);
    addInstr("FLD",                "?????????????????011?????0000111", NULL, out);
    addInstr("FSD",                "?????????????????011?????0100111", NULL, out);
    addInstr("FMADD_D",            "?????01??????????????????1000011", NULL, out);
    addInstr("FMSUB_D",            "?????01??????????????????1000111", NULL, out);
    addInstr("FNMSUB_D",           "?????01??????????????????1001011", NULL, out);
    addInstr("FNMADD_D",           "?????01??????????????????1001111", NULL, out);

    // pseudo-instruction of access to CSR
    def FRFLAGS            = BitPat("b00000000000100000010?????1110011")
    def FSFLAGS            = BitPat("b000000000001?????001?????1110011")
    def FSFLAGSI           = BitPat("b000000000001?????101?????1110011")
    def FRRM               = BitPat("b00000000001000000010?????1110011")
    def FSRM               = BitPat("b000000000010?????001?????1110011")
    def FSRMI              = BitPat("b000000000010?????101?????1110011")
    def FSCSR              = BitPat("b000000000011?????001?????1110011")
    def FRCSR              = BitPat("b00000000001100000010?????1110011")
    */
    portCSR_.write(CSR_misa, portCSR_.read(CSR_misa).val | (1LL << ('F' - 'A')));
}

}  // namespace debugger
