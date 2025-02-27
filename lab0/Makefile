BUILD_DIR = ./build
# Lib
LIB_SRC_DIR = ./lib

# Application
APP_SRC_DIR = .

APP_ASM = \
	$(APP_SRC_DIR)/boot.S \

APP_BUILD_DIR = $(BUILD_DIR)/app
APP_OBJS := $(patsubst %.c,$(APP_BUILD_DIR)/%.o,$(notdir $(APP_SRC)))
APP_OBJS += $(patsubst %.S,$(APP_BUILD_DIR)/%.o,$(notdir $(APP_ASM)))

VPATH += \
	$(LIB_SRC_DIR) \
	$(APP_SRC_DIR) \

# ARM toolchain
CROSS = aarch64-unknown-linux-gnu
CC = $(CROSS)-gcc
AS = $(CROSS)-as
LD = $(CROSS)-ld
OC = $(CROSS)-objcopy
OD = $(CROSS)-objdump
SP = $(CROSS)-strip

# Project
OUT_OBJS = \
	$(LIB_OBJS) \
	$(APP_OBJS) \

BUILD_DIRECTORIES = \
	$(LIB_BUILD_DIR) \
	$(APP_BUILD_DIR) \

PROJ = kernel8
OUT_ELF = ./$(PROJ).elf
OUT_IMG = ./$(PROJ).img
LINKER_FILE = linker.ld

CCFLAGS = -Wall -O0
LDFLAGS = -T$(LINKER_FILE) -nostdlib -nostartfiles

.PHONY: clean directories out_elf run docker_cp
all: directories $(OUT_OBJS) $(OUT_ELF) $(OUT_IMG)
directories: $(BUILD_DIRECTORIES)
out_elf: directories $(OUT_ELF)
out_img: directories $(OUT_IMG)

# Compile Object Files ---------------------------------------------------------
$(APP_BUILD_DIR)/%.o : %.S
	@echo '============================================================'
	@echo ' Building target: $@'
	@echo '============================================================'
	@echo "[+] Building: $<"
	$(CC) $(ASMFLAGS) $(APP_INCLUDES) -g -o $@ -c $<
	@echo "[+] Finished Building: $<"
	@echo ''

# Generate ELF -----------------------------------------------------------------
$(OUT_ELF): $(OUT_OBJS)
	@echo '============================================================'
	@echo ' Building target: $@'
	@echo '============================================================'
	$(LD) $(LDFLAGS) -o $@ $(OUT_OBJS)
	$(OD) -d $@ > $(BUILD_DIR)/$(PROJ).objdump
	# $(SP) $@
	@echo '[+] Finished building target: $@'
	@echo ' '

# Generate IMG -----------------------------------------------------------------
$(OUT_IMG): $(OUT_ELF)
	@echo '============================================================'
	@echo ' Building target: $@'
	@echo '============================================================'
	$(OC) -O binary $(OUT_ELF) $@
	@echo '[+] Finished building target: $@'
	@echo ' '

$(BUILD_DIRECTORIES):
	mkdir -p $@

run:
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -serial null -serial stdio

debug:
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -serial null -serial stdio -s -S

docker_cp:
	docker cp kernel8.elf my_kali:/

clean:
	rm -rf *.elf
	rm -rf *.img
	rm -rf $(BUILD_DIR)