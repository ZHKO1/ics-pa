NAME = fixedptc-test
SRCS_TEST = build/main.c
SRCS = $(SRCS_TEST)
LIBS += libfixedptc
include $(NAVY_HOME)/Makefile

GENERATOR_TEST = build/gen_test

$(GENERATOR_TEST): gen_test.c
	@mkdir -p $(dir $@)
	@gcc -O2 -Wall -Werror -Wno-format-zero-length -Wno-format-truncation -ggdb3 \
	-I$(NAVY_HOME)/libs/libfixedptc/include/  \
	-D__ISA__=\"$(ISA)\" -D__ISA_$(shell echo $(ISA) | tr a-z A-Z)__ \
	 $^ -o $@


$(SRCS_TEST): $(GENERATOR_TEST)
	$^ > $@
