#ifndef ADDR_H__
#define ADDR_H__

#include <rtconfig.h>

#include "loongarch.h"

#ifdef ARCH_CPU_64BIT
#define DMW_PABITS    48
#else
#define DMW_PABITS    32
#endif

#define TO_PHYS_MASK  ((ULL(1) << DMW_PABITS) - 1)

/* Direct Map window 0 */
#define CSR_DMW0_PLV0  (ULL(1) << 0)
#define CSR_DMW0_VSEG  ULL(0x8000)
#define CSR_DMW0_BASE  (CSR_DMW0_VSEG << DMW_PABITS)
#define CSR_DMW0_INIT  (CSR_DMW0_BASE | CSR_DMW0_PLV0)

/* Direct Map window 1 */
#define CSR_DMW1_PLV0  ULL(1 << 0)
#define CSR_DMW1_MAT   ULL(1 << 4)
#define CSR_DMW1_VSEG  ULL(0x9000)
#define CSR_DMW1_BASE  (CSR_DMW1_VSEG << DMW_PABITS)
#define CSR_DMW1_INIT  (CSR_DMW1_BASE | CSR_DMW1_MAT | CSR_DMW1_PLV0)

#define UNCACHE_BASE   CSR_DMW0_BASE
#define CACHE_BASE     CSR_DMW1_BASE

#define TO_PHYS(x)     (((x) & TO_PHYS_MASK))
#define TO_CACHE(x)    (CACHE_BASE | ((x) & TO_PHYS_MASK))
#define TO_UNCACHE(x)  (UNCACHE_BASE | ((x) & TO_PHYS_MASK))

#endif /* ADDR_H__ */
