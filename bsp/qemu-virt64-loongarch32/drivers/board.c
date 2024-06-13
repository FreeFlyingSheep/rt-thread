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
#include <rthw.h>

#include "board.h"
#include "interrupt.h"

void rt_hw_board_init(void)
{
    rt_hw_interrupt_init();

#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif

#ifdef RT_USING_CONSOLE
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
#endif

#ifdef RT_USING_HEAP
    rt_system_heap_init((void *)RT_HW_HEAP_BEGIN, (void *)RT_HW_HEAP_END);
#endif
}
