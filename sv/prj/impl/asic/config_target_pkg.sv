package config_target_pkg;

localparam bit CFG_ASYNC_RESET = 0;

// @brief   Number of processors in a system
// @details This value may be in a range 1 to CFG_TOTAL_CPU_MAX-1
localparam int CFG_CPU_NUM = 1;

// @brief Caches size parameters.
// @note Caches line size configured in river_cfg file and affects L1 memory bus width.
localparam int CFG_ILOG2_LINES_PER_WAY = 7;                 // I$ length: 7=16KB; 8=32KB; ..
localparam int CFG_ILOG2_NWAYS = 2;                         // I$ associativity. Default bits width = 2, means 4 ways

localparam int CFG_DLOG2_LINES_PER_WAY = 7;                 // D$ length: 7=16KB; 8=32KB; ..
localparam int CFG_DLOG2_NWAYS = 2;                         // D$ associativity. Default bits width = 2, means 4 ways

// @brief Enable/disable L2 caching. L2 can be enabled even in 1 CPU config
localparam int CFG_L2CACHE_ENA = 1;
localparam int CFG_L2_LOG2_NWAYS = 4;
localparam int CFG_L2_LOG2_LINES_PER_WAY = 9;               // 7=16KB; 8=32KB; 9=64KB, ..

// Internal Boot ROM size:
localparam int CFG_BOOTROM_LOG2_SIZE = 16;                  // 16=64 KB (default); 17=128KB; ..

// Project relative HEX-file name to init boot ROM without .hex extension:
localparam string CFG_BOOTROM_FILE_HEX = "../../../../examples/bootrom_tests/linuxbuild/bin/bootrom_tests";

// Internal SRAM block:
//     - Increase memory map if need > 2MB FU740
//     - Change bootloader stack pointer if need less than 512 KB
localparam int CFG_SRAM_LOG2_SIZE = 18;                     // 19=512 KB (KC705); 21=2 MB (ASIC); ..

endpackage: config_target_pkg
