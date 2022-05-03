BUILD_DIR = ./build
# Lib
LIB_SRC_DIR = .

LIB_SRC = $(wildcard $(LIB_SRC_DIR)/*.c) 

LIB_ASM = $(wildcard $(LIB_SRC_DIR)/*.S)

LIB_INCLUDES = \
	-I $(LIB_SRC_DIR)/include \

LIB_BUILD_DIR = $(BUILD_DIR)/lib
LIB_OBJS := $(patsubst %.c,$(LIB_BUILD_DIR)/%.o,$(notdir $(LIB_SRC)))
LIB_OBJS += $(patsubst %.S,$(LIB_BUILD_DIR)/%_s.o,$(notdir $(LIB_ASM)))

# ARM toolchain
CROSS = aarch64-unknown-linux-gnu
CC = $(CROSS)-gcc
AS = $(CROSS)-as
LD = $(CROSS)-ld
OC = $(CROSS)-objcopy
OD = $(CROSS)-objdump
SP = $(CROSS)-strip

BUILD_DIRECTORIES = \
	$(LIB_BUILD_DIR) \

PROJ = lib

CCFLAGS = -Wall -O0 -fno-builtin -w
LDFLAGS = -T$(LINKER_FILE) -nostdlib -nostartfiles

.PHONY: clean directories out_elf run
all: directories $(LIB_OBJS)
directories: $(BUILD_DIRECTORIES)

# Compile Object Files ---------------------------------------------------------
$(LIB_BUILD_DIR)/%.o : %.c
	@echo '============================================================'
	@echo ' Building target: $@'
	@echo '============================================================'
	@echo "[+] Building: $<"
	$(CC) $(CCFLAGS) $(LIB_INCLUDES) -g -o $@ -c $<
	@echo "[+] Finish Building: $<"
	@echo ''

$(LIB_BUILD_DIR)/%_s.o : %.S
	@echo '============================================================'
	@echo ' Building target: $@'
	@echo '============================================================'
	@echo "[+] Building: $<"
	$(CC) $(CCFLAGS) $(LIB_INCLUDES) -g -o $@ -c $<
	@echo "[+] Finish Building: $<"
	@echo ''

$(BUILD_DIRECTORIES):
	@echo $(LIB_BUILD_DIR)
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)