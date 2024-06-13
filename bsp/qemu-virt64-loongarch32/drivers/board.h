/*
 * Copyright (c) 2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-13     Feiyang Chen the first version
 */

#ifndef BOARD_H__
#define BOARD_H__

#include "addr.h"

#if defined(SOC_LA464)
#define RT_HW_HEAP_SIZE (64 * 1024 * 1024)
#elif defined(SOC_LA132)
#define RT_HW_HEAP_SIZE (4 * 1024 * 1024)
#else
#error "Unsupported SOC series"
#endif

extern unsigned char __bss_end;

#define RT_HW_HEAP_BEGIN (&__bss_end)
#define RT_HW_HEAP_END   (&__bss_end + RT_HW_HEAP_SIZE)

void rt_hw_board_init(void);

#endif /* BOARD_H__ */
