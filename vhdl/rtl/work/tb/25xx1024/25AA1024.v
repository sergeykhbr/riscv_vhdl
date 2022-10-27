// *******************************************************************************************************
// **                                                                                                   **
// **   M25AA1024.v - 25AA1024 1M-BIT SPI SERIAL EEPROM (VCC = +1.8V TO +5.5V)                          **
// **                                                                                                   **
// *******************************************************************************************************
// **                                                                                                   **
// **                              COPYRIGHT (c) 2006 YOUNG ENGINEERING                                 **
// **                                      ALL RIGHTS RESERVED                                          **
// **                                                                                                   **
// **   THIS PROGRAM IS CONFIDENTIAL AND  A  TRADE SECRET  OF  YOUNG  ENGINEERING.  THE RECEIPT OR      **
// **   POSSESSION  OF THIS PROGRAM  DOES NOT  CONVEY  ANY  RIGHTS TO  REPRODUCE  OR  DISCLOSE ITS      **
// **   CONTENTS,  OR TO MANUFACTURE, USE, OR SELL  ANYTHING  THAT IT MAY DESCRIBE, IN WHOLE OR IN      **
// **   PART, WITHOUT THE SPECIFIC WRITTEN CONSENT OF YOUNG ENGINEERING.                                **
// **                                                                                                   **
// *******************************************************************************************************
// **                                                                                                   **
// **   This information is provided to you for your convenience and use with Microchip products only.  **
// **   Microchip disclaims all liability arising from this information and its use.                    **
// **                                                                                                   **
// **   THIS INFORMATION IS PROVIDED "AS IS." MICROCHIP MAKES NO REPRESENTATION OR WARRANTIES OF        **
// **   ANY KIND WHETHER EXPRESS OR IMPLIED, WRITTEN OR ORAL, STATUTORY OR OTHERWISE, RELATED TO        **
// **   THE INFORMATION PROVIDED TO YOU, INCLUDING BUT NOT LIMITED TO ITS CONDITION, QUALITY,           **
// **   PERFORMANCE, MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR PURPOSE.                         **
// **   MICROCHIP IS NOT LIABLE, UNDER ANY CIRCUMSTANCES, FOR SPECIAL, INCIDENTAL OR CONSEQUENTIAL      **
// **   DAMAGES, FOR ANY REASON WHATSOEVER.                                                             **
// **                                                                                                   **
// **   It is your responsibility to ensure that your application meets with your specifications.       **
// **                                                                                                   **
// *******************************************************************************************************
// **                                                                                                   **
// **   Revision       : 1.0                                                                            **
// **   Modified Date  : 11/05/2007                                                                     **
// **   Revision History:                                                                               **
// **                                                                                                   **
// **   11/05/2007:  Initial design                                                                     **
// **                                                                                                   **
// *******************************************************************************************************
// **                                       TABLE OF CONTENTS                                           **
// *******************************************************************************************************
// **---------------------------------------------------------------------------------------------------**
// **   DECLARATIONS                                                                                    **
// **---------------------------------------------------------------------------------------------------**
// **---------------------------------------------------------------------------------------------------**
// **   INITIALIZATION                                                                                  **
// **---------------------------------------------------------------------------------------------------**
// **---------------------------------------------------------------------------------------------------**
// **   CORE LOGIC                                                                                      **
// **---------------------------------------------------------------------------------------------------**
// **   1.01:  Internal Reset Logic                                                                     **
// **   1.02:  Input Data Shifter                                                                       **
// **   1.03:  Bit Clock Counter                                                                        **
// **   1.04:  Instruction Register                                                                     **
// **   1.05:  Address Register                                                                         **
// **   1.06:  Block Protect Bits                                                                       **
// **   1.07:  Write Protect Enable                                                                     **
// **   1.08:  Write Data Buffer                                                                        **
// **   1.09:  Write Enable Bit                                                                         **
// **   1.10:  Write Cycle Processor                                                                    **
// **   1.11:  Erase Cycle Processor                                                                    **
// **   1.12:  Deep Power-Down Logic                                                                    **
// **   1.13:  Output Data Shifter                                                                      **
// **   1.14:  Output Data Buffer                                                                       **
// **                                                                                                   **
// **---------------------------------------------------------------------------------------------------**
// **   DEBUG LOGIC                                                                                     **
// **---------------------------------------------------------------------------------------------------**
// **   2.01:  Memory Data Bytes                                                                        **
// **   2.02:  Page Buffer Bytes                                                                        **
// **                                                                                                   **
// **---------------------------------------------------------------------------------------------------**
// **   TIMING CHECKS                                                                                   **
// **---------------------------------------------------------------------------------------------------**
// **                                                                                                   **
// *******************************************************************************************************


`timescale 1ns/10ps

module M25AA1024 (SI, SO, SCK, CS_N, WP_N, HOLD_N, RESET);

   input                SI;                             // serial data input
   input                SCK;                            // serial data clock

   input                CS_N;                           // chip select - active low
   input                WP_N;                           // write protect pin - active low

   input                HOLD_N;                         // interface suspend - active low

   input                RESET;                          // model reset/power-on reset

   output               SO;                             // serial data output


// *******************************************************************************************************
// **   DECLARATIONS                                                                                    **
// *******************************************************************************************************

   reg  [23:00]         DataShifterI;                   // serial input data shifter
   reg  [07:00]         DataShifterO;                   // serial output data shifter
   reg  [31:00]         BitCounter;                     // serial input bit counter
   reg  [07:00]         InstRegister;                   // instruction register
   reg  [23:00]         AddrRegister;                   // address register

   wire                 InstructionREAD;                // decoded instruction byte
   wire                 InstructionRDSR;                // decoded instruction byte
   wire                 InstructionWRSR;                // decoded instruction byte
   wire                 InstructionWRDI;                // decoded instruction byte
   wire                 InstructionWREN;                // decoded instruction byte
   wire                 InstructionWRITE;               // decoded instruction byte
   wire                 InstructionPE;                  // decoded instruction byte
   wire                 InstructionSE;                  // decoded instruction byte
   wire                 InstructionCE;                  // decoded instruction byte
   wire                 InstructionDPD;                 // decoded instruction byte
   wire                 InstructionRDID;                // decoded instruction byte

   reg  [07:00]         WriteBuffer [0:255];            // 256-byte page write buffer
   reg  [07:00]         WritePointer;                   // page buffer pointer
   reg  [08:00]         WriteCounter;                   // byte write counter

   reg                  WriteEnable;                    // memory write enable bit
   wire                 RstWriteEnable;                 // asynchronous reset
   wire                 SetWriteEnable;                 // register set
   wire                 ClrWriteEnable;                 // register clear

   reg                  WriteActive;                    // write operation in progress

   reg                  BlockProtect0;                  // memory block write protect
   reg                  BlockProtect1;                  // memory block write protect
   reg                  BlockProtect0_New;              // memory block write protect to be written
   reg                  BlockProtect1_New;              // memory block write protect to be written
   reg                  BlockProtect0_Old;              // old memory block write protect
   reg                  BlockProtect1_Old;              // old memory block write protect

   reg                  WP_Enable;                      // write protect pin enable
   reg                  WP_Enable_New;                  // write protect pin enable to be written
   reg                  WP_Enable_Old;                  // old write protect pin enable
   wire                 StatusWriteProtected;           // status register write protected

   reg  [07:00]         PageAddress;                    // page buffer address
   reg  [16:00]         BaseAddress;                    // memory write base address
   reg  [16:00]         MemWrAddress;                   // memory write address
   reg  [16:00]         MemRdAddress;                   // memory read address

   reg  [07:00]         MemoryBlock [0:131071];         // EEPROM data memory array (131072x8)

   reg                  DeepPowerDown;                  // deep power-down mode

   reg                  SO_DO;                          // serial output data - data
   wire                 SO_OE;                          // serial output data - output enable

   reg                  SO_Enable;                      // serial data output enable

   wire                 OutputEnable1;                  // timing accurate output enable
   wire                 OutputEnable2;                  // timing accurate output enable
   wire                 OutputEnable3;                  // timing accurate output enable

   integer              LoopIndex;                      // iterative loop index

   integer              tWC;                            // timing parameter
   integer              tV;                             // timing parameter
   integer              tHZ;                            // timing parameter
   integer              tHV;                            // timing parameter
   integer              tDIS;                           // timing parameter
   integer              tPD;                            // timing parameter
   integer              tREL;                           // timing parameter
   integer              tCE;                            // timing parameter
   integer              tSE;                            // timing parameter

`define PAGE_SIZE 256                                   // 256-byte page size
`define SECTOR_SIZE 32768                               // 32-Kbyte sector size
`define ARRAY_SIZE 131072                               // 1-Mbit array size
`define MFG_ID 8'h29                                    // Manufacturer's ID
`define WREN  8'b0000_0110                              // Write Enable instruction
`define READ  8'b0000_0011                              // Read instruction
`define WRDI  8'b0000_0100                              // Write Disable instruction
`define WRSR  8'b0000_0001                              // Write Status Register instruction
`define WRITE 8'b0000_0010                              // Write instruction
`define RDSR  8'b0000_0101                              // Read Status Register instruction
`define PE    8'b0100_0010                              // Page Erase instruction
`define SE    8'b1101_1000                              // Sector Erase instruction
`define CE    8'b1100_0111                              // Chip Erase instruction
`define DPD   8'b1011_1001                              // Deep Power-Down instruction
`define RDID  8'b1010_1011                              // Read ID and Release from Deep Power-Down instruction

// *******************************************************************************************************
// **   INITIALIZATION                                                                                  **
// *******************************************************************************************************

   initial begin
      `ifdef VCC_1_8V_TO_2_5V
         tWC  = 6000000;                                // memory write cycle time
         tV   = 250;                                    // output valid from SCK low
         tHZ  = 150;                                    // HOLD_N low to output high-Z
         tHV  = 150;                                    // HOLD_N high to output valid
         tDIS = 250;                                    // CS_N high to output disable
         tPD  = 100000;                                 // CS_N high to deep power-down
         tREL = 100000;                                 // CS_N high to standby mode
      `else
      `ifdef VCC_2_5V_TO_4_5V
         tWC  = 6000000;                                // memory write cycle time
         tV   = 50;                                     // output valid from SCK low
         tHZ  = 30;                                     // HOLD_N low to output high-Z
         tHV  = 30;                                     // HOLD_N high to output valid
         tDIS = 50;                                     // CS_N high to output disable
         tPD  = 100000;                                 // CS_N high to deep power-down
         tREL = 100000;                                 // CS_N high to standby mode
      `else
      `ifdef VCC_4_5V_TO_5_5V
         tWC  = 6000000;                                // memory write cycle time
         tV   = 25;                                     // output valid from SCK low
         tHZ  = 15;                                     // HOLD_N low to output high-Z
         tHV  = 15;                                     // HOLD_N high to output valid
         tDIS = 25;                                     // CS_N high to output disable
         tPD  = 100000;                                 // CS_N high to deep power-down
         tREL = 100000;                                 // CS_N high to standby mode
      `else
         tWC  = 6000000;                                // memory write cycle time
         tV   = 25;                                     // output valid from SCK low
         tHZ  = 15;                                     // HOLD_N low to output high-Z
         tHV  = 15;                                     // HOLD_N high to output valid
         tDIS = 25;                                     // CS_N high to output disable
         tPD  = 100000;                                 // CS_N high to deep power-down
         tREL = 100000;                                 // CS_N high to standby mode
      `endif
      `endif
      `endif
   end

   initial begin
      tCE = 10000000;                                   // chip erase cycle time
      tSE = 10000000;                                   // sector erase cycle time
   end

   initial begin
      BlockProtect0 = 0;
      BlockProtect1 = 0;

      WP_Enable = 0;

      WriteActive = 0;
      WriteEnable = 0;

      DeepPowerDown = 0;
   end


// *******************************************************************************************************
// **   CORE LOGIC                                                                                      **
// *******************************************************************************************************
// -------------------------------------------------------------------------------------------------------
//      1.01:  Internal Reset Logic
// -------------------------------------------------------------------------------------------------------

   always @(negedge CS_N) BitCounter   <= 0;
   always @(negedge CS_N) SO_Enable    <= 0;
   always @(negedge CS_N) if (!WriteActive) WritePointer <= 0;
   always @(negedge CS_N) if (!WriteActive) WriteCounter <= 0;

// -------------------------------------------------------------------------------------------------------
//      1.02:  Input Data Shifter
// -------------------------------------------------------------------------------------------------------

   always @(posedge SCK) begin
      if (HOLD_N == 1) begin
         if (CS_N == 0)         DataShifterI <= {DataShifterI[22:00],SI};
      end
   end

// -------------------------------------------------------------------------------------------------------
//      1.03:  Bit Clock Counter
// -------------------------------------------------------------------------------------------------------

   always @(posedge SCK) begin
      if (HOLD_N == 1) begin
         if (CS_N == 0)         BitCounter <= BitCounter + 1;
      end
   end

// -------------------------------------------------------------------------------------------------------
//      1.04:  Instruction Register
// -------------------------------------------------------------------------------------------------------

   always @(posedge SCK) begin
      if (HOLD_N == 1) begin
         if (BitCounter == 7)   InstRegister <= {DataShifterI[06:00],SI};
      end
   end

   assign InstructionREAD  = (InstRegister[7:0] == `READ);
   assign InstructionRDSR  = (InstRegister[7:0] == `RDSR);
   assign InstructionWRSR  = (InstRegister[7:0] == `WRSR);
   assign InstructionWRDI  = (InstRegister[7:0] == `WRDI);
   assign InstructionWREN  = (InstRegister[7:0] == `WREN);
   assign InstructionWRITE = (InstRegister[7:0] == `WRITE);
   assign InstructionPE    = (InstRegister[7:0] == `PE);
   assign InstructionSE    = (InstRegister[7:0] == `SE);
   assign InstructionCE    = (InstRegister[7:0] == `CE);
   assign InstructionDPD   = (InstRegister[7:0] == `DPD);
   assign InstructionRDID  = (InstRegister[7:0] == `RDID);

// -------------------------------------------------------------------------------------------------------
//      1.05:  Address Register
// -------------------------------------------------------------------------------------------------------

   always @(posedge SCK) begin
      if (HOLD_N == 1) begin
         if ((BitCounter == 31) & !WriteActive) AddrRegister <= {DataShifterI[22:00],SI};
      end
   end

// -------------------------------------------------------------------------------------------------------
//      1.06:  Block Protect Bits
// -------------------------------------------------------------------------------------------------------

   always @(posedge SCK) begin
      if (HOLD_N == 1) begin
         if (DeepPowerDown == 0) begin
            if ((BitCounter == 15) & InstructionWRSR & WriteEnable & !WriteActive & !StatusWriteProtected) begin
                BlockProtect1_New <= DataShifterI[02];
                BlockProtect0_New <= DataShifterI[01];
            end
         end
      end
   end
   
   always @(negedge CS_N) begin
      if (!WriteActive) begin
         BlockProtect0_Old <= BlockProtect0;
         BlockProtect1_Old <= BlockProtect1;
         WP_Enable_Old <= WP_Enable;
      end
   end

// -------------------------------------------------------------------------------------------------------
//      1.07:  Write Protect Enable
// -------------------------------------------------------------------------------------------------------

   always @(posedge SCK) begin
      if (HOLD_N == 1) begin
         if (DeepPowerDown == 0) begin
            if ((BitCounter == 15) & InstructionWRSR & WriteEnable & !WriteActive & !StatusWriteProtected) begin
               WP_Enable_New <= DataShifterI[06];
            end
         end
      end
   end

   assign StatusWriteProtected = WP_Enable & (WP_N == 0);

// -------------------------------------------------------------------------------------------------------
//      1.08:  Write Data Buffer
// -------------------------------------------------------------------------------------------------------

   always @(posedge SCK) begin
      if (HOLD_N == 1) begin
         if (DeepPowerDown == 0) begin
            if ((BitCounter >= 39) & (BitCounter[2:0] == 7) & InstructionWRITE & WriteEnable & !WriteActive) begin
               WriteBuffer[WritePointer] <= {DataShifterI[06:00],SI};

               WritePointer <= WritePointer + 1;
               if (WriteCounter < `PAGE_SIZE) WriteCounter <= WriteCounter + 1;
            end
         end
      end
   end

// -------------------------------------------------------------------------------------------------------
//      1.09:  Write Enable Bit
// -------------------------------------------------------------------------------------------------------

   always @(posedge CS_N or posedge RstWriteEnable) begin
      if (RstWriteEnable)       WriteEnable <= 0;
      else if (DeepPowerDown == 0) begin
         if (SetWriteEnable)    WriteEnable <= 1;
         if (ClrWriteEnable)    WriteEnable <= 0;
      end
   end

   assign RstWriteEnable = RESET;

   assign SetWriteEnable = (BitCounter == 8) & InstructionWREN & !WriteActive;
   assign ClrWriteEnable = (BitCounter == 8) & InstructionWRDI & !WriteActive;

// -------------------------------------------------------------------------------------------------------
//      1.10:  Write Cycle Processor
// -------------------------------------------------------------------------------------------------------

   always @(posedge CS_N) begin
      if (DeepPowerDown == 0) begin
         if ((BitCounter == 16) & (BitCounter[2:0] == 0) & InstructionWRSR  & WriteEnable & !WriteActive) begin
            if (!StatusWriteProtected) begin
                WriteActive = 1;
                #(tWC);

                BlockProtect1 = BlockProtect1_New;
                BlockProtect0 = BlockProtect0_New;
                WP_Enable = WP_Enable_New;
            end

            WriteActive = 0;
            WriteEnable = 0;
         end
         if ((BitCounter >= 40) & (BitCounter[2:0] == 0) & InstructionWRITE & WriteEnable & !WriteActive) begin
            for (LoopIndex = 0; LoopIndex < WriteCounter; LoopIndex = LoopIndex + 1) begin
               BaseAddress = {AddrRegister[16:08],8'h00};
               PageAddress = (AddrRegister[07:00] + LoopIndex);

               MemWrAddress = {BaseAddress[16:08],PageAddress[07:00]};

               if ({BlockProtect1,BlockProtect0} == 2'b00) begin
                  WriteActive = 1;
               end
               if ({BlockProtect1,BlockProtect0} == 2'b01) begin
                  if ((MemWrAddress >= 17'h18000) && (MemWrAddress <= 17'h1FFFF)) begin
                     // write protected region
                  end
                  else begin
                     WriteActive = 1;
                  end
               end
               if ({BlockProtect1,BlockProtect0} == 2'b10) begin
                  if ((MemWrAddress >= 17'h10000) && (MemWrAddress <= 17'h1FFFF)) begin
                     // write protected region
                  end
                  else begin
                     WriteActive = 1;
                  end
               end
               if ({BlockProtect1,BlockProtect0} == 2'b11) begin
                  // write protected region
               end
            end

            if (WriteActive) begin
                #(tWC);
                
                for (LoopIndex = 0; LoopIndex < WriteCounter; LoopIndex = LoopIndex + 1) begin
                   BaseAddress = {AddrRegister[16:08],8'h00};
                   PageAddress = (AddrRegister[07:00] + LoopIndex);
    
                   MemWrAddress = {BaseAddress[16:08],PageAddress[07:00]};
    
                   if ({BlockProtect1,BlockProtect0} == 2'b00) begin
                      MemoryBlock[MemWrAddress] = WriteBuffer[LoopIndex];
                   end
                   if ({BlockProtect1,BlockProtect0} == 2'b01) begin
                      if ((MemWrAddress >= 17'h18000) && (MemWrAddress <= 17'h1FFFF)) begin
                         // write protected region
                      end
                      else begin
                         MemoryBlock[MemWrAddress] = WriteBuffer[LoopIndex];
                      end
                   end
                   if ({BlockProtect1,BlockProtect0} == 2'b10) begin
                      if ((MemWrAddress >= 17'h10000) && (MemWrAddress <= 17'h1FFFF)) begin
                         // write protected region
                      end
                      else begin
                         MemoryBlock[MemWrAddress] = WriteBuffer[LoopIndex];
                      end
                   end
                   if ({BlockProtect1,BlockProtect0} == 2'b11) begin
                      // write protected region
                   end
                end
            end

            WriteActive = 0;
            WriteEnable = 0;
         end
      end
   end

// -------------------------------------------------------------------------------------------------------
//      1.11:  Erase Cycle Processor
// -------------------------------------------------------------------------------------------------------

   always @(posedge CS_N) begin
      if (DeepPowerDown == 0) begin
         if ((BitCounter == 32) & InstructionPE & WriteEnable & !WriteActive) begin
            for (LoopIndex = 0; LoopIndex < `PAGE_SIZE; LoopIndex = LoopIndex + 1) begin
               BaseAddress = {AddrRegister[16:08],8'h00};
               MemWrAddress = BaseAddress + LoopIndex;

               if ({BlockProtect1,BlockProtect0} == 2'b00) begin
                  WriteActive = 1;
               end
               if ({BlockProtect1,BlockProtect0} == 2'b01) begin
                  if ((BaseAddress >= 17'h18000) && (BaseAddress <= 17'h1FFFF)) begin
                     // write protected region
                  end
                  else begin
                     WriteActive = 1;
                  end
               end
               if ({BlockProtect1,BlockProtect0} == 2'b10) begin
                  if ((BaseAddress >= 17'h10000) && (BaseAddress <= 17'h1FFFF)) begin
                     // write protected region
                  end
                  else begin
                     WriteActive = 1;
                  end
               end
               if ({BlockProtect1,BlockProtect0} == 2'b11) begin
                  // write protected region
               end
            end

            if (WriteActive) begin
                #(tWC);
                
                for (LoopIndex = 0; LoopIndex < `PAGE_SIZE; LoopIndex = LoopIndex + 1) begin
                   BaseAddress = {AddrRegister[16:08],8'h00};
                   MemWrAddress = BaseAddress + LoopIndex;
    
                   if ({BlockProtect1,BlockProtect0} == 2'b00) begin
                      MemoryBlock[MemWrAddress] = 8'hFF;
                   end
                   if ({BlockProtect1,BlockProtect0} == 2'b01) begin
                      if ((BaseAddress >= 17'h18000) && (BaseAddress <= 17'h1FFFF)) begin
                         // write protected region
                      end
                      else begin
                         MemoryBlock[MemWrAddress] = 8'hFF;
                      end
                   end
                   if ({BlockProtect1,BlockProtect0} == 2'b10) begin
                      if ((BaseAddress >= 17'h10000) && (BaseAddress <= 17'h1FFFF)) begin
                         // write protected region
                      end
                      else begin
                         MemoryBlock[MemWrAddress] = 8'hFF;
                      end
                   end
                   if ({BlockProtect1,BlockProtect0} == 2'b11) begin
                      // write protected region
                   end
                end                
            end

            WriteActive = 0;
            WriteEnable = 0;
         end
         if ((BitCounter == 32) & InstructionSE & WriteEnable & !WriteActive) begin
            for (LoopIndex = 0; LoopIndex < `SECTOR_SIZE; LoopIndex = LoopIndex + 1) begin
               BaseAddress = {AddrRegister[16:15],15'h0000};
               MemWrAddress = BaseAddress + LoopIndex;

               if ({BlockProtect1,BlockProtect0} == 2'b00) begin
                  WriteActive = 1;
               end
               if ({BlockProtect1,BlockProtect0} == 2'b01) begin
                  if ((BaseAddress >= 17'h18000) && (BaseAddress <= 17'h1FFFF)) begin
                     // write protected region
                  end
                  else begin
                     WriteActive = 1;
                  end
               end
               if ({BlockProtect1,BlockProtect0} == 2'b10) begin
                  if ((BaseAddress >= 17'h10000) && (BaseAddress <= 17'h1FFFF)) begin
                     // write protected region
                  end
                  else begin
                     WriteActive = 1;
                  end
               end
               if ({BlockProtect1,BlockProtect0} == 2'b11) begin
                  // write protected region
               end
            end

            if (WriteActive) begin
                #(tSE);

                for (LoopIndex = 0; LoopIndex < `SECTOR_SIZE; LoopIndex = LoopIndex + 1) begin
                   BaseAddress = {AddrRegister[16:15],15'h0000};
                   MemWrAddress = BaseAddress + LoopIndex;
    
                   if ({BlockProtect1,BlockProtect0} == 2'b00) begin
                      MemoryBlock[MemWrAddress] = 8'hFF;
                   end
                   if ({BlockProtect1,BlockProtect0} == 2'b01) begin
                      if ((BaseAddress >= 17'h18000) && (BaseAddress <= 17'h1FFFF)) begin
                         // write protected region
                      end
                      else begin
                         MemoryBlock[MemWrAddress] = 8'hFF;
                      end
                   end
                   if ({BlockProtect1,BlockProtect0} == 2'b10) begin
                      if ((BaseAddress >= 17'h10000) && (BaseAddress <= 17'h1FFFF)) begin
                         // write protected region
                      end
                      else begin
                         MemoryBlock[MemWrAddress] = 8'hFF;
                      end
                   end
                   if ({BlockProtect1,BlockProtect0} == 2'b11) begin
                      // write protected region
                   end
                end
            end

            WriteActive = 0;
            WriteEnable = 0;
         end
         if ((BitCounter == 8) & InstructionCE & WriteEnable & !WriteActive) begin
            for (LoopIndex = 0; LoopIndex < `ARRAY_SIZE; LoopIndex = LoopIndex + 1) begin
               MemWrAddress = LoopIndex;

               if ({BlockProtect1,BlockProtect0} == 2'b00) begin
                  WriteActive = 1;
               end
            end

            if (WriteActive) begin
                #(tCE);

                for (LoopIndex = 0; LoopIndex < `ARRAY_SIZE; LoopIndex = LoopIndex + 1) begin
                   MemWrAddress = LoopIndex;
    
                   if ({BlockProtect1,BlockProtect0} == 2'b00) begin
                      MemoryBlock[MemWrAddress] = 8'hFF;
                   end
                end
            end

            WriteActive = 0;
            WriteEnable = 0;
         end
      end
   end

// -------------------------------------------------------------------------------------------------------
//      1.12:  Deep Power-Down Logic
// -------------------------------------------------------------------------------------------------------

   always @(posedge CS_N or posedge RESET) begin
      if (RESET)        DeepPowerDown = 0;
      else if ((BitCounter == 8) & InstructionDPD  & !WriteActive) begin
         #(tPD);
         DeepPowerDown = 1;
      end
      else if ((BitCounter > 7) & InstructionRDID & !WriteActive) begin
         #(tREL);
         DeepPowerDown = 0;
      end
   end

// -------------------------------------------------------------------------------------------------------
//      1.13:  Output Data Shifter
// -------------------------------------------------------------------------------------------------------

   always @(negedge SCK) begin
      if (HOLD_N == 1) begin
         if ((BitCounter >= 32) & (BitCounter[2:0] == 0) & InstructionREAD & !DeepPowerDown) begin
            if (BitCounter == 32) begin
               DataShifterO <= MemoryBlock[AddrRegister[16:00]];
               MemRdAddress <= AddrRegister + 1;
               SO_Enable    <= 1;
            end
            else begin
               DataShifterO <= MemoryBlock[MemRdAddress[16:00]];
               MemRdAddress <= MemRdAddress + 1;
            end
         end
         else if ((BitCounter > 7) & (BitCounter[2:0] == 3'b000) & InstructionRDSR & !DeepPowerDown) begin
            DataShifterO <= {WP_Enable_Old,3'b000,BlockProtect1_Old,BlockProtect0_Old,WriteEnable,WriteActive};
            SO_Enable    <= 1;
         end
         else if ((BitCounter >= 32) & (BitCounter[2:0] == 0) & InstructionRDID) begin
            if (BitCounter == 32) begin
               DataShifterO <= `MFG_ID;
               SO_Enable    <= 1;

            end
            else begin
               DataShifterO <= `MFG_ID;
            end
         end
         else begin
            DataShifterO <= DataShifterO << 1;
         end
      end
   end

// -------------------------------------------------------------------------------------------------------
//      1.14:  Output Data Buffer
// -------------------------------------------------------------------------------------------------------

   bufif1 (SO, SO_DO, SO_OE);

   always @(DataShifterO) SO_DO <= #(tV) DataShifterO[07];

   bufif1 #(tV,0)    (OutputEnable1, SO_Enable, 1);
   notif1 #(tDIS)    (OutputEnable2, CS_N,   1);
   bufif1 #(tHV,tHZ) (OutputEnable3, HOLD_N, 1);

   assign SO_OE = OutputEnable1 & OutputEnable2 & OutputEnable3;


// *******************************************************************************************************
// **   DEBUG LOGIC                                                                                     **
// *******************************************************************************************************
// -------------------------------------------------------------------------------------------------------
//      2.01:  Memory Data Bytes
// -------------------------------------------------------------------------------------------------------

   wire [07:00] MemoryByte00000 = MemoryBlock[000000];
   wire [07:00] MemoryByte00001 = MemoryBlock[000001];
   wire [07:00] MemoryByte00002 = MemoryBlock[000002];
   wire [07:00] MemoryByte00003 = MemoryBlock[000003];
   wire [07:00] MemoryByte00004 = MemoryBlock[000004];
   wire [07:00] MemoryByte00005 = MemoryBlock[000005];
   wire [07:00] MemoryByte00006 = MemoryBlock[000006];
   wire [07:00] MemoryByte00007 = MemoryBlock[000007];
   wire [07:00] MemoryByte00008 = MemoryBlock[000008];
   wire [07:00] MemoryByte00009 = MemoryBlock[000009];
   wire [07:00] MemoryByte0000A = MemoryBlock[000010];
   wire [07:00] MemoryByte0000B = MemoryBlock[000011];
   wire [07:00] MemoryByte0000C = MemoryBlock[000012];
   wire [07:00] MemoryByte0000D = MemoryBlock[000013];
   wire [07:00] MemoryByte0000E = MemoryBlock[000014];
   wire [07:00] MemoryByte0000F = MemoryBlock[000015];

   wire [07:00] MemoryByte1FFF0 = MemoryBlock[131056];
   wire [07:00] MemoryByte1FFF1 = MemoryBlock[131057];
   wire [07:00] MemoryByte1FFF2 = MemoryBlock[131058];
   wire [07:00] MemoryByte1FFF3 = MemoryBlock[131059];
   wire [07:00] MemoryByte1FFF4 = MemoryBlock[131060];
   wire [07:00] MemoryByte1FFF5 = MemoryBlock[131061];
   wire [07:00] MemoryByte1FFF6 = MemoryBlock[131062];
   wire [07:00] MemoryByte1FFF7 = MemoryBlock[131063];
   wire [07:00] MemoryByte1FFF8 = MemoryBlock[131064];
   wire [07:00] MemoryByte1FFF9 = MemoryBlock[131065];
   wire [07:00] MemoryByte1FFFA = MemoryBlock[131066];
   wire [07:00] MemoryByte1FFFB = MemoryBlock[131067];
   wire [07:00] MemoryByte1FFFC = MemoryBlock[131068];
   wire [07:00] MemoryByte1FFFD = MemoryBlock[131069];
   wire [07:00] MemoryByte1FFFE = MemoryBlock[131070];
   wire [07:00] MemoryByte1FFFF = MemoryBlock[131071];

// -------------------------------------------------------------------------------------------------------
//      2.02:  Page Buffer Bytes
// -------------------------------------------------------------------------------------------------------

   wire [07:00] PageBuffer00 = WriteBuffer[000];
   wire [07:00] PageBuffer01 = WriteBuffer[001];
   wire [07:00] PageBuffer02 = WriteBuffer[002];
   wire [07:00] PageBuffer03 = WriteBuffer[003];
   wire [07:00] PageBuffer04 = WriteBuffer[004];
   wire [07:00] PageBuffer05 = WriteBuffer[005];
   wire [07:00] PageBuffer06 = WriteBuffer[006];
   wire [07:00] PageBuffer07 = WriteBuffer[007];
   wire [07:00] PageBuffer08 = WriteBuffer[008];
   wire [07:00] PageBuffer09 = WriteBuffer[009];
   wire [07:00] PageBuffer0A = WriteBuffer[010];
   wire [07:00] PageBuffer0B = WriteBuffer[011];
   wire [07:00] PageBuffer0C = WriteBuffer[012];
   wire [07:00] PageBuffer0D = WriteBuffer[013];
   wire [07:00] PageBuffer0E = WriteBuffer[014];
   wire [07:00] PageBuffer0F = WriteBuffer[015];

   wire [07:00] PageBuffer10 = WriteBuffer[016];
   wire [07:00] PageBuffer11 = WriteBuffer[017];
   wire [07:00] PageBuffer12 = WriteBuffer[018];
   wire [07:00] PageBuffer13 = WriteBuffer[019];
   wire [07:00] PageBuffer14 = WriteBuffer[020];
   wire [07:00] PageBuffer15 = WriteBuffer[021];
   wire [07:00] PageBuffer16 = WriteBuffer[022];
   wire [07:00] PageBuffer17 = WriteBuffer[023];
   wire [07:00] PageBuffer18 = WriteBuffer[024];
   wire [07:00] PageBuffer19 = WriteBuffer[025];
   wire [07:00] PageBuffer1A = WriteBuffer[026];
   wire [07:00] PageBuffer1B = WriteBuffer[027];
   wire [07:00] PageBuffer1C = WriteBuffer[028];
   wire [07:00] PageBuffer1D = WriteBuffer[029];
   wire [07:00] PageBuffer1E = WriteBuffer[030];
   wire [07:00] PageBuffer1F = WriteBuffer[031];

   wire [07:00] PageBufferE0 = WriteBuffer[224];
   wire [07:00] PageBufferE1 = WriteBuffer[225];
   wire [07:00] PageBufferE2 = WriteBuffer[226];
   wire [07:00] PageBufferE3 = WriteBuffer[227];
   wire [07:00] PageBufferE4 = WriteBuffer[228];
   wire [07:00] PageBufferE5 = WriteBuffer[229];
   wire [07:00] PageBufferE6 = WriteBuffer[230];
   wire [07:00] PageBufferE7 = WriteBuffer[231];
   wire [07:00] PageBufferE8 = WriteBuffer[232];
   wire [07:00] PageBufferE9 = WriteBuffer[233];
   wire [07:00] PageBufferEA = WriteBuffer[234];
   wire [07:00] PageBufferEB = WriteBuffer[235];
   wire [07:00] PageBufferEC = WriteBuffer[236];
   wire [07:00] PageBufferED = WriteBuffer[237];
   wire [07:00] PageBufferEE = WriteBuffer[238];
   wire [07:00] PageBufferEF = WriteBuffer[239];

   wire [07:00] PageBufferF0 = WriteBuffer[240];
   wire [07:00] PageBufferF1 = WriteBuffer[241];
   wire [07:00] PageBufferF2 = WriteBuffer[242];
   wire [07:00] PageBufferF3 = WriteBuffer[243];
   wire [07:00] PageBufferF4 = WriteBuffer[244];
   wire [07:00] PageBufferF5 = WriteBuffer[245];
   wire [07:00] PageBufferF6 = WriteBuffer[246];
   wire [07:00] PageBufferF7 = WriteBuffer[247];
   wire [07:00] PageBufferF8 = WriteBuffer[248];
   wire [07:00] PageBufferF9 = WriteBuffer[249];
   wire [07:00] PageBufferFA = WriteBuffer[250];
   wire [07:00] PageBufferFB = WriteBuffer[251];
   wire [07:00] PageBufferFC = WriteBuffer[252];
   wire [07:00] PageBufferFD = WriteBuffer[253];
   wire [07:00] PageBufferFE = WriteBuffer[254];
   wire [07:00] PageBufferFF = WriteBuffer[255];


// *******************************************************************************************************
// **   TIMING CHECKS                                                                                   **
// *******************************************************************************************************

   wire TimingCheckEnable = (RESET == 0) & (CS_N == 0);

   specify
      `ifdef VCC_1_8V_TO_2_5V
         specparam
            tHI  =  250,                                // SCK pulse width - high
            tLO  =  250,                                // SCK pulse width - low
            tSU  =  50,                                 // SI to SCK setup time
            tHD  =  100,                                // SI to SCK hold time
            tHS  =  100,                                // HOLD_N to SCK setup time
            tHH  =  100,                                // HOLD_N to SCK hold time
            tCSD =  50,                                 // CS_N disable time
            tCSS =  250,                                // CS_N to SCK setup time
            tCSH = 500,                                 // CS_N to SCK hold time
            tCLD = 50,                                  // Clock delay time
            tCLE = 50;                                  // Clock enable time
      `else
      `ifdef VCC_2_5V_TO_4_5V
         specparam
            tHI  =  50,                                 // SCK pulse width - high
            tLO  =  50,                                 // SCK pulse width - low
            tSU  =  10,                                 // SI to SCK setup time
            tHD  =  20,                                 // SI to SCK hold time
            tHS  =  20,                                 // HOLD_N to SCK setup time
            tHH  =  20,                                 // HOLD_N to SCK hold time
            tCSD =  50,                                 // CS_N disable time
            tCSS =  50,                                 // CS_N to SCK setup time
            tCSH = 100,                                 // CS_N to SCK hold time
            tCLD = 50,                                  // Clock delay time
            tCLE = 50;                                  // Clock enable time
      `else
      `ifdef VCC_4_5V_TO_5_5V
         specparam
            tHI  =  25,                                 // SCK pulse width - high
            tLO  =  25,                                 // SCK pulse width - low
            tSU  =   5,                                 // SI to SCK setup time
            tHD  =  10,                                 // SI to SCK hold time
            tHS  =  10,                                 // HOLD_N to SCK setup time
            tHH  =  10,                                 // HOLD_N to SCK hold time
            tCSD =  50,                                 // CS_N disable time
            tCSS =  25,                                 // CS_N to SCK setup time
            tCSH =  50,                                 // CS_N to SCK hold time
            tCLD = 50,                                  // Clock delay time
            tCLE = 50;                                  // Clock enable time
      `else
         specparam
            tHI  =  25,                                 // SCK pulse width - high
            tLO  =  25,                                 // SCK pulse width - low
            tSU  =   5,                                 // SI to SCK setup time
            tHD  =  10,                                 // SI to SCK hold time
            tHS  =  10,                                 // HOLD_N to SCK setup time
            tHH  =  10,                                 // HOLD_N to SCK hold time
            tCSD =  50,                                 // CS_N disable time
            tCSS =  25,                                 // CS_N to SCK setup time
            tCSH =  50,                                 // CS_N to SCK hold time
            tCLD = 50,                                  // Clock delay time
            tCLE = 50;                                  // Clock enable time
      `endif
      `endif
      `endif

      $width (posedge SCK,  tHI);
      $width (negedge SCK,  tLO);
      $width (posedge CS_N, tCSD);

      $setup (SI, posedge SCK &&& TimingCheckEnable, tSU);
      $setup (negedge CS_N, posedge SCK &&& TimingCheckEnable, tCSS);
      $setup (negedge SCK, negedge HOLD_N &&& TimingCheckEnable, tHS);
      $setup (posedge CS_N, posedge SCK &&& TimingCheckEnable, tCLD);

      $hold  (posedge SCK    &&& TimingCheckEnable, SI,   tHD);
      $hold  (posedge SCK    &&& TimingCheckEnable, posedge CS_N, tCSH);
      $hold  (posedge HOLD_N &&& TimingCheckEnable, posedge SCK,  tHH);
      $hold  (posedge SCK    &&& TimingCheckEnable, negedge CS_N, tCLE);
  endspecify

endmodule
