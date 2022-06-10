include ../config.mk

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

BUILD_DIRECTORIES = \
	$(LIB_BUILD_DIR) \

PROJ = lib

.PHONY: clean directories out_elf run
all: pre_build directories $(LIB_OBJS)
pre_build:
	$(call show_header,${PROJ})
directories: $(BUILD_DIRECTORIES)

# Compile Object Files ---------------------------------------------------------
$(LIB_BUILD_DIR)/%.o : %.c
	$(call show,$@)
	$(CC) $(CCFLAGS) $(LIB_INCLUDES) -g -o $@ -c $<

$(LIB_BUILD_DIR)/%_s.o : %.S
	$(call show,$@)
	$(CC) $(CCFLAGS) $(LIB_INCLUDES) -g -o $@ -c $<

$(BUILD_DIRECTORIES):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)