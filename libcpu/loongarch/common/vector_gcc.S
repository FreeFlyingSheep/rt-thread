#include "asm.h"
#include "stackframe.h"

    .section ".vectors"
    .globl trap_entry
trap_entry:
    BACKUP_T0T1
    SAVE_ALL

    move     $s0, $sp           /* save sp */

    la.pcrel $sp, _system_stack /* switch to kernel stack */

    bl       handle_trap

    move     $sp, $s0           /* restore sp */

    /*
     * switch to the new thread if necessary
     */
    la.pcrel $t0, rt_thread_switch_interrupt_flag
    LD       $t1, $t0, 0
    beqz     $t1, spurious_interrupt

    ST       $zero, $t0, 0

    la.pcrel $t0, rt_interrupt_from_thread
    LD       $t1, $t0, 0
    ST       $sp, $t1, 0

    la.pcrel $t0, rt_interrupt_to_thread
    LD       $t1, $t0, 0
    LD       $sp, $t1, 0

spurious_interrupt:
    RESTORE_ALL_AND_RET
