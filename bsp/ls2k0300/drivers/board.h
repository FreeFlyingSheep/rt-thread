#ifndef BOARD_H__
#define BOARD_H__

#include <rtthread.h>

#define RT_HW_HEAP_SIZE  (64 * 1024 * 1024)

extern rt_uint8_t __bss_end;

#define RT_HW_HEAP_BEGIN (&__bss_end)
#define RT_HW_HEAP_END   (RT_HW_HEAP_BEGIN + RT_HW_HEAP_SIZE)

void rt_hw_board_init(void);

#endif /* BOARD_H__ */
