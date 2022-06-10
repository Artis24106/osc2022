include ../config.mk

BUILD_DIR = ./build
# Lib
LIB_SRC_DIR = ../lib

LIB_SRC := $(wildcard $(LIB_SRC_DIR)/*.c) 
LIB_SRC := $(filter-out $(LIB_SRC_DIR)/uart_boot.c, $(LIB_SRC)) 

LIB_ASM := $(wildcard $(LIB_SRC_DIR)/*.S)

LIB_INCLUDES = \
	-I $(LIB_SRC_DIR)/include \

LIB_BUILD_DIR = $(LIB_SRC_DIR)/build/lib
LIB_OBJS := $(patsubst %.c,$(LIB_BUILD_DIR)/%.o,$(notdir $(LIB_SRC)))
LIB_OBJS += $(patsubst %.S,$(LIB_BUILD_DIR)/%_s.o,$(notdir $(LIB_ASM)))

# Application
APP_SRC_DIR = .

APP_SRC = \
	$(APP_SRC_DIR)/main.c \

APP_ASM = $(wildcard $(APP_SRC_DIR)/*.S) 

APP_INCLUDES = \
	-I ./ \
	$(LIB_INCLUDES) \

APP_BUILD_DIR = $(BUILD_DIR)/app
APP_OBJS := $(patsubst %.c,$(APP_BUILD_DIR)/%.o,$(notdir $(APP_SRC)))
APP_OBJS += $(patsubst %.S,$(APP_BUILD_DIR)/%.o,$(notdir $(APP_ASM)))

# Project
OUT_OBJS = \
	$(LIB_OBJS) \
	$(APP_OBJS) \

BUILD_DIRECTORIES = \
	$(LIB_BUILD_DIR) \
	$(APP_BUILD_DIR) \

PROJ = kernel
OUT_ELF = ./$(PROJ).elf
OUT_IMG = ./$(PROJ).img
LINKER_FILE = linker.ld

.PHONY: clean directories out_elf run
all: pre_build directories $(OUT_ELF) $(OUT_IMG)
pre_build:
	$(call show_header,${PROJ})
directories: $(BUILD_DIRECTORIES)
out_elf: directories $(OUT_ELF)
out_img: directories $(OUT_IMG)

# Compile Object Files ---------------------------------------------------------
$(APP_BUILD_DIR)/%.o : %.c
	$(call show,$@)
	$(CC) $(CCFLAGS) $(APP_INCLUDES) -g -o $@ -c $<

$(APP_BUILD_DIR)/%.o : %.S
	$(call show,$@)
	$(CC) $(ASMFLAGS) $(APP_INCLUDES) -g -o $@ -c $<

# Generate ELF -----------------------------------------------------------------
$(OUT_ELF): $(OUT_OBJS)
	# $(call show,$@)
	$(LD) $(LDFLAGS) -o $@ $(OUT_OBJS)
	$(OD) -d $@ > $(BUILD_DIR)/$(PROJ).objdump
	# $(SP) $@

# Generate IMG -----------------------------------------------------------------
$(OUT_IMG): $(OUT_ELF)
	$(call show,$@)
	$(OC) -O binary $(OUT_ELF) $@

$(BUILD_DIRECTORIES):
	mkdir -p $@

clean:
	rm -rf *.elf
	rm -rf *.img
	rm -rf $(BUILD_DIR)