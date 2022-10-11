  .globl exception_handler_c

  j enter_u_mode

  # trap vector specific for user-mode tests:
u_mode_trap_table:
  # All registers are invalid after riscv-tests finished!!
  csrr sp, 0x7b3          # restore dedicated stack region from dscratch1 dregister
  j exception_handler_c

enter_u_mode:
  csrw 0x7b3, sp          # save dedicated stack region into dscratch1 register to run riscv-tests
  la t0, u_mode_trap_table
  csrw mtvec, t0

  li t0, 0x00001880   # MPP[12:11] = 0x0 MPIE[7]=0 (Previous to machine mode)
  csrc mstatus, t0    # run tests in user mode = 0, by clearing bits

  # Enter U-mode
  la  t0, 1f
  csrw mepc, t0
  mret
1:


