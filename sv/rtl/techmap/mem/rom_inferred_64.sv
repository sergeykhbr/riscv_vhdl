`timescale 1ns/10ps

module rom_inferred_64
#(
    parameter abits = 12,
    parameter hex_filename = ""
)
(
    input clk,
    input logic [abits-1: 0] address,
    output logic [63 : 0] data
);

localparam int ROM_LENGTH = 2**abits;

typedef logic [31:0] rom_block [0 : ROM_LENGTH-1];
typedef rom_block rom_type [0 : 1];

rom_type rom;


initial
    begin: init_proc
        bit [1:0][31:0] line;
        int fd;
        int lcnt;

        if(hex_filename != "")
            begin

                fd = $fopen(hex_filename, "r");

                if(!fd)
                    begin
                        $error("Can't open ROM init file!\nInstance: %m\nInit file: %s", hex_filename);
                    end

                lcnt = 0;

                while($fscanf(fd, "%H", line) == 1)
                    begin
                        for(int i = 0; i < 2; i++)
                            begin
                                rom[i][lcnt] = line[i];
                            end

                        lcnt++;
                    end

                if(lcnt != ROM_LENGTH)
                    begin
                        $warning("ROM is not fully initialized!\nInstance: %m\nRead lines: %d\nROM lines: %d", lcnt, ROM_LENGTH);
                    end
            end
        else
            begin
                $warning("ROM is not initialized!\nInstance: %m");
            end
    end: init_proc



always_ff @(posedge clk)
    begin: main_proc
        data[0 +: 32] <= rom[0][address[abits - 1 : 0]];
        data[32 +: 32] <= rom[1][address[abits - 1 : 0]];
    end: main_proc

endmodule: rom_inferred_64
