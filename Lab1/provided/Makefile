CFLAGS  += -O0 -std=gnu11 -Wall -pedantic -g -fno-omit-frame-pointer -DSTAFF
LDLIBS  += -lcheck -lm -lrt -pthread -lsubunit

COMMON =  stack.o error.o malloc.o sched_policy.o schedule.o thread_list.o               thread.o
HEADERS = stack.h error.h malloc.h sched_policy.h schedule.h thread_list.h thread_info.h thread.h
TESTS = test_threading  
APP = main

## ---------------------------------------------------
## ------- Additions for week 04: scheduling ---------
COMMON  += l1_time.o priority.o
HEADERS += l1_time.h priority.h
TESTS += test_scheduler

## ---------------------------------------------------
## ------- Additions for week 05: allocators ---------
TESTS += test_malloc

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
$(1): $$($(1).c) $(COMMON)
endef

$(foreach app,$(APP),$(eval $(call REQS_template,$(app))))
$(foreach test,$(TESTS),$(eval $(call REQS_template,$(test))))
