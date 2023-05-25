 /**
 * @file hal_thread_s.s
 * @author 王彬浩 (SPGGOGOGO@outlook.com)
 * @brief hal层，线程上下文切换操作
 * @version 1.0
 * @date 2022-07-17
 * @copyright Copyright (c) 2023
 * @revisionHistory
 *  <table>
 *   <tr><th> 版本 <th>作者 <th>日期 <th>修改内容
 *   <tr><td> 0.1 <td>jivin <td>2010-03-08 <td>Created
 *   <tr><td> 1.0 <td>王彬浩 <td> 2022-07-17 <td>Standardized
 *  </table>
 */

.globl HAL_INTR_CTX_SWITCH
.globl HAL_CONTEXT_SWITCH
.globl HAL_CTX_SWITCH_EXIT
.globl HAL_SWITCH_TO

#define REGBYTES    8
#define FREGBYTES   4

HAL_SWITCH_TO:
    ld sp, (a0)

#ifdef ACORAL_USING_SMP /*//SPG */
    mv   a0,   a1
    call  rt_cpus_lock_status_resd
#endif
    ld a0,   2 * REGBYTES(sp)
    csrw mstatus, a0
    j    HAL_CTX_SWITCH_EXIT

HAL_CONTEXT_SWITCH: #//SPG要不要加@function？
#ifdef ARCH_RISCV_FPU
    addi    sp, sp, -32 * FREGBYTES

    fsw  f0, 0 * FREGBYTES(sp)
    fsw  f1, 1 * FREGBYTES(sp)
    fsw  f2, 2 * FREGBYTES(sp)
    fsw  f3, 3 * FREGBYTES(sp)
    fsw  f4, 4 * FREGBYTES(sp)
    fsw  f5, 5 * FREGBYTES(sp)
    fsw  f6, 6 * FREGBYTES(sp)
    fsw  f7, 7 * FREGBYTES(sp)
    fsw  f8, 8 * FREGBYTES(sp)
    fsw  f9, 9 * FREGBYTES(sp)
    fsw  f10, 10 * FREGBYTES(sp)
    fsw  f11, 11 * FREGBYTES(sp)
    fsw  f12, 12 * FREGBYTES(sp)
    fsw  f13, 13 * FREGBYTES(sp)
    fsw  f14, 14 * FREGBYTES(sp)
    fsw  f15, 15 * FREGBYTES(sp)
    fsw  f16, 16 * FREGBYTES(sp)
    fsw  f17, 17 * FREGBYTES(sp)
    fsw  f18, 18 * FREGBYTES(sp)
    fsw  f19, 19 * FREGBYTES(sp)
    fsw  f20, 20 * FREGBYTES(sp)
    fsw  f21, 21 * FREGBYTES(sp)
    fsw  f22, 22 * FREGBYTES(sp)
    fsw  f23, 23 * FREGBYTES(sp)
    fsw  f24, 24 * FREGBYTES(sp)
    fsw  f25, 25 * FREGBYTES(sp)
    fsw  f26, 26 * FREGBYTES(sp)
    fsw  f27, 27 * FREGBYTES(sp)
    fsw  f28, 28 * FREGBYTES(sp)
    fsw  f29, 29 * FREGBYTES(sp)
    fsw  f30, 30 * FREGBYTES(sp)
    fsw  f31, 31 * FREGBYTES(sp)

#endif
    addi  sp,  sp, -32 * REGBYTES
    sd sp,  (a0)

    sd x1,   0 * REGBYTES(sp)
    sd x1,   1 * REGBYTES(sp)

    csrr a0, mstatus
    andi a0, a0, 8
    beqz a0, save_mpie
    li   a0, 0x80
save_mpie:
    sd a0,   2 * REGBYTES(sp)

    sd x4,   4 * REGBYTES(sp)
    sd x5,   5 * REGBYTES(sp)
    sd x6,   6 * REGBYTES(sp)
    sd x7,   7 * REGBYTES(sp)
    sd x8,   8 * REGBYTES(sp)
    sd x9,   9 * REGBYTES(sp)
    sd x10, 10 * REGBYTES(sp)
    sd x11, 11 * REGBYTES(sp)
    sd x12, 12 * REGBYTES(sp)
    sd x13, 13 * REGBYTES(sp)
    sd x14, 14 * REGBYTES(sp)
    sd x15, 15 * REGBYTES(sp)
    sd x16, 16 * REGBYTES(sp)
    sd x17, 17 * REGBYTES(sp)
    sd x18, 18 * REGBYTES(sp)
    sd x19, 19 * REGBYTES(sp)
    sd x20, 20 * REGBYTES(sp)
    sd x21, 21 * REGBYTES(sp)
    sd x22, 22 * REGBYTES(sp)
    sd x23, 23 * REGBYTES(sp)
    sd x24, 24 * REGBYTES(sp)
    sd x25, 25 * REGBYTES(sp)
    sd x26, 26 * REGBYTES(sp)
    sd x27, 27 * REGBYTES(sp)
    sd x28, 28 * REGBYTES(sp)
    sd x29, 29 * REGBYTES(sp)
    sd x30, 30 * REGBYTES(sp)
    sd x31, 31 * REGBYTES(sp)

    /* resd to thread context
     * sp(0) -> epc;
     * sp(1) -> ra;
     * sp(i) -> x(i+2)
     */
    ld sp,  (a1)
#ifdef ACORAL_USING_SMP
    mv   a0,   a2
    call  rt_cpus_lock_status_resd
#endif /*ACORAL_USING_SMP*/
    j HAL_CTX_SWITCH_EXIT


#ifdef ACORAL_USING_SMP
HAL_INTR_CTX_SWITCH:

    sd a0, 0(a1)

    ld  sp, 0(a2)
    move  a0, a3
    call rt_cpus_lock_status_resd

    j HAL_CTX_SWITCH_EXIT
#endif


HAL_CTX_SWITCH_EXIT:
#ifdef ACORAL_USING_SMP
#ifdef ACORAL_USING_SIGNALS
    mv a0, sp

    csrr  t0, mhartid
    /* switch interrupt stack of current cpu */
    la    sp, __stack_start__
    addi  t1, t0, 1
    li    t2, __STACKSIZE__
    mul   t1, t1, t2
    add   sp, sp, t1 /* sp = (cpuid + 1) * __STACKSIZE__ + __stack_start__ */

    call rt_signal_check
    mv sp, a0
#endif
#endif
    /* resw ra to mepc */
    ld a0,   0 * REGBYTES(sp)
    csrw mepc, a0

    ld x1,   1 * REGBYTES(sp)

    li    t0, 0x00007800
    csrw  mstatus, t0
    ld a0,   2 * REGBYTES(sp)
    csrs mstatus, a0

    ld x4,   4 * REGBYTES(sp)
    ld x5,   5 * REGBYTES(sp)
    ld x6,   6 * REGBYTES(sp)
    ld x7,   7 * REGBYTES(sp)
    ld x8,   8 * REGBYTES(sp)
    ld x9,   9 * REGBYTES(sp)
    ld x10, 10 * REGBYTES(sp)
    ld x11, 11 * REGBYTES(sp)
    ld x12, 12 * REGBYTES(sp)
    ld x13, 13 * REGBYTES(sp)
    ld x14, 14 * REGBYTES(sp)
    ld x15, 15 * REGBYTES(sp)
    ld x16, 16 * REGBYTES(sp)
    ld x17, 17 * REGBYTES(sp)
    ld x18, 18 * REGBYTES(sp)
    ld x19, 19 * REGBYTES(sp)
    ld x20, 20 * REGBYTES(sp)
    ld x21, 21 * REGBYTES(sp)
    ld x22, 22 * REGBYTES(sp)
    ld x23, 23 * REGBYTES(sp)
    ld x24, 24 * REGBYTES(sp)
    ld x25, 25 * REGBYTES(sp)
    ld x26, 26 * REGBYTES(sp)
    ld x27, 27 * REGBYTES(sp)
    ld x28, 28 * REGBYTES(sp)
    ld x29, 29 * REGBYTES(sp)
    ld x30, 30 * REGBYTES(sp)
    ld x31, 31 * REGBYTES(sp)

    addi sp,  sp, 32 * REGBYTES

#ifdef ARCH_RISCV_FPU
    flw   f0, 0 * FREGBYTES(sp)
    flw   f1, 1 * FREGBYTES(sp)
    flw   f2, 2 * FREGBYTES(sp)
    flw   f3, 3 * FREGBYTES(sp)
    flw   f4, 4 * FREGBYTES(sp)
    flw   f5, 5 * FREGBYTES(sp)
    flw   f6, 6 * FREGBYTES(sp)
    flw   f7, 7 * FREGBYTES(sp)
    flw   f8, 8 * FREGBYTES(sp)
    flw   f9, 9 * FREGBYTES(sp)
    flw   f10, 10 * FREGBYTES(sp)
    flw   f11, 11 * FREGBYTES(sp)
    flw   f12, 12 * FREGBYTES(sp)
    flw   f13, 13 * FREGBYTES(sp)
    flw   f14, 14 * FREGBYTES(sp)
    flw   f15, 15 * FREGBYTES(sp)
    flw   f16, 16 * FREGBYTES(sp)
    flw   f17, 17 * FREGBYTES(sp)
    flw   f18, 18 * FREGBYTES(sp)
    flw   f19, 19 * FREGBYTES(sp)
    flw   f20, 20 * FREGBYTES(sp)
    flw   f21, 21 * FREGBYTES(sp)
    flw   f22, 22 * FREGBYTES(sp)
    flw   f23, 23 * FREGBYTES(sp)
    flw   f24, 24 * FREGBYTES(sp)
    flw   f25, 25 * FREGBYTES(sp)
    flw   f26, 26 * FREGBYTES(sp)
    flw   f27, 27 * FREGBYTES(sp)
    flw   f28, 28 * FREGBYTES(sp)
    flw   f29, 29 * FREGBYTES(sp)
    flw   f30, 30 * FREGBYTES(sp)
    flw   f31, 31 * FREGBYTES(sp)

    addi    sp, sp, 32 * FREGBYTES
#endif

    mret

    

