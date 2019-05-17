.text
.align 8
.globl _mbist_ram

_mbist_ram:
  // Read 'tech' value from PNP module
  lui	a5,0x100
  addi	a5,a5,-1    # fffff
  slli	a5,a5,0xc   # a5 = PNP base address
  lw	a4,8(a5)
  andi	a4,a4,255             # a4 = tech & 0xFF
  beqz	a4,scaler_inferred    # a4 == inferred
  li	a4,(SYS_HZ/115200/2)
  j	uart_scaler_end
scaler_inferred:
  li	a4,3             # fast rtl simulation
uart_scaler_end:

   //      uart_map *uart = (uart_map *)ADDR_NASTI_SLAVE_UART1;
  lui	a5,0x80
  addi	a5,a5,1 # 80001
  slli	a5,a5,0xc
  sw	a4,4(a5)    # scaler = a4
  // Wait while Tx FIFO becomes empty
mbist_waitempty1:
  lw	a4,0(a5)
  li	a3,2
  and	a4,a4,a3
  beqz	a4,mbist_waitempty1

  //  output "SRAM . . . . . .PASS/FAIL\r\n";
  li	a4,83    # 'S'
  sw	a4,16(a5)
  li	a4,82    # 'R'
  sw	a4,16(a5)
  li	a4,65    # 'A'
  sw	a4,16(a5)
  li	a4,77    # 'M'
  sw	a4,16(a5)
  li	a4,32    # ' '
  sw	a4,16(a5)
  li	a4,46    # '.'
  sw	a4,16(a5)
  li	a4,32    # ' '
  sw	a4,16(a5)
  li	a4,46    # '.'
  sw	a4,16(a5)
  li	a4,32    # ' '
  sw	a4,16(a5)
  li	a4,46    # '.'
  sw	a4,16(a5)
  li	a4,32    # ' '
  sw	a4,16(a5)
  li	a4,46    # '.'
  sw	a4,16(a5)
  li	a4,32    # ' '
  sw	a4,16(a5)
  li	a4,46    # '.'
  sw	a4,16(a5)
  li	a4,32    # ' '
  sw	a4,16(a5)
  li	a4,46    # '.'
  sw	a4,16(a5)

  // Check SRAM full range:
  //    write -1, read, compare
  //    write  0, read, compare

  li    a2,0        # error counter
  lui	a3,0x10000  # a3 = SRAM base
  li    a4,0        # i = 0
  j	mbist_check1
mbist_cycle_start:
  slli	t0,a4,0x3   # t0 = 8*i
  add	a0,a3,t0    # a0 = &SRAM[i]

  // Check  write/read ~0ull
  li	t1,-1
  sd	t1,0(a0)    # SRAM[i] = -1
  ld    t2,0(a0)    
  beq	t1,t2,mbist_noerr1
  // increment error counter
  addiw	a2,a2,1
mbist_noerr1:

  // Check  write/read 0ull
  li	t1,0
  sd	t1,0(a0)    # SRAM[i] = -1
  ld    t2,0(a0)    
  beq	t1,t2,mbist_noerr2
  // increment error counter
  addiw	a2,a2,1
mbist_noerr2:

  addiw	a4,a4,1     # i++
mbist_check1:
  lui	a0,0x1      # 4096 qwords (uint64_t) total
  blt	a4,a0,mbist_cycle_start

  // Wait while Tx FIFO becomes empty
mbist_waitempty2:
  lw	a4,0(a5)
  li	a3,2
  and	a4,a4,a3
  beqz	a4,mbist_waitempty2

  // Check error counter: a2
  beqz	a2,mbist_ok
  li	a4,70    # 'F'
  sw	a4,16(a5)
  li	a4,65    # 'A'
  sw	a4,16(a5)
  li	a4,73    # 'I'
  sw	a4,16(a5)
  li	a4,76    # 'L'
  sw	a4,16(a5)
  j	mbist_sram_end
mbist_ok:
  li	a4,80    # 'P'
  sw	a4,16(a5)
  li	a4,65    # 'A'
  sw	a4,16(a5)
  li	a4,83    # 'S'
  sw	a4,16(a5)
  li	a4,83    # 'S'
  sw	a4,16(a5)
mbist_sram_end:

  li	a4,13    # '\r'
  sw	a4,16(a5)
  li	a4,10    # '\n'
  sw	a4,16(a5)

  jal   _mbist_ram_exit

