IVERILOG = iverilog
PYTHON = python3

RISCV_TOOLCHAIN_PATH =
RISCV_TOOLCHAIN_PREFIX = riscv-none-elf-

AS = ${RISCV_TOOLCHAIN_PATH}${RISCV_TOOLCHAIN_PREFIX}as
OBJCOPY = ${RISCV_TOOLCHAIN_PATH}${RISCV_TOOLCHAIN_PREFIX}objcopy
OBJDUMP = ${RISCV_TOOLCHAIN_PATH}${RISCV_TOOLCHAIN_PREFIX}objdump
CC = ${RISCV_TOOLCHAIN_PATH}${RISCV_TOOLCHAIN_PREFIX}gcc
CXX = ${RISCV_TOOLCHAIN_PATH}${RISCV_TOOLCHAIN_PREFIX}g++

FAT32_SOURCE = fat/fat_access.c fat/fat_cache.c fat/fat_filelib.c fat/fat_format.c fat/fat_misc.c fat/fat_string.c fat/fat_table.c fat/fat_write.c
SRCS = start.S syscalls.c sd_card.c io.c $(FAT32_SOURCE) upng.c graphite.cpp program.cpp
OBJS := $(SRCS:%=./%.o)
DEPS := $(OBJS:.o=.d)

SERIAL ?= /dev/tty.usbserial-D00039

all: program.hex

run: program.hex
	$(PYTHON) ../../../hw/xgsoc/utils/sendhex.py $(SERIAL) program.hex

program: program.hex

clean:
	rm -f *.hex *.elf *.bin *.lst fat/*.o *.o

program.lst: program.elf
	${OBJDUMP} --disassemble program.elf > program.lst

program.hex: program.bin
	$(PYTHON) ../../../hw/xgsoc/utils/makehex.py program.bin > program.hex

program.bin: program.elf program.lst
	${OBJCOPY} -O binary program.elf program.bin

%.S.o: %.S
	${CC} -march=rv32imaf_zicsr -mabi=ilp32f -MMD -MP -c $< -o $@

%.c.o: %.c
	${CC} -march=rv32imaf_zicsr -mabi=ilp32f -MMD -MP -O3 -Ifat -c $< -o $@

%.cpp.o: %.cpp
	${CXX} -march=rv32imaf_zicsr -mabi=ilp32f -std=c++17 -MMD -MP -O3 -Ifat -c $< -o $@

program.elf: $(OBJS)
	${CC} -march=rv32imaf_zicsr -mabi=ilp32f -O3 -nostartfiles -T program.ld $(OBJS) -o program.elf -lm -lstdc++

-include $(DEPS)

.PHONY: all clean run program
