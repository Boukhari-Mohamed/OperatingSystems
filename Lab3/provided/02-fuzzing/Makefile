CC := clang
CXX := clang++
COMMONFLAGS := -O0 -Wall -Wextra -pedantic -g -fsanitize=address,fuzzer \
		-Iinclude
CFLAGS  += $(COMMONFLAGS) -std=gnu11
CXXFLAGS  += $(COMMONFLAGS) -std=gnu++14
LDFLAGS += -Llib

# Detect OS
detected_OS := $(shell uname)

# Add libs if building on Linux, not required on OS X
ifeq ($(detected_OS),Linux)
	LDLIBS += -lz
endif

HEADERS := include/png.h
APP := fuzzer_native_001 fuzzer_native_002 fuzzer_simplified_003

all: $(APP)

fuzzer_native_%: fuzzer_native.o lib/libpng16_%.a
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

fuzzer_simplified_%: fuzzer_simplified.o lib/libpng16_%.a
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	@rm -f $(APP) $(TESTS)

