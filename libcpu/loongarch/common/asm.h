/*
 * Copyright (c) 2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-13     Feiyang Chen the first version
 */

#ifndef ASM_H__
#define ASM_H__

#include <rtconfig.h>

#ifdef ARCH_CPU_64BIT

#define SIZE  8

#define ADD   add.d
#define ADDI  addi.d
#define SUB   sub.d
#define LD    ld.d
#define ST    st.d
#define SLLI  slli.d
#define SLL   sll.d
#define SRLI  srli.d
#define SRL   srl.d
#define SRAI  srai.d
#define SRA   sra.d

#else

#define SIZE  4

#define ADD   add.w
#define ADDI  addi.w
#define SUB   sub.w
#define LD    ld.w
#define ST    st.w
#define SLLI  slli.w
#define SLL   sll.w
#define SRLI  srli.w
#define SRL   srl.w
#define SRAI  srai.w
#define SRA   sra.w

#endif

#endif /* ASM_H__ */
