/*
 * Copyright (c) 2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-13     Feiyang Chen the first version
 */

#include <rthw.h>
#include <rtthread.h>

#include "board.h"
#include "drv_timer.h"
#include "interrupt.h"
#include "loongarch.h"

static void rt_hw_timer_handler(int vector, void *param)
{
    rt_interrupt_enter();

    /* clear interrupt */
    __csrwr_w(CSR_TINTCLR_TI, LOONGARCH_CSR_TINTCLR);

    /* increase a OS tick */
    rt_tick_increase();

    rt_interrupt_leave();
}

static unsigned int get_cpu_freq(void)
{
    unsigned int res;
    unsigned int base_freq;
    unsigned int cfm, cfd;

    base_freq = __cpucfg(LOONGARCH_CPUCFG4);
    res = __cpucfg(LOONGARCH_CPUCFG5);
    cfm = res & 0xffff;
    cfd = (res >> 16) & 0xffff;

    if (!base_freq || !cfm || !cfd)
        return 100000000; /* default CPU frequency */

    return (base_freq * cfm / cfd);
}

int rt_hw_timer_init(void)
{
    rt_ubase_t period;
    rt_ubase_t timer_config;

    period = get_cpu_freq() / RT_TICK_PER_SECOND;
    timer_config = period & CSR_TCFG_VAL;
    timer_config |= (CSR_TCFG_PERIOD | CSR_TCFG_EN);
    __csrwr_d(timer_config, LOONGARCH_CSR_TCFG);

    rt_hw_interrupt_install(IRQ_TO_VECTOR(ESTAT_IS_TIMER_SHIFT), rt_hw_timer_handler, RT_NULL, "tick");
    rt_hw_interrupt_umask(ESTAT_IS_TIMER_SHIFT);

    return 0;
}
INIT_BOARD_EXPORT(rt_hw_timer_init);
