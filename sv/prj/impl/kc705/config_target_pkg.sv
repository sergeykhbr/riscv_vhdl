////////////////////////////////////////////////////////////////////////////-
//! @file
//! @author     Sergey Khabarov - sergeykhbr@gmail.com
//////////////////////////////////////////////////////////////////////////////

package config_target_pkg;

//  `define TARGET_KC705

  localparam bit CFG_ASYNC_RESET = 1'b0;

  localparam CFG_TOPDIR = "../../../../";

  /// @brief   Number of processors in a system
  /// @details This value may be in a range 1 to CFG_TOTAL_CPU_MAX-1
  localparam int CFG_CPU_NUM = 1;
  /// @brief Enable/disable L2 caching. L2 can be enabled even in 1 CPU config
  localparam int CFG_L2CACHE_ENA = 1;

  //! @brief   HEX-image for the initialization of the Boot ROM.
  //! @details This file is used by \e inferred ROM implementation.
  localparam CFG_BOOTROM_FILE =
                {CFG_TOPDIR, "examples/bootrom_tests/linuxbuild/bin/bootrom_tests"};
  //            {CFG_TOPDIR, "examples/boot/linuxbuild/bin/bootimage"};
  localparam CFG_BOOTROM_FILE_HEX = {CFG_BOOTROM_FILE, ".hex"};


  /// @brief Hardware SoC Identificator.
  ///
  /// @details Read Only unique platform identificator that could be
  ///          read by firmware from the Plug'n'Play support module.
  localparam bit [31:0] CFG_HW_ID = 32'h20221101;


endpackage: config_target_pkg
