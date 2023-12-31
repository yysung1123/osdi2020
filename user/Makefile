UCFLAGS = -mcpu=cortex-a53 -std=c17 -O3 -Wall -nostdinc -fno-builtin -fno-stack-protector -I.

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
	$(LD) -r -b binary $< -o $@

user/%: user/%.o user/crt0.o $(ULIB)
	$(LD) $+ -o $@

user/%.o: user/%.c
	$(CC) $(UCFLAGS) -c -o $@ $<

user/%.o: user/%.S
	$(CC) $(UCFLAGS) -c -o $@ $<
