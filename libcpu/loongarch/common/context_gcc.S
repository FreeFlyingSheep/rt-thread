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
#include "loongarch.h"
#include "stackframe.h"

/*
 * void rt_hw_context_switch_to(rt_uint32 to)
 */
    .globl rt_hw_context_switch_to
rt_hw_context_switch_to:
    LD      $sp, $a0, 0 /* get new task stack pointer */
    RESTORE_ALL_AND_RET

/*
 * void rt_hw_context_switch(rt_uint32 from, rt_uint32 to)
 */
    .globl rt_hw_context_switch
rt_hw_context_switch:
    BACKUP_T0T1
    move    $t0, $ra
    csrwr   $t0, LOONGARCH_CSR_ERA
    SAVE_ALL

    ST      $sp, $a0, 0
    LD      $sp, $a1, 0

    RESTORE_ALL_AND_RET

    .globl rt_hw_context_switch_interrupt
    .extern rt_thread_switch_interrupt_flag
    .extern rt_interrupt_from_thread
    .extern rt_interrupt_to_thread
rt_hw_context_switch_interrupt:
    la      $t0, rt_thread_switch_interrupt_flag
    LD      $t1, $t0, 0
    bnez    $t1, _reswitch

    li.w    $t1, 1
    ST      $t1, $t0, 0

    la      $t0, rt_interrupt_from_thread
    ST      $a0, $t0, 0

_reswitch:
    la      $t0, rt_interrupt_to_thread
    ST      $a1, $t0, 0

    jr      $ra
