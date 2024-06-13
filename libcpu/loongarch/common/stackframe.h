/*
 * Copyright (c) 2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-13     Feiyang Chen the first version
 */

#ifndef STACKFRAME_H__
#define STACKFRAME_H__

#include "asm.h"
#include "loongarch.h"

#define FRAME_R0        (0 * SIZE)
#define FRAME_R1        (1 * SIZE)
#define FRAME_R2        (2 * SIZE)
#define FRAME_R3        (3 * SIZE)
#define FRAME_R4        (4 * SIZE)
#define FRAME_R5        (5 * SIZE)
#define FRAME_R6        (6 * SIZE)
#define FRAME_R7        (7 * SIZE)
#define FRAME_R8        (8 * SIZE)
#define FRAME_R9        (9 * SIZE)
#define FRAME_R10       (10 * SIZE)
#define FRAME_R11       (11 * SIZE)
#define FRAME_R12       (12 * SIZE)
#define FRAME_R13       (13 * SIZE)
#define FRAME_R14       (14 * SIZE)
#define FRAME_R15       (15 * SIZE)
#define FRAME_R16       (16 * SIZE)
#define FRAME_R17       (17 * SIZE)
#define FRAME_R18       (18 * SIZE)
#define FRAME_R19       (19 * SIZE)
#define FRAME_R20       (20 * SIZE)
#define FRAME_R21       (21 * SIZE)
#define FRAME_R22       (22 * SIZE)
#define FRAME_R23       (23 * SIZE)
#define FRAME_R24       (24 * SIZE)
#define FRAME_R25       (25 * SIZE)
#define FRAME_R26       (26 * SIZE)
#define FRAME_R27       (27 * SIZE)
#define FRAME_R28       (28 * SIZE)
#define FRAME_R29       (29 * SIZE)
#define FRAME_R30       (30 * SIZE)
#define FRAME_R31       (31 * SIZE)
#define FRAME_ERA       (32 * SIZE)
#define FRAME_PRMD      (33 * SIZE)
#define FRAME_SIZE      (34 * SIZE)

#ifdef __ASSEMBLY__

    .macro BACKUP_T0T1
    csrwr   $t0, LOONGARCH_CSR_KS0
    csrwr   $t1, LOONGARCH_CSR_KS1
    .endm

    .macro RELOAD_T0T1
    csrrd   $t0, LOONGARCH_CSR_KS0
    csrrd   $t1, LOONGARCH_CSR_KS1
    .endm

    .macro SAVE_TEMP
    RELOAD_T0T1
    ST      $t0, $sp, FRAME_R12
    ST      $t1, $sp, FRAME_R13
    ST      $t2, $sp, FRAME_R14
    ST      $t3, $sp, FRAME_R15
    ST      $t4, $sp, FRAME_R16
    ST      $t5, $sp, FRAME_R17
    ST      $t6, $sp, FRAME_R18
    ST      $t7, $sp, FRAME_R19
    ST      $t8, $sp, FRAME_R20
    .endm

    .macro SAVE_STATIC
    ST      $s0, $sp, FRAME_R23
    ST      $s1, $sp, FRAME_R24
    ST      $s2, $sp, FRAME_R25
    ST      $s3, $sp, FRAME_R26
    ST      $s4, $sp, FRAME_R27
    ST      $s5, $sp, FRAME_R28
    ST      $s6, $sp, FRAME_R29
    ST      $s7, $sp, FRAME_R30
    ST      $s8, $sp, FRAME_R31
    .endm

    .macro SAVE_SOME
    move    $t0, $sp
    ADDI    $sp, $sp, -FRAME_SIZE
    ST      $t0, $sp, FRAME_R3
    ST      $zero, $sp, FRAME_R0
    csrrd   $t0, LOONGARCH_CSR_PRMD
    ST      $t0, $sp, FRAME_PRMD
    ST      $ra, $sp, FRAME_R1
    ST      $a0, $sp, FRAME_R4
    ST      $a1, $sp, FRAME_R5
    ST      $a2, $sp, FRAME_R6
    ST      $a3, $sp, FRAME_R7
    ST      $a4, $sp, FRAME_R8
    ST      $a5, $sp, FRAME_R9
    ST      $a6, $sp, FRAME_R10
    ST      $a7, $sp, FRAME_R11
    csrrd   $ra, LOONGARCH_CSR_ERA
    ST      $ra, $sp, FRAME_ERA
    ST      $tp, $sp, FRAME_R2
    ST      $r21, $sp, FRAME_R21
    ST      $fp, $sp, FRAME_R22
    .endm

    .macro SAVE_ALL
    SAVE_SOME
    SAVE_TEMP
    SAVE_STATIC
    .endm

    .macro RESTORE_TEMP
    LD      $t0, $sp, FRAME_R12
    LD      $t1, $sp, FRAME_R13
    LD      $t2, $sp, FRAME_R14
    LD      $t3, $sp, FRAME_R15
    LD      $t4, $sp, FRAME_R16
    LD      $t5, $sp, FRAME_R17
    LD      $t6, $sp, FRAME_R18
    LD      $t7, $sp, FRAME_R19
    LD      $t8, $sp, FRAME_R20
    .endm

    .macro RESTORE_STATIC
    LD      $s0, $sp, FRAME_R23
    LD      $s1, $sp, FRAME_R24
    LD      $s2, $sp, FRAME_R25
    LD      $s3, $sp, FRAME_R26
    LD      $s4, $sp, FRAME_R27
    LD      $s5, $sp, FRAME_R28
    LD      $s6, $sp, FRAME_R29
    LD      $s7, $sp, FRAME_R30
    LD      $s8, $sp, FRAME_R31
    .endm

    .macro RESTORE_SOME
    LD      $a0, $sp, FRAME_ERA
    csrwr   $a0, LOONGARCH_CSR_ERA
    LD      $a0, $sp, FRAME_PRMD
    csrwr   $a0, LOONGARCH_CSR_PRMD
    LD      $ra, $sp, FRAME_R1
    LD      $tp, $sp, FRAME_R2
    LD      $a0, $sp, FRAME_R4
    LD      $a1, $sp, FRAME_R5
    LD      $a2, $sp, FRAME_R6
    LD      $a3, $sp, FRAME_R7
    LD      $a4, $sp, FRAME_R8
    LD      $a5, $sp, FRAME_R9
    LD      $a6, $sp, FRAME_R10
    LD      $a7, $sp, FRAME_R11
    LD      $r21, $sp, FRAME_R21
    LD      $fp, $sp, FRAME_R22
    .endm

    .macro RESTORE_SP_AND_RET
    LD      $sp, $sp, FRAME_R3
    ertn
    .endm

    .macro RESTORE_ALL_AND_RET
    RESTORE_STATIC
    RESTORE_TEMP
    RESTORE_SOME
    RESTORE_SP_AND_RET
    .endm

#endif /* __ASSEMBLY__ */

#endif /* STACKFRAME_H__ */
