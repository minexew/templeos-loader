#!/bin/sh

objdump -b binary --adjust-vma=0x107c00 -D MiniSystem/Kernel/HolyCRT.BIN -m i386:x86-64
