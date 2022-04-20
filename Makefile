# Makefile for the simple kernel.
.DEFAULT_GOAL := all

CC = aarch64-linux-gnu-gcc
LD = aarch64-linux-gnu-ld
OBJCOPY = aarch64-linux-gnu-objcopy
QEMU = qemu-system-aarch64

CFLAGS = -std=c17 -Wall -fstrength-reduce -finline-functions -nostdinc -fno-builtin -fno-stack-protector

# Add debug symbol
CFLAGS += -g

CFLAGS += -I.

OBJDIR = .

include kernel/Makefile

all: kernel/kernel8.elf
	$(OBJCOPY) -O binary $< $(OBJDIR)/kernel8.img

ADDR = 0x80000
kernel_with_addr: kernel/kernel8.$(ADDR).elf
	$(OBJCOPY) -O binary $< $(OBJDIR)/kernel8.$(ADDR).img

clean:
	rm -rf $(OBJDIR)/kernel/*.o $(OBJDIR)/kernel/kernel8.*
	rm -rf $(OBJDIR)/lib/*.o
	rm -rf $(OBJDIR)/kernel8*.img

qemu: all
	$(QEMU) -M raspi3b -kernel kernel8.img -display none -serial pty

debug: all
	$(QEMU) -M raspi3b -kernel kernel8.img -display none -s -S -serial pty
