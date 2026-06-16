# M3 · bringup — регістровий GPIO + таймерна ISR на ESP-IDF

> **Статус:** ✅ закрито
> **Що демонструє:** скидання Arduino-абстракції — збірка через ESP-IDF/CMake, блимання LED **прямими регістрами** (`W1TS`/`W1TC`) із таймерної **ISR**, розміщення коду на ISR-шляху в **IRAM** через linker fragment.
> **Залізо:** емулятор (Wokwi); на реальному ESP32 — той самий `.elf`.

## Мета
Зібрати ціль через ESP-IDF (а не Arduino) і керувати GPIO напряму регістрами з
обробника апаратного таймера, лишивши `app_main` вільним. Це місток «from zero to
`app_main`»: видно шлях reset → старт → мій код і де що лежить у пам'яті.

## Теорія (стисло)
> Загальна теорія (reset-шлях, секції, матриця переривань) — у [README модуля](../README.md).
> Тут лише специфіка проєкту.

- **GPIO без read-modify-write.** `GPIO_OUT_W1TS_REG` (set) і `GPIO_OUT_W1TC_REG`
  (clear) міняють тільки ті біти, де записано `1` — один запис, без `reg |= mask`,
  тож безпечно з ISR. Пін у режим виходу — `GPIO_ENABLE_W1TS_REG` + `PIN_FUNC_SELECT`.
- **ISR в IRAM.** Обробник позначений `IRAM_ATTR`; [main/linker.lf](main/linker.lf)
  додатково кладе `ringbuf` у IRAM (`noflash`) — код на ISR-шляху не має залежати від
  flash-кешу (під час запису у flash кеш недоступний → код із flash там впав би).
  У тілі ISR — лише регістри + лічильник, **жодного `printf`**.
- **Таймер `gptimer`** з роздільністю 1 МГц; подія `alarm` кожні 250 000 мкс з
  `auto_reload_on_alarm` → callback тогглить LED. Блимання жене ISR, а не цикл
  `app_main` (той лише раз на 2 с друкує лічильник — доказ, що CPU вільний).
- **Інтеграція M3.** Компонент `main` лінкує ще `ringbuf.c` і `frame_fsm.c` із
  сусідніх host-проєктів (спільне джерело firmware ↔ host-тести).

## Передумови
- **ESP-IDF 5.5.x** для ESP32.
  - Linux: `. ~/esp/esp-idf/export.sh` (або обгортка `make` сама активує).
  - Windows: ярлик **«ESP-IDF PowerShell»** (там `idf.py` уже активований) або
    [build.ps1](build.ps1).
- Кореневий [CMakeLists.txt](CMakeLists.txt) бере IDF через `$ENV{IDF_PATH}` — шлях
  не хардкоджений.

## Запуск (відтворюваний з нуля)
**Linux:**
```bash
cd m3-arch/bringup
make build            # = активувати IDF (якщо треба) + idf.py build
```
**Windows (PowerShell):**
```powershell
cd m3-arch\bringup
.\build.ps1
```
_Перевірка:_ наприкінці `Project build complete`, з'являються `build/bringup.elf` і
`build/bringup.bin`.

**Симуляція (Wokwi):** після білду (потрібен `.elf` + `flasher_args.json`, шляхи в
[wokwi.toml](wokwi.toml)) — `F1 → Wokwi: Start Simulator`.
_Перевірка:_ LED на GPIO4 блимає ~2 рази/с; Serial друкує `app_main free; toggles so far: N`.

**Прошивка (реальна плата):** `make flash PORT=/dev/ttyUSB0` (Linux) /
`.\build.ps1 flash -Port COM3` (Windows).

## Артефакти
- `main/main.c` — регістровий GPIO + `gptimer`-ISR блимання, розвантажений `app_main`.
- `main/CMakeLists.txt` — `idf_component_register` зі `SRCS` (main + ringbuf + frame_fsm),
  `REQUIRES esp_driver_gptimer`, `LDFRAGMENTS "linker.lf"`.
- `main/linker.lf` — кладе `ringbuf` у IRAM (`noflash`).
- `CMakeLists.txt`, `sdkconfig.defaults`, `wokwi.toml`, `diagram.json`.
- `Makefile` (Unix) + `build.ps1` (Windows) — одна команда збірки на обох ОС.

## Граблі / лог факапів
> Симптом → причину → фікс.
- **`idf_component_register` викликаний двічі** → причина: ESP-IDF дозволяє **один**
  виклик на компонент; другий валить CMake (`called more than once`), але закешований
  `build/` маскував — `idf.py build` без змін ще проходив, а `reconfigure`/`fullclean`
  ламались; фікс: один виклик із `REQUIRES esp_driver_gptimer`.
- **`make test` у firmware-теці** → причина: тут немає юніт-тестів (вони на хості —
  `isr-ringbuf`/`frame_fsm`); фікс: ціль `test` дає підказку замість криптичної помилки
  make. Firmware лише збирається.
- **`SRCS "../../isr-ringbuf/src/ringbuf.c"` поза каталогом компонента** — працює, але
  не канонічно; за потреби перевикористання — окремий компонент
  (`components/ringbuf` / `EXTRA_COMPONENT_DIRS`). Для навчального модуля лишено свідомо.
- **`export.sh` треба `source`, не запускати, і він повільний** → причина: міняє
  `PATH`/`IDF_PATH` поточного shell; фікс: обгортки активують IDF лише якщо `idf.py`
  ще не в PATH (`command -v idf.py || . export.sh`).

## Чекліст «працює»
- [x] збірка через ESP-IDF/CMake (`make build` / `build.ps1`) — `Project build complete`
- [x] регістровий GPIO (`W1TS`/`W1TC`) без Arduino
- [x] таймерна ISR (`gptimer`) блимає LED, `app_main` вільний
- [x] `ringbuf` у IRAM через `linker.lf` (`noflash`)
- [x] блимання видно у Wokwi (GPIO4, ~2 Гц)

## Джерела
- ESP-IDF Programming Guide — build system (CMake), `gptimer`, IRAM/переривання, linker fragments.
- ESP32 Technical Reference Manual — GPIO `W1TS`/`W1TC`, матриця переривань.
- «From Zero to main(): Bare metal C» (Interrupt) — reset → `main`, секції.
