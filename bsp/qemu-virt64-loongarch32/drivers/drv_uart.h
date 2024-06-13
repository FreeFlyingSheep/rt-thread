/*
 * Copyright (c) 2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-13     Feiyang Chen the first version
 */

#ifndef DRV_UART_H__
#define DRV_UART_H__

#include "addr.h"

#define LOONGSON3_UART0 TO_UNCACHE(0x1fe001e0)
#define LOONGSON3_UART1 TO_UNCACHE(0x01f001e8)

int rt_hw_uart_init(void);

#endif /* DRV_UART_H__ */
