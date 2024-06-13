/*
 * Copyright (c) 2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-13     Feiyang Chen the first version
 */

#ifndef INTERRUPT_H__
#define INTERRUPT_H__

#include <rthw.h>
#include <rtthread.h>

#define RT_MAX_INTR 13
#define RT_MAX_EXC  64

#define IRQ_TO_VECTOR(x) ((x) + RT_MAX_EXC)
#define VECTOR_TO_IRQ(x) ((x) - RT_MAX_EXC)

void rt_hw_interrupt_mask(int irq);
void rt_hw_interrupt_umask(int irq);
void rt_hw_interrupt_init(void);
rt_isr_handler_t rt_hw_interrupt_install(int vector, rt_isr_handler_t handler,
                                         void *param, const char *name);

#endif /* INTERRUPT_H__ */
