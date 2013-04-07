#Unit name.
UNIT:=utils

UNIT_EXES:=
UNIT_LIBS:=wslutils
UNIT_SHLS:=

wslutils_SRC=src/array.c src/msglog.c src/outstream.c src/random.c\
		  src/logger.cpp src/tokenizer.cpp src/utf8.c src/enc_3k.c

$(foreach lib, $(UNIT_LIBS), $(eval $(call add_output_library,$(lib),$(UNIT))))

