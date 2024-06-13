/*
 * Copyright (c) 2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-13     Feiyang Chen the first version
 */

#include "asm.h"
#include "stackframe.h"

    .section ".exc_vectors", "ax"
    .extern interrupt_dispatch
    .extern rt_thread_switch_interrupt_flag
    .extern rt_interrupt_from_thread
    .extern rt_interrupt_to_thread
__eentry_start:
general_exception:
    BACKUP_T0T1
    SAVE_ALL

    move    $s0, $sp /* save sp */

    la      $sp, _system_stack /* switch to kernel stack */

    bl      interrupt_dispatch

    move    $sp, $s0 /* restore sp */

    /*
     * switch to the new thread if necessary
     */
    la      $t0, rt_thread_switch_interrupt_flag
    LD      $t1, $t0, 0
    beqz    $t1, spurious_interrupt

    ST      $zero, $t0, 0

    la      $t0, rt_interrupt_from_thread
    LD      $t1, $t0, 0
    ST      $sp, $t1, 0

    la      $t0, rt_interrupt_to_thread
    LD      $t1, $t0, 0
    LD      $sp, $t1, 0

spurious_interrupt:
    RESTORE_ALL_AND_RET

    .org __eentry_start + 0x1000
    .extern tlb_refill_handler
tlb_refill_exception:
    b       tlb_refill_handler

    .org __eentry_start + 0x2000
    .extern machine_error_handler
machine_error_exception:
    b       machine_error_handler
