KERN_OBJS = kernel/start.o \
	kernel/main.o \
	kernel/uart.o \
	kernel/mbox.o \
	kernel/printk.o \
	kernel/fb.o \
	kernel/entry.o \
	kernel/exc.o \
	kernel/timer.o \
	kernel/irq.o \
	kernel/syscall.o \
	kernel/utils.o \
	kernel/task.o \
	kernel/switch_to.o \
	kernel/sched.o \
	kernel/signal.o \
	kernel/assert.o \
	kernel/mm.o \
	kernel/fault.o

ifeq ($(WITH_BMP_SPLASH),1)
	KERN_OBJS += kernel/splash.o
	CFLAGS += -DWITH_BMP_SPLASH
endif

KERN_LDFLAGS = -T kernel/kern.ld

KLIB = lib/printfmt.o \
	lib/string.o \
	lib/mango_tree.o

USER_PROGS = user/shell.bin

include lib/Makefile
include user/Makefile

kernel/kernel8.elf: $(KERN_OBJS) $(KLIB) $(USER_PROGS)
	$(LD) $(KERN_LDFLAGS) $(KERN_OBJS) $(KLIB) $(USER_PROGS) -o $@

kernel/kernel8.%.elf: $(KERN_OBJS) $(KLIB)
	$(LD) -Ttext=$* $(KERN_LDFLAGS) $(KERN_OBJS) $(KLIB) -o $@

kernel/%.o: kernel/%.S
	$(CC) $(CFLAGS) -c -o $@ $<

kernel/%.o: kernel/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
