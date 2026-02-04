# PS2 Makefile
# This Makefile works with the ps2dev toolchain

# Get the name from command line, default to "main"
TARGET ?= main

# PS2Dev paths - these should be set by the flake
PS2DEV ?= $(HOME)/ps2dev
PS2SDK ?= $(PS2DEV)/ps2sdk
GSKIT ?= $(PS2DEV)/gsKit

# Ensure PATH includes toolchain
export PATH := $(PATH):$(PS2DEV)/ee/bin:$(PS2DEV)/bin:$(PS2SDK)/bin

# Toolchain prefix
EE_PREFIX = mips64r5900el-ps2-elf-

# Compiler and tools
EE_CC = $(EE_PREFIX)gcc
EE_CXX = $(EE_PREFIX)g++
EE_LD = $(EE_PREFIX)gcc
EE_AS = $(EE_PREFIX)as
EE_OBJCOPY = $(EE_PREFIX)objcopy
EE_STRIP = $(EE_PREFIX)strip

# Compiler flags
EE_CFLAGS = -D_EE -O2 -G0 -Wall
EE_CFLAGS += -I$(PS2SDK)/ee/include
EE_CFLAGS += -I$(PS2SDK)/common/include
EE_CFLAGS += -I$(PS2SDK)/sbv/include
EE_CFLAGS += -I$(GSKIT)/include
EE_CFLAGS += -I$(GSKIT)/ee/gs/include
EE_CFLAGS += -I$(GSKIT)/ee/include

# Linker flags
EE_LDFLAGS = -L$(PS2SDK)/ee/lib
EE_LDFLAGS += -L$(PS2DEV)/ee/lib
EE_LDFLAGS += -L$(GSKIT)/lib
EE_LDFLAGS += -L$(PS2SDK)/ee/lib/startup
EE_LDFLAGS += -T$(PS2SDK)/ee/startup/linkfile

# Libraries (startup code is handled by linker script)
EE_LIBS = -lkernel -lcglue -lcdvd -lgskit -ldmakit -lpad -lpacket -lm

# Source files
EE_SRC = src/demo.c
EE_OBJS = src/demo.o

# Output
EE_BIN = $(TARGET).elf

.PHONY: all clean run help

all: $(EE_BIN)

$(EE_BIN): $(EE_OBJS)
	$(EE_LD) $(EE_LDFLAGS) -o $@ $^ $(EE_LIBS)
	@echo "Built: $@"

%.o: %.c
	$(EE_CC) $(EE_CFLAGS) -c $< -o $@

src/%.o: src/%.c
	$(EE_CC) $(EE_CFLAGS) -c $< -o $@

clean:
	rm -f src/demo.o *.elf
	@echo "Cleaned build artifacts"

run: $(EE_BIN)
	@if command -v pcsx2-emulator > /dev/null; then \
		echo "Launching PCSX2..."; \
		echo "Load this ELF through the GUI: $(EE_BIN)"; \
		pcsx2-emulator; \
	elif command -v pcsx2-qt > /dev/null; then \
		pcsx2-qt "$(PWD)/$(EE_BIN)"; \
	elif command -v pcsx2 > /dev/null; then \
		pcsx2 "$(PWD)/$(EE_BIN)"; \
	elif command -v PCSX2 > /dev/null; then \
		PCSX2 "$(PWD)/$(EE_BIN)"; \
	else \
		echo "PCSX2 not found. Install and load ELF manually:"; \
		echo "  File: $(EE_BIN)"; \
		echo "  Full path: $(PWD)/$(EE_BIN)"; \
	fi

help:
	@echo "PS2 Makefile"
	@echo ""
	@echo "Usage:"
	@echo "  make TARGET=<name>     Build specific target (default: main)"
	@echo "  make all               Build the ELF file"
	@echo "  make clean             Clean build artifacts"
	@echo "  make run               Build and run in PCSX2"
	@echo ""
	@echo "Example:"
	@echo "  make TARGET=main all"
