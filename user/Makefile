USER_LDFLAGS = -T user/user.ld

ULIB = lib/printfmt.o \
	lib/string.o \
	lib/syscall.o \
	lib/printf.o \
	lib/puts.o \
	lib/putchar.o \
	lib/gets_s.o \
	lib/getchar.o

include lib/Makefile

user/%.bin: user/%
	$(OBJCOPY) -O binary $< $<
	$(LD) -r -b binary $< -o $@

user/%: user/%.o user/crt0.o $(ULIB)
	$(LD) $(USER_LDFLAGS) $+ -o $@

user/%.o: user/%.c
	$(CC) $(CFLAGS) -fno-zero-initialized-in-bss -c -o $@ $<

user/%.o: user/%.S
	$(CC) $(CFLAGS) -c -o $@ $<
