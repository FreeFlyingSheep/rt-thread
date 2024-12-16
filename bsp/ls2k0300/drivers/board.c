#include <rthw.h>
#include <rtthread.h>

#include "board.h"
#include "exception.h"
#include "interrupt.h"

#include "addr.h"

void rt_hw_board_init(void)
{
    rt_hw_exception_init();
    rt_hw_interrupt_init();

#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif

#ifdef RT_USING_CONSOLE
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
#endif

#ifdef RT_USING_HEAP
    rt_system_heap_init((void *)RT_HW_HEAP_BEGIN, (void *)RT_HW_HEAP_END);
#endif
}
