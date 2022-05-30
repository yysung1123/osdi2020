KERN_OBJS = kernel/start.o \
	kernel/main.o \
	kernel/mmio.o \
	kernel/uart.o \
	kernel/shell.o \
	kernel/mbox.o \
	kernel/printk.o \
	kernel/fb.o \
	kernel/entry.o \
	kernel/exc.o \
	kernel/timer.o \
	kernel/irq.o

ifeq ($(WITH_BMP_SPLASH),1)
	KERN_OBJS += kernel/splash.o
	CFLAGS += -DWITH_BMP_SPLASH
endif

KERN_LDFLAGS = -T kernel/kern.ld

ULIB = lib/printfmt.o \
       lib/string.o

kernel/kernel8.elf: $(KERN_OBJS) $(ULIB)
	$(LD) $(KERN_LDFLAGS) $(KERN_OBJS) $(ULIB) -o $@

kernel/kernel8.%.elf: $(KERN_OBJS) $(ULIB)
	$(LD) -Ttext=$* $(KERN_LDFLAGS) $(KERN_OBJS) $(ULIB) -o $@

kernel/%.o: kernel/%.S
	$(CC) $(CFLAGS) -c -o $@ $<

kernel/%.o: kernel/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

lib/%.o: lib/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
