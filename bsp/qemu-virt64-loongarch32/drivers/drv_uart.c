/*
 * Copyright (c) 2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-13     Feiyang Chen the first version
 */

#include <rtdevice.h>
#include <rthw.h>
#include <rtthread.h>

#ifdef BSP_USING_UART

#include "board.h"
#include "drv_uart.h"
#include "interrupt.h"
#include "loongarch.h"

#define UART_DAT(base)  HWREG8(base + 0x00)
#define UART_IER(base)  HWREG8(base + 0x01)
#define UART_LSR(base)  HWREG8(base + 0x05)

#define IER_IRxE        BIT(1)
#define IER_ILE         BIT(4)

#define LSR_DR          BIT(0)
#define LSR_RXRDY       BIT(1)
#define LSR_TE          BIT(6)
#define LSR_TFE         BIT(5)

struct rt_uart_device
{
    const char name[RT_NAME_MAX];
    rt_ubase_t base;
    rt_uint32_t irq;
};

static rt_err_t uart_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
    return RT_EOK;
}

static rt_err_t uart_control(struct rt_serial_device *serial, int cmd, void *arg)
{
    struct rt_uart_device *uart = RT_NULL;

    RT_ASSERT(serial != RT_NULL);

    uart = (struct rt_uart_device *)serial->parent.user_data;

    switch (cmd)
    {
    case RT_DEVICE_CTRL_CLR_INT:
        rt_hw_interrupt_mask(uart->irq);
        break;

    case RT_DEVICE_CTRL_SET_INT:
        rt_hw_interrupt_umask(uart->irq);
        UART_IER(uart->base) |= (IER_IRxE | IER_ILE);
        break;

    default:
        break;
    }

    return RT_EOK;
}

static int uart_putc(struct rt_serial_device *serial, char c)
{
    rt_uint32_t status;
    struct rt_uart_device *uart = RT_NULL;

    RT_ASSERT(serial != RT_NULL);

    uart = (struct rt_uart_device *)serial->parent.user_data;

    status = UART_LSR(uart->base);
    while (!(status & (LSR_TE | LSR_TFE)))
    {
        status = UART_LSR(uart->base);
    }

    UART_DAT(uart->base) = c;

    return 1;
}

static int uart_getc(struct rt_serial_device *serial)
{
    struct rt_uart_device *uart = RT_NULL;

    RT_ASSERT(serial != RT_NULL);

    uart = (struct rt_uart_device *)serial->parent.user_data;

    if (LSR_RXRDY & UART_LSR(uart->base))
    {
        return UART_DAT(uart->base);
    }

    return -1;
}

static const struct rt_uart_ops uart_ops =
{
    .configure    = uart_configure,
    .control      = uart_control,
    .putc         = uart_putc,
    .getc         = uart_getc,
    .dma_transmit = RT_NULL
};

#ifdef BSP_USING_UART0
static struct rt_uart_device uart0_device =
{
    .name = "uart0",
    .base = LOONGSON3_UART0
};
static struct rt_serial_device serial0;
#endif /* BSP_USING_UART0 */

#ifdef BSP_USING_UART1
static struct rt_uart_device uart1_device =
{
    .name = "uart1",
    .base = LOONGSON3_UART1
};
static struct rt_serial_device serial1;
#endif /* BSP_USING_UART1 */

int rt_hw_uart_init(void)
{
    rt_err_t ret;
    struct rt_uart_device *uart;
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;

#ifdef BSP_USING_UART0
    uart = &uart0_device;

    serial0.ops = &uart_ops;
    serial0.config = config;

    ret = rt_hw_serial_register(&serial0, uart->name,
                                RT_DEVICE_FLAG_WRONLY, uart);
    if (ret != RT_EOK)
    {
        return ret;
    }
#endif /* BSP_USING_UART0 */

#ifdef BSP_USING_UART1
    uart = &uart1_device;

    serial1.ops = &uart_ops;
    serial1.config = config;

    ret = rt_hw_serial_register(&serial1, uart->name,
                                RT_DEVICE_FLAG_WRONLY, uart);
    if (ret != RT_EOK)
    {
        return ret;
    }
#endif /* BSP_USING_UART1 */

    return 0;
}
INIT_BOARD_EXPORT(rt_hw_uart_init);

#endif /* BSP_USING_UART */
