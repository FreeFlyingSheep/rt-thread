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

#include "addr.h"
#include "loongarch.h"
#include "interrupt.h"

extern rt_ubase_t __eentry_start;
struct rt_irq_desc irq_handle_table[RT_MAX_EXC + RT_MAX_INTR];

rt_ubase_t rt_thread_switch_interrupt_flag;
rt_ubase_t rt_interrupt_from_thread;
rt_ubase_t rt_interrupt_to_thread;

void tlb_refill_handler(void)
{
    rt_kprintf("TLB Refill Exception!\n");
    RT_ASSERT(0);
}

void machine_error_handler(void)
{
    rt_kprintf("Machine Error Exception!\n");
    RT_ASSERT(0);
}

static void unhandled_interrupt_handler(int vector, void *param)
{
    rt_uint32_t estat, exccode, excsubcode;

    estat = __csrrd_w(LOONGARCH_CSR_ESTAT);
    exccode = (estat & CSR_ESTAT_EXC) >> CSR_ESTAT_EXC_SHIFT;
    excsubcode = (estat & CSR_ESTAT_ESUBCODE) >> CSR_ESTAT_ESUBCODE_SHIFT;
    if (exccode == 0)
    {
        rt_kprintf("Unhandled interrupt 0x%x occured!\n", VECTOR_TO_IRQ(vector));
    }
    else
    {
        rt_kprintf("Unhandled exception 0x%x (0x%x) occured!\n", exccode, excsubcode);
        rt_kprintf("Insn: %08x\n", __csrrd_w(LOONGARCH_CSR_BADI));
    }

    RT_ASSERT(0);
}

void rt_hw_interrupt_init(void)
{
    int i;
    rt_base_t eentry = (rt_base_t)&__eentry_start;
    rt_base_t tlbrentry = eentry + 0x1000;
    rt_base_t uncached_eentry = TO_UNCACHE(eentry + 0x2000);

    __csrwr_w(0x0, LOONGARCH_CSR_ECFG);
#ifdef ARCH_CPU_64BIT
    __csrwr_d(eentry, LOONGARCH_CSR_EENTRY);
    __csrwr_d(tlbrentry, LOONGARCH_CSR_MERRENTRY);
    __csrwr_d(uncached_eentry, LOONGARCH_CSR_TLBRENTRY);
#else
    __csrwr_w(eentry, LOONGARCH_CSR_EENTRY);
    __csrwr_w(tlbrentry, LOONGARCH_CSR_MERRENTRY);
    __csrwr_w(uncached_eentry, LOONGARCH_CSR_TLBRENTRY);
#endif

    for (i = 0; i < RT_MAX_EXC + RT_MAX_INTR; i++)
    {
        irq_handle_table[i].handler = unhandled_interrupt_handler;
        irq_handle_table[i].param = RT_NULL;
    }
}

rt_isr_handler_t rt_hw_interrupt_install(int vector, rt_isr_handler_t handler,
                                         void *param, const char *name)
{
    rt_isr_handler_t old_handler = RT_NULL;

    RT_ASSERT(vector >= 0 && vector < RT_MAX_EXC + RT_MAX_INTR);
    old_handler = irq_handle_table[vector].handler;

#ifdef RT_USING_INTERRUPT_INFO
    rt_strncpy(irq_handle_table[vector].name, name, RT_NAME_MAX);
#endif /* RT_USING_INTERRUPT_INFO */

    irq_handle_table[vector].handler = handler;
    irq_handle_table[vector].param = param;

    return old_handler;
}

static void handle_interrupt(rt_base_t exccode)
{
    void *param;
    rt_isr_handler_t irq_func;

    RT_ASSERT(exccode < RT_MAX_EXC + RT_MAX_INTR);
    irq_func = irq_handle_table[exccode].handler;
    param = irq_handle_table[exccode].param;
    irq_func(exccode, param);
}

void interrupt_dispatch(void)
{
    rt_uint32_t estat, exccode, mask, pending;

    estat = __csrrd_w(LOONGARCH_CSR_ESTAT);
    exccode = (estat & CSR_ESTAT_EXC) >> CSR_ESTAT_EXC_SHIFT;
    if (exccode == 0)
    {
        mask = __csrrd_w(LOONGARCH_CSR_ECFG) & CSR_ECFG_IM;
        pending = estat & CSR_ESTAT_IS & mask;
        if (pending & ESTAT_IS_IPI)
            handle_interrupt(IRQ_TO_VECTOR(ESTAT_IS_IPI_SHIFT));
        if (pending & ESTAT_IS_TIMER)
            handle_interrupt(IRQ_TO_VECTOR(ESTAT_IS_TIMER_SHIFT));
        if (pending & ESTAT_IS_PMC)
            handle_interrupt(IRQ_TO_VECTOR(ESTAT_IS_PMC_SHIFT));
        if (pending & ESTAT_IS_IP7)
            handle_interrupt(IRQ_TO_VECTOR(ESTAT_IS_IP7_SHIFT));
        if (pending & ESTAT_IS_IP6)
            handle_interrupt(IRQ_TO_VECTOR(ESTAT_IS_IP6_SHIFT));
        if (pending & ESTAT_IS_IP5)
            handle_interrupt(IRQ_TO_VECTOR(ESTAT_IS_IP5_SHIFT));
        if (pending & ESTAT_IS_IP4)
            handle_interrupt(IRQ_TO_VECTOR(ESTAT_IS_IP4_SHIFT));
        if (pending & ESTAT_IS_IP3)
            handle_interrupt(IRQ_TO_VECTOR(ESTAT_IS_IP3_SHIFT));
        if (pending & ESTAT_IS_IP2)
            handle_interrupt(IRQ_TO_VECTOR(ESTAT_IS_IP2_SHIFT));
        if (pending & ESTAT_IS_IP1)
            handle_interrupt(IRQ_TO_VECTOR(ESTAT_IS_IP1_SHIFT));
        if (pending & ESTAT_IS_IP0)
            handle_interrupt(IRQ_TO_VECTOR(ESTAT_IS_IP0_SHIFT));
        if (pending & ESTAT_IS_SIP1)
            handle_interrupt(IRQ_TO_VECTOR(ESTAT_IS_SIP1_SHIFT));
        if (pending & ESTAT_IS_SIP0)
            handle_interrupt(IRQ_TO_VECTOR(ESTAT_IS_SIP0_SHIFT));
    }
    else
    {
        handle_interrupt(exccode);
    }
}

rt_base_t rt_hw_interrupt_disable(void)
{
    return __csrxchg_w(0, CSR_CRMD_IE, LOONGARCH_CSR_CRMD);
}

void rt_hw_interrupt_enable(rt_base_t level)
{
    __csrwr_w(level, LOONGARCH_CSR_CRMD);
}

void rt_hw_interrupt_mask(int irq)
{
    RT_ASSERT(irq >= 0 && irq < RT_MAX_INTR);
    __csrxchg_w(0 << irq, 1 << irq, LOONGARCH_CSR_ECFG);
}

void rt_hw_interrupt_umask(int irq)
{
    RT_ASSERT(irq >= 0 && irq < RT_MAX_INTR);
    __csrxchg_w(1 << irq, 1 << irq, LOONGARCH_CSR_ECFG);
}
