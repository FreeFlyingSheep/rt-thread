/*
 * Copyright (c) 2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-13     Feiyang Chen the first version
 */

#include <rtthread.h>

#include "loongarch.h"
#include "stackframe.h"

rt_uint8_t *rt_hw_stack_init(void *tentry, void *parameter, rt_uint8_t *stack_addr, void *texit)
{
    rt_ubase_t *stk;

    /* get aligned */
    stk = (rt_ubase_t *)RT_ALIGN_DOWN((rt_base_t)stack_addr, SIZE * 2);
    stk -= FRAME_SIZE / SIZE;

    stk[0]  = 0;                                            /* zero */
    stk[1]  = (rt_ubase_t)texit;                            /* ra */
    stk[2]  = 0;                                            /* tp */
    stk[3]  = (rt_ubase_t)stk;                              /* sp */
    stk[4]  = (rt_ubase_t)parameter;                        /* a0 */
    stk[5]  = 0;                                            /* a1 */
    stk[6]  = 0;                                            /* a2 */
    stk[7]  = 0;                                            /* a3 */
    stk[8]  = 0;                                            /* a4 */
    stk[9]  = 0;                                            /* a5 */
    stk[10] = 0;                                            /* a6 */
    stk[11] = 0;                                            /* a7 */
    stk[12] = 0;                                            /* t0 */
    stk[13] = 0;                                            /* t1 */
    stk[14] = 0;                                            /* t2 */
    stk[15] = 0;                                            /* t3 */
    stk[16] = 0;                                            /* t4 */
    stk[17] = 0;                                            /* t5 */
    stk[18] = 0;                                            /* t6 */
    stk[19] = 0;                                            /* t7 */
    stk[20] = 0;                                            /* t8 */
    stk[21] = 0;                                            /* r21 */
    stk[22] = 0;                                            /* fp */
    stk[23] = 0;                                            /* s0 */
    stk[24] = 0;                                            /* s1 */
    stk[25] = 0;                                            /* s2 */
    stk[26] = 0;                                            /* s3 */
    stk[27] = 0;                                            /* s4 */
    stk[28] = 0;                                            /* s5 */
    stk[29] = 0;                                            /* s6 */
    stk[30] = 0;                                            /* s7 */
    stk[31] = 0;                                            /* s8 */

    stk[32] = (rt_ubase_t)tentry;                           /* csr_era */
    /* enable interrupt */
    stk[33] = __csrrd_w(LOONGARCH_CSR_PRMD) | CSR_PRMD_PIE; /* csr_prmd */

    return (rt_uint8_t *)stk;
}
