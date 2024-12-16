#ifndef STACK_H__
#define STACK_H__

#include <rtthread.h>

struct rt_hw_stack_frame
{
    rt_base_t zero;
    rt_base_t ra;
    rt_base_t tp;
    rt_base_t sp;
    rt_base_t a0;
    rt_base_t a1;
    rt_base_t a2;
    rt_base_t a3;
    rt_base_t a4;
    rt_base_t a5;
    rt_base_t a6;
    rt_base_t a7;
    rt_base_t t0;
    rt_base_t t1;
    rt_base_t t2;
    rt_base_t t3;
    rt_base_t t4;
    rt_base_t t5;
    rt_base_t t6;
    rt_base_t t7;
    rt_base_t t8;
    rt_base_t r21;
    rt_base_t fp;
    rt_base_t s0;
    rt_base_t s1;
    rt_base_t s2;
    rt_base_t s3;
    rt_base_t s4;
    rt_base_t s5;
    rt_base_t s6;
    rt_base_t s7;
    rt_base_t s8;
    rt_base_t csr_era;
    rt_base_t csr_prmd;
};

#endif /* STACK_H__ */
