@echo off

:run
qemu-system-loongarch64 ^
-nographic ^
-machine virt ^
-cpu la464 ^
-m 1G ^
-kernel rtthread.elf
