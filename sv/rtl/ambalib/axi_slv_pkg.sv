package axi_slv_pkg;

import types_amba_pkg::*;

//! Slave device states during reading value operation.
typedef enum logic [1:0] {rwait = 0, rhold = 1, rtrans = 2} axi_slave_rstatetype;
//! Slave device states during writting data operation.
typedef enum logic {wwait = 0, wtrans = 1} axi_slave_wstatetype;

//! @brief Template bank of registers for any slave device.
typedef struct  {
    axi_slave_rstatetype rstate;
    axi_slave_wstatetype wstate;

    logic [1:0] rburst;
    int rsize;
    logic [CFG_SYSBUS_ADDR_BITS-1 : 0] raddr;
    logic [7:0] rlen;                       //! AXI4 supports 256 burst operation
    logic [CFG_SYSBUS_ID_BITS - 1:0] rid;
    logic [1:0] rresp;  //! OK=0
    logic ruser;
    logic rswap;
    logic rwaitready;                 //! Reading wait state flag: 0=waiting. User's waitstates
    logic [CFG_SYSBUS_DATA_BITS-1 : 0] skip_rdata;
    logic rskip;

    logic [1:0] wburst;  // 0=INCREMENT
    int wsize;                // code in range 0=1 Bytes upto 7=128 Bytes.
    logic [CFG_SYSBUS_ADDR_BITS-1 : 0] waddr;        //! 4 KB bank
    logic [7:0] wlen;                //! AXI4 supports 256 burst operation
    logic [CFG_SYSBUS_ID_BITS - 1:0] wid;
    logic [1:0] wresp;  //! OK=0
    logic wuser;
    logic wswap;
    logic b_valid;
} axi_slave_bank_type;

//! Reset value of the template bank of registers of a slave device.
function automatic axi_slave_bank_type AXI_SLAVE_BANK_RESET();
begin

    AXI_SLAVE_BANK_RESET.rstate = rwait;
    AXI_SLAVE_BANK_RESET.wstate = wwait;
    AXI_SLAVE_BANK_RESET.rburst = AXI_BURST_FIXED;
    AXI_SLAVE_BANK_RESET.rsize  = 0;
    AXI_SLAVE_BANK_RESET.raddr  = '0;

    AXI_SLAVE_BANK_RESET.rlen       = '0;
    AXI_SLAVE_BANK_RESET.rid        = '0;
    AXI_SLAVE_BANK_RESET.rresp      = AXI_RESP_OKAY;
    AXI_SLAVE_BANK_RESET.ruser      = '0;
    AXI_SLAVE_BANK_RESET.rswap      = '0;
    AXI_SLAVE_BANK_RESET.rwaitready = 1'b1;
    AXI_SLAVE_BANK_RESET.skip_rdata = '0;
    AXI_SLAVE_BANK_RESET.rskip      = 1'b0;
    AXI_SLAVE_BANK_RESET.wburst     = AXI_BURST_FIXED;
    AXI_SLAVE_BANK_RESET.wsize      = 0;
    AXI_SLAVE_BANK_RESET.waddr      = '0;

    AXI_SLAVE_BANK_RESET.wlen    = '0;
    AXI_SLAVE_BANK_RESET.wid     = '0;
    AXI_SLAVE_BANK_RESET.wresp   = AXI_RESP_OKAY;
    AXI_SLAVE_BANK_RESET.wuser   = '0;
    AXI_SLAVE_BANK_RESET.wswap   = '0;
    AXI_SLAVE_BANK_RESET.b_valid = '0;

end
endfunction: AXI_SLAVE_BANK_RESET

endpackage: axi_slv_pkg
