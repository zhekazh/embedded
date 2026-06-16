CC   := gcc
STD  := -std=c11
WARN := -Wall -Wextra -Werror
SAN  := -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer
DBG  := -g -O1

UNITY      := ../test/unity
MYFLAGS    := $(STD) $(WARN) $(SAN) $(DBG) -Isrc -I$(UNITY)
UNITYFLAGS := $(STD)        $(SAN) $(DBG)       -I$(UNITY)

B       := build
SRC     := $(wildcard src/*.c)
TST     := $(wildcard test/test_*.c)
RUNNERS := $(patsubst test/%.c,$(B)/%,$(TST))

ASANOPT := ASAN_OPTIONS=detect_leaks=1:detect_stack_use_after_return=1 UBSAN_OPTIONS=print_stacktrace=1

$(B)/unity.o: $(UNITY)/unity.c | $(B)
	$(CC) $(UNITYFLAGS) -c $< -o $@

$(B)/%: test/%.c $(SRC) $(B)/unity.o | $(B)
	$(CC) $(MYFLAGS) $< $(SRC) $(B)/unity.o -o $@

$(B):
	mkdir -p $(B)

test: $(RUNNERS)
	@for r in $(RUNNERS); do echo "=== $$r ==="; $(ASANOPT) ./$$r || exit 1; done

lint:
	cppcheck --enable=warning,performance,portability --std=c11 --error-exitcode=1 --suppress=missingIncludeSystem -Isrc src

clean:
	rm -rf $(B)
# ThreadSanitizer + setarch -R (вимкнути ASLR) — лише Linux.
# MinGW/MSYS2 на Windows не має libtsan; macOS не має setarch.
tsan:
ifeq ($(shell uname -s),Linux)
	$(MAKE) build/tsan_ringbuf
	setarch -R ./build/tsan_ringbuf
else
	@echo "tsan: ThreadSanitizer доступний лише на Linux (поточна ОС: $(shell uname -s)); пропускаю."
endif

build/tsan_ringbuf: test/tsan_ringbuf.c src/ringbuf.c | build
	gcc -std=c11 -Wall -Wextra -fsanitize=thread -pthread -g -O1 -Isrc $^ -o $@

.PHONY: test lint clean
