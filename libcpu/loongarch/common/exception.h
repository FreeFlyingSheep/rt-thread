#ifndef EXCEPTION_H__
#define EXCEPTION_H__

#define LOONGARCH_EXC_INT       0
#define LOONGARCH_EXC_PIL       1
#define LOONGARCH_EXC_PIS       2
#define LOONGARCH_EXC_PIF       3
#define LOONGARCH_EXC_PME       4
#define LOONGARCH_EXC_PNR       5
#define LOONGARCH_EXC_PNX       6
#define LOONGARCH_EXC_PPI       7
#define LOONGARCH_EXC_ADEF_ADEM 8
#define LOONGARCH_EXC_ALE       9
#define LOONGARCH_EXC_BCE       10
#define LOONGARCH_EXC_SYSCALL   11
#define LOONGARCH_EXC_BREAK     12
#define LOONGARCH_EXC_INE       13
#define LOONGARCH_EXC_IPE       14
#define LOONGARCH_EXC_FPD       15
#define LOONGARCH_EXC_SXD       16
#define LOONGARCH_EXC_ASXD      17
#define LOONGARCH_EXC_FPE_VFPE  18
#define LOONGARCH_EXC_WPEF_WPEM 19
#define LOONGARCH_EXC_BTD       20
#define LOONGARCH_EXC_BTE       21
#define LOONGARCH_EXC_GSPR      22
#define LOONGARCH_EXC_HVC       23
#define LOONGARCH_EXC_GCSC_GCHC 24

#define LOONGARCH_EXC_NUM       63

void handle_exception(int code, int subcode);
void rt_hw_exception_init(void);

#endif /* EXCEPTION_H__ */
