#!/bin/bash

"/c/Program Files (x86)/GNU Tools Arm Embedded/5.4 2016q3/bin/arm-none-eabi-gcc" -ffreestanding -nostdlib -Wall -mcpu=cortex-m0 -mlittle-endian -mthumb -T STM32F07x.ld min.S -o min.elf

"/c/Program Files (x86)/GNU Tools Arm Embedded/5.4 2016q3/bin/arm-none-eabi-objcopy" -Oihex min.elf min.hex
