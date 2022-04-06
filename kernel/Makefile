KERN_OBJS = kernel/entry.o \
	kernel/main.o \
	kernel/mmio.o \
	kernel/uart.o \
	kernel/shell.o

KERN_LDFLAGS = -T kernel/kern.ld

ULIB = lib/printfmt.o \
       lib/string.o

kernel/kernel8.elf: $(KERN_OBJS) $(ULIB)
	$(LD) $(KERN_LDFLAGS) $(KERN_OBJS) $(ULIB) -o $@

kernel/%.o: kernel/%.S
	$(CC) $(CFLAGS) -c -o $@ $<

kernel/%.o: kernel/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
