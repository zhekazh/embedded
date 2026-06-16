# M3 · Архітектура / bare-metal

> **Статус:** ✅ закрито
> **Критерій переходу:** пояснюю шлях reset → мій код; розумію linker script і `.map`; копаю регістри без Arduino-обгорток.
> **Залізо:** емулятор (Wokwi) зараз; ESP32 на руках — для реальних таймінгів/переривань пізніше.

## Мета
Скинути абстракцію: зібрати ціль через **ESP-IDF (CMake)** і керувати залізом
напряму регістрами, без Arduino-магії. На виході — обробник переривання на
регістрах ESP32 (таймерна ISR блимає GPIO) + **ISR-safe SPSC ring buffer**
(ISR-producer → main-consumer) — канонічний кістяк UART-драйвера, де на
двоядерному Xtensa вже не вистачає `volatile`, а потрібні справжні
атоміки/бар'єри. ⤳ той самий патерн розгорнемо в M7 (RTOS).

## Підпроєкти
- [bringup/](bringup/) — ESP-IDF firmware: регістровий GPIO (`W1TS`/`W1TC`) +
  `gptimer`-ISR блимає LED на GPIO4; `app_main` вільний. Збірка — CMake/`idf.py`.
- [isr-ringbuf/](isr-ringbuf/) — host-C: SPSC lock-free кільцевий буфер на
  `_Atomic` з acquire/release-семантикою ([ringbuf.c](isr-ringbuf/src/ringbuf.c)),
  Unity-тести + окремий **ThreadSanitizer**-харнес
  ([tsan_ringbuf.c](isr-ringbuf/test/tsan_ringbuf.c)).
- [frame_fsm/](frame_fsm/) — host-C: байтовий **кінцевий автомат** кадру
  (STX/ETX-кадрування з ре-синком на шумі та вкладеному STX),
  [frame_fsm.c](frame_fsm/src/frame_fsm.c); Unity-тести (чистий кадр, шум до STX,
  ре-синк, back-to-back).
- [test/unity/](test/unity/) — вендорнутий Unity, спільний для host-проєктів M3
  (так само, як `m2-c/test/unity`).

## Теорія (стисло, своїми словами)
- **Регістровий GPIO без read-modify-write.** `GPIO_OUT_W1TS_REG` (write-1-to-set)
  і `GPIO_OUT_W1TC_REG` (write-1-to-clear) міняють **лише** ті біти, де записано `1`.
  Тому виставити/скинути пін — це один запис, без `read|=mask`/`read&=~mask`, тобто
  без гонки з паралельним записом інших пінів. Саме тому це безпечно робити в ISR
  ([main.c](bringup/main/main.c)).
- **ISR має жити в IRAM.** `IRAM_ATTR` кладе код обробника в інструкційну RAM: під
  час операцій із flash (кеш недоступний) ISR із flash впав би. У тілі ISR — **жодного
  `printf`** і нічого, що блокує/чіпає flash; лише регістри + лічильник.
- **Таймер → переривання через матрицю.** `gptimer` рахує з роздільністю 1 МГц;
  подія `alarm` через матрицю переривань ESP32 (⤳ NVIC на Cortex-M) смикає лінію CPU,
  і викликається зареєстрований callback. `auto_reload_on_alarm` робить періодичність
  без ручного перезаряду.
- **`volatile` замало на двоядерному.** `volatile` забороняє компілятору кешувати
  значення в регістрі, але **не** дає міжпроцесорного впорядкування. Для обміну
  ISR↔task потрібні `_Atomic` + memory_order: producer робить release-store індексу
  **після** запису даних, consumer — acquire-load **перед** читанням. Це і є різниця
  M2 (звичайний SPSC на хості) → M3 (ISR-safe). ⤳ M7.
- **ESP-IDF скидає Arduino-магію.** Кореневий [CMakeLists.txt](bringup/CMakeLists.txt)
  через `$ENV{IDF_PATH}/tools/cmake/project.cmake` перевизначає `project()`, який сам
  тягне тулчейн, сканує компоненти й лінкує bootloader+app. Жодного `setup()/loop()` —
  точка входу `app_main()`.
- **Reset → `app_main` і розміщення секцій.** Після reset ROM-бутлоадер → 2-й
  бутлоадер → `call_start_cpu0` (ESP-IDF startup: ініціалізація `.data`/`.bss`,
  heap, FreeRTOS) → планувальник запускає таск, що кличе `app_main()` — звідти вже
  мій код. Що де лежить, видно в `.map`: `.text`/`.rodata` — у flash (виконання
  через кеш), `.data`/`.bss` — у DRAM. **Linker fragment** [main/linker.lf](bringup/main/linker.lf)
  явно кладе `ringbuf` у **IRAM** (`noflash`): код на ISR-шляху не має залежати від
  flash-кешу (під час запису у flash кеш недоступний → код із flash там впав би).
  ⤳ глибше про linker script і власні секції — на Cortex-M у пізніх модулях.

## Передумови
Обидві ОС **нативно** (без WSL) — гілка одна, шляхи не хардкодяться (див.
[крос-платформну стратегію](#кросплатформність-linux--windows)).

- **Firmware (bringup):** ESP-IDF **5.5.x** для ESP32.
  - Linux: встановлений у `~/esp/esp-idf`; активація `. ~/esp/esp-idf/export.sh`.
  - Windows: офіційний ESP-IDF-інсталятор; працювати з ярлика **«ESP-IDF
    PowerShell»** (там `idf.py` уже активований).
- **host-C (isr-ringbuf):** `gcc` + `make`.
  - Linux: нативно (ASan/UBSan/**TSan** у gcc з коробки).
  - Windows: **MSYS2** (gcc+make). ASan/UBSan працюють; **TSan недоступний**
    (нема `libtsan`) — ціль `make tsan` сама це повідомить і пропуститься.
- Теки `bringup` та `isr-ringbuf` уже додані як `folders` у кореневий
  [embedded.code-workspace](../embedded.code-workspace).

## Кроки (відтворювані з нуля)

### 1. Firmware `bringup` — регістрова ISR-блималка
**Linux:**
```bash
cd m3-arch/bringup
make build          # = активувати IDF (якщо треба) + idf.py build
```
**Windows (звичайний PowerShell):**
```powershell
cd m3-arch\bringup
.\build.ps1         # = активувати IDF (якщо треба) + idf.py build
```
> Або з «ESP-IDF PowerShell» / уже активованого терміналу — напряму `idf.py build`
> на будь-якій ОС.

_Перевірка:_ наприкінці — `Project build complete`, з'являються `build/bringup.elf`
і `build/bringup.bin`. У Wokwi (потрібен зібраний `.elf`/`flasher_args.json`,
шляхи вже в [wokwi.toml](bringup/wokwi.toml)) LED на GPIO4 блимає ~2 рази/с
(toggle кожні 250 мс), а серійник друкує `app_main free; toggles so far: N`.

### 2. host-C `isr-ringbuf` — SPSC lock-free + санітайзери
**Linux і Windows(MSYS2):**
```bash
cd m3-arch/isr-ringbuf
make test           # Unity під ASan/UBSan
```
_Перевірка:_ `6 Tests 0 Failures 0 Ignored` / `OK`.

**Лише Linux — перевірка на гонки:**
```bash
make tsan           # ThreadSanitizer, ASLR вимкнено (setarch -R)
```
_Перевірка:_ ганяє producer/consumer у двох потоках, TSan **без** data race.
На Windows ця ціль чесно повідомить, що TSan лише для Linux, і пропуститься.

### 3. host-C `frame_fsm` — кінцевий автомат кадру
**Linux і Windows(MSYS2):**
```bash
cd m3-arch/frame_fsm
make test           # Unity під ASan/UBSan
```
_Перевірка:_ `4 Tests 0 Failures 0 Ignored` / `OK` (чистий кадр, шум до STX,
ре-синк на вкладеному STX, back-to-back).

## Артефакти цього модуля
- `bringup/` — ESP-IDF-проєкт: регістровий GPIO + `gptimer`-ISR блимання, `app_main`
  розвантажений; лінкує `ringbuf.c`+`frame_fsm.c` із сусідніх host-проєктів, а
  [main/linker.lf](bringup/main/linker.lf) кладе `ringbuf` у IRAM (`noflash`); обгортки
  збірки [Makefile](bringup/Makefile) (Unix) і [build.ps1](bringup/build.ps1) (Windows).
- `isr-ringbuf/` — SPSC lock-free ring buffer на `_Atomic` (acquire/release) з
  Unity-тестами (порожній/повний, round-trip, FIFO через стик-wraparound) і
  окремим TSan-харнесом.
- `frame_fsm/` — байтовий кінцевий автомат кадру (STX/ETX, ре-синк) на чистому C з
  Unity-тестами.
- `test/unity/` — вендорнутий Unity для host-проєктів M3.

## CI / автоматизація
- Джоб `m3-arch` у [.github/workflows/ci.yml](../.github/workflows/ci.yml) (ubuntu):
  `isr-ringbuf` → `make test` (ASan/UBSan) + `make tsan` (TSan, гонки) + `make lint`
  (cppcheck); `frame_fsm` → `make test` + `make lint`. Firmware-build свідомо поза CI.
- _Статус:_ 🟢 локально зелено (6+4 тести, TSan без гонок, cppcheck `0`); у GitHub
  Actions запуститься після push.

## Безпека / захист
- **ISR-safe обмін без гонок:** ring buffer розрахований на одного producer (ISR) і
  одного consumer (task); межі повний/порожній відрізняються жертвою одного слота,
  кожна сторона пише лише свій індекс, публікація через release/acquire — відсутність
  data race підтверджена **ThreadSanitizer**, а не очима.
- **Атомарність на рівні заліза:** `W1TS`/`W1TC` змінюють лише цільові біти, тож
  паралельні записи інших пінів не б'ються — менше поверхні для глитчів у ISR.

## Граблі / лог факапів
- **`idf_component_register` викликаний двічі** (у `main/CMakeLists.txt`) → причина:
  ESP-IDF дозволяє **один** виклик на компонент; другий валить CMake (`called more
  than once`). Закешований `build/` маскував проблему — `idf.py build` без змін ще
  проходив, але `reconfigure`/`fullclean`/свіжий клон ламались; фікс: один виклик із
  `REQUIRES esp_driver_gptimer` (саме він дає `driver/gptimer.h`).
- **`make test` в isr-ringbuf: «Нема правила для `../test/unity/unity.c`»** → причина:
  `Makefile` скопійований з `m2-c/frame-codec`, де `../test/unity` існує; у M3
  спільної теки Unity не було; фікс: вендорнув Unity у `m3-arch/test/unity/` — тепер
  `../test/unity` працює для всіх host-проєктів M3.
- **Абсолютний `/usr/bin/python3` у `embedded.code-workspace`** → причина: машинний
  Linux-шлях у закоміченому конфігу ламав би проєкт на Windows; фікс: виніс
  `platformio-ide.customPMPython` у локальний git-ignored `.vscode/settings.json`, у
  воркспейсі — нічого абсолютного.
- **`export.sh` повільний і його треба `source`, не запускати** → причина: він міняє
  `PATH`/`IDF_PATH` поточного shell (тому `./export.sh` не дав би ефекту), а повна
  активація — кілька секунд; фікс: обгортки [Makefile](bringup/Makefile)/[build.ps1](bringup/build.ps1)
  активують IDF **лише якщо `idf.py` ще не в PATH** (`command -v idf.py || . export.sh`).
- **TSan/`setarch` — Linux-only** → причина: MinGW/MSYS2 не має `libtsan`, а `setarch`
  (вимкнути ASLR) — Linux-утиліта; фікс: ціль `tsan` під `ifeq ($(shell uname -s),Linux)`,
  на інших ОС — зрозуміле повідомлення замість падіння лінкера.
- **`SRCS "../../isr-ringbuf/src/ringbuf.c"` поза каталогом компонента** — працює, але
  не канонічно для ESP-IDF; за потреби перевикористання краще зробити окремий компонент
  (`components/ringbuf` або `EXTRA_COMPONENT_DIRS`). Для навчального модуля прямий шлях
  лишаю свідомо.

## Кросплатформність (Linux + Windows)
- **Одна гілка**, не дві ОС-гілки (дві = антипатерн: кожен комміт довелось би
  мерджити в обидві).
- Жодних абсолютних шляхів у git: env-змінні (`$ENV{IDF_PATH}`, `${userHome}`),
  відносні шляхи; машинні значення — у локальному git-ignored `.vscode/settings.json`.
- Те, що різниться між ОС, існує у двох формах: Unix `Makefile` + Windows `build.ps1`;
  порт параметризований (`/dev/ttyUSB0` ↔ `COM3`).

## Чекліст критерію переходу
- [x] регістровий GPIO без Arduino (`W1TS`/`W1TC`) на ESP-IDF
- [x] ISR на регістрах + `gptimer`; **ISR-safe SPSC ring buffer** (atomics, TSan чисто)
- [x] збірка через ESP-IDF/CMake без Arduino-магії
- [x] пояснюю шлях reset → `app_main` → мій код
- [x] linker script / секції: `linker.lf` кладе `ringbuf` у IRAM (`noflash`); читаю `.map`
- [x] простий кінцевий автомат: байтовий FSM кадру (STX/ETX, ре-синк) з тестами (`frame_fsm`)

## Джерела
- ESP-IDF Programming Guide — build system (CMake), `gptimer`, IRAM/переривання.
- ESP32 Technical Reference Manual — GPIO `W1TS`/`W1TC`, матриця переривань.
- «From Zero to main(): Bare metal C» (Interrupt) — reset → `main`, секції, linker script.
- Preshing, «Acquire and Release Semantics» — модель пам'яті для lock-free SPSC.
