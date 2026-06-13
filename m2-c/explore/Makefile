# explore/ — розвідка пам'яті та вказівників (журнальний деліверабл m2).
#
# Це НЕ продакшн-код, а навчальна розвідка з навмисними «гострими краями»,
# тож explore/ свідомо тримаємо ПОЗА -Werror:
#   - ptr_arith.c навмисно дає -Wsizeof-array-argument — у цьому й урок
#     (sizeof параметра-масиву = розмір ВКАЗІВНИКА, а не масиву);
#   - dangle.c навмисно читає мертвий стек-фрейм (UB). Без санітайзера він
#     тихо «працює» (друкує сміття й exit 0!), тому ганяємо його під ASAN і
#     ОЧІКУЄМО аварійний вихід. Якщо dangle раптом успішний — ламаємо таргет,
#     бо це означає, що санітайзер UB не впіймав і урок не відтворився.

CC   := gcc
STD  := -std=c11
WARN := -Wall -Wextra            # без -Werror: розвідка, не продакшн
ASAN := -fsanitize=address,undefined -fno-omit-frame-pointer -g -O1

B        := build
PEACEFUL := endian layout mem_map ptr_arith
PBINS    := $(addprefix $(B)/,$(PEACEFUL))

.DEFAULT_GOAL := run

# Мирні програми: звичайна збірка з попередженнями, без санітайзера.
$(B)/%: %.c | $(B)
	$(CC) $(STD) $(WARN) $< -o $@

# dangle: під ASAN, щоб санітайзер упіймав читання мертвого фрейму.
$(B)/dangle: dangle.c | $(B)
	$(CC) $(STD) $(WARN) $(ASAN) $< -o $@

$(B):
	mkdir -p $(B)

all: $(PBINS) $(B)/dangle

# Запускаємо мирні (мусять exit 0), а dangle — під ASAN з очікуваною аварією.
run: $(PBINS) $(B)/dangle
	@for p in $(PBINS); do \
	  echo "=== $$p (очікую exit 0) ==="; \
	  ./$$p || { echo "FAIL: $$p мав завершитися успішно"; exit 1; }; \
	done
	@echo "=== $(B)/dangle під ASAN (очікую аварійний вихід) ==="
	@if ASAN_OPTIONS=detect_stack_use_after_return=1 ./$(B)/dangle; then \
	  echo "FAIL: dangle завершився успішно — санітайзер НЕ впіймав UB"; exit 1; \
	else \
	  echo "OK: ASAN упіймав stack-use-after-return (як і задумано)"; \
	fi

clean:
	rm -rf $(B)

.PHONY: all run clean
