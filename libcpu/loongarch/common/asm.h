#ifndef ASM_H__
#define ASM_H__

#include <rtconfig.h>

#ifdef ARCH_CPU_64BIT

#define SIZE  8

#define ADDI  addi.d
#define LD    ld.d
#define LI    li.d
#define ST    st.d

#else

#define SIZE  4

#define ADDI  addi.w
#define LD    ld.w
#define LI    li.w
#define ST    st.w

#endif /* ARCH_CPU_64BIT */

#endif /* ASM_H__ */
