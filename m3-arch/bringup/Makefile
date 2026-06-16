# embedded/m3-arch/bringup/Makefile
# Тонка обгортка над idf.py: активує ESP-IDF (якщо ще не активований у цій
# сесії) і одразу викликає потрібну ціль — щоб не писати export + idf.py щоразу.
#   make build                     — зібрати (build/bringup.elf|.bin)
#   make flash PORT=/dev/ttyUSB0   — прошити (PORT за замовчуванням нижче)
#   make monitor | clean | fullclean | menuconfig | size
SHELL := bash
IDF   ?= $(HOME)/esp/esp-idf
PORT  ?= /dev/ttyUSB0

# source export.sh лише якщо idf.py ще не в PATH (повторний source не потрібен).
# Усе в одному рядку рецепта → один під-shell, тож активація діє на idf.py нижче.
ACT = command -v idf.py >/dev/null 2>&1 || . "$(IDF)/export.sh" >/dev/null

build menuconfig clean fullclean size:
	@$(ACT); idf.py $@

flash monitor:
	@$(ACT); idf.py -p "$(PORT)" $@

# У firmware-проєкті юніт-тестів немає (за конвенцією модуля host-тести живуть
# у m3-arch/isr-ringbuf та m3-arch/frame_fsm). Ця ціль лише підказує, а не падає.
test:
	@echo "bringup — це ESP-IDF firmware, юніт-тестів тут немає."
	@echo "  збірка прошивки:  make build"
	@echo "  host-тести M3:    (cd ../isr-ringbuf && make test) && (cd ../frame_fsm && make test)"

.PHONY: build menuconfig clean fullclean size flash monitor test
