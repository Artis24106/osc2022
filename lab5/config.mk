# ARM toolchain
CROSS = aarch64-unknown-linux-gnu
CC = $(CROSS)-gcc
AS = $(CROSS)-as
LD = $(CROSS)-ld
OC = $(CROSS)-objcopy
OD = $(CROSS)-objdump
SP = $(CROSS)-strip

# Flags
CCFLAGS = -Wall -O0 -fno-builtin -w
LDFLAGS = -T$(LINKER_FILE) -nostdlib -nostartfiles

# ANSI Color
RED = \x1b[38;5;1m
GREEN = \x1b[38;5;2m
YELLOW = \x1b[38;5;3m
BLUE = \x1b[38;5;4m
NC = \x1b[0m

define show
	@printf "${BLUE}[+]${NC} Building: ${1}\n"
endef

define log
	@printf "${YELLOW}[~] ${1}${NC}\n"
endef

define show_header
	@printf "${GREEN}[-] ====== ${1} ======${NC}\n"
endef