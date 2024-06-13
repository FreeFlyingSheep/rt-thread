/*
 * Copyright (c) 2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-13     Feiyang Chen the first version
 */

#ifndef LOONGARCH_H__
#define LOONGARCH_H__

#include <rtconfig.h>

#ifdef __ASSEMBLY__
#define UL
#else
#include <larchintrin.h> /* for __csr*, __iocsr* and __cpucfg */
#define UL (unsigned long)
#endif

#ifdef ARCH_CPU_64BIT
#define BITS_PER_LONG 64
#else
#define BITS_PER_LONG 32
#endif

#define BIT(nr) (UL(1) << (nr))
#define GENMASK(h, l) \
    (((~UL(0)) - (UL(1) << (l)) + 1) & \
    (~UL(0) >> (BITS_PER_LONG - 1 - (h))))

/* Basic CSR registers */
#define LOONGARCH_CSR_CRMD          0x0 /* Current mode info */
#define  CSR_CRMD_WE_SHIFT          9
#define  CSR_CRMD_WE                (UL(0x1) << CSR_CRMD_WE_SHIFT)
#define  CSR_CRMD_DACM_SHIFT        7
#define  CSR_CRMD_DACM_WIDTH        2
#define  CSR_CRMD_DACM              (UL(0x3) << CSR_CRMD_DACM_SHIFT)
#define  CSR_CRMD_DACF_SHIFT        5
#define  CSR_CRMD_DACF_WIDTH        2
#define  CSR_CRMD_DACF              (UL(0x3) << CSR_CRMD_DACF_SHIFT)
#define  CSR_CRMD_PG_SHIFT          4
#define  CSR_CRMD_PG                (UL(0x1) << CSR_CRMD_PG_SHIFT)
#define  CSR_CRMD_DA_SHIFT          3
#define  CSR_CRMD_DA                (UL(0x1) << CSR_CRMD_DA_SHIFT)
#define  CSR_CRMD_IE_SHIFT          2
#define  CSR_CRMD_IE                (UL(0x1) << CSR_CRMD_IE_SHIFT)
#define  CSR_CRMD_PLV_SHIFT         0
#define  CSR_CRMD_PLV_WIDTH         2
#define  CSR_CRMD_PLV               (UL(0x3) << CSR_CRMD_PLV_SHIFT)

#define LOONGARCH_CSR_PRMD          0x1 /* Prev-exception mode info */
#define  CSR_PRMD_PWE_SHIFT         3
#define  CSR_PRMD_PWE               (UL(0x1) << CSR_PRMD_PWE_SHIFT)
#define  CSR_PRMD_PIE_SHIFT         2
#define  CSR_PRMD_PIE               (UL(0x1) << CSR_PRMD_PIE_SHIFT)
#define  CSR_PRMD_PPLV_SHIFT        0
#define  CSR_PRMD_PPLV_WIDTH        2
#define  CSR_PRMD_PPLV              (UL(0x3) << CSR_PRMD_PPLV_SHIFT)

#define LOONGARCH_CSR_EUEN          0x2 /* Extended unit enable */
#define  CSR_EUEN_LBTEN_SHIFT       3
#define  CSR_EUEN_LBTEN             (UL(0x1) << CSR_EUEN_LBTEN_SHIFT)
#define  CSR_EUEN_LASXEN_SHIFT      2
#define  CSR_EUEN_LASXEN            (UL(0x1) << CSR_EUEN_LASXEN_SHIFT)
#define  CSR_EUEN_LSXEN_SHIFT       1
#define  CSR_EUEN_LSXEN             (UL(0x1) << CSR_EUEN_LSXEN_SHIFT)
#define  CSR_EUEN_FPEN_SHIFT        0
#define  CSR_EUEN_FPEN              (UL(0x1) << CSR_EUEN_FPEN_SHIFT)

#define LOONGARCH_CSR_MISC          0x3 /* Misc config */

#define LOONGARCH_CSR_ECFG          0x4/* Exception config */
#define  CSR_ECFG_VS_SHIFT          16
#define  CSR_ECFG_VS_WIDTH          3
#define  CSR_ECFG_VS                (UL(0x7) << CSR_ECFG_VS_SHIFT)
#define  CSR_ECFG_IM_SHIFT          0
#define  CSR_ECFG_IM_WIDTH          13
#define  CSR_ECFG_IM                (UL(0x1fff) << CSR_ECFG_IM_SHIFT)

#define LOONGARCH_CSR_ESTAT         0x5 /* Exception status */
#define  CSR_ESTAT_ESUBCODE_SHIFT   22
#define  CSR_ESTAT_ESUBCODE_WIDTH   9
#define  CSR_ESTAT_ESUBCODE         (UL(0x1ff) << CSR_ESTAT_ESUBCODE_SHIFT)
#define  CSR_ESTAT_EXC_SHIFT        16
#define  CSR_ESTAT_EXC_WIDTH        6
#define  CSR_ESTAT_EXC              (UL(0x3f) << CSR_ESTAT_EXC_SHIFT)
#define  CSR_ESTAT_IS_SHIFT         0
#define  CSR_ESTAT_IS_WIDTH         15
#define  CSR_ESTAT_IS               (UL(0x7fff) << CSR_ESTAT_IS_SHIFT)

#define LOONGARCH_CSR_ERA           0x6 /* ERA */

#define LOONGARCH_CSR_BADV          0x7 /* Bad virtual address */

#define LOONGARCH_CSR_BADI          0x8 /* Bad instruction */

#define LOONGARCH_CSR_EENTRY        0xc /* Exception entry */

/* TLB related CSR registers */
#define LOONGARCH_CSR_TLBIDX        0x10 /* TLB Index, EHINV, PageSize, NP */
#define  CSR_TLBIDX_EHINV_SHIFT     31
#define  CSR_TLBIDX_EHINV           (UL(1) << CSR_TLBIDX_EHINV_SHIFT)
#define  CSR_TLBIDX_PS_SHIFT        24
#define  CSR_TLBIDX_PS_WIDTH        6
#define  CSR_TLBIDX_PS              (UL(0x3f) << CSR_TLBIDX_PS_SHIFT)
#define  CSR_TLBIDX_IDX_SHIFT       0
#define  CSR_TLBIDX_IDX_WIDTH       12
#define  CSR_TLBIDX_IDX             (UL(0xfff) << CSR_TLBIDX_IDX_SHIFT)
#define  CSR_TLBIDX_SIZEM           0x3f000000
#define  CSR_TLBIDX_SIZE            CSR_TLBIDX_PS_SHIFT
#define  CSR_TLBIDX_IDXM            0xfff
#define  CSR_INVALID_ENTRY(e)       (CSR_TLBIDX_EHINV | e)

#define LOONGARCH_CSR_TLBEHI        0x11 /* TLB EntryHi */

#define LOONGARCH_CSR_TLBELO0       0x12 /* TLB EntryLo0 */
#define  CSR_TLBLO0_RPLV_SHIFT      63
#define  CSR_TLBLO0_RPLV            (UL(0x1) << CSR_TLBLO0_RPLV_SHIFT)
#define  CSR_TLBLO0_NX_SHIFT        62
#define  CSR_TLBLO0_NX              (UL(0x1) << CSR_TLBLO0_NX_SHIFT)
#define  CSR_TLBLO0_NR_SHIFT        61
#define  CSR_TLBLO0_NR              (UL(0x1) << CSR_TLBLO0_NR_SHIFT)
#define  CSR_TLBLO0_PFN_SHIFT       12
#define  CSR_TLBLO0_PFN_WIDTH       36
#define  CSR_TLBLO0_PFN             (UL(0xfffffffff) << CSR_TLBLO0_PFN_SHIFT)
#define  CSR_TLBLO0_GLOBAL_SHIFT    6
#define  CSR_TLBLO0_GLOBAL          (UL(0x1) << CSR_TLBLO0_GLOBAL_SHIFT)
#define  CSR_TLBLO0_CCA_SHIFT       4
#define  CSR_TLBLO0_CCA_WIDTH       2
#define  CSR_TLBLO0_CCA             (UL(0x3) << CSR_TLBLO0_CCA_SHIFT)
#define  CSR_TLBLO0_PLV_SHIFT       2
#define  CSR_TLBLO0_PLV_WIDTH       2
#define  CSR_TLBLO0_PLV             (UL(0x3) << CSR_TLBLO0_PLV_SHIFT)
#define  CSR_TLBLO0_WE_SHIFT        1
#define  CSR_TLBLO0_WE              (UL(0x1) << CSR_TLBLO0_WE_SHIFT)
#define  CSR_TLBLO0_V_SHIFT         0
#define  CSR_TLBLO0_V               (UL(0x1) << CSR_TLBLO0_V_SHIFT)

#define LOONGARCH_CSR_TLBELO1       0x13 /* TLB EntryLo1 */
#define  CSR_TLBLO1_RPLV_SHIFT      63
#define  CSR_TLBLO1_RPLV            (UL(0x1) << CSR_TLBLO1_RPLV_SHIFT)
#define  CSR_TLBLO1_NX_SHIFT        62
#define  CSR_TLBLO1_NX              (UL(0x1) << CSR_TLBLO1_NX_SHIFT)
#define  CSR_TLBLO1_NR_SHIFT        61
#define  CSR_TLBLO1_NR              (UL(0x1) << CSR_TLBLO1_NR_SHIFT)
#define  CSR_TLBLO1_PFN_SHIFT       12
#define  CSR_TLBLO1_PFN_WIDTH       36
#define  CSR_TLBLO1_PFN             (UL(0xfffffffff) << CSR_TLBLO1_PFN_SHIFT)
#define  CSR_TLBLO1_GLOBAL_SHIFT    6
#define  CSR_TLBLO1_GLOBAL          (UL(0x1) << CSR_TLBLO1_GLOBAL_SHIFT)
#define  CSR_TLBLO1_CCA_SHIFT       4
#define  CSR_TLBLO1_CCA_WIDTH       2
#define  CSR_TLBLO1_CCA             (UL(0x3) << CSR_TLBLO1_CCA_SHIFT)
#define  CSR_TLBLO1_PLV_SHIFT       2
#define  CSR_TLBLO1_PLV_WIDTH       2
#define  CSR_TLBLO1_PLV             (UL(0x3) << CSR_TLBLO1_PLV_SHIFT)
#define  CSR_TLBLO1_WE_SHIFT        1
#define  CSR_TLBLO1_WE              (UL(0x1) << CSR_TLBLO1_WE_SHIFT)
#define  CSR_TLBLO1_V_SHIFT         0
#define  CSR_TLBLO1_V               (UL(0x1) << CSR_TLBLO1_V_SHIFT)

#define LOONGARCH_CSR_GTLBC         0x15 /* Guest TLB control */
#define  CSR_GTLBC_TGID_SHIFT       16
#define  CSR_GTLBC_TGID_WIDTH       8
#define  CSR_GTLBC_TGID             (UL(0xff) << CSR_GTLBC_TGID_SHIFT)
#define  CSR_GTLBC_TOTI_SHIFT       13
#define  CSR_GTLBC_TOTI             (UL(0x1) << CSR_GTLBC_TOTI_SHIFT)
#define  CSR_GTLBC_USETGID_SHIFT    12
#define  CSR_GTLBC_USETGID          (UL(0x1) << CSR_GTLBC_USETGID_SHIFT)
#define  CSR_GTLBC_GMTLBSZ_SHIFT    0
#define  CSR_GTLBC_GMTLBSZ_WIDTH    6
#define  CSR_GTLBC_GMTLBSZ          (UL(0x3f) << CSR_GTLBC_GMTLBSZ_SHIFT)

#define LOONGARCH_CSR_TRGP          0x16 /* TLBR read guest info */
#define  CSR_TRGP_RID_SHIFT         16
#define  CSR_TRGP_RID_WIDTH         8
#define  CSR_TRGP_RID               (UL(0xff) << CSR_TRGP_RID_SHIFT)
#define  CSR_TRGP_GTLB_SHIFT        0
#define  CSR_TRGP_GTLB              (1 << CSR_TRGP_GTLB_SHIFT)

#define LOONGARCH_CSR_ASID          0x18 /* ASID */
#define  CSR_ASID_BIT_SHIFT         16   /* ASIDBits */
#define  CSR_ASID_BIT_WIDTH         8
#define  CSR_ASID_BIT               (UL(0xff) << CSR_ASID_BIT_SHIFT)
#define  CSR_ASID_ASID_SHIFT        0
#define  CSR_ASID_ASID_WIDTH        10
#define  CSR_ASID_ASID                        (UL(0x3ff) << CSR_ASID_ASID_SHIFT)

#define LOONGARCH_CSR_PGDL          0x19 /* Page table base address when VA[47] = 0 */

#define LOONGARCH_CSR_PGDH          0x1a /* Page table base address when VA[47] = 1 */

#define LOONGARCH_CSR_PGD           0x1b /* Page table base */

#define LOONGARCH_CSR_PWCTL0        0x1c /* PWCtl0 */
#define  CSR_PWCTL0_PTEW_SHIFT      30
#define  CSR_PWCTL0_PTEW_WIDTH      2
#define  CSR_PWCTL0_PTEW            (UL(0x3) << CSR_PWCTL0_PTEW_SHIFT)
#define  CSR_PWCTL0_DIR1WIDTH_SHIFT 25
#define  CSR_PWCTL0_DIR1WIDTH_WIDTH 5
#define  CSR_PWCTL0_DIR1WIDTH       (UL(0x1f) << CSR_PWCTL0_DIR1WIDTH_SHIFT)
#define  CSR_PWCTL0_DIR1BASE_SHIFT  20
#define  CSR_PWCTL0_DIR1BASE_WIDTH  5
#define  CSR_PWCTL0_DIR1BASE        (UL(0x1f) << CSR_PWCTL0_DIR1BASE_SHIFT)
#define  CSR_PWCTL0_DIR0WIDTH_SHIFT 15
#define  CSR_PWCTL0_DIR0WIDTH_WIDTH 5
#define  CSR_PWCTL0_DIR0WIDTH       (UL(0x1f) << CSR_PWCTL0_DIR0WIDTH_SHIFT)
#define  CSR_PWCTL0_DIR0BASE_SHIFT  10
#define  CSR_PWCTL0_DIR0BASE_WIDTH  5
#define  CSR_PWCTL0_DIR0BASE        (UL(0x1f) << CSR_PWCTL0_DIR0BASE_SHIFT)
#define  CSR_PWCTL0_PTWIDTH_SHIFT   5
#define  CSR_PWCTL0_PTWIDTH_WIDTH   5
#define  CSR_PWCTL0_PTWIDTH         (UL(0x1f) << CSR_PWCTL0_PTWIDTH_SHIFT)
#define  CSR_PWCTL0_PTBASE_SHIFT    0
#define  CSR_PWCTL0_PTBASE_WIDTH    5
#define  CSR_PWCTL0_PTBASE          (UL(0x1f) << CSR_PWCTL0_PTBASE_SHIFT)

#define LOONGARCH_CSR_PWCTL1        0x1d /* PWCtl1 */
#define  CSR_PWCTL1_DIR3WIDTH_SHIFT 18
#define  CSR_PWCTL1_DIR3WIDTH_WIDTH 5
#define  CSR_PWCTL1_DIR3WIDTH       (UL(0x1f) << CSR_PWCTL1_DIR3WIDTH_SHIFT)
#define  CSR_PWCTL1_DIR3BASE_SHIFT  12
#define  CSR_PWCTL1_DIR3BASE_WIDTH  5
#define  CSR_PWCTL1_DIR3BASE        (UL(0x1f) << CSR_PWCTL0_DIR3BASE_SHIFT)
#define  CSR_PWCTL1_DIR2WIDTH_SHIFT 6
#define  CSR_PWCTL1_DIR2WIDTH_WIDTH 5
#define  CSR_PWCTL1_DIR2WIDTH       (UL(0x1f) << CSR_PWCTL1_DIR2WIDTH_SHIFT)
#define  CSR_PWCTL1_DIR2BASE_SHIFT  0
#define  CSR_PWCTL1_DIR2BASE_WIDTH  5
#define  CSR_PWCTL1_DIR2BASE        (UL(0x1f) << CSR_PWCTL0_DIR2BASE_SHIFT)

#define LOONGARCH_CSR_STLBPGSIZE    0x1e
#define  CSR_STLBPGSIZE_PS_WIDTH    6
#define  CSR_STLBPGSIZE_PS          (UL(0x3f))

#define LOONGARCH_CSR_RVACFG        0x1f
#define  CSR_RVACFG_RDVA_WIDTH      4
#define  CSR_RVACFG_RDVA            (UL(0xf))

/* Config CSR registers */
#define LOONGARCH_CSR_CPUID         0x20 /* CPU core id */
#define  CSR_CPUID_COREID_WIDTH     9
#define  CSR_CPUID_COREID           UL(0x1ff)

#define LOONGARCH_CSR_PRCFG1        0x21 /* Config1 */
#define  CSR_CONF1_VSMAX_SHIFT      12
#define  CSR_CONF1_VSMAX_WIDTH      3
#define  CSR_CONF1_VSMAX            (UL(7) << CSR_CONF1_VSMAX_SHIFT)
#define  CSR_CONF1_TMRBITS_SHIFT    4
#define  CSR_CONF1_TMRBITS_WIDTH    8
#define  CSR_CONF1_TMRBITS          (UL(0xff) << CSR_CONF1_TMRBITS_SHIFT)
#define  CSR_CONF1_KSNUM_WIDTH      4
#define  CSR_CONF1_KSNUM            UL(0xf)

#define LOONGARCH_CSR_PRCFG2        0x22 /* Config2 */
#define  CSR_CONF2_PGMASK_SUPP      0x3ffff000

#define LOONGARCH_CSR_PRCFG3        0x23 /* Config3 */
#define  CSR_CONF3_STLBIDX_SHIFT    20
#define  CSR_CONF3_STLBIDX_WIDTH    6
#define  CSR_CONF3_STLBIDX          (UL(0x3f) << CSR_CONF3_STLBIDX_SHIFT)
#define  CSR_CONF3_STLBWAYS_SHIFT   12
#define  CSR_CONF3_STLBWAYS_WIDTH   8
#define  CSR_CONF3_STLBWAYS         (UL(0xff) << CSR_CONF3_STLBWAYS_SHIFT)
#define  CSR_CONF3_MTLBSIZE_SHIFT   4
#define  CSR_CONF3_MTLBSIZE_WIDTH   8
#define  CSR_CONF3_MTLBSIZE         (UL(0xff) << CSR_CONF3_MTLBSIZE_SHIFT)
#define  CSR_CONF3_TLBTYPE_SHIFT    0
#define  CSR_CONF3_TLBTYPE_WIDTH    4
#define  CSR_CONF3_TLBTYPE          (UL(0xf) << CSR_CONF3_TLBTYPE_SHIFT)

/* KSave registers */
#define LOONGARCH_CSR_KS0           0x30
#define LOONGARCH_CSR_KS1           0x31
#define LOONGARCH_CSR_KS2           0x32
#define LOONGARCH_CSR_KS3           0x33
#define LOONGARCH_CSR_KS4           0x34
#define LOONGARCH_CSR_KS5           0x35
#define LOONGARCH_CSR_KS6           0x36
#define LOONGARCH_CSR_KS7           0x37
#define LOONGARCH_CSR_KS8           0x38

/* Exception allocated KS0, KS1 and KS2 statically */
#define EXCEPTION_KS0               LOONGARCH_CSR_KS0
#define EXCEPTION_KS1               LOONGARCH_CSR_KS1
#define EXCEPTION_KS2               LOONGARCH_CSR_KS2
#define EXC_KSAVE_MASK              (1 << 0 | 1 << 1 | 1 << 2)

/* Percpu-data base allocated KS3 statically */
#define PERCPU_BASE_KS              LOONGARCH_CSR_KS3
#define PERCPU_KSAVE_MASK           (1 << 3)

/* KVM allocated KS4 and KS5 statically */
#define KVM_VCPU_KS                 LOONGARCH_CSR_KS4
#define KVM_TEMP_KS                 LOONGARCH_CSR_KS5
#define KVM_KSAVE_MASK              (1 << 4 | 1 << 5)

/* Timer registers */
#define LOONGARCH_CSR_TMID          0x40 /* Timer ID */

#define LOONGARCH_CSR_TCFG          0x41 /* Timer config */
#define  CSR_TCFG_VAL_SHIFT         2
#ifdef ARCH_CPU_64BIT
#define  CSR_TCFG_VAL_WIDTH         48
#define  CSR_TCFG_VAL               (UL(0x3fffffffffff) << CSR_TCFG_VAL_SHIFT)
#else
#define  CSR_TCFG_VAL_WIDTH         30
#define  CSR_TCFG_VAL               (UL(0x3fffffff) << CSR_TCFG_VAL_SHIFT)
#endif
#define  CSR_TCFG_PERIOD_SHIFT      1
#define  CSR_TCFG_PERIOD            (UL(0x1) << CSR_TCFG_PERIOD_SHIFT)
#define  CSR_TCFG_EN                (UL(0x1))

#define LOONGARCH_CSR_TVAL          0x42 /* Timer value */

#define LOONGARCH_CSR_CNTC          0x43 /* Timer offset */

#define LOONGARCH_CSR_TINTCLR       0x44 /* Timer interrupt clear */
#define  CSR_TINTCLR_TI_SHIFT       0
#define  CSR_TINTCLR_TI             (1 << CSR_TINTCLR_TI_SHIFT)

/* Guest registers */
#define LOONGARCH_CSR_GSTAT         0x50 /* Guest status */
#define  CSR_GSTAT_GID_SHIFT        16
#define  CSR_GSTAT_GID_WIDTH        8
#define  CSR_GSTAT_GID              (UL(0xff) << CSR_GSTAT_GID_SHIFT)
#define  CSR_GSTAT_GIDBIT_SHIFT     4
#define  CSR_GSTAT_GIDBIT_WIDTH     6
#define  CSR_GSTAT_GIDBIT           (UL(0x3f) << CSR_GSTAT_GIDBIT_SHIFT)
#define  CSR_GSTAT_PVM_SHIFT        1
#define  CSR_GSTAT_PVM              (UL(0x1) << CSR_GSTAT_PVM_SHIFT)
#define  CSR_GSTAT_VM_SHIFT         0
#define  CSR_GSTAT_VM               (UL(0x1) << CSR_GSTAT_VM_SHIFT)

#define LOONGARCH_CSR_GCFG          0x51 /* Guest config */
#define  CSR_GCFG_GPERF_SHIFT       24
#define  CSR_GCFG_GPERF_WIDTH       3
#define  CSR_GCFG_GPERF             (UL(0x7) << CSR_GCFG_GPERF_SHIFT)
#define  CSR_GCFG_GCI_SHIFT         20
#define  CSR_GCFG_GCI_WIDTH         2
#define  CSR_GCFG_GCI               (UL(0x3) << CSR_GCFG_GCI_SHIFT)
#define  CSR_GCFG_GCI_ALL           (UL(0x0) << CSR_GCFG_GCI_SHIFT)
#define  CSR_GCFG_GCI_HIT           (UL(0x1) << CSR_GCFG_GCI_SHIFT)
#define  CSR_GCFG_GCI_SECURE        (UL(0x2) << CSR_GCFG_GCI_SHIFT)
#define  CSR_GCFG_GCIP_SHIFT        16
#define  CSR_GCFG_GCIP              (UL(0xf) << CSR_GCFG_GCIP_SHIFT)
#define  CSR_GCFG_GCIP_ALL          (UL(0x1) << CSR_GCFG_GCIP_SHIFT)
#define  CSR_GCFG_GCIP_HIT          (UL(0x1) << (CSR_GCFG_GCIP_SHIFT + 1))
#define  CSR_GCFG_GCIP_SECURE       (UL(0x1) << (CSR_GCFG_GCIP_SHIFT + 2))
#define  CSR_GCFG_TORU_SHIFT        15
#define  CSR_GCFG_TORU              (UL(0x1) << CSR_GCFG_TORU_SHIFT)
#define  CSR_GCFG_TORUP_SHIFT       14
#define  CSR_GCFG_TORUP             (UL(0x1) << CSR_GCFG_TORUP_SHIFT)
#define  CSR_GCFG_TOP_SHIFT         13
#define  CSR_GCFG_TOP               (UL(0x1) << CSR_GCFG_TOP_SHIFT)
#define  CSR_GCFG_TOPP_SHIFT        12
#define  CSR_GCFG_TOPP              (UL(0x1) << CSR_GCFG_TOPP_SHIFT)
#define  CSR_GCFG_TOE_SHIFT         11
#define  CSR_GCFG_TOE               (UL(0x1) << CSR_GCFG_TOE_SHIFT)
#define  CSR_GCFG_TOEP_SHIFT        10
#define  CSR_GCFG_TOEP              (UL(0x1) << CSR_GCFG_TOEP_SHIFT)
#define  CSR_GCFG_TIT_SHIFT         9
#define  CSR_GCFG_TIT               (UL(0x1) << CSR_GCFG_TIT_SHIFT)
#define  CSR_GCFG_TITP_SHIFT        8
#define  CSR_GCFG_TITP              (UL(0x1) << CSR_GCFG_TITP_SHIFT)
#define  CSR_GCFG_SIT_SHIFT         7
#define  CSR_GCFG_SIT               (UL(0x1) << CSR_GCFG_SIT_SHIFT)
#define  CSR_GCFG_SITP_SHIFT        6
#define  CSR_GCFG_SITP              (UL(0x1) << CSR_GCFG_SITP_SHIFT)
#define  CSR_GCFG_MATC_SHITF        4
#define  CSR_GCFG_MATC_WIDTH        2
#define  CSR_GCFG_MATC_MASK         (UL(0x3) << CSR_GCFG_MATC_SHITF)
#define  CSR_GCFG_MATC_GUEST        (UL(0x0) << CSR_GCFG_MATC_SHITF)
#define  CSR_GCFG_MATC_ROOT         (UL(0x1) << CSR_GCFG_MATC_SHITF)
#define  CSR_GCFG_MATC_NEST         (UL(0x2) << CSR_GCFG_MATC_SHITF)
#define  CSR_GCFG_MATP_SHITF        0
#define  CSR_GCFG_MATP_WIDTH        4
#define  CSR_GCFG_MATP_MASK         (UL(0x3) << CSR_GCFG_MATP_SHITF)
#define  CSR_GCFG_MATP_GUEST        (UL(0x0) << CSR_GCFG_MATP_SHITF)
#define  CSR_GCFG_MATP_ROOT         (UL(0x1) << CSR_GCFG_MATP_SHITF)
#define  CSR_GCFG_MATP_NEST         (UL(0x2) << CSR_GCFG_MATP_SHITF)

#define LOONGARCH_CSR_GINTC         0x52 /* Guest interrupt control */
#define  CSR_GINTC_HC_SHIFT         16
#define  CSR_GINTC_HC_WIDTH         8
#define  CSR_GINTC_HC               (UL(0xff) << CSR_GINTC_HC_SHIFT)
#define  CSR_GINTC_PIP_SHIFT        8
#define  CSR_GINTC_PIP_WIDTH        8
#define  CSR_GINTC_PIP              (UL(0xff) << CSR_GINTC_PIP_SHIFT)
#define  CSR_GINTC_VIP_SHIFT        0
#define  CSR_GINTC_VIP_WIDTH        8
#define  CSR_GINTC_VIP              (UL(0xff))

#define LOONGARCH_CSR_GCNTC         0x53 /* Guest timer offset */

/* LLBCTL register */
#define LOONGARCH_CSR_LLBCTL        0x60 /* LLBit control */
#define  CSR_LLBCTL_ROLLB_SHIFT     0
#define  CSR_LLBCTL_ROLLB           (UL(1) << CSR_LLBCTL_ROLLB_SHIFT)
#define  CSR_LLBCTL_WCLLB_SHIFT     1
#define  CSR_LLBCTL_WCLLB           (UL(1) << CSR_LLBCTL_WCLLB_SHIFT)
#define  CSR_LLBCTL_KLO_SHIFT       2
#define  CSR_LLBCTL_KLO             (UL(1) << CSR_LLBCTL_KLO_SHIFT)

/* Implement dependent */
#define LOONGARCH_CSR_IMPCTL1       0x80 /* Loongson config1 */
#define  CSR_MISPEC_SHIFT           20
#define  CSR_MISPEC_WIDTH           8
#define  CSR_MISPEC                 (UL(0xff) << CSR_MISPEC_SHIFT)
#define  CSR_SSEN_SHIFT             18
#define  CSR_SSEN                   (UL(1) << CSR_SSEN_SHIFT)
#define  CSR_SCRAND_SHIFT           17
#define  CSR_SCRAND                 (UL(1) << CSR_SCRAND_SHIFT)
#define  CSR_LLEXCL_SHIFT           16
#define  CSR_LLEXCL                 (UL(1) << CSR_LLEXCL_SHIFT)
#define  CSR_DISVC_SHIFT            15
#define  CSR_DISVC                  (UL(1) << CSR_DISVC_SHIFT)
#define  CSR_VCLRU_SHIFT            14
#define  CSR_VCLRU                  (UL(1) << CSR_VCLRU_SHIFT)
#define  CSR_DCLRU_SHIFT            13
#define  CSR_DCLRU                  (UL(1) << CSR_DCLRU_SHIFT)
#define  CSR_FASTLDQ_SHIFT          12
#define  CSR_FASTLDQ                (UL(1) << CSR_FASTLDQ_SHIFT)
#define  CSR_USERCAC_SHIFT          11
#define  CSR_USERCAC                (UL(1) << CSR_USERCAC_SHIFT)
#define  CSR_ANTI_MISPEC_SHIFT      10
#define  CSR_ANTI_MISPEC            (UL(1) << CSR_ANTI_MISPEC_SHIFT)
#define  CSR_AUTO_FLUSHSFB_SHIFT    9
#define  CSR_AUTO_FLUSHSFB          (UL(1) << CSR_AUTO_FLUSHSFB_SHIFT)
#define  CSR_STFILL_SHIFT           8
#define  CSR_STFILL                 (UL(1) << CSR_STFILL_SHIFT)
#define  CSR_LIFEP_SHIFT            7
#define  CSR_LIFEP                  (UL(1) << CSR_LIFEP_SHIFT)
#define  CSR_LLSYNC_SHIFT           6
#define  CSR_LLSYNC                 (UL(1) << CSR_LLSYNC_SHIFT)
#define  CSR_BRBTDIS_SHIFT          5
#define  CSR_BRBTDIS                (UL(1) << CSR_BRBTDIS_SHIFT)
#define  CSR_RASDIS_SHIFT           4
#define  CSR_RASDIS                 (UL(1) << CSR_RASDIS_SHIFT)
#define  CSR_STPRE_SHIFT            2
#define  CSR_STPRE_WIDTH            2
#define  CSR_STPRE                  (UL(3) << CSR_STPRE_SHIFT)
#define  CSR_INSTPRE_SHIFT          1
#define  CSR_INSTPRE                (UL(1) << CSR_INSTPRE_SHIFT)
#define  CSR_DATAPRE_SHIFT          0
#define  CSR_DATAPRE                (UL(1) << CSR_DATAPRE_SHIFT)

#define LOONGARCH_CSR_IMPCTL2       0x81 /* Loongson config2 */
#define  CSR_FLUSH_MTLB_SHIFT       0
#define  CSR_FLUSH_MTLB             (UL(1) << CSR_FLUSH_MTLB_SHIFT)
#define  CSR_FLUSH_STLB_SHIFT       1
#define  CSR_FLUSH_STLB             (UL(1) << CSR_FLUSH_STLB_SHIFT)
#define  CSR_FLUSH_DTLB_SHIFT       2
#define  CSR_FLUSH_DTLB             (UL(1) << CSR_FLUSH_DTLB_SHIFT)
#define  CSR_FLUSH_ITLB_SHIFT       3
#define  CSR_FLUSH_ITLB             (UL(1) << CSR_FLUSH_ITLB_SHIFT)
#define  CSR_FLUSH_BTAC_SHIFT       4
#define  CSR_FLUSH_BTAC             (UL(1) << CSR_FLUSH_BTAC_SHIFT)

#define LOONGARCH_CSR_GNMI          0x82

/* TLB Refill registers */
#define LOONGARCH_CSR_TLBRENTRY     0x88 /* TLB refill exception entry */
#define LOONGARCH_CSR_TLBRBADV      0x89 /* TLB refill badvaddr */
#define LOONGARCH_CSR_TLBRERA       0x8a /* TLB refill ERA */
#define LOONGARCH_CSR_TLBRSAVE      0x8b /* KSave for TLB refill exception */
#define LOONGARCH_CSR_TLBRELO0      0x8c /* TLB refill entrylo0 */
#define LOONGARCH_CSR_TLBRELO1      0x8d /* TLB refill entrylo1 */
#define LOONGARCH_CSR_TLBREHI       0x8e /* TLB refill entryhi */
#define  CSR_TLBREHI_PS_SHIFT       0
#define  CSR_TLBREHI_PS             (UL(0x3f) << CSR_TLBREHI_PS_SHIFT)
#define LOONGARCH_CSR_TLBRPRMD      0x8f /* TLB refill mode info */

/* Machine Error registers */
#define LOONGARCH_CSR_MERRCTL       0x90 /* MERRCTL */
#define LOONGARCH_CSR_MERRINFO1     0x91 /* MError info1 */
#define LOONGARCH_CSR_MERRINFO2     0x92 /* MError info2 */
#define LOONGARCH_CSR_MERRENTRY     0x93 /* MError exception entry */
#define LOONGARCH_CSR_MERRERA       0x94 /* MError exception ERA */
#define LOONGARCH_CSR_MERRSAVE      0x95 /* KSave for machine error exception */

#define LOONGARCH_CSR_CTAG          0x98 /* TagLo + TagHi */

/* Direct Map windows registers */
#define LOONGARCH_CSR_DMWIN0        0x180 /* 64 direct map win0: MEM & IF */
#define LOONGARCH_CSR_DMWIN1        0x181 /* 64 direct map win1: MEM & IF */
#define LOONGARCH_CSR_DMWIN2        0x182 /* 64 direct map win2: MEM */
#define LOONGARCH_CSR_DMWIN3        0x183 /* 64 direct map win3: MEM */

/* Performance Counter registers */
#define LOONGARCH_CSR_PERFCTRL0     0x200 /* 32 perf event 0 config */
#define LOONGARCH_CSR_PERFCNTR0     0x201 /* 64 perf event 0 count value */
#define LOONGARCH_CSR_PERFCTRL1     0x202 /* 32 perf event 1 config */
#define LOONGARCH_CSR_PERFCNTR1     0x203 /* 64 perf event 1 count value */
#define LOONGARCH_CSR_PERFCTRL2     0x204 /* 32 perf event 2 config */
#define LOONGARCH_CSR_PERFCNTR2     0x205 /* 64 perf event 2 count value */
#define LOONGARCH_CSR_PERFCTRL3     0x206 /* 32 perf event 3 config */
#define LOONGARCH_CSR_PERFCNTR3     0x207 /* 64 perf event 3 count value */

/* Debug registers */
#define LOONGARCH_CSR_MWPC          0x300 /* data breakpoint config */
#define LOONGARCH_CSR_MWPS          0x301 /* data breakpoint status */

#define LOONGARCH_CSR_DB0ADDR       0x310 /* data breakpoint 0 address */
#define LOONGARCH_CSR_DB0MASK       0x311 /* data breakpoint 0 mask */
#define LOONGARCH_CSR_DB0CTL        0x312 /* data breakpoint 0 control */
#define LOONGARCH_CSR_DB0ASID       0x313 /* data breakpoint 0 asid */

#define LOONGARCH_CSR_DB1ADDR       0x318 /* data breakpoint 1 address */
#define LOONGARCH_CSR_DB1MASK       0x319 /* data breakpoint 1 mask */
#define LOONGARCH_CSR_DB1CTL        0x31a /* data breakpoint 1 control */
#define LOONGARCH_CSR_DB1ASID       0x31b /* data breakpoint 1 asid */

#define LOONGARCH_CSR_DB2ADDR       0x320 /* data breakpoint 2 address */
#define LOONGARCH_CSR_DB2MASK       0x321 /* data breakpoint 2 mask */
#define LOONGARCH_CSR_DB2CTL        0x322 /* data breakpoint 2 control */
#define LOONGARCH_CSR_DB2ASID       0x323 /* data breakpoint 2 asid */

#define LOONGARCH_CSR_DB3ADDR       0x328 /* data breakpoint 3 address */
#define LOONGARCH_CSR_DB3MASK       0x329 /* data breakpoint 3 mask */
#define LOONGARCH_CSR_DB3CTL        0x32a /* data breakpoint 3 control */
#define LOONGARCH_CSR_DB3ASID       0x32b /* data breakpoint 3 asid */

#define LOONGARCH_CSR_DB4ADDR       0x330 /* data breakpoint 4 address */
#define LOONGARCH_CSR_DB4MASK       0x331 /* data breakpoint 4 maks */
#define LOONGARCH_CSR_DB4CTL        0x332 /* data breakpoint 4 control */
#define LOONGARCH_CSR_DB4ASID       0x333 /* data breakpoint 4 asid */

#define LOONGARCH_CSR_DB5ADDR       0x338 /* data breakpoint 5 address */
#define LOONGARCH_CSR_DB5MASK       0x339 /* data breakpoint 5 mask */
#define LOONGARCH_CSR_DB5CTL        0x33a /* data breakpoint 5 control */
#define LOONGARCH_CSR_DB5ASID       0x33b /* data breakpoint 5 asid */

#define LOONGARCH_CSR_DB6ADDR       0x340 /* data breakpoint 6 address */
#define LOONGARCH_CSR_DB6MASK       0x341 /* data breakpoint 6 mask */
#define LOONGARCH_CSR_DB6CTL        0x342 /* data breakpoint 6 control */
#define LOONGARCH_CSR_DB6ASID       0x343 /* data breakpoint 6 asid */

#define LOONGARCH_CSR_DB7ADDR       0x348 /* data breakpoint 7 address */
#define LOONGARCH_CSR_DB7MASK       0x349 /* data breakpoint 7 mask */
#define LOONGARCH_CSR_DB7CTL        0x34a /* data breakpoint 7 control */
#define LOONGARCH_CSR_DB7ASID       0x34b /* data breakpoint 7 asid */

#define LOONGARCH_CSR_FWPC          0x380 /* instruction breakpoint config */
#define LOONGARCH_CSR_FWPS          0x381 /* instruction breakpoint status */

#define LOONGARCH_CSR_IB0ADDR       0x390 /* inst breakpoint 0 address */
#define LOONGARCH_CSR_IB0MASK       0x391 /* inst breakpoint 0 mask */
#define LOONGARCH_CSR_IB0CTL        0x392 /* inst breakpoint 0 control */
#define LOONGARCH_CSR_IB0ASID       0x393 /* inst breakpoint 0 asid */

#define LOONGARCH_CSR_IB1ADDR       0x398 /* inst breakpoint 1 address */
#define LOONGARCH_CSR_IB1MASK       0x399 /* inst breakpoint 1 mask */
#define LOONGARCH_CSR_IB1CTL        0x39a /* inst breakpoint 1 control */
#define LOONGARCH_CSR_IB1ASID       0x39b /* inst breakpoint 1 asid */

#define LOONGARCH_CSR_IB2ADDR       0x3a0 /* inst breakpoint 2 address */
#define LOONGARCH_CSR_IB2MASK       0x3a1 /* inst breakpoint 2 mask */
#define LOONGARCH_CSR_IB2CTL        0x3a2 /* inst breakpoint 2 control */
#define LOONGARCH_CSR_IB2ASID       0x3a3 /* inst breakpoint 2 asid */

#define LOONGARCH_CSR_IB3ADDR       0x3a8 /* inst breakpoint 3 address */
#define LOONGARCH_CSR_IB3MASK       0x3a9 /* breakpoint 3 mask */
#define LOONGARCH_CSR_IB3CTL        0x3aa /* inst breakpoint 3 control */
#define LOONGARCH_CSR_IB3ASID       0x3ab /* inst breakpoint 3 asid */

#define LOONGARCH_CSR_IB4ADDR       0x3b0 /* inst breakpoint 4 address */
#define LOONGARCH_CSR_IB4MASK       0x3b1 /* inst breakpoint 4 mask */
#define LOONGARCH_CSR_IB4CTL        0x3b2 /* inst breakpoint 4 control */
#define LOONGARCH_CSR_IB4ASID       0x3b3 /* inst breakpoint 4 asid */

#define LOONGARCH_CSR_IB5ADDR       0x3b8 /* inst breakpoint 5 address */
#define LOONGARCH_CSR_IB5MASK       0x3b9 /* inst breakpoint 5 mask */
#define LOONGARCH_CSR_IB5CTL        0x3ba /* inst breakpoint 5 control */
#define LOONGARCH_CSR_IB5ASID       0x3bb /* inst breakpoint 5 asid */

#define LOONGARCH_CSR_IB6ADDR       0x3c0 /* inst breakpoint 6 address */
#define LOONGARCH_CSR_IB6MASK       0x3c1 /* inst breakpoint 6 mask */
#define LOONGARCH_CSR_IB6CTL        0x3c2 /* inst breakpoint 6 control */
#define LOONGARCH_CSR_IB6ASID       0x3c3 /* inst breakpoint 6 asid */

#define LOONGARCH_CSR_IB7ADDR       0x3c8 /* inst breakpoint 7 address */
#define LOONGARCH_CSR_IB7MASK       0x3c9 /* inst breakpoint 7 mask */
#define LOONGARCH_CSR_IB7CTL        0x3ca /* inst breakpoint 7 control */
#define LOONGARCH_CSR_IB7ASID       0x3cb /* inst breakpoint 7 asid */

#define LOONGARCH_CSR_DEBUG         0x500 /* debug config */
#define LOONGARCH_CSR_DERA          0x501 /* debug era */
#define LOONGARCH_CSR_DESAVE        0x502 /* debug save */

/* Bit fields for ESTAT IP */
#define ESTAT_IS_SIP0_SHIFT         0
#define ESTAT_IS_SIP0               (0x1 << ESTAT_IS_SIP0_SHIFT)
#define ESTAT_IS_SIP1_SHIFT         1
#define ESTAT_IS_SIP1               (0x1 << ESTAT_IS_SIP1_SHIFT)
#define ESTAT_IS_IP0_SHIFT          2
#define ESTAT_IS_IP0                (0x1 << ESTAT_IS_IP0_SHIFT)
#define ESTAT_IS_IP1_SHIFT          3
#define ESTAT_IS_IP1                (0x1 << ESTAT_IS_IP1_SHIFT)
#define ESTAT_IS_IP2_SHIFT          4
#define ESTAT_IS_IP2                (0x1 << ESTAT_IS_IP2_SHIFT)
#define ESTAT_IS_IP3_SHIFT          5
#define ESTAT_IS_IP3                (0x1 << ESTAT_IS_IP3_SHIFT)
#define ESTAT_IS_IP4_SHIFT          6
#define ESTAT_IS_IP4                (0x1 << ESTAT_IS_IP4_SHIFT)
#define ESTAT_IS_IP5_SHIFT          7
#define ESTAT_IS_IP5                (0x1 << ESTAT_IS_IP5_SHIFT)
#define ESTAT_IS_IP6_SHIFT          8
#define ESTAT_IS_IP6                (0x1 << ESTAT_IS_IP6_SHIFT)
#define ESTAT_IS_IP7_SHIFT          9
#define ESTAT_IS_IP7                (0x1 << ESTAT_IS_IP7_SHIFT)
#define ESTAT_IS_PMC_SHIFT          10
#define ESTAT_IS_PMC                (0x1 << ESTAT_IS_PMC_SHIFT)
#define ESTAT_IS_TIMER_SHIFT        11
#define ESTAT_IS_TIMER              (0x1 << ESTAT_IS_TIMER_SHIFT)
#define ESTAT_IS_IPI_SHIFT          12
#define ESTAT_IS_IPI                (0x1 << ESTAT_IS_IPI_SHIFT)

/* Bit fields for CPUCFG registers */
#define LOONGARCH_CPUCFG0           0x0
#define  CPUCFG0_PRID               GENMASK(31, 0)

#define LOONGARCH_CPUCFG1           0x1
#define  CPUCFG1_ISGR32             BIT(0)
#define  CPUCFG1_ISGR64             BIT(1)
#define  CPUCFG1_PAGING             BIT(2)
#define  CPUCFG1_IOCSR              BIT(3)
#define  CPUCFG1_PABITS             GENMASK(11, 4)
#define  CPUCFG1_VABITS             GENMASK(19, 12)
#define  CPUCFG1_UAL                BIT(20)
#define  CPUCFG1_RI                 BIT(21)
#define  CPUCFG1_EP                 BIT(22)
#define  CPUCFG1_RPLV               BIT(23)
#define  CPUCFG1_HUGEPG             BIT(24)
#define  CPUCFG1_IOCSRBRD           BIT(25)
#define  CPUCFG1_MSGINT             BIT(26)

#define LOONGARCH_CPUCFG2           0x2
#define  CPUCFG2_FP                 BIT(0)
#define  CPUCFG2_FPSP               BIT(1)
#define  CPUCFG2_FPDP               BIT(2)
#define  CPUCFG2_FPVERS             GENMASK(5, 3)
#define  CPUCFG2_LSX                BIT(6)
#define  CPUCFG2_LASX               BIT(7)
#define  CPUCFG2_COMPLEX            BIT(8)
#define  CPUCFG2_CRYPTO             BIT(9)
#define  CPUCFG2_LVZP               BIT(10)
#define  CPUCFG2_LVZVER             GENMASK(13, 11)
#define  CPUCFG2_LLFTP              BIT(14)
#define  CPUCFG2_LLFTPREV           GENMASK(17, 15)
#define  CPUCFG2_X86BT              BIT(18)
#define  CPUCFG2_ARMBT              BIT(19)
#define  CPUCFG2_MIPSBT             BIT(20)
#define  CPUCFG2_LSPW               BIT(21)
#define  CPUCFG2_LAM                BIT(22)

#define LOONGARCH_CPUCFG3           0x3
#define  CPUCFG3_CCDMA              BIT(0)
#define  CPUCFG3_SFB                BIT(1)
#define  CPUCFG3_UCACC              BIT(2)
#define  CPUCFG3_LLEXC              BIT(3)
#define  CPUCFG3_SCDLY              BIT(4)
#define  CPUCFG3_LLDBAR             BIT(5)
#define  CPUCFG3_ITLBT              BIT(6)
#define  CPUCFG3_ICACHET            BIT(7)
#define  CPUCFG3_SPW_LVL            GENMASK(10, 8)
#define  CPUCFG3_SPW_HG_HF          BIT(11)
#define  CPUCFG3_RVA                BIT(12)
#define  CPUCFG3_RVAMAX             GENMASK(16, 13)

#define LOONGARCH_CPUCFG4           0x4
#define  CPUCFG4_CCFREQ             GENMASK(31, 0)

#define LOONGARCH_CPUCFG5           0x5
#define  CPUCFG5_CCMUL              GENMASK(15, 0)
#define  CPUCFG5_CCDIV              GENMASK(31, 16)

#define LOONGARCH_CPUCFG6           0x6
#define  CPUCFG6_PMP                BIT(0)
#define  CPUCFG6_PAMVER             GENMASK(3, 1)
#define  CPUCFG6_PMNUM              GENMASK(7, 4)
#define  CPUCFG6_PMBITS             GENMASK(13, 8)
#define  CPUCFG6_UPM                BIT(14)

#define LOONGARCH_CPUCFG16          0x10
#define  CPUCFG16_L1_IUPRE          BIT(0)
#define  CPUCFG16_L1_IUUNIFY        BIT(1)
#define  CPUCFG16_L1_DPRE           BIT(2)
#define  CPUCFG16_L2_IUPRE          BIT(3)
#define  CPUCFG16_L2_IUUNIFY        BIT(4)
#define  CPUCFG16_L2_IUPRIV         BIT(5)
#define  CPUCFG16_L2_IUINCL         BIT(6)
#define  CPUCFG16_L2_DPRE           BIT(7)
#define  CPUCFG16_L2_DPRIV          BIT(8)
#define  CPUCFG16_L2_DINCL          BIT(9)
#define  CPUCFG16_L3_IUPRE          BIT(10)
#define  CPUCFG16_L3_IUUNIFY        BIT(11)
#define  CPUCFG16_L3_IUPRIV         BIT(12)
#define  CPUCFG16_L3_IUINCL         BIT(13)
#define  CPUCFG16_L3_DPRE           BIT(14)
#define  CPUCFG16_L3_DPRIV          BIT(15)
#define  CPUCFG16_L3_DINCL          BIT(16)

#define LOONGARCH_CPUCFG17          0x11
#define LOONGARCH_CPUCFG18          0x12
#define LOONGARCH_CPUCFG19          0x13
#define LOONGARCH_CPUCFG20          0x14
#define  CPUCFG_CACHE_WAYS_M        GENMASK(15, 0)
#define  CPUCFG_CACHE_SETS_M        GENMASK(23, 16)
#define  CPUCFG_CACHE_LSIZE_M       GENMASK(30, 24)
#define  CPUCFG_CACHE_WAYS          0
#define  CPUCFG_CACHE_SETS          16
#define  CPUCFG_CACHE_LSIZE         24

#endif /* LOONGARCH_H__ */
