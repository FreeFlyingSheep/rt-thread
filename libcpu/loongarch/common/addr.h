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
