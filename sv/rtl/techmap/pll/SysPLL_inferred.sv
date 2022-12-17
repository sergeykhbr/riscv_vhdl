////////////////////////////////////////////////////////////////////////////-
//! @file
//! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
//! @author    Sergey Khabarov - sergeykhbr@gmail.com
//! @details  	PLL instance for the behaviour simulation
//!
//! "Output    Output      Phase     Duty      Pk-to-Pk        Phase"
//! "Clock    Freq (MHz) (degrees) Cycle (%) Jitter (ps)  Error (ps)"
//!
//! CLK_OUT1____70.000
////////////////////////////////////////////////////////////////////////////-

module SysPLL_inferred(
  // Clock in ports
  input     CLK_IN,
  // Clock out ports
  output    CLK_OUT1,
  output    CLK_OUT2,
  // Status and control signals
  input     RESET,
  output    LOCKED
 );

  assign CLK_OUT1 = CLK_IN;
  assign CLK_OUT2 = CLK_IN;
  assign LOCKED = ~RESET;

endmodule
