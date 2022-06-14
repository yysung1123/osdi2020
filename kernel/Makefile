KERN_OBJS = kernel/start.o \
	kernel/main.o \
	kernel/mmio.o \
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
	kernel/test_case.o

ifeq ($(WITH_BMP_SPLASH),1)
	KERN_OBJS += kernel/splash.o
	CFLAGS += -DWITH_BMP_SPLASH
endif

KERN_LDFLAGS = -T kernel/kern.ld

ULIB = lib/printfmt.o \
	lib/string.o \
	lib/syscall.o \
	lib/printf.o \
	lib/puts.o \
	lib/putchar.o \
	lib/gets_s.o \
	lib/getchar.o

USER_OBJS = user/shell.o \
	user/utask.o

kernel/kernel8.elf: $(KERN_OBJS) $(ULIB) $(USER_OBJS)
	$(LD) $(KERN_LDFLAGS) $(KERN_OBJS) $(ULIB) $(USER_OBJS) -o $@

kernel/kernel8.%.elf: $(KERN_OBJS) $(ULIB) $(USER_OBJS)
	$(LD) -Ttext=$* $(KERN_LDFLAGS) $(KERN_OBJS) $(ULIB) $(USER_OBJS) -o $@

kernel/%.o: kernel/%.S
	$(CC) $(CFLAGS) -c -o $@ $<

kernel/%.o: kernel/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

lib/%.o: lib/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

user/%.o: user/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
