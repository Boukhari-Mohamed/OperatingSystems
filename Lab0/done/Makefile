WEEKS = 01 02
WEEKS_H = $(patsubst %,week%.h,$(WEEKS))
WEEKS_C = $(patsubst %,week%.c,$(WEEKS))
WEEKS_O = $(patsubst %,week%.o,$(WEEKS))

CFLAGS  += -std=c11 -Wall -pedantic -g
LDLIBS  += -lcheck -lm -lrt -pthread -lsubunit
TARGETS += $(WEEKS_O) tests.o main.o tests main

all: $(TARGETS)

week%.o: week%.c week%.h

tests.o: tests.c $(WEEKS_H)

main.o: main.c $(WEEKS_H)

tests: tests.o $(WEEKS_O)

main: main.o $(WEEKS_O)

feedback: $(TARGETS)
	./tests

clean:
	@rm -f $(TARGETS)
