/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Core attribute class declaration.
 */

#ifndef __DEBUGGER_ATTRIBUTE_H__
#define __DEBUGGER_ATTRIBUTE_H__

#include <stdint.h>
#include <string.h>
#include "iattr.h"

namespace debugger {

enum KindType {
        Attr_Invalid,
        Attr_String,
        Attr_Integer,
        Attr_UInteger,
        Attr_Floating,
        Attr_List,
        Attr_Data,
        Attr_Nil,
        Attr_Dict,
        Attr_Boolean,
        Attr_Interface,
        Attr_PyObject,
};

class AttributePairType;

class AttributeType : public IAttribute {
  public:
    KindType kind_;
    unsigned size_;
    union {
        char *string;
        int64_t integer;
        bool boolean;
        double floating;
        AttributeType *list;
        AttributePairType *dict;
        uint8_t *data;
        void *py_object;
        IFace *iface;
        char *uobject;
    } u_;

    AttributeType(const AttributeType& other) {
        size_ = 0;
        clone(&other);
    }

    AttributeType() {
        kind_ = Attr_Invalid;
        size_ = 0;
        u_.integer = 0;
    }
    ~AttributeType() {
        attr_free();
    }

    void attr_free();

    explicit AttributeType(const char *str) {
        make_string(str);
    }

    explicit AttributeType(IFace *mod) {
        kind_ = Attr_Interface;
        u_.iface = mod;
    }

    explicit AttributeType(KindType type) {
        kind_ = type;
        size_ = 0;
        u_.integer = 0;
    }

    explicit AttributeType(bool val) {
        kind_ = Attr_Boolean;
        size_ = 0;
        u_.boolean = val;
    }

    AttributeType(KindType type, uint64_t v) {
        if (type == Attr_Integer) {
            make_int64(static_cast<int64_t>(v));
        } else if (type == Attr_UInteger) {
            make_uint64(v);
        }
    }

    unsigned size() const { return size_; }

    bool is_floating() const {
        return kind_ == Attr_Floating;
    }

    double to_float() const {
        return u_.floating;
    }

    bool is_integer() const {
        return kind_ == Attr_Integer || kind_ == Attr_UInteger;
    }

    bool is_int64() const {
        return kind_ == Attr_Integer;
    }

    int to_int() const {
        return static_cast<int>(u_.integer);
    }

    int64_t to_int64() const {
        return u_.integer;
    }

    bool is_uint64() const {
        return kind_ == Attr_UInteger;
    }

    uint64_t to_uint64() const {
        return u_.integer;
    }

    bool is_bool() const {
        return kind_ == Attr_Boolean;
    }

    bool to_bool() const { return u_.boolean; }

    bool is_string() const {
        return kind_ == Attr_String;
    }

    const char * to_string() const {
        return u_.string;
    }

    // capitalize letters in string;
    const char * to_upper() const {
        if (kind_ != Attr_String) {
            return 0;
        }
        char *p = u_.string;
        while (*p) {
            if (p[0] >= 'a' && p[0] <= 'z') {
                p[0] = p[0] - 'a' + 'A';
            }
            p++;
        }
        return u_.string;
    }

    bool is_list() const {
        return kind_ == Attr_List;
    }

    bool is_dict() const {
        return kind_ == Attr_Dict;
    }

    bool is_data() const {
        return kind_ == Attr_Data;
    }

    bool is_iface() const {
        return kind_ == Attr_Interface;
    }

    IFace *to_iface() const {
        return u_.iface;
    }

    bool is_nil() const {
        return kind_ == Attr_Nil;
    }

    bool is_invalid() const {
        return kind_ == Attr_Invalid;
    }


    void clone(const AttributeType *v);

    void make_nil() {
        kind_ = Attr_Nil;
        size_ = 0;
        u_.integer = 0;
    }

    void make_floating(double value) {
        kind_ = Attr_Floating;
        u_.floating = value;
    }

    void make_int64(int64_t value) {
        kind_ = Attr_Integer;
        u_.integer = value;
    }

    void make_uint64(uint64_t value) {
        kind_ = Attr_UInteger;
        u_.integer = value;
    }

    void make_boolean(bool value) {
        kind_ = Attr_Boolean;
        size_ = 0;
        u_.boolean = value;
    }

    void make_string(const char *value) {
        if (value) {
            kind_ = Attr_String;
            size_ = strlen(value);
            u_.string = new char[size_ + 1];
            memcpy(u_.string, value, size_ + 1);
        } else {
            kind_ = Attr_Nil;
        }
    }

    void make_data(unsigned size, const void *data) {
        kind_ = Attr_Data;
        size_ = size;
        u_.data = new uint8_t[size];
        memcpy(u_.data, data, size);
    }

    void make_list(unsigned size) {
        kind_ = Attr_List;
        size_ = size;
        u_.list = new AttributeType[size];
    }

    void add_to_list(const AttributeType *item) {
        realloc_list(size()+1);
        (*this)[size()-1] = (*item);
    }

    void trim_list(unsigned start, unsigned end) {
        for (unsigned i = start; i < (size_ - end); i++) {
            u_.list[start + i] = u_.list[end + i];
        }
        size_ -= (end - start);
    }

    void swap_list_item(unsigned n, unsigned m) {
        if (n == m) {
            return;
        }
        unsigned tsize = u_.list[n].size_;
        KindType tkind = u_.list[n].kind_;
        int64_t tinteger = u_.list[n].u_.integer;
        u_.list[n].size_ = u_.list[m].size_;
        u_.list[n].kind_ = u_.list[m].kind_;
        u_.list[n].u_.integer = u_.list[m].u_.integer;
        u_.list[m].size_ = tsize;
        u_.list[m].kind_ = tkind;
        u_.list[m].u_.integer = tinteger;
    }

    void realloc_list(unsigned size) {
        AttributeType * t1 = new AttributeType[size];
        for (unsigned i = 0; i < size_; i++) {
            t1[i].clone(&u_.list[i]);
        }
        if (size_) {
            delete [] u_.list;
        }
        u_.list = t1;
        size_ = size;
    }

    void make_dict();
    void realloc_dict(unsigned size);

    // Getter:
    double floating() const { return u_.floating; }

    int64_t integer() const { return u_.integer; }

    const char *string() const { return u_.string; }

    bool boolean() const { return u_.boolean; }

    const AttributeType *list(unsigned idx) const {
        return &u_.list[idx];
    }
    AttributeType *list(unsigned idx) {
        return &u_.list[idx];
    }

    bool has_key(const char *key) const;

    const AttributeType *dict_key(unsigned idx) const;
    AttributeType *dict_key(unsigned idx);

    const AttributeType *dict_value(unsigned idx) const;
    AttributeType *dict_value(unsigned idx);

    const uint8_t *data() const { return u_.data; }
    uint8_t *data() { return u_.data; }

    AttributeType& operator=(const AttributeType& other);

    /**
     * @brief Access to the single element of the 'list' attribute:
     */
    const AttributeType& operator[](unsigned idx) const;
    /** @overload */
    AttributeType& operator[](unsigned idx);

    /**
     * @brief Access to the single value attribute of the 'dictionary':
     */
    const AttributeType& operator[](const char *key) const;
    /** @overload */
    AttributeType& operator[](const char *key);

    /**
     * @brief Access to the single byte of the 'data' attribute:
     */
    const uint8_t& operator()(unsigned idx) const;

    char *to_config();
    void from_config(const char *str);
};

class AttributePairType {
  public:
    AttributeType key_;
    AttributeType value_;
};

}  // namespace debugger

#endif  // __DEBUGGER_ATTRIBUTE_H__
