#include <rtthread.h>

#include "asm.h"
#include "loongarch.h"
#include "stack.h"

rt_uint8_t *rt_hw_stack_init(void *tentry, void *parameter, rt_uint8_t *stack_addr, void *texit)
{
    struct rt_hw_stack_frame *frame;
    rt_uint8_t *stk;

    /* stack align */
    stk = stack_addr + sizeof(rt_ubase_t);
    stk = (rt_uint8_t *)RT_ALIGN_DOWN((rt_ubase_t)stk, SIZE * 2);
    stk -= sizeof(struct rt_hw_stack_frame);

    frame = (struct rt_hw_stack_frame *)stk;
    frame->zero = 0;
    frame->ra = (rt_ubase_t)texit;
    frame->tp = 0;
    frame->sp = (rt_ubase_t)stk;
    frame->a0 = (rt_ubase_t)parameter;
    frame->a1 = 0;
    frame->a2 = 0;
    frame->a3 = 0;
    frame->a4 = 0;
    frame->a5 = 0;
    frame->a6 = 0;
    frame->a7 = 0;
    frame->t0 = 0;
    frame->t1 = 0;
    frame->t2 = 0;
    frame->t3 = 0;
    frame->t4 = 0;
    frame->t5 = 0;
    frame->t6 = 0;
    frame->t7 = 0;
    frame->t8 = 0;
    frame->s0 = 0;
    frame->s1 = 0;
    frame->s2 = 0;
    frame->s3 = 0;
    frame->s4 = 0;
    frame->s5 = 0;
    frame->s6 = 0;
    frame->s7 = 0;
    frame->s8 = 0;

    frame->csr_era = (rt_ubase_t)tentry;

    /* enable interrupt */
    frame->csr_prmd = __csrrd_w(LOONGARCH_CSR_PRMD) | CSR_PRMD_PIE;

    return stk;
}
