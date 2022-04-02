KERN_OBJS = kernel/entry.o

KERN_LDFLAGS = -T kernel/kern.ld

kernel/kernel8.elf: $(KERN_OBJS)
	$(LD) $(KERN_LDFLAGS) $(KERN_OBJS) -o $@

kernel/%.o: kernel/%.S
	$(CC) $(CFLAGS) -c -o $@ $<
