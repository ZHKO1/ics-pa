NAME = klibtest
SRCS_FORMAT = build/test_sprintf.c
SRCS = $(shell find src/ -name "*.[cS]") $(SRCS_FORMAT)
include $(AM_HOME)/Makefile

GENERATOR = build/format

$(GENERATOR): gen_format_test.c
	@mkdir -p $(dir $@)
	gcc -O2 -Wall -Werror -Wno-format-zero-length -ggdb3 $^ -o $@

$(SRCS_FORMAT): $(GENERATOR)
	$^ > $@