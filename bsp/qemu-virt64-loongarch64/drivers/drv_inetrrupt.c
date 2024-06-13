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
#include "drv_interrupt.h"
#include "loongarch.h"

static void pch_pic_init(void)
{
    /* unmask interrupt */
    HWREG32(PCH_PIC_INT_MASK_BASE) = 0x0;
    HWREG32(PCH_PIC_INT_MASK_BASE + 0x4) = 0x0;
}

static void ext_ioi_init(void)
{
    int i;

    for (i = 0; i < 8; i++)
    {
        __iocsrwr_w(0xffffffff, EIOINTC_REG_ENABLE_BASE + i * 4);
    }
}

int rt_hw_io_interrupt_init(void)
{
    pch_pic_init();
    ext_ioi_init();

    return 0;
}
INIT_BOARD_EXPORT(rt_hw_io_interrupt_init);
