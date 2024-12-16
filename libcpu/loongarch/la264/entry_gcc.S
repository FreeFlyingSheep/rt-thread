#include "addr.h"
#include "asm.h"
#include "loongarch.h"

    .text
    .section ".startup"
    .globl _start
_start:
    /* disable interrupt */
    LI       $t0, CSR_CRMD_IE
    csrxchg  $zero, $t0, LOONGARCH_CSR_CRMD

    /* config addressing mode */
    LI       $t0, CSR_CRMD_PG
    LI       $t1, CSR_CRMD_DA | CSR_CRMD_PG
    csrxchg  $t0, $t1, LOONGARCH_CSR_CRMD

    /* config direct window */
    LI       $t0, CSR_DMW0_INIT
    csrwr    $t0, LOONGARCH_CSR_DMWIN0
    LI       $t0, CSR_DMW1_INIT
    csrwr    $t0, LOONGARCH_CSR_DMWIN1

    /* jump to link address */
    la.abs   $t0, 1f
    jr       $t0
1:

    /* clear bss */
    la.pcrel $t0, __bss_start
    la.pcrel $t1, __bss_end
2:
    ST       $zero, $t0, 0
    ADDI     $t0, $t0, 8
    blt      $t0, $t1, 2b

    /* initialize sp */
    la.pcrel $sp, _system_stack

    /* jump to RT-Thread kernel */
    bl       rtthread_startup
    /* never reach here */

    /* enable interrupt */
    LI       $t0, CSR_CRMD_IE
    csrxchg  $t0, $t0, LOONGARCH_CSR_CRMD

    idle     0
