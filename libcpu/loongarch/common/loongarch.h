#ifndef LOONGARCH_H__
#define LOONGARCH_H__

#include <rtconfig.h>

#ifdef __ASSEMBLER__

#define UL(x)  x
#define ULL(x) x

#else

#include <larchintrin.h>

#define UL(x)  x ## UL
#define ULL(x) x ## ULL

#endif /* __ASSEMBLER__ */

#define BIT(n)     (UL(1) << (n))
#define MASK(l, h) (((~UL(0)) - (UL(1) << (l)) + 1) & (~UL(0) >> (64 - 1 - (h))))

/* LoongArch CSR registers */
#define LOONGARCH_CSR_CRMD        0x0
#define  CSR_CRMD_IE              BIT(2)
#define  CSR_CRMD_DA              BIT(3)
#define  CSR_CRMD_PG              BIT(4)
#define LOONGARCH_CSR_PRMD        0x1
#define  CSR_PRMD_PIE             BIT(2)
#define LOONGARCH_CSR_ECFG        0x4
#define  CSR_ECFG_LIE             MASK(0, 12)
#define LOONGARCH_CSR_ESTAT       0x5
#define  CSR_ESTAT_IS             MASK(0, 12)
#define   ESTAT_IS_SIP0           BIT(0)
#define   ESTAT_IS_SIP1           BIT(1)
#define   ESTAT_IS_IP0            BIT(2)
#define   ESTAT_IS_IP1            BIT(3)
#define   ESTAT_IS_IP2            BIT(4)
#define   ESTAT_IS_IP3            BIT(5)
#define   ESTAT_IS_IP4            BIT(6)
#define   ESTAT_IS_IP5            BIT(7)
#define   ESTAT_IS_IP6            BIT(8)
#define   ESTAT_IS_IP7            BIT(9)
#define   ESTAT_IS_PMC            BIT(10)
#define   ESTAT_IS_TIMER          BIT(11)
#define   ESTAT_IS_IPI            BIT(12)
#define  CSR_ESTAT_EXC            MASK(16, 21)
#define  CSR_ESTAT_EXC_SHIFT      16
#define  CSR_ESTAT_ESUBCODE       MASK(22, 30)
#define  CSR_ESTAT_ESUBCODE_SHIFT 22
#define LOONGARCH_CSR_ERA         0x6
#define LOONGARCH_CSR_EENTRY      0xc
#define LOONGARCH_CSR_KS0         0x30
#define LOONGARCH_CSR_KS1         0x31
#define LOONGARCH_CSR_TCFG        0x41
#define  CSR_TCFG_EN              BIT(0)
#define  CSR_TCFG_PERIOD          BIT(1)
#ifdef ARCH_CPU_64BIT
#define  CSR_TCFG_VAL             MASK(2, 47)
#else
#define  CSR_TCFG_VAL             MASK(2, 31)
#endif
#define LOONGARCH_CSR_TINTCLR     0x44
#define  CSR_TINTCLR_TI           BIT(0)
#define LOONGARCH_CSR_DMWIN0      0x180
#define LOONGARCH_CSR_DMWIN1      0x181

/* LoongArch CPUCFG registers */
#define LOONGARCH_CPUCFG4         0x4
#define LOONGARCH_CPUCFG5         0x5
#define  CPUCFG5_CCMUL            MASK(0, 15)
#define  CPUCFG5_CCDIV            MASK(16, 31)
#define  CPUCFG5_CCDIV_SHIFT      16

#endif /* LOONGARCH_H__ */
