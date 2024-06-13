#!/bin/bash

qemu-system-loongarch64 \
-nographic \
-machine virt \
-cpu la132 \
-m 1G \
-kernel rtthread.elf \
-s -S
