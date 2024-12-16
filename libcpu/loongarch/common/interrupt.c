#include <rthw.h>
#include <rtthread.h>

#include "exception.h"
#include "interrupt.h"
#include "loongarch.h"

extern rt_uint8_t trap_entry;

rt_ubase_t rt_thread_switch_interrupt_flag;
rt_ubase_t rt_interrupt_from_thread;
rt_ubase_t rt_interrupt_to_thread;

#ifdef RT_USING_INTERRUPT_INFO
static const char *irq_name[LOONGARCH_INT_NUM] =
{
    "SWI0",
    "SWI1",
    "HWI0",
    "HWI1",
    "HWI2",
    "HWI3",
    "HWI4",
    "HWI5",
    "HWI6",
    "HWI7",
    "PMI",
    "TI",
    "IPI",
};
#endif /* RT_USING_INTERRUPT_INFO */

static struct rt_irq_desc irq_desc[LOONGARCH_INT_NUM];

rt_base_t rt_hw_interrupt_disable(void)
{
    return __csrxchg_w(0, CSR_CRMD_IE, LOONGARCH_CSR_CRMD);
}

void rt_hw_interrupt_enable(rt_base_t level)
{
    __csrwr_w(level, LOONGARCH_CSR_CRMD);
}

void rt_hw_interrupt_mask(int vector)
{
    RT_ASSERT(vector >= 0 && vector < LOONGARCH_INT_NUM);
    __csrxchg_w(0 << vector, 1 << vector, LOONGARCH_CSR_ECFG);
}

void rt_hw_interrupt_umask(int vector)
{
    RT_ASSERT(vector >= 0 && vector < LOONGARCH_INT_NUM);
    __csrxchg_w(1 << vector, 1 << vector, LOONGARCH_CSR_ECFG);
}

rt_isr_handler_t rt_hw_interrupt_install(int vector, rt_isr_handler_t handler,
                                         void *param, const char *name)
{
    rt_isr_handler_t old = RT_NULL;

    RT_ASSERT(vector >= 0 && vector < LOONGARCH_INT_NUM);
    old = irq_desc[vector].handler;

#ifdef RT_USING_INTERRUPT_INFO
    rt_strncpy(irq_desc[vector].name, name, RT_NAME_MAX);
#endif

    irq_desc[vector].handler = handler;
    irq_desc[vector].param = param;

    return old;
}

void rt_hw_interrupt_init(void)
{
    rt_hw_exception_init();

    __csrwr_w(0, LOONGARCH_CSR_ECFG);
#ifdef ARCH_CPU_64BIT
    __csrwr_d((rt_ubase_t)&trap_entry, LOONGARCH_CSR_EENTRY);
#else
    __csrwr_w((rt_ubase_t)&trap_entry, LOONGARCH_CSR_EENTRY);
#endif
}

static void handle_interrupt(int vector)
{
    rt_isr_handler_t handler;
    void *param;

    handler = irq_desc[vector].handler;
    param = irq_desc[vector].param;

    if (handler != RT_NULL)
    {
        handler(vector, param);
    }
    else
    {
        rt_interrupt_enter();

#ifdef RT_USING_INTERRUPT_INFO
        rt_kprintf("Unhandled interrupt %d: %s\n", vector, irq_name[vector]);
#else
        rt_kprintf("Unhandled interrupt %d\n", vector);
#endif

        /* mask interrupt */
        rt_hw_interrupt_mask(vector);

        rt_interrupt_leave();
    }
}

void handle_trap(void)
{
    rt_uint32_t estat, ecode, esubcode, mask, pending;

    estat = __csrrd_w(LOONGARCH_CSR_ESTAT);
    ecode = (estat & CSR_ESTAT_EXC) >> CSR_ESTAT_EXC_SHIFT;
    esubcode = (estat & CSR_ESTAT_ESUBCODE) >> CSR_ESTAT_ESUBCODE_SHIFT;

    if (ecode == 0)
    {
        mask = __csrrd_w(LOONGARCH_CSR_ECFG) & CSR_ECFG_LIE;
        pending = estat & CSR_ESTAT_IS & mask;
        if (pending & ESTAT_IS_IPI)
            handle_interrupt(LOONGARCH_INT_IPI);
        if (pending & ESTAT_IS_TIMER)
            handle_interrupt(LOONGARCH_INT_TI);
        if (pending & ESTAT_IS_PMC)
            handle_interrupt(LOONGARCH_INT_PMI);
        if (pending & ESTAT_IS_IP7)
            handle_interrupt(LOONGARCH_INT_HWI7);
        if (pending & ESTAT_IS_IP6)
            handle_interrupt(LOONGARCH_INT_HWI6);
        if (pending & ESTAT_IS_IP5)
            handle_interrupt(LOONGARCH_INT_HWI5);
        if (pending & ESTAT_IS_IP4)
            handle_interrupt(LOONGARCH_INT_HWI4);
        if (pending & ESTAT_IS_IP3)
            handle_interrupt(LOONGARCH_INT_HWI3);
        if (pending & ESTAT_IS_IP2)
            handle_interrupt(LOONGARCH_INT_HWI2);
        if (pending & ESTAT_IS_IP1)
            handle_interrupt(LOONGARCH_INT_HWI1);
        if (pending & ESTAT_IS_IP0)
            handle_interrupt(LOONGARCH_INT_HWI0);
        if (pending & ESTAT_IS_SIP1)
            handle_interrupt(LOONGARCH_INT_SWI1);
        if (pending & ESTAT_IS_SIP0)
            handle_interrupt(LOONGARCH_INT_SWI0);
    }
    else
    {
        handle_exception(ecode, esubcode);
    }
}
