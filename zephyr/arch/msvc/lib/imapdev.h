#pragma once

#include <inttypes.h>

class IMappedDevice {
public:
    virtual bool isAddrValid(uint64_t off) =0;
};
