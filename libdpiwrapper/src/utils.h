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

#pragma once

#include "types.h"
#include <inttypes.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

int LIB_init();
void LIB_cleanup();

void LIB_thread_create(void *data);
uint64_t LIB_thread_id();
void LIB_thread_join(thread_def th, int ms);
void LIB_sleep_ms(int ms);

int LIB_mutex_init(mutex_def *mutex);
int LIB_mutex_lock(mutex_def *mutex);
int LIB_mutex_unlock(mutex_def *mutex);
int LIB_mutex_destroy(mutex_def *mutex);
void LIB_thread_join(thread_def th, int ms);

void LIB_event_create(event_def *ev, const char *name);
void LIB_event_close(event_def *ev);
void LIB_event_set(event_def *ev);
int LIB_event_is_set(event_def *ev);
void LIB_event_clear(event_def *ev);
void LIB_event_wait(event_def *ev);
int LIB_event_wait_ms(event_def *ev, int ms);

int LIB_printf(const char *fmt, ...);
int LIB_sprintf(char *s, size_t len, const char *fmt, ...);

void *LIB_get_proc_addr(const char *f);

#ifdef __cplusplus
}
#endif

