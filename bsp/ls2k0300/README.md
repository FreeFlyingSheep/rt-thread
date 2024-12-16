# 移植 RT-Thread 到久久派

本文将介绍如何从零开始，将 RT-Thread 移植到久久派。
本文假定读者拥有一定的操作系统和嵌入式开发知识，以及知晓如何在 Linux 上编译开发。

代码仓库：<https://github.com/FreeFlyingSheep/rt-thread/tree/ls2k0300>。

## 参考资料

本文到的资料主要分以下三类：

- 久久派相关的资料可以通过中科云论坛（<https://bbs.ctcisz.com/forum.php>）和龙芯久久派官方交流 QQ 群（群号 876920347）获得。
- LoongArch 相关的资料可以在龙芯官网的下载平台（<https://www.loongson.cn/download/index>）获得。
- RT-Thread 相关的资料可以在 RT-Thread 官网的文档中心获得（<https://www.rt-thread.org/document/site/#/>）。

说实话，久久派相关的开发资料获取有一定难度，并不是所有资料都直接对外公开，部分资料需要进群才能获得。

## 启动流程

在开始正式的移植前，我们先把 RT-Thread 的启动流程熟悉一下。

### 处理器核初始化

处理器核初始化的代码位于 `libcpu` 的各个架构下，它是 RT-Thread 最早执行的代码。

这部分代码都由汇编编写，这时候 C 环境还没建立（BSS 段还没清空，栈还没初始化，内核也未必处于自己希望的内存地址）。

因此，这部分代码可能涉及寄存器的初始化、内核的重定位、BSS 段段清空、栈的初始化、跳转到公共代码初始化代码等。

### 板级初始化

这里我们假定启用了 `RT_USING_USER_MAIN` 配置，它会让 RT-Thread 最终调用户自定的 `main()` 函数。

在跳转到公共初始化代码（即 `rtthread_startup()`）后，公共代码会立刻调用板级初始化函数 `rt_hw_board_init()`。
该函数是在 `bsp` 的对应板卡目录下定义的。

板级初始化通常会包括中断和 Tick 的初始化，以及各种驱动的初始化，这里需要考虑中断处理。

### 其他部分初始化

`rt_hw_board_init()` 返回后，回到 `rtthread_startup()`。
`rtthread_startup()` 继续完成内核的初始化工作，剩下的基本都是公共代码。

初始化工作的最后，我们需要跳转到用户的 `main()` 函数（开始跑用户应用程序），这里需要考虑上下文切换和线程栈。

### 移植步骤

*那么我们需要做什么呢？*

`documentation/kernel-porting/kernel-porting.md` 文档就描述了移植需要实现的内容。

抛开这个文档，我们从启动流程来看，也能明确我们的移植流程：

1. 搭建构建环境。
2. 实现处理器核初始化。
3. 实现陷阱（包括中断和异常）处理。
4. 实现上下文切换。
5. 实现线程栈。
6. 实现板级初始化。
7. 实现基本的驱动。
8. 实现用户应用程序。
9. 添加、完善功能。

为了编译测试，以及方便调试，这些步骤可能相互穿插。

## 准备工作

RT-Thread 的构建系统是 SCons。RT-Thread 官方贴心地提供了 Env 工具来帮助用户在 Windows 平台上开发。这个工具其实就是集成了 MingW 环境下编译的 GNU 工具链，Python 和 SCons 等工具。**显然，这个工具不支持我们要移植的板卡。**

目前大部分开发者使用的工具链是 LoongIDE 自带的 GNU 工具链。这需要开发者安装 LoongIDE（<http://www.loongide.com/content/article.asp?style=nodate&typeid=26&id=149>），而这个工具链是定制过且不开源的。

虽然我们能够通过使用 LoongIDE 自带的 GNU 工具链结合 RT-Thread 官方的 Env 工具来在 Windows 下开发，但是我们并不知道这个工具链魔改了哪些东西，而且我的生产工具是 Mac M2 芯片的，根本没法用这个工具链。于是，我决定完全使用上游代码，从头构建一个编译环境。

既然要从头构建，为了方便起见，我们将在 Linux 平台上编译开发。如果你使用的是 Windows，那我推荐使用 WSL（<https://learn.microsoft.com/en-us/windows/wsl/install>）来安装 Linux 虚拟机；如果你使用的是 Mac，那我推荐使用 OrbStack（<https://orbstack.dev/>）来安装一个 Linux 虚拟机。

### 构建交叉编译工具链

因为我们需要在 x86-64 或者 aarch64 平台上交叉编译 64 位 LoongArch 的代码，所以我们需要一个交叉编译工具链。而运行 RT-Thread 的环境显然不可能有 C 库（glibc）的支持，因此这里必须使用裸机版本的工具链，即 `loongarch64-unknown-elf-xxx`。

制作一个这样的工具链需要交叉编译 binutils、gcc 以及 newlib。binutils 和 gcc 可以直接从上游获取，但 newlib 但上游仍然不支持 LoongArch。这里可以使用我移植的一个最简版本的 newlib（<https://github.com/FreeFlyingSheep/newlib>）。

编译的参考脚本如下：

```bash
#!/bin/bash
set -e

PREFIX=~/cross-tools
JOBS=4

pushd binutils-gdb
    rm -rf build
    mkdir build
    pushd build
        ../configure --prefix=${PREFIX} --target=loongarch64-unknown-elf
        make -j${JOBS}
        make install
    popd
popd

pushd gcc
    rm -rf build
    mkdir build
    pushd build
        ../configure --prefix=${PREFIX} --target=loongarch64-unknown-elf --without-headers --with-newlib --with-gnu-as --with-gnu-ld -enable-languages=c,c++
        make -j${JOBS} all-gcc all-target-libgcc
        make install-gcc install-target-libgcc
    popd
popd

rm -rf newlib-loongarch64-elf
mkdir newlib-loongarch64-elf
pushd newlib-loongarch64-elf
    ../newlib-cygwin/configure --prefix=${PREFIX} --target=loongarch64-unknown-elf
    make -j${JOBS}
    make install
popd

pushd gcc
    rm -rf build
    mkdir build
    pushd build
        ../configure --prefix=${PREFIX} --target=loongarch64-unknown-elf --with-newlib --with-gnu-as --with-gnu-ld --enable-languages=c,c++ --disable-shared
        make -j${JOBS}
        make install
    popd
popd
```

顺利编译完成后你应该能在安装目录下找到 `loongarch64-unknown-elf-` 开头的一系列二进制文件。

### 安装 SCons

系统默认情况应该已经安装了 Python，如果没有，推荐直接安装最新版本的 Python 3。

1. 切换到 RT-Thread 目录，使用 `python -m venv .venv`（部分发行版可能需要使用 `python3 -m venv .venv`）创建虚拟环境。
2. 执行 `source .venv/bin/activate` 启用该环境。
3. 执行 `pip install kconfiglib SCons` 安装 menuconfig、SCons 等工具。

当然你也可以直接用系统的包管理工具来全局安装。

## 构建 RT-Thread

只考虑我们需要移植的部分，RT-Thread 的项目结构如下：

```text
rt-thread
├── bsp
│   └── xxx                 # 平台名称
│       ├── applications    # 应用程序目录
│       │   ├── SConscript  # （当前目录）构建脚本
│       │   └── ...
│       ├── drivers         # 驱动目录
│       │   ├── SConscript
│       │   └── ...
│       ├── Kconfig         # 配置文件
│       ├── link.lds        # 链接脚本
│       ├── README.md       # 说明文档
│       ├── rtconfig.py     # 编译器配置文件
│       ├── SConscript
│       ├── SConstruct      # 总构建脚本
│       └── ...
└── libcpu
│   ├── xxx                 # 架构名称
│   │   ├── common          # 该架构通用的代码
│   │   │   ├── SConscript
│   │   │   └── ...
│   │   ├── xxx             # 处理器核名称
│   │   │   ├── SConscript
│   │   │   └── ...
│   │   ├── SConscript
│   │   └── ...
│   └── ...
└── ...
```

在完成准备工作后，我们要做的第一件事就是按照这个项目结构，新建对应的文件，填上一些必要的代码（现阶段具体的实现只需要用空函数就够了），最终让项目顺利构建。

### 新建文件

我们要移植的久久派是 ls2k0300 平台的，其处理器核为 LA264。

因此我们此刻的项目结构如下（总共 11 个新文件）：

```text
rt-thread
├── bsp
│   ├── ls2k0300
│   │   ├── applications
│   │   │   └── SConscript
│   │   ├── drivers
│   │   │   └── SConscript
│   │   ├── Kconfig
│   │   ├── link.lds
│   │   ├── README.md
│   │   ├── rtconfig.py
│   │   ├── SConscript
│   │   └── SConstruct
│   └── ...
└── libcpu
│   ├── loongarch
│   │   ├── common
│   │   │   └── SConscript
│   │   ├── la264
│   │   │   └── SConscript
│   │   └── SConscript
│   └── ...
└── ...
```

### 完善构建系统相关的文件

与构建系统直接相关的文件是 `rtconfig.py`、`SConscript` 和 `SConstruct`。大部分文件都能直接抄 risc-v 的。

`bsp/ls2k0300/rtconfig.py` 可以参考 `bsp/qemu-virt64-riscv/rtconfig.py`：

```python
import os

# toolchains options
ARCH        ='loongarch'
CPU         ='la264'
CROSS_TOOL  ='gcc'

RTT_ROOT = os.getenv('RTT_ROOT') or os.path.join(os.getcwd(), '..', '..')

if os.getenv('RTT_CC'):
    CROSS_TOOL = os.getenv('RTT_CC')

if  CROSS_TOOL == 'gcc':
    PLATFORM    = 'gcc'
    EXEC_PATH   = os.getenv('RTT_EXEC_PATH') or '/usr/bin'
else:
    print('Please make sure your toolchains is GNU GCC!')
    exit(0)

BUILD = 'debug'

if PLATFORM == 'gcc':
    # toolchains
    PREFIX  = os.getenv('RTT_CC_PREFIX') or 'loongarch64-unknown-elf-'
    CC      = PREFIX + 'gcc'
    CXX     = PREFIX + 'g++'
    AS      = PREFIX + 'gcc'
    AR      = PREFIX + 'ar'
    LINK    = PREFIX + 'gcc'
    TARGET_EXT = 'elf'
    SIZE    = PREFIX + 'size'
    OBJDUMP = PREFIX + 'objdump'
    OBJCPY  = PREFIX + 'objcopy'

    DEVICE  = ''
    CFLAGS  = DEVICE + ' -Wall'
    AFLAGS  = ' -c' + DEVICE + ' -x assembler-with-cpp -D__ASSEMBLY__ -Wall'
    LFLAGS  = DEVICE + ' -nostartfiles -Wl,--no-warn-rwx-segments -Wl,--gc-sections,-Map=rtthread.map,-cref,-u,_start -T link.lds'
    CPATH   = ''
    LPATH   = ''

    if BUILD == 'debug':
        CFLAGS += ' -O0 -ggdb'
        AFLAGS += ' -ggdb'
    else:
        CFLAGS += ' -O2 -Os'

    CXXFLAGS = CFLAGS

DUMP_ACTION = OBJDUMP + ' -D -S $TARGET > rtthread.asm\n'
POST_ACTION = OBJCPY + ' -O binary $TARGET rtthread.bin\n' + SIZE + ' $TARGET \n'
```

`bsp/ls2k0300/SConstruct` 可以完全照搬 `bsp/qemu-vexpress-a9/SConstruct`（为什么不抄 risc-v？因为 risc-v 的这个文件过于复杂了）：

```python
import os
import sys
import rtconfig

if os.getenv('RTT_ROOT'):
    RTT_ROOT = os.getenv('RTT_ROOT')
else:
    RTT_ROOT = os.path.join(os.getcwd(), '..', '..')

sys.path = sys.path + [os.path.join(RTT_ROOT, 'tools')]
from building import *

TARGET = 'rtthread.' + rtconfig.TARGET_EXT

DefaultEnvironment(tools=[])
env = Environment(tools = ['mingw'],
    AS   = rtconfig.AS, ASFLAGS = rtconfig.AFLAGS,
    CC   = rtconfig.CC, CFLAGS = rtconfig.CFLAGS,
    CXX  = rtconfig.CXX, CXXFLAGS = rtconfig.CXXFLAGS,
    AR   = rtconfig.AR, ARFLAGS = '-rc',
    LINK = rtconfig.LINK, LINKFLAGS = rtconfig.LFLAGS)
env.PrependENVPath('PATH', rtconfig.EXEC_PATH)
env['ASCOM'] = env['ASPPCOM']

Export('RTT_ROOT')
Export('rtconfig')

# prepare building environment
objs = PrepareBuilding(env, RTT_ROOT)

if GetDepend('RT_USING_SMART'):
    # use smart link.lds
    env['LINKFLAGS'] = env['LINKFLAGS'].replace('link.lds', 'link_smart.lds')

# make a building
DoBuilding(TARGET, objs)
```

`SConscript` 文件比较简单，参考 risc-v 对应的文件，去掉不必要的内容，保留最简单的形式。

`bsp/ls2k0300/applications/SConscript`：

```python
# RT-Thread building script for component

from building import *

cwd     = GetCurrentDir()
src     = Glob('*.c') + Glob('*.cpp') + Glob('*_gcc.S')
CPPPATH = [cwd]

group = DefineGroup('Applications', src, depend = [''], CPPPATH = CPPPATH)

Return('group')
```

`bsp/ls2k0300/drivers/SConscript`：

```python
# RT-Thread building script for component

from building import *

cwd     = GetCurrentDir()
src     = Glob('*.c') + Glob('*.cpp') + Glob('*_gcc.S')
CPPPATH = [cwd]

group = DefineGroup('Drivers', src, depend = [''], CPPPATH = CPPPATH)

Return('group')
```

`bsp/ls2k0300/SConscript`：

```python
# RT-Thread building script for module

import os
from building import *

cwd = GetCurrentDir()
objs = []
list = os.listdir(cwd)

for d in list:
    path = os.path.join(cwd, d)
    if os.path.isfile(os.path.join(path, 'SConscript')):
        objs = objs + SConscript(os.path.join(d, 'SConscript'))

Return('objs')
```

`libcpu/loongarch/common/SConscript` 和 `libcpu/loongarch/la264/SConscript`：

```python
# RT-Thread building script for component

from building import *

cwd     = GetCurrentDir()
src     = Glob('*.c') + Glob('*.cpp') + Glob('*_gcc.S')
CPPPATH = [cwd]

group = DefineGroup('libcpu', src, depend = [''], CPPPATH = CPPPATH)

Return('group')
```

`libcpu/loongarch/SConscript`：

```python
# RT-Thread building script for bridge

import os
from building import *

Import('rtconfig')

cwd   = GetCurrentDir()
group = []
list  = os.listdir(cwd)

# add common code files
group += SConscript(os.path.join('common', 'SConscript'))

# cpu porting code files
if rtconfig.CPU in list:
    group = group + SConscript(os.path.join(rtconfig.CPU, 'SConscript'))

Return('group')
```

### 生成配置文件

我们还需要完善 `Kconfig` 文件来对项目进行配置（参考其他平台，只保留最简代码）：

```text
mainmenu "RT-Thread Configuration"

BSP_DIR := .

RTT_DIR := ../..

PKGS_DIR := packages

source "$(RTT_DIR)/Kconfig"
osource "$PKGS_DIR/Kconfig"
```

之后就可以尝试构建项目了：

1. 进入当前平台目录（`cd bsp/ls2k0300`）。
2. 执行 `export RTT_EXEC_PATH="~/cross-tools/bin"` 设置工具链路径。
3. 新建`rtconfig.h`
4. 执行 `scons --menuconfig`，什么也不需要更改，保存退出。
5. 确认 `rtconfig.h` 已经被更新，且生产了新的 `.config` 文件。

*理论上直接执行 `scons --menuconfig` 会自动生成 `rtconfig.h`，但目前的构建脚本逻辑很奇怪，会直接报错找不到该文件。我严重怀疑是社区改出问题了，暂时先用这种方绕过。*

### 尝试编译

尝试执行 `scons` 构建项目，项目应该能正常编译，并最终会产生 `loongarch64-unknown-elf-objcopy: error: the input file 'rtthread.elf' has no sections` 的报错。

## LoongArch 架构移植 - 初始化

在初步完善了构建系统后，我们可以开始对 LoongArch 架构进行移植。

### 添加 LoongArch 相关的定义

参考其他架构，我们在 `libcpu/Kconfig` 末尾加入三个配置选项：

```text
config ARCH_LOONGARCH
    bool

config ARCH_LOONGARCH32
    bool
    select ARCH_LOONGARCH

config ARCH_LOONGARCH64
    bool
    select ARCH_LOONGARCH
    select ARCH_CPU_64BIT
```

如果是 32 位指令集，那么 `ARCH_LOONGARCH` 将被选上；如果是 64 位指令集，那么 `ARCH_LOONGARCH` 和 `ARCH_CPU_64BIT` 将被选上。

### 实现初始化代码

初始化代码位于 `libcpu/loongarch/la264/entry_gcc.S`（RT-Thread 中使用 GCC 编译的汇编文件通常要加上 `_gcc` 后缀）：

```text
#include "loongarch.h"

    .text
    .section ".startup"
    .globl _start
_start:
    /* disable interrupt */
    li.d     $t0, CSR_CRMD_IE
    csrxchg  $zero, $t0, LOONGARCH_CSR_CRMD

    /* config addressing mode */
    li.d     $t0, CSR_CRMD_PG
    li.d     $t1, CSR_CRMD_DA | CSR_CRMD_PG
    csrxchg  $t0, $t1, LOONGARCH_CSR_CRMD

    /* config direct window */
    li.d     $t0, 0x8000000000000001
    csrwr    $t0, LOONGARCH_CSR_DMWIN0
    li.d     $t0, 0x9000000000000011
    csrwr    $t0, LOONGARCH_CSR_DMWIN1

    /* jump to link address */
    la.abs   $t0, 1f
    jr       $t0
1:

    /* clear bss */
    la.pcrel $t0, __bss_start
    la.pcrel $t1, __bss_end
2:
    st.d     $zero, $t0, 0
    addi.d   $t0, $t0, 8
    blt      $t0, $t1, 2b

    /* initialize sp */
    la.pcrel $sp, _system_stack

    /* jump to RT-Thread kernel */
    bl       rtthread_startup
    /* never reach here */

    /* enable interrupt */
    li.d     $t0, CSR_CRMD_IE
    csrxchg  $t0, $t0, LOONGARCH_CSR_CRMD

    idle     0
```

初始化是最早执行的代码，它将完成以下工作：

1. 关中断。
2. 设置 MMU 为直接映射地址翻译模式。
3. 设置直接映射配置窗口。
4. 跳转到链接地址。
5. 清理 BSS。
6. 初始化栈指针。
7. 跳转到 RT-Thread 初始化函数。

理论上初始化函数永远不会返回，如果返回，说明出现了无法恢复的严重错误，那么：

1. 开中断。
2. 让处理器核进入等待状态（放弃治疗，啥也不干）。

理论上 CPU 上电是处于关中断状态的，但 RT-Thread 也可能是从 PMON 或者 U-Boot 启动（它们还会配置 MMU 和其他配置寄存器），所以我们要一律重新初始化，确保系统状态和我们预期一致。

为了简化，我们采用了直接映射地址翻译模式，配置 `DWM0` 和 `DWM1` 使映射方式如下表（参考 LoongArch 手册卷一第五章）：

| 虚拟地址 | 物理地址 | 存储访问类型 |
| --- | --- | --- |
| `0x8000000000000000` - `0x8000ffffffffffff` | `0x0000000000000000` - `0x0000ffffffffffff` | 强续非缓存 |
| `0x9000000000000000` - `0x9000ffffffffffff` | `0x0000000000000000` - `0x0000ffffffffffff` | 一致可缓存 |

这样，我们只需要知道 `0x8000` 开头的地址是不缓存的， `0x9000` 开头的地址是缓存的，暂时不用考虑页表和 TLB 相关的异常处理。

头文件 `libcpu/loongarch/common/loongarch.h` 包含了常见的配置寄存器，目前我们只需要以下这些：

```c
#ifndef LOONGARCH_H__
#define LOONGARCH_H__

#ifdef __ASSEMBLY__
#define BIT(x) (1 << (x))
#else
#define BIT(x) (1UL << (x))
#endif

#define LOONGARCH_CSR_CRMD 0x0
#define  CSR_CRMD_IE       BIT(2)
#define  CSR_CRMD_DA       BIT(3)
#define  CSR_CRMD_PG       BIT(4)

#define LOONGARCH_CSR_DMWIN0 0x180
#define LOONGARCH_CSR_DMWIN1 0x181

#endif /* LOONGARCH_H__ */
```

我们通过 `__ASSEMBLY__` 宏来区别汇编和 C 代码（汇编器并不认得 `UL`）。

### 实现链接脚本

链接脚本位于 `bsp/ls2k0300/link.lds`，它是板级的（位于 `bsp` 下），不属于架构移植，但为了测试我们的代码，必须要编译运行，所以我们在这里先提供它：

```text
OUTPUT_FORMAT("elf64-loongarch", "elf64-loongarch", "elf64-loongarch")
OUTPUT_ARCH(loongarch)

ENTRY(_start)

SECTIONS
{
    . = 0x9000000000200000;
    .text :
    {
        *(.startup)

        *(.text)
        *(.text.*)

        *(.rodata)
        *(.rodata.*)
        *(.rodata1)
        *(.rodata1.*)

        /* section information for finsh shell */
        . = ALIGN(8);
        __fsymtab_start = .;
        KEEP(*(FSymTab))
        __fsymtab_end = .;
        . = ALIGN(8);
        __vsymtab_start = .;
        KEEP(*(VSymTab))
        __vsymtab_end = .;
        . = ALIGN(8);

        . = ALIGN(8);
        __rt_init_start = .;
        KEEP(*(SORT(.rti_fn*)))
        __rt_init_end = .;
        . = ALIGN(8);

        . = ALIGN(8);
        __rt_utest_tc_tab_start = .;
        KEEP(*(UtestTcTab))
        __rt_utest_tc_tab_end = .;
        . = ALIGN(8);
    }

    .eh_frame_hdr :
    {
         *(.eh_frame_hdr)
         *(.eh_frame_entry)
    }
    .eh_frame : ONLY_IF_RO { KEEP (*(.eh_frame)) }

    . = ALIGN(8);
    .data :
    {
         *(.data)
         *(.data.*)

         *(.data1)
         *(.data1.*)

        . = ALIGN(8);
        *(.sdata)
        *(.sdata.*)
    }

    .stack :
    {
        . = ALIGN(16);
        _system_stack_start = .;
        . += 0x1000;
        _system_stack = .;
    }

    .sbss :
    {
        __bss_start = .;
        *(.sbss)
        *(.sbss.*)
        *(.dynsbss)
        *(.scommon)
    }

    .bss :
    {
        *(.bss)
        *(.bss.*)
        *(.dynbss)
        *(COMMON)
        __bss_end = .;
    }
    _end = .;

    /* Stabs debugging sections.  */
    .stab          0 : { *(.stab) }
    .stabstr       0 : { *(.stabstr) }
    .stab.excl     0 : { *(.stab.excl) }
    .stab.exclstr  0 : { *(.stab.exclstr) }
    .stab.index    0 : { *(.stab.index) }
    .stab.indexstr 0 : { *(.stab.indexstr) }
    .comment       0 : { *(.comment) }
    /* DWARF debug sections.
     * Symbols in the DWARF debugging sections are relative to the beginning
     * of the section so we begin them at 0.  */
    /* DWARF 1 */
    .debug          0 : { *(.debug) }
    .line           0 : { *(.line) }
    /* GNU DWARF 1 extensions */
    .debug_srcinfo  0 : { *(.debug_srcinfo) }
    .debug_sfnames  0 : { *(.debug_sfnames) }
    /* DWARF 1.1 and DWARF 2 */
    .debug_aranges  0 : { *(.debug_aranges) }
    .debug_pubnames 0 : { *(.debug_pubnames) }
    /* DWARF 2 */
    .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
    .debug_abbrev   0 : { *(.debug_abbrev) }
    .debug_line     0 : { *(.debug_line) }
    .debug_frame    0 : { *(.debug_frame) }
    .debug_str      0 : { *(.debug_str) }
    .debug_loc      0 : { *(.debug_loc) }
    .debug_macinfo  0 : { *(.debug_macinfo) }
    /* SGI/MIPS DWARF 2 extensions */
    .debug_weaknames 0 : { *(.debug_weaknames) }
    .debug_funcnames 0 : { *(.debug_funcnames) }
    .debug_typenames 0 : { *(.debug_typenames) }
    .debug_varnames  0 : { *(.debug_varnames) }
}
```

链接脚本的大部分代码都和其他架构相同，值得注意的是我们把链接地址放在 `0x9000000000200000`，并且将 `.startup` 段（启动部分代码）放置于该地址。

### 继续尝试编译

执行 `scons`，会发现一堆 `undefined reference` 报错，我们需要先把这些函数补全（函数主体留空即可），才能完成编译。
我们也可以在 `documentation/kernel-porting/kernel-porting.md` 中找到这些函数的原型。

`libcpu/loongarch/common/interrupt.c`：

```c
#include <rtthread.h>

rt_ubase_t rt_thread_switch_interrupt_flag;
rt_ubase_t rt_interrupt_from_thread;
rt_ubase_t rt_interrupt_to_thread;

rt_base_t rt_hw_interrupt_disable(void)
{
    // TODO
}

void rt_hw_interrupt_enable(rt_base_t level)
{
    // TODO
}
```

`libcpu/loongarch/common/context_gcc.S`：

```text
/* void rt_hw_context_switch_to(rt_uint32 to) */
    .globl rt_hw_context_switch_to
rt_hw_context_switch_to:
    // TODO

/* void rt_hw_context_switch(rt_uint32 from, rt_uint32 to) */
    .globl rt_hw_context_switch
rt_hw_context_switch:
    // TODO

/* void rt_hw_context_switch_interrupt(rt_uint32 from, rt_uint32 to) */
    .globl rt_hw_context_switch_interrupt
rt_hw_context_switch_interrupt:
    // TODO
```

`libcpu/loongarch/common/stack.c`：

```c
#include <rtthread.h>

rt_uint8_t *rt_hw_stack_init(void *tentry, void *parameter, rt_uint8_t *stack_addr, void *texit)
{
    // TODO
}
```

再次执行 `scons`，会报错 ``undefined reference to `rtthread_startup'``，这是因为 RT-Thread 的公共代码并未被编译，我们需要在 `bsp/ls2k0300/Kconfig` 末尾加上以下内容：

```text
config SOC_LA264
    bool
    select ARCH_LOONGARCH64
    select RT_USING_COMPONENTS_INIT
    select RT_USING_USER_MAIN
    default y
```

执行 `scons --menuconfig`，保存退出，让配置生效。
然后再次执行 `scons`，这时候会发现缺少 `main` 和 `rt_hw_board_init`。
同样，补上这两个函数。

`bsp/ls2k0300/applications/main.c`：

```c
#include <stdio.h>

int main(void)
{
    printf("Welcome to RT-Thread/LoongArch64!\n");
    return 0;
}
```

`bsp/ls2k0300/drivers/board.c`：

```c
#include <rtthread.h>

void rt_hw_board_init(void)
{
    // TODO
}
```

再次执行 `scons`，应该能顺利编译生成 `rtthread.elf`。

### 测试启动代码

为了能确定我们的代码能正常工作，我们需要往串口塞个字符，查阅 2k0300 手册可以知道串口地址 （UART0）为 `0x16100000`。
我们可以在启动文件中 `bl rtthread_startup` 前加入以下代码：

```text
li.d     $t0, '0'
li.d     $t1, 0x8000000016100000
st.d     $t0, $t1, 0
```

再次编译，启动久久派验证。
如果一切顺利，串口应该会输出 `0`。

## LoongArch 架构移植 - 陷阱

在实现了初始化代码后，我们需要实现陷阱的通用代码，包括基本的中断和异常处理。

由于具体的中断是板级相关的，我们在这里只需要遵循 LoongArch 架构手册来实现公共部分，其余部分由板级初始化时完成。

### 中断

根据手册，线中断共13个。
我们在 `libcpu/loongarch/common/interrupt.h` 中定义这些宏：

```c
#define LOONGARCH_INT_SWI0      0
#define LOONGARCH_INT_SWI1      1
#define LOONGARCH_INT_HWI0      2
#define LOONGARCH_INT_HWI1      3
#define LOONGARCH_INT_HWI2      4
#define LOONGARCH_INT_HWI3      5
#define LOONGARCH_INT_HWI4      6
#define LOONGARCH_INT_HWI5      7
#define LOONGARCH_INT_HWI6      8
#define LOONGARCH_INT_HWI7      9
#define LOONGARCH_INT_PMI       10
#define LOONGARCH_INT_TI        11
#define LOONGARCH_INT_IPI       12

#define LOONGARCH_INT_NUM       13
```

然后在 `libcpu/loongarch/common/interrupt.c` 中把相关的代码补上：

```c
static const char *irq_name[LOONGARCH_INT_NUM] =
{
    "SWI0",
    "SWI1",
    "HWI0",
    "HWI1",
    "HWI2",
    "HWI3",
    "HWI4",
    "HWI5",
    "HWI6",
    "HWI7",
    "PMI",
    "TI",
    "IPI",
};

static struct rt_irq_desc irq_desc[LOONGARCH_INT_NUM];

rt_base_t rt_hw_interrupt_disable(void)
{
    return __csrxchg_w(0, CSR_CRMD_IE, LOONGARCH_CSR_CRMD);
}

void rt_hw_interrupt_enable(rt_base_t level)
{
    __csrwr_w(level, LOONGARCH_CSR_CRMD);
}

void rt_hw_interrupt_mask(int vector)
{
    RT_ASSERT(vector >= 0 && vector < LOONGARCH_INT_NUM);
    __csrxchg_w(0 << vector, 1 << vector, LOONGARCH_CSR_ECFG);
}

void rt_hw_interrupt_umask(int vector)
{
    RT_ASSERT(vector >= 0 && vector < LOONGARCH_INT_NUM);
    __csrxchg_w(1 << vector, 1 << vector, LOONGARCH_CSR_ECFG);
}

rt_isr_handler_t rt_hw_interrupt_install(int vector, rt_isr_handler_t handler,
                                         void *param, const char *name)
{
    rt_isr_handler_t old = RT_NULL;

    RT_ASSERT(vector >= 0 && vector < LOONGARCH_INT_NUM);
    old = irq_desc[vector].handler;

#ifdef RT_USING_INTERRUPT_INFO
    rt_strncpy(irq_desc[vector].name, name, RT_NAME_MAX);
#endif

    irq_desc[vector].handler = handler;
    irq_desc[vector].param = param;

    return old;
}

void rt_hw_interrupt_init(void)
{
    __csrwr_w(0, LOONGARCH_CSR_ECFG);
#ifdef ARCH_CPU_64BIT
    __csrwr_d((rt_ubase_t)&trap_entry, LOONGARCH_CSR_EENTRY);
#else
    __csrwr_w((rt_ubase_t)&trap_entry, LOONGARCH_CSR_EENTRY);
#endif
}

static void handle_interrupt(int vector)
{
    rt_isr_handler_t handler;
    void *param;

    handler = irq_desc[vector].handler;
    param = irq_desc[vector].param;

    if (handler != RT_NULL)
    {
        handler(vector, param);
    }
    else
    {
        rt_interrupt_enter();

#ifdef RT_USING_INTERRUPT_INFO
        rt_kprintf("Unhandled interrupt %d: %s\n", vector, irq_name[vector]);
#else
        rt_kprintf("Unhandled interrupt %d\n", vector);
#endif

        /* mask interrupt */
        rt_hw_interrupt_mask(vector);

        rt_interrupt_leave();
    }
}
```

我们可以通过执行 `scons --menuconfig` 把 `RT_USING_INTERRUPT_INFO` 配置打开。

注意，这里使用了 `__csr` 开头的函数，这些函数由 GCC 提供，需要引入头文件 `larchintrin.h`。
为了方便以后使用，我们可以在 `libcpu/loongarch/common/loongarch.h` 中引入该头文件。
使用这些函数的好处是我们不用再写內联汇编了。

### 异常

同样的，我们在 `libcpu/loongarch/common/exception.h` 定义异常相关的宏：

```c
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
```

可以注意到，包括保留的异常，异常总共 63 个，这就是中断编号是从 64 开始的原因。

完善 `libcpu/loongarch/common/exception.c`：

```c
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
```

这里 `[25 ... LOONGARCH_EXC_NUM - 1]` 之类的写法叫[指定初始化器（Designated Initializers）](https://gcc.gnu.org/onlinedocs/gcc/Designated-Inits.html)。

异常初始化暂时留空。

### 入口

根据手册，LoongArch 的异常入口由 `CSR.ECFG.VS` 和编号共同决定。
对于 RT-Thread 这个嵌入式操作系统，我们没必要让异常走不同的入口，走一个公共入口即可，还能节省空间。
因为，我们需要设置 `CSR.ECFG.VS` 为 0。

陷阱的入口需要用汇编编写，位于 `libcpu/loongarch/common/vector_gcc.S`：

```text
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
    ld.d     $t1, $t0, 0
    beqz     $t1, spurious_interrupt

    st.d     $zero, $t0, 0

    la.pcrel $t0, rt_interrupt_from_thread
    ld.d     $t1, $t0, 0
    st.d     $sp, $t1, 0

    la.pcrel $t0, rt_interrupt_to_thread
    ld.d     $t1, $t0, 0
    ld.d     $sp, $t1, 0

spurious_interrupt:
    RESTORE_ALL_AND_RET
```

该文件可以参考 `libcpu/risc-v/common/interrupt_gcc.S`，我们暂时不管浮点寄存器。

`libcpu/loongarch/common/stackframe.h` 可以参考 Linux 内核的相关文件（记得在 `libcpu/loongarch/common/loongarch.h` 中补上缺失的宏）：

```c
#ifndef STACKFRAME_H__
#define STACKFRAME_H__

#include "loongarch.h"

#define SIZE       8

#define FRAME_R0   (0 * SIZE)
#define FRAME_R1   (1 * SIZE)
#define FRAME_R2   (2 * SIZE)
#define FRAME_R3   (3 * SIZE)
#define FRAME_R4   (4 * SIZE)
#define FRAME_R5   (5 * SIZE)
#define FRAME_R6   (6 * SIZE)
#define FRAME_R7   (7 * SIZE)
#define FRAME_R8   (8 * SIZE)
#define FRAME_R9   (9 * SIZE)
#define FRAME_R10  (10 * SIZE)
#define FRAME_R11  (11 * SIZE)
#define FRAME_R12  (12 * SIZE)
#define FRAME_R13  (13 * SIZE)
#define FRAME_R14  (14 * SIZE)
#define FRAME_R15  (15 * SIZE)
#define FRAME_R16  (16 * SIZE)
#define FRAME_R17  (17 * SIZE)
#define FRAME_R18  (18 * SIZE)
#define FRAME_R19  (19 * SIZE)
#define FRAME_R20  (20 * SIZE)
#define FRAME_R21  (21 * SIZE)
#define FRAME_R22  (22 * SIZE)
#define FRAME_R23  (23 * SIZE)
#define FRAME_R24  (24 * SIZE)
#define FRAME_R25  (25 * SIZE)
#define FRAME_R26  (26 * SIZE)
#define FRAME_R27  (27 * SIZE)
#define FRAME_R28  (28 * SIZE)
#define FRAME_R29  (29 * SIZE)
#define FRAME_R30  (30 * SIZE)
#define FRAME_R31  (31 * SIZE)
#define FRAME_ERA  (32 * SIZE)
#define FRAME_PRMD (33 * SIZE)

#define FRAME_SIZE (34 * SIZE)

#ifdef __ASSEMBLY__

    .macro BACKUP_T0T1
    csrwr  $t0, LOONGARCH_CSR_KS0
    csrwr  $t1, LOONGARCH_CSR_KS1
    .endm

    .macro RELOAD_T0T1
    csrrd  $t0, LOONGARCH_CSR_KS0
    csrrd  $t1, LOONGARCH_CSR_KS1
    .endm

    .macro SAVE_TEMP
    RELOAD_T0T1
    st.d   $t0, $sp, FRAME_R12
    st.d   $t1, $sp, FRAME_R13
    st.d   $t2, $sp, FRAME_R14
    st.d   $t3, $sp, FRAME_R15
    st.d   $t4, $sp, FRAME_R16
    st.d   $t5, $sp, FRAME_R17
    st.d   $t6, $sp, FRAME_R18
    st.d   $t7, $sp, FRAME_R19
    st.d   $t8, $sp, FRAME_R20
    .endm

    .macro SAVE_STATIC
    st.d   $s0, $sp, FRAME_R23
    st.d   $s1, $sp, FRAME_R24
    st.d   $s2, $sp, FRAME_R25
    st.d   $s3, $sp, FRAME_R26
    st.d   $s4, $sp, FRAME_R27
    st.d   $s5, $sp, FRAME_R28
    st.d   $s6, $sp, FRAME_R29
    st.d   $s7, $sp, FRAME_R30
    st.d   $s8, $sp, FRAME_R31
    .endm

    .macro SAVE_SOME
    move   $t0, $sp
    addi.d $sp, $sp, -FRAME_SIZE
    st.d   $t0, $sp, FRAME_R3
    st.d   $zero, $sp, FRAME_R0
    csrrd  $t0, LOONGARCH_CSR_PRMD
    st.d   $t0, $sp, FRAME_PRMD
    st.d   $ra, $sp, FRAME_R1
    st.d   $a0, $sp, FRAME_R4
    st.d   $a1, $sp, FRAME_R5
    st.d   $a2, $sp, FRAME_R6
    st.d   $a3, $sp, FRAME_R7
    st.d   $a4, $sp, FRAME_R8
    st.d   $a5, $sp, FRAME_R9
    st.d   $a6, $sp, FRAME_R10
    st.d   $a7, $sp, FRAME_R11
    csrrd  $ra, LOONGARCH_CSR_ERA
    st.d   $ra, $sp, FRAME_ERA
    st.d   $tp, $sp, FRAME_R2
    st.d   $r21, $sp, FRAME_R21
    st.d   $fp, $sp, FRAME_R22
    .endm

    .macro SAVE_ALL
    SAVE_SOME
    SAVE_TEMP
    SAVE_STATIC
    .endm

    .macro RESTORE_TEMP
    ld.d   $t0, $sp, FRAME_R12
    ld.d   $t1, $sp, FRAME_R13
    ld.d   $t2, $sp, FRAME_R14
    ld.d   $t3, $sp, FRAME_R15
    ld.d   $t4, $sp, FRAME_R16
    ld.d   $t5, $sp, FRAME_R17
    ld.d   $t6, $sp, FRAME_R18
    ld.d   $t7, $sp, FRAME_R19
    ld.d   $t8, $sp, FRAME_R20
    .endm

    .macro RESTORE_STATIC
    ld.d   $s0, $sp, FRAME_R23
    ld.d   $s1, $sp, FRAME_R24
    ld.d   $s2, $sp, FRAME_R25
    ld.d   $s3, $sp, FRAME_R26
    ld.d   $s4, $sp, FRAME_R27
    ld.d   $s5, $sp, FRAME_R28
    ld.d   $s6, $sp, FRAME_R29
    ld.d   $s7, $sp, FRAME_R30
    ld.d   $s8, $sp, FRAME_R31
    .endm

    .macro RESTORE_SOME
    ld.d   $a0, $sp, FRAME_ERA
    csrwr  $a0, LOONGARCH_CSR_ERA
    ld.d   $a0, $sp, FRAME_PRMD
    csrwr  $a0, LOONGARCH_CSR_PRMD
    ld.d   $ra, $sp, FRAME_R1
    ld.d   $tp, $sp, FRAME_R2
    ld.d   $a0, $sp, FRAME_R4
    ld.d   $a1, $sp, FRAME_R5
    ld.d   $a2, $sp, FRAME_R6
    ld.d   $a3, $sp, FRAME_R7
    ld.d   $a4, $sp, FRAME_R8
    ld.d   $a5, $sp, FRAME_R9
    ld.d   $a6, $sp, FRAME_R10
    ld.d   $a7, $sp, FRAME_R11
    ld.d   $r21, $sp, FRAME_R21
    ld.d   $fp, $sp, FRAME_R22
    .endm

    .macro RESTORE_SP_AND_RET
    ld.d   $sp, $sp, FRAME_R3
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
```

注意到，我们把 `trap_entry` 放在了 `.vectors` 段中，这是为了让我们能在链接脚本里设置地址对齐。
我们在 `*(.startup)` 后补上以下内容：

```text
. = ALIGN(0x1000);
KEEP(*(.vectors))
```

尽管手册并没明示 `CSR.EENTRY` 存的地址需要 4KB 对齐，不过通过实验和仔细阅读相关章节，其实是有这个要求的，很容易被忽视。
同样的，`CSR.TLBRENTRY` 和 `CSR.MERRENTRY` 也有这个要求。

### 陷阱分发处理

`handle_trap()` 函数负责分发处理陷阱，位于 `libcpu/loongarch/common/interrupt.c`（放 `trap.c` 可能更好，但因为只有这一个函数，就放中断里了）。
就目前来说，如果实现正确，任何异常都不应该被触发。
因此我们暂时只管分发，不实现具体的处理。

```c
void handle_trap(void)
{
    rt_uint32_t estat, ecode, esubcode, mask, pending;

    estat = __csrrd_w(LOONGARCH_CSR_ESTAT);
    ecode = (estat & CSR_ESTAT_EXC) >> CSR_ESTAT_EXC_SHIFT;
    esubcode = (estat & CSR_ESTAT_ESUBCODE) >> CSR_ESTAT_ESUBCODE_SHIFT;

    if (ecode == 0)
    {
        mask = __csrrd_w(LOONGARCH_CSR_ECFG) & CSR_ECFG_LIE;
        pending = estat & CSR_ESTAT_IS & mask;
        if (pending & ESTAT_IS_IPI)
            handle_interrupt(LOONGARCH_INT_IPI);
        if (pending & ESTAT_IS_TIMER)
            handle_interrupt(LOONGARCH_INT_TI);
        if (pending & ESTAT_IS_PMC)
            handle_interrupt(LOONGARCH_INT_PMI);
        if (pending & ESTAT_IS_IP7)
            handle_interrupt(LOONGARCH_INT_HWI7);
        if (pending & ESTAT_IS_IP6)
            handle_interrupt(LOONGARCH_INT_HWI6);
        if (pending & ESTAT_IS_IP5)
            handle_interrupt(LOONGARCH_INT_HWI5);
        if (pending & ESTAT_IS_IP4)
            handle_interrupt(LOONGARCH_INT_HWI4);
        if (pending & ESTAT_IS_IP3)
            handle_interrupt(LOONGARCH_INT_HWI3);
        if (pending & ESTAT_IS_IP2)
            handle_interrupt(LOONGARCH_INT_HWI2);
        if (pending & ESTAT_IS_IP1)
            handle_interrupt(LOONGARCH_INT_HWI1);
        if (pending & ESTAT_IS_IP0)
            handle_interrupt(LOONGARCH_INT_HWI0);
        if (pending & ESTAT_IS_SIP1)
            handle_interrupt(LOONGARCH_INT_SWI1);
        if (pending & ESTAT_IS_SIP0)
            handle_interrupt(LOONGARCH_INT_SWI0);
    }
    else
    {
        handle_exception(ecode, esubcode);
    }
}
```

现在，我们的 `libcpu/loongarch/common/loongarch.h` 已经有些复杂了，参考 Linux 内核适当重构一下：

```c
#ifndef LOONGARCH_H__
#define LOONGARCH_H__

#ifdef __ASSEMBLY__
#define UL(x) x
#else
#include <larchintrin.h>
#define UL(x) x ## UL
#endif

#define BIT(n)     (UL(1) << (n))
#define MASK(l, h) (((~UL(0)) - (UL(1) << (l)) + 1) & (~UL(0) >> (64 - 1 - (h))))

#define LOONGARCH_CSR_CRMD        0x0
#define  CSR_CRMD_IE              BIT(2)
#define  CSR_CRMD_DA              BIT(3)
#define  CSR_CRMD_PG              BIT(4)
#define LOONGARCH_CSR_PRMD        0x1
#define LOONGARCH_CSR_ECFG        0x4
#define  CSR_ECFG_LIE             MASK(0, 12)
#define LOONGARCH_CSR_ESTAT       0x5
#define  CSR_ESTAT_IS             MASK(0, 12)
#define   ESTAT_IS_SIP0           BIT(0)
#define   ESTAT_IS_SIP1           BIT(1)
#define   ESTAT_IS_IP0            BIT(2)
#define   ESTAT_IS_IP1            BIT(3)
#define   ESTAT_IS_IP2            BIT(4)
#define   ESTAT_IS_IP3            BIT(5)
#define   ESTAT_IS_IP4            BIT(6)
#define   ESTAT_IS_IP5            BIT(7)
#define   ESTAT_IS_IP6            BIT(8)
#define   ESTAT_IS_IP7            BIT(9)
#define   ESTAT_IS_PMC            BIT(10)
#define   ESTAT_IS_TIMER          BIT(11)
#define   ESTAT_IS_IPI            BIT(12)
#define  CSR_ESTAT_EXC            MASK(16, 21)
#define  CSR_ESTAT_EXC_SHIFT      16
#define  CSR_ESTAT_ESUBCODE       MASK(22, 30)
#define  CSR_ESTAT_ESUBCODE_SHIFT 22
#define LOONGARCH_CSR_ERA         0x6
#define LOONGARCH_CSR_EENTRY      0xc
#define LOONGARCH_CSR_KS0         0x30
#define LOONGARCH_CSR_KS1         0x31
#define LOONGARCH_CSR_DMWIN0      0x180
#define LOONGARCH_CSR_DMWIN1      0x181

#endif /* LOONGARCH_H__ */
```

后面还会用到更多的宏，如果有需要，可以直接把 Linux 的 `arch/loongarch/include/asm/loongarch.h` 搬过来稍加修改。

## LoongArch 架构移植 - 上下文切换

上下文切换说白了就是保存恢复寄存器，在 RT-Thread 中还多了个中断标志。

### 实现上下文切换

上下文切换的代码比较简单，直接参考 RISC-V 和 MIPS 即可。

`libcpu/loongarch/common/context_gcc.S`：

```text
#include "stackframe.h"

/* void rt_hw_context_switch_to(rt_uint32 to) */
    .globl rt_hw_context_switch_to
rt_hw_context_switch_to:
    ld.d     $sp, $a0, 0 /* get new task stack pointer */
    RESTORE_ALL_AND_RET

/* void rt_hw_context_switch(rt_uint32 from, rt_uint32 to) */
    .globl rt_hw_context_switch
rt_hw_context_switch:
    BACKUP_T0T1
    move     $t0, $ra
    csrwr    $t0, LOONGARCH_CSR_ERA
    SAVE_ALL

    st.d     $sp, $a0, 0
    ld.d     $sp, $a1, 0

    RESTORE_ALL_AND_RET

/* void rt_hw_context_switch_interrupt(rt_uint32 from, rt_uint32 to) */
    .globl rt_hw_context_switch_interrupt
rt_hw_context_switch_interrupt:
    la.pcrel $t0, rt_thread_switch_interrupt_flag
    ld.d     $t1, $t0, 0
    bnez     $t1, _reswitch

    li.w     $t1, 1
    st.d     $t1, $t0, 0

    la       $t0, rt_interrupt_from_thread
    st.d     $a0, $t0, 0

_reswitch:
    la       $t0, rt_interrupt_to_thread
    st.d     $a1, $t0, 0

    jr       $ra
```

### ARM 的上下文切换

如果你看了 `documentation/kernel-porting/kernel-porting.md`，会发现上下文切换的实现完全不同。
官服文档里提供的是 ARM 的实现，ARM 架构是利用 PendSV 特性进行上下文切换的。
ARM 的上下文切换函数会触发 PendSV 异常，然后借由 PendSV 异常处理函数来完成上下文切换，所以显得相对复杂。

*我们可以通过类似的方式来实现上下文切换吗？*

可以，但没必要。
我们可以通过触发软中断的方式来模拟 ARM 的 PendSV，但这样非但增加了复杂度，而且未必能得到性能上的好处。
所以 RISC-V 和 MIPS 都是直接在上下文切换函数中就完成了切换，而不额外触发异常。

### 重构和调试

代码到此已经有些多了，为了可读性和扩展性，有必要重构一下。

目前我们只考虑了 64 位架构，后续可能会添加 32 位支持。
但这部分代码 32 位和 64 位应该是共通的，而我们的汇编和 `__csr` 开头的目前只考虑了 64 位。

为此，我们引入头文件 `libcpu/loongarch/common/asm.h`，并借助 `ARCH_CPU_64BIT` 宏来区分 32 位和 64 位：

```c
#ifndef ASM_H__
#define ASM_H__

#include <rtconfig.h>

#ifdef ARCH_CPU_64BIT

#define SIZE  8

#define ADDI  addi.d
#define LD    ld.d
#define LI    li.d
#define ST    st.d

#else

#define SIZE  4

#define ADDI  addi.w
#define LD    ld.w
#define LI    li.w
#define ST    st.w

#endif /* ARCH_CPU_64BIT */

#endif /* ASM_H__ */
```

之前引入 `libcpu/loongarch/common/stackframe.h`，除了能让陷阱和上下文切换复用外，也有这个目的。
现在我们需要在 `stackframe.h` 导入 `asm.h` ，然后用宏替换相关指令。
同样的，也需要修改所有的汇编文件。

虽然 CSR 指令本身不区分 32 位和 64 位，但 GCC 提供的 `__csrwr` 开头的函数区分。
我们也需要修改对应的代码，目前只影响了 `rt_hw_interrupt_init()`：

```c
#ifdef ARCH_CPU_64BIT
    __csrwr_d((rt_ubase_t)&trap_entry, LOONGARCH_CSR_EENTRY);
#else
    __csrwr_w((rt_ubase_t)&trap_entry, LOONGARCH_CSR_EENTRY);
#endif
```

`rt_ubase_t` 的定义可以在 `include/rttypes.h` 中找到：

```c
#ifdef ARCH_CPU_64BIT
typedef rt_int64_t                      rt_base_t;      /**< Nbit CPU related data type */
typedef rt_uint64_t                     rt_ubase_t;     /**< Nbit unsigned CPU related data type */
#else
typedef rt_int32_t                      rt_base_t;      /**< Nbit CPU related data type */
typedef rt_uint32_t                     rt_ubase_t;     /**< Nbit unsigned CPU related data type */
#endif
```

可以看到，它也是根据 `ARCH_CPU_64BIT` 来决定 32 位还是 64 位的。

这样重构后，我们的代码就同时兼顾了 32 位和 64 位，以后添加其他处理器核和板级支持也会更容易。

最后是调试相关的注意事项，目前我们还没法直接上机调试，毕竟代码还很不完整，只能通过往串口塞字符的方式判断代码流程。
中断和上下文切换相关的代码是很容易出错的，而且很难追踪，这部分代码写错，可能会出现很多莫名其妙的问题：

- 程序跑飞（代码执行路径非常奇怪，不知道跳哪里去了），跑飞后还可能引发各种异常，特别是 `CSR.CRMD` 这种寄存器出错，但这些都不是最早出问题的地方，很有迷惑性。
- 数据错误，比如我想打印 `'0'`，结果打印了 `'A'`，数据错误发生在一些条件跳转的地方，也会造成程序跑飞。

如果遇到这种问题，都可能是寄存器保存恢复时出了差错，这时候就应该怀疑中断和上下文切换代码了。
在只有串口的情况下，我们可以在关键位置（比如进入中断处理函数的时候和上下文切换前后）打印所有关键寄存器的值，来进行判断。

## LoongArch 架构移植 - 线程栈

上下文切换通过保存恢复寄存器来切换线程，但首次启动的线程的寄存器并没有保存，所以我们需要线程栈来构造一个可以用于上下文切换的栈帧。

### 栈结构体

参考 RISC-V，在 `libcpu/loongarch/common/stack.h` 中定义栈结构体：

```c
#ifndef STACK_H__
#define STACK_H__

#include <rtthread.h>

struct rt_hw_stack_frame
{
    rt_base_t zero;
    rt_base_t ra;
    rt_base_t tp;
    rt_base_t sp;
    rt_base_t a0;
    rt_base_t a1;
    rt_base_t a2;
    rt_base_t a3;
    rt_base_t a4;
    rt_base_t a5;
    rt_base_t a6;
    rt_base_t a7;
    rt_base_t t0;
    rt_base_t t1;
    rt_base_t t2;
    rt_base_t t3;
    rt_base_t t4;
    rt_base_t t5;
    rt_base_t t6;
    rt_base_t t7;
    rt_base_t t8;
    rt_base_t r21;
    rt_base_t fp;
    rt_base_t s0;
    rt_base_t s1;
    rt_base_t s2;
    rt_base_t s3;
    rt_base_t s4;
    rt_base_t s5;
    rt_base_t s6;
    rt_base_t s7;
    rt_base_t s8;
    rt_base_t csr_era;
    rt_base_t csr_prmd;
};

#endif /* STACK_H__ */
```

这里的栈结构体成员偏移必须和 `stackframe.h` 中的宏完全一致，否则在上下文切换时就会发生寄存器数据错位。

各个寄存器的作用可以参考 [LoongArch 汇编手册](https://github.com/loongson/la-asm-manual/tree/main)。

### 栈初始化

栈初始化的代码位于 `libcpu/loongarch/common/stack.c`：

```c
#include <rtthread.h>

#include "asm.h"
#include "loongarch.h"
#include "stack.h"

rt_uint8_t *rt_hw_stack_init(void *tentry, void *parameter, rt_uint8_t *stack_addr, void *texit)
{
    struct rt_hw_stack_frame *frame;
    rt_uint8_t *stk;

    /* stack align */
    stk = stack_addr + sizeof(rt_ubase_t);
    stk = (rt_uint8_t *)RT_ALIGN_DOWN((rt_ubase_t)stk, SIZE * 2);
    stk -= sizeof(struct rt_hw_stack_frame);

    frame = (struct rt_hw_stack_frame *)stk;
    frame->zero = 0;
    frame->ra = (rt_ubase_t)texit;
    frame->tp = 0;
    frame->sp = (rt_ubase_t)stk;
    frame->a0 = (rt_ubase_t)parameter;
    frame->a1 = 0;
    frame->a2 = 0;
    frame->a3 = 0;
    frame->a4 = 0;
    frame->a5 = 0;
    frame->a6 = 0;
    frame->a7 = 0;
    frame->t0 = 0;
    frame->t1 = 0;
    frame->t2 = 0;
    frame->t3 = 0;
    frame->t4 = 0;
    frame->t5 = 0;
    frame->t6 = 0;
    frame->t7 = 0;
    frame->t8 = 0;
    frame->s0 = 0;
    frame->s1 = 0;
    frame->s2 = 0;
    frame->s3 = 0;
    frame->s4 = 0;
    frame->s5 = 0;
    frame->s6 = 0;
    frame->s7 = 0;
    frame->s8 = 0;

    frame->csr_era = (rt_ubase_t)tentry;

    /* enable interrupt */
    frame->csr_prmd = __csrrd_w(LOONGARCH_CSR_PRMD) | CSR_PRMD_PIE;

    return stk;
}
```

这里我们重点关注几个寄存器（其他寄存器一律初始化为 0 即可）：

- `RA`：函数返回地址，线程结束后跳转的地址。
- `SP`：线程的栈地址，64 位需要 16 字节对齐，32 位需要 8 字节对齐，所以使用 `SIZE * 2`。
- `A0`：函数参数，即传给线程的参数。
- `CSR.ERA`：陷阱返回地址，这里填入线程入口，就能让陷阱返回后开始执行线程。
- `CSR.PRMD`：设置 `CSR.PRMD.PIE`，让上下文切换后的线程默认开中断。

最容易搞混的就是 `RA` 和 `CSR.ERA`。
C 语言中进行函数调用，父函数调用子函数时，会把返回地址保存在 `RA` 里（通常是跳转指令的下一条指令的地址），子函数最后通过跳转到 `RA` 返回父函数。
而内核运行时，如果发生了陷阱，触发陷阱的指令地址会被保存在 `CSR.ERA` 中，当内核处理完陷阱后通过 `ertn` 指令返回到 `CSR.ERA` 继续执行。
这里借助 `CSR.ERA`，让上下文切换 `RESTORE_SP_AND_RET` 的时候能执行线程的第一条指令，同时按照 ABI 规范把线程函数的参数、返回地址、栈地址填上。

最后一个要关注的点就是我们修改了 `CSR.PRMD`，手动打开了中断。
在切换到这个首次创建的线程后，我们必须开中断，不然它可能永远运行在关中断的环境中，这是不合理的。

至此，我们已经完成了最基本的架构移植。

### 进一步重构

之前我们把地址空间划分好了，但没有用宏去统一定义，而是直接在各个文件中直接使用。
后续我们要完善板级支持，必然会用到缓存和非缓存地址，因此我们需要引入 `libcpu/loongarch/common/addr.h`（参考 Linux 内核的 `arch/loongarch/include/asm/addrspace.h`）：

```c
#ifndef ADDR_H__
#define ADDR_H__

#include <rtconfig.h>

#ifdef __ASSEMBLER__
#define ULL(x) x
#else
#define ULL(x) x ## ULL
#endif

#ifdef ARCH_CPU_64BIT
#define DMW_PABITS    48
#else
#define DMW_PABITS    32
#endif

#define TO_PHYS_MASK  ((ULL(1) << DMW_PABITS) - 1)

/* Direct Map window 0 */
#define CSR_DMW0_PLV0  (ULL(1) << 0)
#define CSR_DMW0_VSEG  ULL(0x8000)
#define CSR_DMW0_BASE  (CSR_DMW0_VSEG << DMW_PABITS)
#define CSR_DMW0_INIT  (CSR_DMW0_BASE | CSR_DMW0_PLV0)

/* Direct Map window 1 */
#define CSR_DMW1_PLV0  ULL(1 << 0)
#define CSR_DMW1_MAT   ULL(1 << 4)
#define CSR_DMW1_VSEG  ULL(0x9000)
#define CSR_DMW1_BASE  (CSR_DMW1_VSEG << DMW_PABITS)
#define CSR_DMW1_INIT  (CSR_DMW1_BASE | CSR_DMW1_MAT | CSR_DMW1_PLV0)

#define UNCACHE_BASE   CSR_DMW0_BASE
#define CACHE_BASE     CSR_DMW1_BASE

#define TO_PHYS(x)     (((x) & TO_PHYS_MASK))
#define TO_CACHE(x)    (CACHE_BASE | ((x) & TO_PHYS_MASK))
#define TO_UNCACHE(x)  (UNCACHE_BASE | ((x) & TO_PHYS_MASK))

#endif /* ADDR_H__ */
```

修改 `libcpu/loongarch/la264/entry_gcc.S`：

```text
/* config direct window */
LI       $t0, CSR_DMW0_INIT
csrwr    $t0, LOONGARCH_CSR_DMWIN0
LI       $t0, CSR_DMW1_INIT
csrwr    $t0, LOONGARCH_CSR_DMWIN1
```

这时候我们不难发现 `addr.h` 定义了 `ULL()` 宏， `loongarch.h` 定义了 `UL()` 宏，不妨把它们都放到 `loongarch.h` 中。

## LS2K0300 移植 - 板级初始化

基本的架构移植完成后，我们要实现板级初始化函数 `rt_hw_board_init()`，这部分内容也可以参考 RISC-V。

### 实现板级初始化函数

由于我们启用了 `RT_USING_COMPONENTS_INIT` 配置，所有的驱动初始化会自动完成（后面会讲），因此我们只需要调用函数初始化陷阱、终端和堆即可。

`bsp/ls2k0300/drivers/board.h`:

```c
#ifndef BOARD_H__
#define BOARD_H__

#include <rtthread.h>

#define RT_HW_HEAP_SIZE  (64 * 1024 * 1024)

extern rt_uint8_t __bss_end;

#define RT_HW_HEAP_BEGIN (&__bss_end)
#define RT_HW_HEAP_END   (RT_HW_HEAP_BEGIN + RT_HW_HEAP_SIZE)

void rt_hw_board_init(void);

#endif /* BOARD_H__ */
```

`bsp/ls2k0300/drivers/board.c`：

```c
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
```

我们设置堆的起始地址是 `__bss_end`（该变量由链接脚本导出），大小是 64 MB。

### 进一步测试

板级初始化函数实现好后，下一步就是实现各种驱动了。
但我们不要着急，是时候停下来测试了，步子迈太大容易跌倒。
我们至少要确保 RT-Thread 能正常运行完 `rt_hw_board_init()`。
于是我们可以在 `rt_hw_board_init()` 开头和结尾往串口塞字符：

```c
...
#include "addr.h"
...
void rt_hw_board_init(void)
{
    HWREG8(TO_UNCACHE(0x16100000)) = '1';
    rt_hw_interrupt_init();
    ...
    HWREG8(TO_UNCACHE(0x16100000)) = '2';
}
```

编译运行，这时候串口应该能看到 `12`。

## LS2K0300 移植 - 驱动

常用且必须实现的驱动就两个：时钟驱动和串口驱动。

### 时钟驱动

时钟驱动是 RT-Thread 的重要部分，有了 Tick 内核才能正常调度、实现计时器等功能。

`bsp/ls2k0300/drivers/drv_timer.h`：

```c
#ifndef DRV_TIMER_H__
#define DRV_TIMER_H__

int rt_hw_timer_init(void);

#endif /* DRV_TIMER_H__ */
```

`bsp/ls2k0300/drivers/drv_timer.c`：

```c
#include <rthw.h>
#include <rtthread.h>

#include "board.h"
#include "drv_timer.h"
#include "interrupt.h"
#include "loongarch.h"

static void rt_hw_timer_handler(int vector, void *param)
{
    rt_interrupt_enter();

    /* clear interrupt */
    __csrwr_w(CSR_TINTCLR_TI, LOONGARCH_CSR_TINTCLR);

    /* increase a OS tick */
    rt_tick_increase();

    rt_interrupt_leave();
}

static unsigned int get_cpu_freq(void)
{
    unsigned int res;
    unsigned int base_freq;
    unsigned int cfm, cfd;

    base_freq = __cpucfg(LOONGARCH_CPUCFG4);
    res = __cpucfg(LOONGARCH_CPUCFG5);
    cfm = res & CPUCFG5_CCMUL;
    cfd = (res & CPUCFG5_CCDIV) >> CPUCFG5_CCDIV_SHIFT;

    if (!base_freq || !cfm || !cfd)
        return 100000000; /* default CPU frequency */

    return (base_freq * cfm / cfd);
}

int rt_hw_timer_init(void)
{
    rt_ubase_t period;
    rt_ubase_t timer_config;

    period = get_cpu_freq() / RT_TICK_PER_SECOND;
    timer_config = period & CSR_TCFG_VAL;
    timer_config |= (CSR_TCFG_PERIOD | CSR_TCFG_EN);
    __csrwr_d(timer_config, LOONGARCH_CSR_TCFG);

    rt_hw_interrupt_install(LOONGARCH_INT_TI, rt_hw_timer_handler, RT_NULL, "tick");
    rt_hw_interrupt_umask(LOONGARCH_INT_TI);

    return 0;
}
INIT_BOARD_EXPORT(rt_hw_timer_init);
```

时钟的初始化参考了 Linux 内核 `arch/loongarch/include/asm/time.h` 中的 `calc_const_freq()` 函数。

我们通过 `rt_hw_interrupt_install()` 函数安装时钟中断，时钟中断根据 `documentation/kernel-porting/kernel-porting.md` 文档来实现。

在文件的最后，我们使用了 `INIT_BOARD_EXPORT()` 宏来让时钟驱动在启动过程中自动初始化（`rt_hw_timer_init()` 会被 `rt_components_board_init()` 调用）。

### 串口驱动

理论上应该先实现 I/O 中断，但有了串口驱动，我们就不需要再用原始的方式塞串口了，所以把它提前实现。
串口驱动也可以参考 RISC-V，我们暂时不管中断，只关心输出的功能。
串口地址参考 LS2K0300 手册，共有 10 个 UART 控制器，我们目前只用 UART0。

`bsp/ls2k0300/drivers/drv_uart.h`：

```c
#ifndef DRV_UART_H__
#define DRV_UART_H__

#define LS2K0300_UART0 0x16100000

int rt_hw_uart_init(void);

#endif /* DRV_UART_H__ */
```

`bsp/ls2k0300/drivers/drv_uart.c`：

```c
#include <rtdevice.h>
#include <rthw.h>
#include <rtthread.h>

#include "addr.h"
#include "board.h"
#include "drv_uart.h"
#include "loongarch.h"
#include "ls2k0300.h"

#define UART_DAT(base) HWREG8(base + 0x00)
#define UART_IER(base) HWREG8(base + 0x01)
# define IER_IRXE      BIT(0)
# define IER_ILE       BIT(2)
#define UART_IIR(base) HWREG8(base + 0x02)
# define IIR_RXRDY     BIT(2)
# define IIR_RXTOUT    (BIT(3) | BIT(2))
#define UART_FCR(base) HWREG8(base + 0x02)
#define UART_LCR(base) HWREG8(base + 0x03)
#define UART_MCR(base) HWREG8(base + 0x04)
#define UART_LSR(base) HWREG8(base + 0x05)
# define LSR_DR        BIT(0)
# define LSR_TFE       BIT(5)
# define LSR_TE        BIT(6)
#define UART_MSR(base) HWREG8(base + 0x06)

struct rt_uart_device
{
    const char name[RT_NAME_MAX];
    rt_ubase_t base;
    rt_uint32_t irq;
};

static rt_err_t uart_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
    struct rt_uart_device *uart = RT_NULL;

    RT_ASSERT(serial != RT_NULL);
    RT_ASSERT(cfg != RT_NULL);

    uart = (struct rt_uart_device *)serial->parent.user_data;

    UART_IER(uart->base) = 0x00;
    UART_FCR(uart->base) = 0xc1;
    UART_LCR(uart->base) = 0x03;
    UART_MCR(uart->base) = 0x03;
    UART_LSR(uart->base) = 0x60;
    UART_MSR(uart->base) = 0xb0;

    return RT_EOK;
}

static rt_err_t uart_control(struct rt_serial_device *serial, int cmd, void *arg)
{
    return RT_EOK;
}

static int uart_putc(struct rt_serial_device *serial, char c)
{
    struct rt_uart_device *uart = RT_NULL;
    rt_uint32_t status;

    RT_ASSERT(serial != RT_NULL);

    uart = (struct rt_uart_device *)serial->parent.user_data;

    status = UART_LSR(uart->base);
    while (!(status & (LSR_TE | LSR_TFE)))
    {
        status = UART_LSR(uart->base);
    }

    UART_DAT(uart->base) = c;

    return 1;
}

static int uart_getc(struct rt_serial_device *serial)
{
    struct rt_uart_device *uart = RT_NULL;

    RT_ASSERT(serial != RT_NULL);

    uart = (struct rt_uart_device *)serial->parent.user_data;

    if (UART_LSR(uart->base) & LSR_DR)
    {
        return UART_DAT(uart->base);
    }

    return -1;
}

static const struct rt_uart_ops uart_ops =
{
    .configure    = uart_configure,
    .control      = uart_control,
    .putc         = uart_putc,
    .getc         = uart_getc,
    .dma_transmit = RT_NULL,
};

static struct rt_uart_device uart0_device =
{
    .name = "uart0",
    .base = TO_UNCACHE(LS2K0300_UART0),
    .irq = 0 // TODO,
};
static struct rt_serial_device serial0;

int rt_hw_uart_init(void)
{
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
    struct rt_uart_device *uart = RT_NULL;
    rt_err_t ret;

    uart = &uart0_device;

    serial0.ops = &uart_ops;
    serial0.config = config;

    ret = rt_hw_serial_register(&serial0, uart->name, RT_DEVICE_FLAG_WRONLY, uart);
    if (ret != RT_EOK)
    {
        return ret;
    }

    return 0;
}
INIT_BOARD_EXPORT(rt_hw_uart_init);
```

记得执行 `scons --menuconfig` 将 `RT_CONSOLE_DEVICE_NAME` 改成 `uart0`。

### 继续调试

这部分内容是我首次移植时遇到的问题，细节不展开了，仅列举调试思路供大家参考。

我们编译运行，发现串口在打印 RT-Thread 版本信息后就不动了。
这说明我们的代码出了问题，我首先怀疑是上下文切换出了问题。
找到 `src/scheduler_up.c` 中的 `rt_system_scheduler_start()` 函数，在 `rt_hw_context_switch_to((rt_uintptr_t)&to_thread->sp);` 前添加打印。

再次编译运行，发现打印正常，但产生了断言错误。
由于没有 EJTAG，我们只能不停添加打印来追踪错误源头。
从已知信息入手，我们知道断言错误产生在切换线程后，我们需要判断它现在进入了哪个线程，是不是上下文切换的实现错了。
我们知道切换后的地址放在栈桢中的 `CSR.ERA` 中，打印这个值，然后用 `objdump` 指令反汇编，查看地址确认是要跳转到 `_timer_thread_entry()`。
在 `src/timer.c` 中找到该函数，添加打印，确认跳转成功。
查看该函数代码，之后会调用 `rt_sem_take()`，然后执行 `_rt_sem_take()`，这正是我们断言错误的地方。
查看代码，可以确定是 `_soft_timer_sem` 变量中的数据在运行时被篡改了。

现在可以初步判断上下文切换成功了，那会不会是栈初始化时错位了呢。
我们添加打印，比对相关信息，没有发现问题。
继续思考，还有什么情况会篡改这个数据。
之后能想到的是栈或者堆地址出现了问题，我们在初始化的地方打印这些地址。
我们还可以通过 `readelf` 指令查看变量地址。
果然发现了奇怪的地方，线程栈用的地址似乎覆盖了变量地址！

继续查看代码，发现这里线程栈是用 `static rt_uint8_t _timer_thread_stack[RT_TIMER_THREAD_STACK_SIZE];` 定义的，所以没用到堆。
而且这个地址和 `_soft_timer_sem` 变量是紧挨着的，那大概率就是这里栈发生了溢出，导致变量被篡改。
我们通过 `scons --menuconfig` 修改 `RT_TIMER_THREAD_STACK_SIZE` 为 2048（默认是 512）。
再次编译运行，RT-Thread 成功跑下去了，在执行完用户 `main()` 后再次报错： `[E/kernel.sched] thread:tidle0 stack overflow`。
这印证了我的猜想。

RT-Thread 默认的栈大小不够我们架构，保险起见，我们把 `IDLE_THREAD_STACK_SIZE` 改为 2048，把 `RT_MAIN_THREAD_STACK_SIZE` 改为 `8192`。
然后删除之前添加的调试打印（先保留屏蔽中断），编译运行，成功进入 MSH。
这时候输入东西是没反应的，因为我们把中断屏蔽了。

### I/O 中断

在架构移植的时候，我们只实现了通用中断，而 I/O 中断配置是板级相关的。
现在我们需要根据 LS2K0300 手册中的《中断配置及路由》章节把这部分内容补上。

LoongArch 架构提供了两种中断模式：传统 I/O 中断和扩展 I/O 中断。
简单起见，我们只考虑传统 I/O 中断。
在 LS2K0300 上，该模式最多支持 64 个中断源。
我们需要通过设置中断配置寄存器，让所有的中断低电平触发，并路由到处理器核中断 INT0（IP0），即 HWI0。
仿照通用中断处理，定义相关的 I/O 中断处理函数。

`bsp/ls2k0300/drivers/io_interrupt.h`：

```c
#ifndef IO_INTERRUPT_H__
#define IO_INTERRUPT_H__

#define LS2K0300_IO_INT_UART00      0
#define LS2K0300_IO_INT_UART01      1
#define LS2K0300_IO_INT_UART_02_05  2
#define LS2K0300_IO_INT_UART_06_09  3
#define LS2K0300_IO_INT_I2C_00_01   4
#define LS2K0300_IO_INT_I2C_02_03   5
#define LS2K0300_IO_INT_SPI2        6
#define LS2K0300_IO_INT_SPI3        7
#define LS2K0300_IO_INT_CAN0        8
#define LS2K0300_IO_INT_CAN1        9
#define LS2K0300_IO_INT_CAN2        10
#define LS2K0300_IO_INT_CAN3        11
#define LS2K0300_IO_INT_I2S         12
#define LS2K0300_IO_INT_ATIMER      13
#define LS2K0300_IO_INT_GTIMER      14
#define LS2K0300_IO_INT_BTIMER      15
#define LS2K0300_IO_INT_PWM_0_1     16
#define LS2K0300_IO_INT_PWM_2_3     17
#define LS2K0300_IO_INT_ADC         18
#define LS2K0300_IO_INT_HPET0       19
#define LS2K0300_IO_INT_HPET1       20
#define LS2K0300_IO_INT_HPET2       21
#define LS2K0300_IO_INT_HPET3       22
#define LS2K0300_IO_INT_APB_DMA0    23
#define LS2K0300_IO_INT_APB_DMA1    24
#define LS2K0300_IO_INT_APB_DMA2    25
#define LS2K0300_IO_INT_APB_DMA3    26
#define LS2K0300_IO_INT_APB_DMA4    27
#define LS2K0300_IO_INT_APB_DMA5    28
#define LS2K0300_IO_INT_APB_DMA6    29
#define LS2K0300_IO_INT_APB_DMA7    30
#define LS2K0300_IO_INT_SDIO0_CTRL  31
#define LS2K0300_IO_INT_SDIO1_CTRL  32
#define LS2K0300_IO_INT_SDIO0_DMA   33
#define LS2K0300_IO_INT_SDIO1_DMA   34
#define LS2K0300_IO_INT_ENCRYPT_DMA 35
#define LS2K0300_IO_INT_AES         36
#define LS2K0300_IO_INT_DES         37
#define LS2K0300_IO_INT_SM3         38
#define LS2K0300_IO_INT_SM4         39
#define LS2K0300_IO_INT_RTC         40
#define LS2K0300_IO_INT_TOY         41
#define LS2K0300_IO_INT_RTC_TICK    42
#define LS2K0300_IO_INT_TOY_TICK    43
#define LS2K0300_IO_INT_SPI0        44
#define LS2K0300_IO_INT_SPI1        45
#define LS2K0300_IO_INT_EHCI        46
#define LS2K0300_IO_INT_OHCI        47
#define LS2K0300_IO_INT_OTG         48
#define LS2K0300_IO_INT_GMAC0       49
#define LS2K0300_IO_INT_GMAC1       50
#define LS2K0300_IO_INT_DC          51
#define LS2K0300_IO_INT_THSENS      52
#define LS2K0300_IO_INT_GPIO_0_15   53
#define LS2K0300_IO_INT_GPIO_16_31  54
#define LS2K0300_IO_INT_GPIO_32_47  55
#define LS2K0300_IO_INT_GPIO_48_63  56
#define LS2K0300_IO_INT_GPIO_64_79  57
#define LS2K0300_IO_INT_GPIO_80_95  58
#define LS2K0300_IO_INT_GPIO_96_105 59
/* #define LS2K0300_IO_INT_RESERVED    60 */
#define LS2K0300_IO_INT_DDR_ECC0    61
#define LS2K0300_IO_INT_DDR_ECC1    62
/* #define LS2K0300_IO_INT_RESERVED    63 */

#define LS2K0300_IO_ENTRY_NUM       8
#define LS2K0300_IO_INT_LOW_NUM     32
#define LS2K0300_IO_INT_HIGH_NUM    32
#define LS2K0300_IO_INT_NUM         (LS2K0300_IO_INT_LOW_NUM + LS2K0300_IO_INT_HIGH_NUM)

#define LS2K0300_IP0                0x10
#define LS2K0300_IP1                0x20
#define LS2K0300_IP2                0x40
#define LS2K0300_IP3                0x80

void rt_hw_io_interrupt_mask(int vector);
void rt_hw_io_interrupt_umask(int vector);
rt_isr_handler_t rt_hw_io_interrupt_install(int vector, rt_isr_handler_t handler,
                                            void *param, const char *name);
int rt_hw_io_interrupt_init(void);

#endif /* IO_INTERRUPT_H__ */
```

`bsp/ls2k0300/drivers/io_interrupt.c`：

```c
#include <rthw.h>
#include <rtthread.h>

#include "addr.h"
#include "board.h"
#include "interrupt.h"
#include "io_interrupt.h"
#include "ls2k0300.h"

static struct rt_irq_desc io_irq_desc[LS2K0300_IO_INT_NUM];

#ifdef RT_USING_INTERRUPT_INFO
static const char *io_irq_name[LS2K0300_IO_INT_NUM] =
{
    "UART00",
    "UART01",
    "UART_02_05",
    "UART_06_09",
    "I2C_00_01",
    "I2C_02_03",
    "SPI2",
    "SPI3",
    "CAN0",
    "CAN1",
    "CAN2",
    "CAN3",
    "I2S",
    "ATIMER",
    "GTIMER",
    "BTIMER",
    "PWM_0_1",
    "PWM_2_3",
    "ADC",
    "HPET0",
    "HPET1",
    "HPET2",
    "HPET3",
    "APB_DMA0",
    "APB_DMA1",
    "APB_DMA2",
    "APB_DMA3",
    "APB_DMA4",
    "APB_DMA5",
    "APB_DMA6",
    "APB_DMA7",
    "SDIO0_CTRL",
    "SDIO1_CTRL",
    "SDIO0_DMA",
    "SDIO1_DMA",
    "ENCRYPT_DMA",
    "AES",
    "DES",
    "SM3",
    "SM4",
    "RTC",
    "TOY",
    "RTC_TICK",
    "TOY_TICK",
    "SPI0",
    "SPI1",
    "EHCI",
    "OHCI",
    "OTG",
    "GMAC0",
    "GMAC1",
    "DC",
    "THSENS",
    "GPIO_0_15",
    "GPIO_16_31",
    "GPIO_32_47",
    "GPIO_48_63",
    "GPIO_64_79",
    "GPIO_80_95",
    "GPIO_96_105",
    "RESERVED",
    "DDR_ECC0",
    "DDR_ECC1",
    "RESERVED",
};
#endif /* RT_USING_INTERRUPT_INFO */

void rt_hw_io_interrupt_mask(int vector)
{
    RT_ASSERT(vector >= 0 && vector < LS2K0300_IO_INT_NUM);
    if (vector < LS2K0300_IO_INT_LOW_NUM)
        HWREG32(TO_UNCACHE(LS2K0300_INTCLR_0)) |= 1 << vector;
    else
        HWREG32(TO_UNCACHE(LS2K0300_INTCLR_1)) |= 1 << (vector - LS2K0300_IO_INT_LOW_NUM);
}

void rt_hw_io_interrupt_umask(int vector)
{
    RT_ASSERT(vector >= 0 && vector < LS2K0300_IO_INT_NUM);
    if (vector < LS2K0300_IO_INT_LOW_NUM)
        HWREG32(TO_UNCACHE(LS2K0300_INTSET_0)) |= 1 << vector;
    else
        HWREG32(TO_UNCACHE(LS2K0300_INTSET_1)) |= 1 << (vector - LS2K0300_IO_INT_LOW_NUM);
}

rt_isr_handler_t rt_hw_io_interrupt_install(int vector, rt_isr_handler_t handler,
                                            void *param, const char *name)
{
    rt_isr_handler_t old = RT_NULL;

    RT_ASSERT(vector >= 0 && vector < LS2K0300_IO_INT_NUM);
    old = io_irq_desc[vector].handler;

#ifdef RT_USING_INTERRUPT_INFO
    rt_strncpy(io_irq_desc[vector].name, name, RT_NAME_MAX);
#endif

    io_irq_desc[vector].handler = handler;
    io_irq_desc[vector].param = param;

    return old;
}

static void route(int vector, int ip)
{
    RT_ASSERT(vector >= 0 && vector < LS2K0300_IO_INT_NUM);
    RT_ASSERT(ip == LS2K0300_IP0 || ip == LS2K0300_IP1 || ip == LS2K0300_IP2 || ip == LS2K0300_IP3);

    if (vector < LS2K0300_IO_ENTRY_NUM * 1)
        HWREG32(TO_UNCACHE(LS2K0300_ENTRY0_0)) = ip;
    else if (vector < LS2K0300_IO_ENTRY_NUM * 2)
        HWREG32(TO_UNCACHE(LS2K0300_ENTRY8_0)) = ip;
    else if (vector < LS2K0300_IO_ENTRY_NUM * 3)
        HWREG32(TO_UNCACHE(LS2K0300_ENTRY16_0)) = ip;
    else if (vector < LS2K0300_IO_ENTRY_NUM * 4)
        HWREG32(TO_UNCACHE(LS2K0300_ENTRY24_0)) = ip;
    else if (vector < LS2K0300_IO_ENTRY_NUM * 5)
        HWREG32(TO_UNCACHE(LS2K0300_ENTRY0_1)) = ip;
    else if (vector < LS2K0300_IO_ENTRY_NUM * 6)
        HWREG32(TO_UNCACHE(LS2K0300_ENTRY8_1)) = ip;
    else if (vector < LS2K0300_IO_ENTRY_NUM * 7)
        HWREG32(TO_UNCACHE(LS2K0300_ENTRY16_1)) = ip;
    else
        HWREG32(TO_UNCACHE(LS2K0300_ENTRY24_1)) = ip;
}

static void handle_io_interrupt(int vector)
{
    rt_isr_handler_t handler;
    void *param;

    handler = io_irq_desc[vector].handler;
    param = io_irq_desc[vector].param;

    if (handler != RT_NULL)
    {
        handler(vector, param);
    }
    else
    {
        rt_interrupt_enter();

#ifdef RT_USING_INTERRUPT_INFO
        rt_kprintf("Unhandled io interrupt %d: %s\n", vector, io_irq_name[vector]);
#else
        rt_kprintf("Unhandled io interrupt %d\n", vector);
#endif

        /* clear and mask interrupt */
        rt_hw_io_interrupt_mask(vector);

        rt_interrupt_leave();
    }
}

static void io_interrupt_handler(int vector, void *param)
{
    rt_uint32_t ien0, isr0, ien1, isr1;
    int i;

    ien0 = HWREG32(TO_UNCACHE(LS2K0300_INTIEN_0));
    isr0 = HWREG32(TO_UNCACHE(LS2K0300_INTISR_0));
    ien1 = HWREG32(TO_UNCACHE(LS2K0300_INTIEN_1));
    isr1 = HWREG32(TO_UNCACHE(LS2K0300_INTISR_1));

    for (i = 0; i < LS2K0300_IO_INT_LOW_NUM; i++)
    {
        if ((isr0 & (1 << i)) && (ien0 & (1 << i)))
        {
            handle_io_interrupt(i);
        }
    }
    for (i = 0; i < LS2K0300_IO_INT_HIGH_NUM; i++)
    {
        if ((isr1 & (1 << i)) && (ien1 & (1 << i)))
        {
            handle_io_interrupt(i + LS2K0300_IO_INT_LOW_NUM);
        }
    }
}

int rt_hw_io_interrupt_init(void)
{
    int i;

    /* disable extended io interrupt */
    HWREG32(TO_UNCACHE(LS2K0300_CHIP_CTRL00)) &= ~CTRL00_EXTIOINT_EN;

    /* clear interrupt status */
    HWREG32(TO_UNCACHE(LS2K0300_INTCLR_0)) = ~0;
    /* low level trigger */
    HWREG32(TO_UNCACHE(LS2K0300_INTEDGE_0)) = 0;
    HWREG32(TO_UNCACHE(LS2K0300_INTPOL_0)) = 0;

    /* clear interrupt status */
    HWREG32(TO_UNCACHE(LS2K0300_INTCLR_1)) = ~0;
    /* low level trigger */
    HWREG32(TO_UNCACHE(LS2K0300_INTEDGE_1)) = 0;
    HWREG32(TO_UNCACHE(LS2K0300_INTPOL_1)) = 0;

    for (i = 0; i < LS2K0300_IO_INT_NUM; i++)
    {
        /* route all interrupts to IP0 (HWI0) */
        route(i, LS2K0300_IP0);
    }

    rt_hw_interrupt_install(LOONGARCH_INT_HWI0, io_interrupt_handler, RT_NULL, "io");
    rt_hw_interrupt_umask(LOONGARCH_INT_HWI0);

    return 0;
}
INIT_BOARD_EXPORT(rt_hw_io_interrupt_init);
```

### 完善串口驱动

在有了 I/O 中断支持后，我们可以为我们的串口驱动添加输入功能了。

除此以外，串口驱动应该作为一个可选驱动，能在配置里打开关闭。
同时，久久派支持 10 个 UART，我们把它们一起添加上。

`bsp/ls2k0300/drivers/drv_uart.c`：

```c
#include <rtdevice.h>
#include <rthw.h>
#include <rtthread.h>

#ifdef BSP_USING_UART

#include "addr.h"
#include "board.h"
#include "drv_uart.h"
#include "interrupt.h"
#include "io_interrupt.h"
#include "loongarch.h"
#include "ls2k0300.h"

#define UART_DAT(base) HWREG8(base + 0x00)
#define UART_IER(base) HWREG8(base + 0x01)
# define IER_IRXE      BIT(0)
# define IER_ILE       BIT(2)
#define UART_IIR(base) HWREG8(base + 0x02)
# define IIR_RXRDY     BIT(2)
# define IIR_RXTOUT    (BIT(3) | BIT(2))
#define UART_FCR(base) HWREG8(base + 0x02)
#define UART_LCR(base) HWREG8(base + 0x03)
#define UART_MCR(base) HWREG8(base + 0x04)
#define UART_LSR(base) HWREG8(base + 0x05)
# define LSR_DR        BIT(0)
# define LSR_TFE       BIT(5)
# define LSR_TE        BIT(6)
#define UART_MSR(base) HWREG8(base + 0x06)

struct rt_uart_device
{
    const char name[RT_NAME_MAX];
    rt_ubase_t base;
    rt_uint32_t irq;
};

static rt_err_t uart_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
    struct rt_uart_device *uart = RT_NULL;

    RT_ASSERT(serial != RT_NULL);
    RT_ASSERT(cfg != RT_NULL);

    uart = (struct rt_uart_device *)serial->parent.user_data;

    UART_IER(uart->base) = 0x00;
    UART_FCR(uart->base) = 0xc1;
    UART_LCR(uart->base) = 0x03;
    UART_MCR(uart->base) = 0x03;
    UART_LSR(uart->base) = 0x60;
    UART_MSR(uart->base) = 0xb0;

    return RT_EOK;
}

static rt_err_t uart_control(struct rt_serial_device *serial, int cmd, void *arg)
{
    struct rt_uart_device *uart = RT_NULL;

    RT_ASSERT(serial != RT_NULL);

    uart = (struct rt_uart_device *)serial->parent.user_data;

    switch (cmd)
    {
    case RT_DEVICE_CTRL_CLR_INT:
        rt_hw_io_interrupt_mask(uart->irq);
        break;

    case RT_DEVICE_CTRL_SET_INT:
        UART_IER(uart->base) |= (IER_IRXE | IER_ILE);
        rt_hw_io_interrupt_umask(uart->irq);
        break;

    default:
        break;
    }

    return RT_EOK;
}

static int uart_putc(struct rt_serial_device *serial, char c)
{
    struct rt_uart_device *uart = RT_NULL;
    rt_uint32_t status;

    RT_ASSERT(serial != RT_NULL);

    uart = (struct rt_uart_device *)serial->parent.user_data;

    status = UART_LSR(uart->base);
    while (!(status & (LSR_TE | LSR_TFE)))
    {
        status = UART_LSR(uart->base);
    }

    UART_DAT(uart->base) = c;

    return 1;
}

static int uart_getc(struct rt_serial_device *serial)
{
    struct rt_uart_device *uart = RT_NULL;

    RT_ASSERT(serial != RT_NULL);

    uart = (struct rt_uart_device *)serial->parent.user_data;

    if (UART_LSR(uart->base) & LSR_DR)
    {
        return UART_DAT(uart->base);
    }

    return -1;
}

static const struct rt_uart_ops uart_ops =
{
    .configure    = uart_configure,
    .control      = uart_control,
    .putc         = uart_putc,
    .getc         = uart_getc,
    .dma_transmit = RT_NULL,
};

static void rt_hw_uart_handler(int vector, void *param)
{
    struct rt_serial_device *serial = RT_NULL;
    struct rt_uart_device *uart = RT_NULL;
    rt_uint8_t iir;

    rt_interrupt_enter();

    serial = (struct rt_serial_device *)param;
    uart = (struct rt_uart_device *)serial->parent.user_data;
    iir = UART_IIR(uart->base);
    if ((iir & IIR_RXRDY) || (iir & IIR_RXTOUT))
    {
        rt_hw_serial_isr(serial, RT_SERIAL_EVENT_RX_IND);
    }

    rt_interrupt_leave();
}

#ifdef BSP_USING_UART0
static struct rt_uart_device uart0_device =
{
    .name = "uart0",
    .base = TO_UNCACHE(LS2K0300_UART0),
    .irq = LS2K0300_IO_INT_UART00,
};
static struct rt_serial_device serial0;
#endif /* BSP_USING_UART0 */

#ifdef BSP_USING_UART1
static struct rt_uart_device uart1_device =
{
    .name = "uart1",
    .base = TO_UNCACHE(LS2K0300_UART1),
    .irq = LS2K0300_IO_INT_UART01,
};
static struct rt_serial_device serial1;
#endif /* BSP_USING_UART1 */

#ifdef BSP_USING_UART2
static struct rt_uart_device uart2_device =
{
    .name = "uart2",
    .base = TO_UNCACHE(LS2K0300_UART2),
    .irq = LS2K0300_IO_INT_UART_02_05,
};
static struct rt_serial_device serial2;
#endif /* BSP_USING_UART2 */

#ifdef BSP_USING_UART3
static struct rt_uart_device uart3_device =
{
    .name = "uart3",
    .base = TO_UNCACHE(LS2K0300_UART3),
    .irq = LS2K0300_IO_INT_UART_02_05,
};
static struct rt_serial_device serial3;
#endif /* BSP_USING_UART3 */

#ifdef BSP_USING_UART4
static struct rt_uart_device uart4_device =
{
    .name = "uart4",
    .base = TO_UNCACHE(LS2K0300_UART4),
    .irq = LS2K0300_IO_INT_UART_02_05,
};
static struct rt_serial_device serial4;
#endif /* BSP_USING_UART4 */

#ifdef BSP_USING_UART5
static struct rt_uart_device uart5_device =
{
    .name = "uart5",
    .base = TO_UNCACHE(LS2K0300_UART5),
    .irq = LS2K0300_IO_INT_UART_02_05,
};
static struct rt_serial_device serial5;
#endif /* BSP_USING_UART5 */

#ifdef BSP_USING_UART6
static struct rt_uart_device uart6_device =
{
    .name = "uart6",
    .base = TO_UNCACHE(LS2K0300_UART6),
    .irq = LS2K0300_IO_INT_UART_06_09,
};
static struct rt_serial_device serial6;
#endif /* BSP_USING_UART6 */

#ifdef BSP_USING_UART7
static struct rt_uart_device uart7_device =
{
    .name = "uart7",
    .base = TO_UNCACHE(LS2K0300_UART7),
    .irq = LS2K0300_IO_INT_UART_06_09,
};
static struct rt_serial_device serial7;
#endif /* BSP_USING_UART7 */

#ifdef BSP_USING_UART8
static struct rt_uart_device uart8_device =
{
    .name = "uart8",
    .base = TO_UNCACHE(LS2K0300_UART8),
    .irq = LS2K0300_IO_INT_UART_06_09,
};
static struct rt_serial_device serial8;
#endif /* BSP_USING_UART8 */

#ifdef BSP_USING_UART9
static struct rt_uart_device uart9_device =
{
    .name = "uart9",
    .base = TO_UNCACHE(LS2K0300_UART9),
    .irq = LS2K0300_IO_INT_UART_06_09,
};
static struct rt_serial_device serial9;
#endif /* BSP_USING_UART9 */

static rt_err_t uart_init(struct rt_serial_device *serial, struct rt_uart_device *uart)
{
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
    rt_err_t ret;

    RT_ASSERT(serial != RT_NULL);
    RT_ASSERT(uart != RT_NULL);

    serial->ops = &uart_ops;
    serial->config = config;

    ret = rt_hw_serial_register(serial, uart->name,
                                RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                                uart);

    if (ret != RT_EOK)
    {
        return ret;
    }

    rt_hw_io_interrupt_install(uart->irq, rt_hw_uart_handler, (void *)serial, uart->name);
    rt_hw_io_interrupt_umask(uart->irq);

    return ret;
}

int rt_hw_uart_init(void)
{
#ifdef BSP_USING_UART0
    if (uart_init(&serial0, &uart0_device) != RT_EOK)
    {
        return -1;
    }
#endif /* BSP_USING_UART0 */

#ifdef BSP_USING_UART1
    if (uart_init(&serial1, &uart1_device) != RT_EOK)
    {
        return -1;
    }
#endif /* BSP_USING_UART1 */

#ifdef BSP_USING_UART2
    if (uart_init(&serial2, &uart2_device) != RT_EOK)
    {
        return -1;
    }
#endif /* BSP_USING_UART2 */

#ifdef BSP_USING_UART3
    if (uart_init(&serial3, &uart3_device) != RT_EOK)
    {
        return -1;
    }
#endif /* BSP_USING_UART3 */

#ifdef BSP_USING_UART4
    if (uart_init(&serial4, &uart4_device) != RT_EOK)
    {
        return -1;
    }
#endif /* BSP_USING_UART4 */

#ifdef BSP_USING_UART5
    if (uart_init(&serial5, &uart5_device) != RT_EOK)
    {
        return -1;
    }
#endif /* BSP_USING_UART5 */

#ifdef BSP_USING_UART6
    if (uart_init(&serial6, &uart6_device) != RT_EOK)
    {
        return -1;
    }
#endif /* BSP_USING_UART6 */

#ifdef BSP_USING_UART7
    if (uart_init(&serial7, &uart7_device) != RT_EOK)
    {
        return -1;
    }
#endif /* BSP_USING_UART7 */

#ifdef BSP_USING_UART8
    if (uart_init(&serial8, &uart8_device) != RT_EOK)
    {
        return -1;
    }
#endif /* BSP_USING_UART8 */

#ifdef BSP_USING_UART9
    if (uart_init(&serial9, &uart9_device) != RT_EOK)
    {
        return -1;
    }
#endif /* BSP_USING_UART9 */

    return 0;
}
INIT_BOARD_EXPORT(rt_hw_uart_init);

#endif /* BSP_USING_UART */
```

添加配置选项需要增加 `bsp/ls2k0300/drivers/Kconfig`，同时需要在上级配置文件（`bsp/ls2k0300/Kconfig`）中引入该文件。

`bsp/ls2k0300/Kconfig`：

```text
...
source "$(RTT_DIR)/Kconfig"
osource "$PKGS_DIR/Kconfig"
rsource "drivers/Kconfig"
...
```

`bsp/ls2k0300/drivers/Kconfig`：

```text
menu "Hardware Drivers Config"

    menuconfig BSP_USING_UART
        bool "Using UART"
        select RT_USING_SERIAL
        default y

        if BSP_USING_UART
            config BSP_USING_UART0
                bool "Using UART0"
                default y

            config BSP_USING_UART1
                bool "Using UART1"
                default n

            config BSP_USING_UART2
                bool "Using UART2"
                default n

            config BSP_USING_UART3
                bool "Using UART3"
                default n

            config BSP_USING_UART4
                bool "Using UART4"
                default n

            config BSP_USING_UART5
                bool "Using UART5"
                default n

            config BSP_USING_UART6
                bool "Using UART6"
                default n

            config BSP_USING_UART7
                bool "Using UART7"
                default n

            config BSP_USING_UART8
                bool "Using UART8"
                default n

            config BSP_USING_UART9
                bool "Using UART9"
                default n
        endif

endmenu
```

别忘记执行 `scons --menuconfig` 更新配置。
编译运行，RT-Thread 应该能正常进入 MSH，能够执行基本的指令。

## 后续工作

因为这是非常简化版本的内核，所以很多功能还没实现，但雏形已经有了，后续就是不停添加功能和适当重构。

*那么之后可以做哪些工作呢？*

- 添加分页支持，实现 TLB 相关内容。
- 完善异常处理。
- 添加其他驱动，如 I2C、GPIO、GMAC 等。
- 实现其他 RT-Thread 的功能，如支持 RT-Smart 等。
- ……

移植 RT-Thread 就是不停查文档、抄代码、该代码、重构、调试的过程，等时机成熟，还可以把代码推到上游。

RT-Thread 移植教程到此结束，感谢大佬赠送的久久派！
