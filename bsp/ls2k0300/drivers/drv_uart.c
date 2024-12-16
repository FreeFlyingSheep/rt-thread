#include <rtdevice.h>
#include <rthw.h>
#include <rtthread.h>

#ifdef BSP_USING_UART

#include "addr.h"
#include "board.h"
#include "drv_uart.h"
#include "interrupt.h"
#include "io_interrupt.h"
#include "loongarch.h"
#include "ls2k0300.h"

#define UART_DAT(base) HWREG8(base + 0x00)
#define UART_IER(base) HWREG8(base + 0x01)
# define IER_IRXE      BIT(0)
# define IER_ILE       BIT(2)
#define UART_IIR(base) HWREG8(base + 0x02)
# define IIR_RXRDY     BIT(2)
# define IIR_RXTOUT    (BIT(3) | BIT(2))
#define UART_FCR(base) HWREG8(base + 0x02)
#define UART_LCR(base) HWREG8(base + 0x03)
#define UART_MCR(base) HWREG8(base + 0x04)
#define UART_LSR(base) HWREG8(base + 0x05)
# define LSR_DR        BIT(0)
# define LSR_TFE       BIT(5)
# define LSR_TE        BIT(6)
#define UART_MSR(base) HWREG8(base + 0x06)

struct rt_uart_device
{
    const char name[RT_NAME_MAX];
    rt_ubase_t base;
    rt_uint32_t irq;
};

static rt_err_t uart_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
    struct rt_uart_device *uart = RT_NULL;

    RT_ASSERT(serial != RT_NULL);
    RT_ASSERT(cfg != RT_NULL);

    uart = (struct rt_uart_device *)serial->parent.user_data;

    UART_IER(uart->base) = 0x00;
    UART_FCR(uart->base) = 0xc1;
    UART_LCR(uart->base) = 0x03;
    UART_MCR(uart->base) = 0x03;
    UART_LSR(uart->base) = 0x60;
    UART_MSR(uart->base) = 0xb0;

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
        rt_hw_io_interrupt_mask(uart->irq);
        break;

    case RT_DEVICE_CTRL_SET_INT:
        UART_IER(uart->base) |= (IER_IRXE | IER_ILE);
        rt_hw_io_interrupt_umask(uart->irq);
        break;

    default:
        break;
    }

    return RT_EOK;
}

static int uart_putc(struct rt_serial_device *serial, char c)
{
    struct rt_uart_device *uart = RT_NULL;
    rt_uint32_t status;

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

    if (UART_LSR(uart->base) & LSR_DR)
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
    .dma_transmit = RT_NULL,
};

static void rt_hw_uart_handler(int vector, void *param)
{
    struct rt_serial_device *serial = RT_NULL;
    struct rt_uart_device *uart = RT_NULL;
    rt_uint8_t iir;

    rt_interrupt_enter();

    serial = (struct rt_serial_device *)param;
    uart = (struct rt_uart_device *)serial->parent.user_data;
    iir = UART_IIR(uart->base);
    if ((iir & IIR_RXRDY) || (iir & IIR_RXTOUT))
    {
        rt_hw_serial_isr(serial, RT_SERIAL_EVENT_RX_IND);
    }

    rt_interrupt_leave();
}

#ifdef BSP_USING_UART0
static struct rt_uart_device uart0_device =
{
    .name = "uart0",
    .base = TO_UNCACHE(LS2K0300_UART0),
    .irq = LS2K0300_IO_INT_UART00,
};
static struct rt_serial_device serial0;
#endif /* BSP_USING_UART0 */

#ifdef BSP_USING_UART1
static struct rt_uart_device uart1_device =
{
    .name = "uart1",
    .base = TO_UNCACHE(LS2K0300_UART1),
    .irq = LS2K0300_IO_INT_UART01,
};
static struct rt_serial_device serial1;
#endif /* BSP_USING_UART1 */

#ifdef BSP_USING_UART2
static struct rt_uart_device uart2_device =
{
    .name = "uart2",
    .base = TO_UNCACHE(LS2K0300_UART2),
    .irq = LS2K0300_IO_INT_UART_02_05,
};
static struct rt_serial_device serial2;
#endif /* BSP_USING_UART2 */

#ifdef BSP_USING_UART3
static struct rt_uart_device uart3_device =
{
    .name = "uart3",
    .base = TO_UNCACHE(LS2K0300_UART3),
    .irq = LS2K0300_IO_INT_UART_02_05,
};
static struct rt_serial_device serial3;
#endif /* BSP_USING_UART3 */

#ifdef BSP_USING_UART4
static struct rt_uart_device uart4_device =
{
    .name = "uart4",
    .base = TO_UNCACHE(LS2K0300_UART4),
    .irq = LS2K0300_IO_INT_UART_02_05,
};
static struct rt_serial_device serial4;
#endif /* BSP_USING_UART4 */

#ifdef BSP_USING_UART5
static struct rt_uart_device uart5_device =
{
    .name = "uart5",
    .base = TO_UNCACHE(LS2K0300_UART5),
    .irq = LS2K0300_IO_INT_UART_02_05,
};
static struct rt_serial_device serial5;
#endif /* BSP_USING_UART5 */

#ifdef BSP_USING_UART6
static struct rt_uart_device uart6_device =
{
    .name = "uart6",
    .base = TO_UNCACHE(LS2K0300_UART6),
    .irq = LS2K0300_IO_INT_UART_06_09,
};
static struct rt_serial_device serial6;
#endif /* BSP_USING_UART6 */

#ifdef BSP_USING_UART7
static struct rt_uart_device uart7_device =
{
    .name = "uart7",
    .base = TO_UNCACHE(LS2K0300_UART7),
    .irq = LS2K0300_IO_INT_UART_06_09,
};
static struct rt_serial_device serial7;
#endif /* BSP_USING_UART7 */

#ifdef BSP_USING_UART8
static struct rt_uart_device uart8_device =
{
    .name = "uart8",
    .base = TO_UNCACHE(LS2K0300_UART8),
    .irq = LS2K0300_IO_INT_UART_06_09,
};
static struct rt_serial_device serial8;
#endif /* BSP_USING_UART8 */

#ifdef BSP_USING_UART9
static struct rt_uart_device uart9_device =
{
    .name = "uart9",
    .base = TO_UNCACHE(LS2K0300_UART9),
    .irq = LS2K0300_IO_INT_UART_06_09,
};
static struct rt_serial_device serial9;
#endif /* BSP_USING_UART9 */

static rt_err_t uart_init(struct rt_serial_device *serial, struct rt_uart_device *uart)
{
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
    rt_err_t ret;

    RT_ASSERT(serial != RT_NULL);
    RT_ASSERT(uart != RT_NULL);

    serial->ops = &uart_ops;
    serial->config = config;

    ret = rt_hw_serial_register(serial, uart->name,
                                RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                                uart);

    if (ret != RT_EOK)
    {
        return ret;
    }

    rt_hw_io_interrupt_install(uart->irq, rt_hw_uart_handler, (void *)serial, uart->name);
    rt_hw_io_interrupt_umask(uart->irq);

    return ret;
}

int rt_hw_uart_init(void)
{
#ifdef BSP_USING_UART0
    if (uart_init(&serial0, &uart0_device) != RT_EOK)
    {
        return -1;
    }
#endif /* BSP_USING_UART0 */

#ifdef BSP_USING_UART1
    if (uart_init(&serial1, &uart1_device) != RT_EOK)
    {
        return -1;
    }
#endif /* BSP_USING_UART1 */

#ifdef BSP_USING_UART2
    if (uart_init(&serial2, &uart2_device) != RT_EOK)
    {
        return -1;
    }
#endif /* BSP_USING_UART2 */

#ifdef BSP_USING_UART3
    if (uart_init(&serial3, &uart3_device) != RT_EOK)
    {
        return -1;
    }
#endif /* BSP_USING_UART3 */

#ifdef BSP_USING_UART4
    if (uart_init(&serial4, &uart4_device) != RT_EOK)
    {
        return -1;
    }
#endif /* BSP_USING_UART4 */

#ifdef BSP_USING_UART5
    if (uart_init(&serial5, &uart5_device) != RT_EOK)
    {
        return -1;
    }
#endif /* BSP_USING_UART5 */

#ifdef BSP_USING_UART6
    if (uart_init(&serial6, &uart6_device) != RT_EOK)
    {
        return -1;
    }
#endif /* BSP_USING_UART6 */

#ifdef BSP_USING_UART7
    if (uart_init(&serial7, &uart7_device) != RT_EOK)
    {
        return -1;
    }
#endif /* BSP_USING_UART7 */

#ifdef BSP_USING_UART8
    if (uart_init(&serial8, &uart8_device) != RT_EOK)
    {
        return -1;
    }
#endif /* BSP_USING_UART8 */

#ifdef BSP_USING_UART9
    if (uart_init(&serial9, &uart9_device) != RT_EOK)
    {
        return -1;
    }
#endif /* BSP_USING_UART9 */

    return 0;
}
INIT_BOARD_EXPORT(rt_hw_uart_init);

#endif /* BSP_USING_UART */
