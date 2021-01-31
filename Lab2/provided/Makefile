CFLAGS  += -O0 -std=gnu11 -Wall -Wextra -pedantic -g -fsanitize=address

LDLIBS  += -lcheck -pthread

# Detect OS
detected_OS := $(shell uname)

# Add -lasan, -lm, -lsubunit and -lrt if building on Linux, not required on OS X
ifeq ($(detected_OS),Linux)
	LDLIBS += -lasan -lrt -lm -lsubunit
endif

COMMON += blk_io.o close.o fs_util.o open.o open_file_table.o read.o readdir.o seek.o disk.o
HEADERS += blk_io.h fs_api.h fs_structs.h fs_util.h open_file_table.h disk.h
#PROVIDED_OBJS =
APP = lab3_cat lab3_ls
TESTS = test_fs

## ---------------------------------------------------
## --------- Template stuff : Do not touch -----------

all: $(APP) $(TESTS)

feedback: $(TESTS)
	$(foreach test,$(TESTS),./$(test))

%.o: %.c $(HEADERS)

clean:
	@rm -f $(APP) $(TESTS) $(COMMON)

# Template for requirements for APPS and TESTS
# Apart from the corresponding .c files, APPS and TESTS
# require COMMON and HEADERS too
define REQS_template
$(1): $$($(1).c) $(COMMON) $(PROVIDED_OBJS)
endef

$(foreach app,$(APP),$(eval $(call REQS_template,$(app))))
$(foreach test,$(TESTS),$(eval $(call REQS_template,$(test))))
