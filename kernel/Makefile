KERN_OBJS = kernel/entry.o \
	kernel/main.o \
	kernel/mmio.o \
	kernel/uart.o

KERN_LDFLAGS = -T kernel/kern.ld

kernel/kernel8.elf: $(KERN_OBJS)
	$(LD) $(KERN_LDFLAGS) $(KERN_OBJS) -o $@

kernel/%.o: kernel/%.S
	$(CC) $(CFLAGS) -c -o $@ $<

kernel/%.o: kernel/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
