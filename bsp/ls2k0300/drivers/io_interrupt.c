#include <rthw.h>
#include <rtthread.h>

#include "addr.h"
#include "board.h"
#include "interrupt.h"
#include "io_interrupt.h"
#include "ls2k0300.h"

static struct rt_irq_desc io_irq_desc[LS2K0300_IO_INT_NUM];

#ifdef RT_USING_INTERRUPT_INFO
static const char *io_irq_name[LS2K0300_IO_INT_NUM] =
{
    "UART00",
    "UART01",
    "UART_02_05",
    "UART_06_09",
    "I2C_00_01",
    "I2C_02_03",
    "SPI2",
    "SPI3",
    "CAN0",
    "CAN1",
    "CAN2",
    "CAN3",
    "I2S",
    "ATIMER",
    "GTIMER",
    "BTIMER",
    "PWM_0_1",
    "PWM_2_3",
    "ADC",
    "HPET0",
    "HPET1",
    "HPET2",
    "HPET3",
    "APB_DMA0",
    "APB_DMA1",
    "APB_DMA2",
    "APB_DMA3",
    "APB_DMA4",
    "APB_DMA5",
    "APB_DMA6",
    "APB_DMA7",
    "SDIO0_CTRL",
    "SDIO1_CTRL",
    "SDIO0_DMA",
    "SDIO1_DMA",
    "ENCRYPT_DMA",
    "AES",
    "DES",
    "SM3",
    "SM4",
    "RTC",
    "TOY",
    "RTC_TICK",
    "TOY_TICK",
    "SPI0",
    "SPI1",
    "EHCI",
    "OHCI",
    "OTG",
    "GMAC0",
    "GMAC1",
    "DC",
    "THSENS",
    "GPIO_0_15",
    "GPIO_16_31",
    "GPIO_32_47",
    "GPIO_48_63",
    "GPIO_64_79",
    "GPIO_80_95",
    "GPIO_96_105",
    "RESERVED",
    "DDR_ECC0",
    "DDR_ECC1",
    "RESERVED",
};
#endif /* RT_USING_INTERRUPT_INFO */

void rt_hw_io_interrupt_mask(int vector)
{
    RT_ASSERT(vector >= 0 && vector < LS2K0300_IO_INT_NUM);
    if (vector < LS2K0300_IO_INT_LOW_NUM)
        HWREG32(TO_UNCACHE(LS2K0300_INTCLR_0)) |= 1 << vector;
    else
        HWREG32(TO_UNCACHE(LS2K0300_INTCLR_1)) |= 1 << (vector - LS2K0300_IO_INT_LOW_NUM);
}

void rt_hw_io_interrupt_umask(int vector)
{
    RT_ASSERT(vector >= 0 && vector < LS2K0300_IO_INT_NUM);
    if (vector < LS2K0300_IO_INT_LOW_NUM)
        HWREG32(TO_UNCACHE(LS2K0300_INTSET_0)) |= 1 << vector;
    else
        HWREG32(TO_UNCACHE(LS2K0300_INTSET_1)) |= 1 << (vector - LS2K0300_IO_INT_LOW_NUM);
}

rt_isr_handler_t rt_hw_io_interrupt_install(int vector, rt_isr_handler_t handler,
                                            void *param, const char *name)
{
    rt_isr_handler_t old = RT_NULL;

    RT_ASSERT(vector >= 0 && vector < LS2K0300_IO_INT_NUM);
    old = io_irq_desc[vector].handler;

#ifdef RT_USING_INTERRUPT_INFO
    rt_strncpy(io_irq_desc[vector].name, name, RT_NAME_MAX);
#endif

    io_irq_desc[vector].handler = handler;
    io_irq_desc[vector].param = param;

    return old;
}

static void route(int vector, int ip)
{
    RT_ASSERT(vector >= 0 && vector < LS2K0300_IO_INT_NUM);
    RT_ASSERT(ip == LS2K0300_IP0 || ip == LS2K0300_IP1 || ip == LS2K0300_IP2 || ip == LS2K0300_IP3);

    if (vector < LS2K0300_IO_ENTRY_NUM * 1)
        HWREG32(TO_UNCACHE(LS2K0300_ENTRY0_0)) = ip;
    else if (vector < LS2K0300_IO_ENTRY_NUM * 2)
        HWREG32(TO_UNCACHE(LS2K0300_ENTRY8_0)) = ip;
    else if (vector < LS2K0300_IO_ENTRY_NUM * 3)
        HWREG32(TO_UNCACHE(LS2K0300_ENTRY16_0)) = ip;
    else if (vector < LS2K0300_IO_ENTRY_NUM * 4)
        HWREG32(TO_UNCACHE(LS2K0300_ENTRY24_0)) = ip;
    else if (vector < LS2K0300_IO_ENTRY_NUM * 5)
        HWREG32(TO_UNCACHE(LS2K0300_ENTRY0_1)) = ip;
    else if (vector < LS2K0300_IO_ENTRY_NUM * 6)
        HWREG32(TO_UNCACHE(LS2K0300_ENTRY8_1)) = ip;
    else if (vector < LS2K0300_IO_ENTRY_NUM * 7)
        HWREG32(TO_UNCACHE(LS2K0300_ENTRY16_1)) = ip;
    else
        HWREG32(TO_UNCACHE(LS2K0300_ENTRY24_1)) = ip;
}

static void handle_io_interrupt(int vector)
{
    rt_isr_handler_t handler;
    void *param;

    handler = io_irq_desc[vector].handler;
    param = io_irq_desc[vector].param;

    if (handler != RT_NULL)
    {
        handler(vector, param);
    }
    else
    {
        rt_interrupt_enter();

#ifdef RT_USING_INTERRUPT_INFO
        rt_kprintf("Unhandled io interrupt %d: %s\n", vector, io_irq_name[vector]);
#else
        rt_kprintf("Unhandled io interrupt %d\n", vector);
#endif

        /* clear and mask interrupt */
        rt_hw_io_interrupt_mask(vector);

        rt_interrupt_leave();
    }
}

static void io_interrupt_handler(int vector, void *param)
{
    rt_uint32_t ien0, isr0, ien1, isr1;
    int i;

    ien0 = HWREG32(TO_UNCACHE(LS2K0300_INTIEN_0));
    isr0 = HWREG32(TO_UNCACHE(LS2K0300_INTISR_0));
    ien1 = HWREG32(TO_UNCACHE(LS2K0300_INTIEN_1));
    isr1 = HWREG32(TO_UNCACHE(LS2K0300_INTISR_1));

    for (i = 0; i < LS2K0300_IO_INT_LOW_NUM; i++)
    {
        if ((isr0 & (1 << i)) && (ien0 & (1 << i)))
        {
            handle_io_interrupt(i);
        }
    }
    for (i = 0; i < LS2K0300_IO_INT_HIGH_NUM; i++)
    {
        if ((isr1 & (1 << i)) && (ien1 & (1 << i)))
        {
            handle_io_interrupt(i + LS2K0300_IO_INT_LOW_NUM);
        }
    }
}

int rt_hw_io_interrupt_init(void)
{
    int i;

    /* disable extended io interrupt */
    HWREG32(TO_UNCACHE(LS2K0300_CHIP_CTRL00)) &= ~CTRL00_EXTIOINT_EN;

    /* clear interrupt status */
    HWREG32(TO_UNCACHE(LS2K0300_INTCLR_0)) = ~0;
    /* low level trigger */
    HWREG32(TO_UNCACHE(LS2K0300_INTEDGE_0)) = 0;
    HWREG32(TO_UNCACHE(LS2K0300_INTPOL_0)) = 0;

    /* clear interrupt status */
    HWREG32(TO_UNCACHE(LS2K0300_INTCLR_1)) = ~0;
    /* low level trigger */
    HWREG32(TO_UNCACHE(LS2K0300_INTEDGE_1)) = 0;
    HWREG32(TO_UNCACHE(LS2K0300_INTPOL_1)) = 0;

    for (i = 0; i < LS2K0300_IO_INT_NUM; i++)
    {
        /* route all interrupts to IP0 (HWI0) */
        route(i, LS2K0300_IP0);
    }

    rt_hw_interrupt_install(LOONGARCH_INT_HWI0, io_interrupt_handler, RT_NULL, "io");
    rt_hw_interrupt_umask(LOONGARCH_INT_HWI0);

    return 0;
}
INIT_BOARD_EXPORT(rt_hw_io_interrupt_init);
