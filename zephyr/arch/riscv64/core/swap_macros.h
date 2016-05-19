/* swap_macros.h - helper macros for context switch */

/*
 * Copyright (c) 2014 Wind River Systems, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _SWAP_MACROS__H_
#define _SWAP_MACROS__H_

/** 
 * Saved by callee function registers:
 *      s0..s11, sp, tp 
 */
#define COOP_REGS_OFFSET    (3*sizeof(uint64_t))
#define COOP_REG_S0         (COOP_REGS_OFFSET + 0*sizeof(uint64_t))
#define COOP_REG_S1         (COOP_REGS_OFFSET + 1*sizeof(uint64_t))
#define COOP_REG_S2         (COOP_REGS_OFFSET + 2*sizeof(uint64_t))
#define COOP_REG_S3         (COOP_REGS_OFFSET + 3*sizeof(uint64_t))
#define COOP_REG_S4         (COOP_REGS_OFFSET + 4*sizeof(uint64_t))
#define COOP_REG_S5         (COOP_REGS_OFFSET + 5*sizeof(uint64_t))
#define COOP_REG_S6         (COOP_REGS_OFFSET + 6*sizeof(uint64_t))
#define COOP_REG_S7         (COOP_REGS_OFFSET + 7*sizeof(uint64_t))
#define COOP_REG_S8         (COOP_REGS_OFFSET + 8*sizeof(uint64_t))
#define COOP_REG_S9         (COOP_REGS_OFFSET + 9*sizeof(uint64_t))
#define COOP_REG_S10        (COOP_REGS_OFFSET + 10*sizeof(uint64_t))
#define COOP_REG_S11        (COOP_REGS_OFFSET + 11*sizeof(uint64_t))
#define COOP_REG_SP         (COOP_REGS_OFFSET + 12*sizeof(uint64_t))
#define COOP_REG_TP         (COOP_REGS_OFFSET + 13*sizeof(uint64_t))
#define COOP_REGS_TOTAL     14
#define COOP_STACKFRAME_SIZE (COOP_REGS_TOTAL*sizeof(uint64_t))


/* entering this macro, current is in r2 */
#define _save_callee_saved_regs() \
  addi sp, sp, -272; \
  sd x1, 8(sp); \
  sd x2, 16(sp); \
  sd x3, 24(sp); \
  sd x4, 32(sp); \
  sd x5, 40(sp); \
  sd x6, 48(sp); \
  sd x7, 56(sp); \
  sd x8, 64(sp); \
  sd x9, 72(sp); \
  sd x10, 80(sp); \
  sd x11, 88(sp); \
  sd x12, 96(sp); \
  sd x13, 104(sp); \
  sd x14, 112(sp); \
  sd x15, 120(sp); \
  sd x16, 128(sp); \
  sd x17, 136(sp); \
  sd x18, 144(sp); \
  sd x19, 152(sp); \
  sd x20, 160(sp); \
  sd x21, 168(sp); \
  sd x22, 176(sp); \
  sd x23, 184(sp); \
  sd x24, 192(sp); \
  sd x25, 200(sp); \
  sd x26, 208(sp); \
  sd x27, 216(sp); \
  sd x28, 224(sp); \
  sd x29, 232(sp); \
  sd x30, 240(sp); \
  sd x31, 248(sp); \

	/* save stack pointer in struct tcs */
	//st sp, [r2, __tTCS_preempReg_OFFSET + __tPreempt_sp_OFFSET]


/* entering this macro, current is in r2 */
	/* restore stack pointer from struct tcs */
	//ld sp, [r2, __tTCS_preempReg_OFFSET + __tPreempt_sp_OFFSET]
#define _load_callee_saved_regs() \
  ld x1, 8(sp); \
  ld x2, 16(sp); \
  ld x3, 24(sp); \
  ld x4, 32(sp); \
  ld x5, 40(sp); \
  ld x6, 48(sp); \
  ld x7, 56(sp); \
  ld x8, 64(sp); \
  ld x9, 72(sp); \
  ld x10, 80(sp); \
  ld x11, 88(sp); \
  ld x12, 96(sp); \
  ld x13, 104(sp); \
  ld x14, 112(sp); \
  ld x15, 120(sp); \
  ld x16, 128(sp); \
  ld x17, 136(sp); \
  ld x18, 144(sp); \
  ld x19, 152(sp); \
  ld x20, 160(sp); \
  ld x21, 168(sp); \
  ld x22, 176(sp); \
  ld x23, 184(sp); \
  ld x24, 192(sp); \
  ld x25, 200(sp); \
  ld x26, 208(sp); \
  ld x27, 216(sp); \
  ld x28, 224(sp); \
  ld x29, 232(sp); \
  ld x30, 240(sp); \
  ld x31, 248(sp); \
  addi sp, sp, 272; \


#endif /*  _SWAP_MACROS__H_ */
