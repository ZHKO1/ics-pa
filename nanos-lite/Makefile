HAS_NAVY = 1
RAMDISK_FILE = build/ramdisk.img

NAME = nanos-lite
SRCS = $(shell find -L ./src/ -name "*.c" -o -name "*.cpp" -o -name "*.S")
include $(AM_HOME)/Makefile

ifeq ($(ARCH),native)
ISA = am_native
else
INC_PATH += include $(NAVY_HOME)/libs/libc/include
endif

# 获取nemu目录下的CONFIG配置
INC_PATH += $(NEMU_HOME)/include/generated 

./src/resources.S: $(RAMDISK_FILE)
	@touch $@

ifeq ($(HAS_NAVY),)
files = $(RAMDISK_FILE) src/files.h src/syscall.h
# create an empty file if it does not exist
$(foreach f,$(files),$(if $(wildcard $f),, $(shell touch $f)))

# 这里函数调用的踪迹（ftrace）先只显示RAMDISK_FILE内部的踪迹
NEMUFLAGS = -l $(shell dirname $(IMAGE).elf)/nemu-log.txt -b --elf=$(shell realpath $(RAMDISK_FILE))

create_img:
	$(MAKE) -s -C $(NAVY_HOME)/tests/$(TARGET) ISA=$(ISA)
	@cp $(NAVY_HOME)/tests/$(TARGET)/build/$(TARGET)-$(ISA) build/ramdisk.img

else

ifeq ($(wildcard $(NAVY_HOME)/libs/libos/src/syscall.h),)
  $(error $$NAVY_HOME must be a Navy-apps repo)
endif

# 引入多个elf // TODO 按理说这里APPS和TESTS应该跟navy-apps/Makefile保持统一，有待优化
APPS =
TESTS = 
NAVY_ELF_PATHS = $(foreach elf,$(APPS),$(NAVY_HOME)/apps/$(elf)/build/$(elf)-$(ISA))
NAVY_ELF_PATHS += $(foreach elf,$(TESTS),$(NAVY_HOME)/tests/$(elf)/build/$(elf)-$(ISA))
NEMUFLAGS = -l $(shell dirname $(IMAGE).elf)/nemu-log.txt -b $(addprefix --elf=, $(NAVY_ELF_PATHS))

update:
	$(MAKE) -s -C $(NAVY_HOME) ISA=$(ISA) ramdisk
	@ln -sf $(NAVY_HOME)/build/ramdisk.img $(RAMDISK_FILE)
	@ln -sf $(NAVY_HOME)/build/ramdisk.h src/files.h
	@ln -sf $(NAVY_HOME)/libs/libos/src/syscall.h src/syscall.h

.PHONY: update
endif
