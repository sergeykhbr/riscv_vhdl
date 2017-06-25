/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      EDCL user defined transport structure.
 */

 #ifndef __DEBUGGER_EDCL_TYPES_H__
 #define __DEBUGGER_EDCL_TYPES_H__

 #include <inttypes.h>

 namespace debugger {

struct EdclControlRequestType {
    // 32 bits fields:
    uint32_t unused : 7;
    uint32_t len    : 10;
    uint32_t write  : 1;    // read = 0; write = 1
    uint32_t seqidx : 14;   // sequence id
    //uint32 data; // 0 to 242 words
};


struct EdclControlResponseType {
    // 32 bits fields:
    uint32_t unused : 7;
    uint32_t len    : 10;
    uint32_t nak    : 1;    // ACK = 0; NAK = 1
    uint32_t seqidx : 14;   // sequence id
    //uint32 data; // 0 to 242 words
};

#pragma pack(1)
struct UdpEdclCommonType {
    uint16_t offset;
    union ControlType {
        uint32_t word;
        EdclControlRequestType request;
        EdclControlResponseType response;
    } control;
    uint32_t address;
    //uint32 data; // 0 to 242 words
};
#pragma pack()

}  // namespace debugger

#endif  // __DEBUGGER_EDCL_TYPES_H__
