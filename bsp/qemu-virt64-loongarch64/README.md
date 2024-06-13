# QEMU/LoongArch64 VIRT64 BSP Introduction

## 1. Introduction

This project ported RT-Thread on QEMU LoongArch64 VIRT64 machine.

## 2. Compiling

Download the `loongarch64-newlib-elf-xxx` of x86_64 hosted platform，set the `RTT_EXEC_PATH` is system environment after decompress the binary.

```bash
cd rt-thread/bsp/qemu-virt64-loongarch64
scons
```

## 3. Execution

The project execution tool is `qemu-system-loongarch64`.

```bash
cd rt-thread/bsp/qemu-virt64-loongarch64
./qemu.sh
```
