/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
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

/**
 * @file
 * @brief MSVC toolchain abstraction
 *
 * Macros to abstract compiler capabilities for MSVC toolchain.
 */
#include <toolchain/common.h>

#define _ALIAS_OF(of) __attribute__((alias(#of)))

#define CODE_UNREACHABLE 
#define FUNC_NORETURN    
#define FUNC_NO_FP

/* The GNU assembler for Cortex-M3 uses # for immediate values, not
 * comments, so the @nobits# trick does not work.
 */
#if defined(CONFIG_ARM)
#define _NODATA_SECTION(segment)  __attribute__((section(#segment)))
#else
#define _NODATA_SECTION(segment)				\
	__attribute__((section(#segment ",\"wa\",@nobits#")))
#endif

/* Unaligned access */
#define UNALIGNED_GET(p)						\
__extension__ ({							\
	struct  __attribute__((__packed__)) {				\
		__typeof__(*(p)) __v;					\
	} *__p = (__typeof__(__p)) (p);					\
	__p->__v;							\
})

#define UNALIGNED_PUT(v, p)                                             \
do {                                                                    \
	struct __attribute__((__packed__)) {                            \
		__typeof__(*p) __v;                                     \
	} *__p = (__typeof__(__p)) (p);                                 \
	__p->__v = (v);                                               \
} while (0)

#define _GENERIC_SECTION(segment) __attribute__((section(#segment)))

#ifndef __packed
#define __packed        __attribute__((__packed__))
#endif
#ifndef __aligned
#define __aligned(x)	__attribute__((__aligned__(x)))
#endif
#define __may_alias     __attribute__((__may_alias__))
#define __printf_like(f, a)   __attribute__((format (printf, f, a)))
#define __used
#define __deprecated	
#define ARG_UNUSED(x) (void)(x)

#define likely(x)   (x)
#define unlikely(x) !(x)

