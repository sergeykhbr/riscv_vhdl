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

package pkg_amba4;

parameter CFG_SYSBUS_ADDR_BITS  = 32;
parameter CFG_SYSBUS_DATA_BYTES = 8;
parameter CFG_SYSBUS_DATA_BITS  = 64;
parameter CFG_SYSBUS_ID_BITS    = 2;

const int log2[0:512] = {
0,0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
  6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
  8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
  8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
  8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9
};

const bit [1:0] AXI_BURST_FIXED = 2'b00;
const bit [1:0] AXI_BURST_INCR  = 2'b01;
const bit [1:0] AXI_BURST_WRAP  = 2'b10;


typedef struct packed {
    /** @brief Read address.
        @details The read address gives the address of the first transfer
                 in a read burst transaction.
    */
    bit [CFG_SYSBUS_ADDR_BITS-1:0] addr;
    /** @brief   Burst length.
        @details This signal indicates the exact number of transfers in 
                 a burst. This changes between AXI3 and AXI4. nastiXLenBits=8 so
                 this is an AXI4 implementation.
                     Burst_Length = len[7:0] + 1
    */
    bit [7:0] len;
    /** @brief   Burst size.
        @details This signal indicates the size of each transfer 
              in the burst: 0=1 byte; ..., 6=64 bytes; 7=128 bytes;
    */
    bit [2:0] size;
    /** @brief   Read response.
      @details This signal indicates the status of the read transfer. 
      The responses are:
           0b00 FIXED - In a fixed burst, the address is the same for every transfer 
                       in the burst. Typically is used for FIFO.
           0b01 INCR - Incrementing. In an incrementing burst, the address for each
                       transfer in the burst is an increment of the address for the 
                       previous transfer. The increment value depends on the size of 
                       the transfer.
           0b10 WRAP - A wrapping burst is similar to an incrementing burst, except 
                       that the address wraps around to a lower address if an upper address 
                       limit is reached.
           0b11 resrved.
    */
    bit [1:0] burst;
    /** @brief   Lock type.
        @details Not supported in AXI4.
    */
    bit lock;
    /** @brief   Memory type.
        @details See table for write and read transactions.
    */
    bit [3:0] cache;
    /** @brief   Protection type.
        @details This signal indicates the privilege and security level 
                 of the transaction, and whether the transaction is a data access
                 or an instruction access:
         [0] :   0 = Unpriviledge access
                 1 = Priviledge access
         [1] :   0 = Secure access
                 1 = Non-secure access
         [2] :   0 = Data access
                 1 = Instruction access
    */
    bit [2:0] prot;
    /** @brief   Quality of Service, QoS. 
        @details QoS identifier sent for each read transaction. 
                 Implemented only in AXI4:
                     0b0000 - default value. Indicates that the interface is 
                              not participating in any QoS scheme.
    */
    bit [3:0] qos;
    /** @brief Region identifier.
        @details Permits a single physical interface on a slave to be used for 
                 multiple logical interfaces. Implemented only in AXI4. This is 
                 similar to the banks implementation in Leon3 without address 
                 decoding.
    */
    bit [3:0] region;
} axi4_metadata_type;

const axi4_metadata_type META_NONE = {
   0, 8'd0, 3'd0, 2'd0, 1'b0, 
   4'd0, 3'd0, 4'd0, 4'd0
};

/// @brief Slave device AMBA AXI input signals.
typedef struct packed {
  /// Write Address channel:
  bit aw_valid;
  axi4_metadata_type aw_bits;
  bit [CFG_SYSBUS_ID_BITS-1:0] aw_id;
  bit aw_user;
  /// Write Data channel:
  bit w_valid;
  bit [CFG_SYSBUS_DATA_BITS-1:0] w_data;
  bit w_last;
  bit [CFG_SYSBUS_DATA_BYTES-1:0] w_strb;
  bit w_user;
  /// Write Response channel:
  bit b_ready;
  /// Read Address Channel:
  bit ar_valid;
  axi4_metadata_type ar_bits;
  bit [CFG_SYSBUS_ID_BITS-1:0] ar_id;
  bit ar_user;
  /// Read Data channel:
  bit r_ready;
} axi4_slave_in_type;

//const axi4_slave_in_type axi4_slave_in_none = {
//      1'b0, META_NONE, 0, 1'b0, 1'b0,
//      0, 1'b0, 0, 1'b0, 1'b0, 1'b0, META_NONE,
//      0, 1'b0, 1'b0};

/// @brief Slave device AMBA AXI output signals.
typedef struct packed {
  /// Write Address channel:
  bit aw_ready;
  /// Write Data channel:
  bit w_ready;
  /// Write Response channel:
  bit b_valid;
  bit [1:0] b_resp;
  bit [CFG_SYSBUS_ID_BITS-1:0] b_id;
  bit b_user;
  /// Read Address Channel
  bit ar_ready;
  /// Read Data channel:
  bit r_valid;
  /** @brief Read response.
      @details This signal indicates the status of the read transfer. 
       The responses are:
           0b00 OKAY - Normal access success. Indicates that a normal access has
                       been successful. Can also indicate an exclusive access
                       has failed.
           0b01 EXOKAY - Exclusive access okay. Indicates that either the read or
                       write portion of an exclusive access has been successful.
           0b10 SLVERR - Slave error. Used when the access has reached the slave 
                       successfully, but the slave wishes to return an error
                       condition to the originating master.
           0b11 DECERR - Decode error. Generated, typically by an interconnect 
                       component, to indicate that there is no slave at the
                       transaction address.
  */
  bit [1:0] r_resp;
  /// Read data
  bit [CFG_SYSBUS_DATA_BITS-1:0] r_data;
  /// Read last. This signal indicates the last transfer in a read burst.
  bit r_last;
  /** @brief Read ID tag. 
      @details This signal is the identification tag for the read data
                group of signals generated by the slave.
  */
  bit [CFG_SYSBUS_ID_BITS-1:0] r_id;
  /** @brief User signal. 
      @details Optinal User-defined signal in the read channel. Supported 
               only in AXI4.
  */
  bit r_user;
} axi4_slave_out_type;

function int xmask2abits(input int xmask);
begin
  const int size_4kbytes = -(xmask - 1048576); 
  return 12 + log2[size_4kbytes];
end
endfunction: xmask2abits


endpackage

