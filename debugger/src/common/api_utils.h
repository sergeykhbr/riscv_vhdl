/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Core API utils methods declaration.
 */

#ifndef __DEBUGGER_API_UTILS_H__
#define __DEBUGGER_API_UTILS_H__

#include <stdarg.h>
#include <api_types.h>
#include <attribute.h>

namespace debugger {

#define LOG_ERROR     1
#define LOG_IMPORTANT 2   // can be used for the autotest messages
#define LOG_INFO      3
#define LOG_DEBUG     4

#ifdef __cplusplus
extern "C" {
#endif

/** Generate unique string. */
void RISCV_generate_name(AttributeType *name);

/** Redirect output to specified console. */
void RISCV_add_default_output(void *iout);
void RISCV_remove_default_output(void *iout);

/** 
 * @brief Write output to logfile.
 * @return 0 on success, 1 when failed
 */
int RISCV_enable_log(const char *filename);

void RISCV_disable_log();

/** Format output to string. */
int RISCV_sprintf(char *s, size_t len, const char *fmt, ...);

/** Format output to the default stream. */
int RISCV_printf(void *iface, int level, const char *fmt, ...);

/** Output always */
#define RISCV_printf0(fmt, ...) \
    RISCV_printf(getInterface(IFACE_SERVICE), 0, fmt, __VA_ARGS__)

/** Output with the maximal logging level */
#define RISCV_error(fmt, ...) \
    RISCV_printf(getInterface(IFACE_SERVICE), LOG_ERROR, "%s:%d " fmt, \
                 __FILE__, __LINE__, __VA_ARGS__)

/** Output with the information logging level */
#define RISCV_important(fmt, ...) \
    RISCV_printf(getInterface(IFACE_SERVICE), LOG_IMPORTANT, fmt, __VA_ARGS__)

/** Output with the information logging level */
#define RISCV_info(fmt, ...) \
    RISCV_printf(getInterface(IFACE_SERVICE), LOG_INFO, fmt, __VA_ARGS__)

/** Output with the lower logging level */
#define RISCV_debug(fmt, ...) \
    RISCV_printf(getInterface(IFACE_SERVICE), LOG_DEBUG, fmt, __VA_ARGS__)

/** Suspend thread on certain number of milliseconds */
void RISCV_sleep_ms(int ms);

/** Get current time in milliseconds. */
uint64_t RISCV_get_time_ms();

/** Get process ID. */
int RISCV_get_pid();

/** Memory barrier */
void RISCV_memory_barrier();

void RISCV_thread_create(void *data);
uint64_t RISCV_thread_id();

int RISCV_mutex_init(mutex_def *mutex);
int RISCV_mutex_lock(mutex_def *mutex);
int RISCV_mutex_unlock(mutex_def *mutex);
int RISCV_mutex_destroy(mutex_def *mutex);
void RISCV_thread_join(thread_def th, int ms);

void RISCV_event_create(event_def *ev, const char *name);
void RISCV_event_close(event_def *ev);
void RISCV_event_set(event_def *ev);
int RISCV_event_is_set(event_def *ev);
void RISCV_event_clear(event_def *ev);
void RISCV_event_wait(event_def *ev);
int RISCV_event_wait_ms(event_def *ev, int ms);

/** Memory allocator/de-allocator */
void *RISCV_malloc(uint64_t sz);
void RISCV_free(void *p);

/** Get absolute directory where core library is placed. */
int RISCV_get_core_folder(char *out, int sz);

/** Set $(pwd) directory equals to executable location */
void RISCV_set_current_dir();

/** Reading configuration from JSON formatted file. */
int RISCV_read_json_file(const char *filename, void *outattr);

/** Write configuration string to JSON formatted file. */
void RISCV_write_json_file(const char *filename, const char *s);

#ifdef __cplusplus
}
#endif

}  // namespace debugger

#endif  // __DEBUGGER_API_UTILS_H__
