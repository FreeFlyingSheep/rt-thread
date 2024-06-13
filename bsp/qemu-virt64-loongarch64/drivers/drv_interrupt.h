/*
 * Copyright (c) 2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-13     Feiyang Chen the first version
 */

#ifndef DRV_INTERRUPT_H__
#define DRV_INTERRUPT_H__

#include "addr.h"

#define PCH_PIC_REG_BASE        TO_UNCACHE(0x10000000)
#define PCH_PIC_INT_MASK_BASE   (PCH_PIC_REG_BASE + 0x20)
#define EIOINTC_REG_ENABLE_BASE 0x1600

int rt_hw_io_interrupt_init(void);

#endif /* DRV_INTERRUPT_H__ */
