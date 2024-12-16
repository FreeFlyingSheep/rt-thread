#include <rtthread.h>

#include "exception.h"

/* Exception handler definition */
typedef void (*exc_handler_t)(int code, int subcode);

struct exc_desc
{
    exc_handler_t handler;
    char          name[RT_NAME_MAX];
};

static struct exc_desc exc_desc[LOONGARCH_EXC_NUM];

static const char *exc_name[LOONGARCH_EXC_NUM] =
{
    [0]                            = "INT",
    [1]                            = "PIL",
    [2]                            = "PIS",
    [3]                            = "PIF",
    [4]                            = "PME",
    [5]                            = "PNR",
    [6]                            = "PNX",
    [7]                            = "PPI",
    [8]                            = "ADEF/ADEM",
    [9]                            = "ALE",
    [10]                           = "BCE",
    [11]                           = "SYSCALL",
    [12]                           = "BREAK",
    [13]                           = "INE",
    [14]                           = "IPE",
    [15]                           = "FPD",
    [16]                           = "SXD",
    [17]                           = "ASXD",
    [18]                           = "FPE/VFPE",
    [19]                           = "WPEF/WPEM",
    [20]                           = "BTD",
    [21]                           = "BTE",
    [22]                           = "GSPR",
    [23]                           = "HVC",
    [24]                           = "GCSC/GCHC",
    [25 ... LOONGARCH_EXC_NUM - 1] = "RESERVED",
};

void handle_exception(int code, int subcode)
{
    exc_handler_t handler;

    handler = exc_desc[code].handler;

    if (handler != RT_NULL)
    {
        handler(code, subcode);
    }
    else
    {
        rt_kprintf("Unhandled exception %d (%d): %s\n", code, subcode, exc_name[code]);
    }
}

void rt_hw_exception_init(void)
{
}
