/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Dynamically allocated buffer declaration.
 */

#ifndef __DEBUGGER_AUTOBUFFER_H__
#define __DEBUGGER_AUTOBUFFER_H__

#include <stdint.h>
#include <string.h>

namespace debugger {

/**
 * @brief String buffer declaration.
 * @details This buffer is used to form configuration string of the kernel.
 */
class AutoBuffer {
  public:
    /** Create empty string buffer. */
    AutoBuffer();
    ~AutoBuffer();

    /**
     * @brief Write input data of the specified size into buffer's memory.
     * @param[in] p Pointer on input data.
     * @param[in] sz Input buffer size in bytes.
     */
    void write_bin(const char *p, int sz);
    /**
     * @brief Write single symbol into buffer's memory.
     * @param[in] s Input character value.
     */
    void write_string(const char s);
    /**
     * @brief Write input string into buffer's memory.
     * @param[in] s Pointer on string buffer.
     */
    void write_string(const char *s);
    /**
     * @brief Write integer value as a hex string into buffer's memory.
     * @param[in] s Input integer value.
     */
    void write_uint64(uint64_t v);
    /**
     * @brief Write a single byte into buffer's memory.
     * @details This method is very usefull to write special characters, like
     *          '\0', '\n' etc.
     * @param[in] s Input byte value.
     */
    void write_byte(uint8_t v);

    /**
     * @brief Get allocated memory pointer.
     * @return Pointer on allocated memory region.
     */
    char *getBuffer() { return buf_; }

    /** Get total number of written symbols. */
    int size() { return buf_len_; }
    /** Reset buffer's value. */
    void clear() { 
        buf_len_ = 0;
        if (buf_) {
            buf_[buf_len_] = 0;
        }
    } 

  private:
    char *buf_;
    int buf_len_;
    int buf_size_;
};

}  // namespace debugger

#endif  // __DEBUGGER_AUTOBUFFER_H__
