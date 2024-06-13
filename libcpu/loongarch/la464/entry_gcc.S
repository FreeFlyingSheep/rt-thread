/*
 * Copyright (c) 2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-13     Feiyang Chen the first version
 */

#include "loongarch.h"

    .text
    .section ".startup", "ax"
    .globl _start
_start:
    /* config direct window */
    li.d    $t0, 0x8000000000000001
    csrwr   $t0, LOONGARCH_CSR_DMWIN0
    li.d    $t0, 0x9000000000000011
    csrwr   $t0, LOONGARCH_CSR_DMWIN1

    /* jump to link address */
    la.abs  $t0, 1f
    jr      $t0
1:

    /* clear bss */
    la      $t0, __bss_start
    la      $t1, __bss_end
2:
    st.d    $zero, $t0, 0
    addi.d  $t0, $t0, 8
    blt     $t0, $t1, 2b

    /* initialize sp */
    la      $sp, _system_stack

    /* jump to RT-Thread kernel */
    bl      rtthread_startup
    /* never reach here */

    /* enable interrupt */
    li.d    $t0, CSR_CRMD_IE
    csrxchg $t0, $t0, LOONGARCH_CSR_CRMD

    idle    0
