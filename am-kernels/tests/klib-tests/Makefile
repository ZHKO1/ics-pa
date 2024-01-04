NAME = klibtest
SRCS_SPRINTF = build/test_sprintf.c
SRCS_SNPRINTF = build/test_snprintf.c
SRCS = $(shell find src/ -name "*.[cS]") $(SRCS_SPRINTF) $(SRCS_SNPRINTF)
include $(AM_HOME)/Makefile

GENERATOR_SPRINTF = build/gen_format_test_sprintf
GENERATOR_SNPRINTF = build/gen_format_test_snprintf

$(GENERATOR_SPRINTF): gen_format_test.c
	@mkdir -p $(dir $@)
	@gcc -O2 -Wall -Werror -Wno-format-zero-length -Wno-format-truncation -ggdb3 \
	-D__ISA__=\"$(ISA)\" -D__ISA_$(shell echo $(ISA) | tr a-z A-Z)__ \
	 $^ -o $@

$(GENERATOR_SNPRINTF): gen_format_test.c
	@mkdir -p $(dir $@)
	@gcc -O2 -Wall -Werror -Wno-format-zero-length -Wno-format-truncation -ggdb3 \
	-D__ISA__=\"$(ISA)\" -D__ISA_$(shell echo $(ISA) | tr a-z A-Z)__  -D__TEST_SNPRINTF__\
	 $^ -o $@

$(SRCS_SPRINTF): $(GENERATOR_SPRINTF)
	$^ > $@

$(SRCS_SNPRINTF): $(GENERATOR_SNPRINTF)
	$^ > $@