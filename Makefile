# Makefile for the simple kernel.
.DEFAULT_GOAL := all

CC	= aarch64-linux-gnu-gcc
LD	= aarch64-linux-gnu-ld
OBJCOPY = aarch64-linux-gnu-objcopy
QEMU = qemu-system-aarch64

CFLAGS = -mcpu=cortex-a72+nofp -Wall -fstrength-reduce -finline-functions -nostdinc -fno-builtin -fno-stack-protector

# Add debug symbol
CFLAGS += -g

CFLAGS += -I.

OBJDIR = .

include kernel/Makefile

all: kernel/kernel8.elf
	$(OBJCOPY) -O binary $< $(OBJDIR)/kernel8.img

clean:
	rm -rf $(OBJDIR)/kernel/*.o $(OBJDIR)/kernel/kernel8.*
	rm -rf $(OBJDIR)/lib/*.o
	rm -rf $(OBJDIR)/kernel8.img
