/*
 * Copyright (c) 2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-13     Feiyang Chen the first version
 */

#ifndef ADDR_H__
#define ADDR_H__

#include <rtconfig.h>

#ifdef SOC_LA464
#define TO_PHYS_MASK  0x1fffffffffffffff
#define CACHE_BASE    0x8000000000000000
#define UNCACHE_BASE  0x9000000000000000

#define TO_PHYS(x)    ((x) & TO_PHYS_MASK)
#define TO_CACHE(x)   (CACHE_BASE | ((x) & TO_PHYS_MASK))
#define TO_UNCACHE(x) (UNCACHE_BASE | ((x) & TO_PHYS_MASK))
#endif

#ifndef TO_PHYS
#define TO_PHYS(x) (x)
#endif

#ifndef TO_CACHE
#define TO_CACHE(x) (x)
#endif

#ifndef TO_UNCACHE
#define TO_UNCACHE(x) (x)
#endif

#endif /* ADDR_H__ */
