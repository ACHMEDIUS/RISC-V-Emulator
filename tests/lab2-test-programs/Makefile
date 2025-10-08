# Compile all the test programs for CA 2025 assignment 2

# The x86 target will compile x86 versions of the main test files so you
# can check what the output is supposed to be

TARGETS = basic.bin simple.bin simple-stalls.bin msg.bin triangle.bin \
	hellof.bin comp.bin brainfck.bin \
	hellof.s comp.s roman.s

SRC_DIR = src/
BIN_DIR =
CRT = $(SRC_DIR)minicrt-riscv.s
HEADERS = $(wildcard $(SRC_DIR)*.h)

BIN_TARGETS = $(patsubst %.bin,$(BIN_DIR)%.bin,$(TARGETS))

CC = riscv64-unknown-elf-gcc
CFLAGS_COMMON = -Wall -nostdlib -fno-builtin -nodefaultlibs
CFLAGS = -march=rv64imafd $(CFLAGS_COMMON)

.phony: all clean x86

all:	$(TARGETS)


#
# The targets for the different levels
# The .s files are generated for level 7, in which case stalls need to
# be inserted manually.
#

# Level 1: basic.bin
basic.bin:	$(SRC_DIR)basic.s $(HEADERS)
		$(CC) $(CFLAGS) -o $@ $<

# Level 2: simple.bin
simple.bin:	$(SRC_DIR)simple.s $(HEADERS)
		$(CC) $(CFLAGS) -o $@ $<

# Level 3: msg.bin
msg.bin:	$(SRC_DIR)msg.s $(HEADERS)
		$(CC) $(CFLAGS) -o $@ $<

# Level 4: triangle.bin
triangle.bin:	$(SRC_DIR)triangle.s $(HEADERS)
		$(CC) $(CFLAGS) -o $@ $<

# Level 5: hellof.bin, compiled with -O1
hellof.bin:	$(SRC_DIR)hellof.c $(HEADERS)
		$(CC) $(CFLAGS) -O1 -o $@ $< $(CRT)

hellof.s:	$(SRC_DIR)hellof.c $(HEADERS)
		$(CC) $(CFLAGS) -O1 -S -o $@ $<

# Level 6: comp.bin, compiled with -O2
comp.bin:	$(SRC_DIR)comp.c $(SRC_DIR)roman.c $(HEADERS)
		$(CC) $(CFLAGS) -O2 -o $@ $< $(SRC_DIR)roman.c $(CRT)

comp.s:		$(SRC_DIR)comp.c $(HEADERS)
		$(CC) $(CFLAGS) -O2 -S -o $@ $<

roman.s:	$(SRC_DIR)roman.c $(HEADERS)
		$(CC) $(CFLAGS) -O2 -S -o $@ $<

# Level 7: example: simple-stalls.bin
simple-stalls.bin:	$(SRC_DIR)simple-stalls.s $(HEADERS)
			$(CC) $(CFLAGS) -o $@ $<

# Level 9: brainfck.bin, compiled with -O2
brainfck.bin:	$(SRC_DIR)brainfck.c $(HEADERS)
		$(CC) $(CFLAGS) -O2 -o $@ $< $(CRT)


clean:
		rm -f $(BIN_TARGETS)
		rm -fr $(X86_DIR)

#
# x86 support
#
# Compile x86 binaries for all the test programs to see the correct output
#

X86_CC = gcc
X86_CFLAGS = -Wall -O2

# Subdirectory for output
X86_DIR = x86out
# Targets for everything but basic, simple, simple-stalls, msg, triangle (can't compile RISC-V asm as x86)
X86_NOTARGETS = basic.bin simple.bin simple-stalls.bin msg.bin triangle.bin
X86_TARGETS = $(patsubst %.bin,$(X86_DIR)/%,$(filter-out $(X86_NOTARGETS),$(TARGETS)))

x86: $(X86_DIR) $(X86_TARGETS)

$(X86_DIR):
		mkdir -p $(X86_DIR)

# The three examples that require roman.c
$(X86_DIR)/comp: $(SRC_DIR)comp.c $(SRC_DIR)roman.c $(HEADERS)
		$(X86_CC) $(X86_CFLAGS) -o $@ $< $(SRC_DIR)roman.c

# Generic rule for the rest
$(X86_DIR)/%: $(SRC_DIR)%.c $(HEADERS)
		$(X86_CC) $(X86_CFLAGS) -o $@ $<
