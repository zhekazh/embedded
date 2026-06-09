# M0 · Setup, мислення, інструментарій

> **Статус:** ✅ закрито
> **Критерій переходу:** blink працює на емуляторі; чиста C-вправа збирається й запускається на хості; можу пояснити кожну ланку тулчейну.
> **Залізо:** емулятор-first (фізична плата ще в дорозі)

## Мета
Зробити ментальний перехід з вебу на embedded, підняти локальний toolchain
(VS Code + PlatformIO), отримати перший blink на емуляторі й зрозуміти ланцюг
`.c → .o → .elf → .bin` та роль GDB/OpenOCD.

## Підпроєкти
- [blink/](blink/README.md) — перший blink на ESP32 + локальний дебаг-цикл
- [host-c/](host-c/README.md) — чиста C-вправа: set/clear/toggle біта в «регістрі» (+ розбір бітів/масок/вказівників)

## Теорія (стисло, своїми словами)
- **Мислення.** Програма не «відповіла й завершилась» — вона крутиться у вічному
  циклі, поки є живлення. Я сам відповідаю за кожен байт; немає GC; «затримка» —
  це реальний фізичний час.
- **`digitalWrite(pin, HIGH)`** під капотом — це **запис біта в регістр порту**.
  Те саме руками: `*reg |= (1u << pin)`. ⤳ на регістрах робитимемо в M3–M5.
- **Ланцюг збірки.** Препроцесор (`#include`/`#define`) → компіляція кожного
  `.c` у `.o` (код із «дірками» — невідомими адресами) → лінкування всіх `.o`
  у `.elf` (дірки заповнено, linker script розклав по адресах) → `objcopy`
  витягує `.bin` (голі байти у flash).
- **`.elf` vs `.bin`.** `.elf` = код + символи + дебаг-інфо (для GDB).
  `.bin` = чисті байти прошивки (для flash). ⤳ деталі linker script — M3.
- **Навіщо GDB+OpenOCD.** Код для чіпа не біжить на ПК (інша архітектура, немає ОС).
  GDB вантажить `.elf` (символи) і дебажить, але говорити із залізом напряму не вміє:
  `GDB ↔ OpenOCD ↔ ST-Link ↔ чіп (SWD/JTAG)`. В емуляторі праву частину підміняє симулятор.

## Передумови
- Windows + VS Code
- Git (`git --version` працює)

## Кроки (відтворювані з нуля)
1. Встановити **PlatformIO IDE** (розширення VS Code). Окремо Python/Core ставити не треба —
   PlatformIO несе portable Python і тулчейни сам.
   _Перевірка:_ внизу зліва з'явилась іконка 🏠 PlatformIO Home.

2. Браузерний blink (миттєвий результат): wokwi.com → New Project → ESP32 → код у `sketch.ino`.
   _Перевірка:_ LED блимає в симуляції.
   ⚠️ На `ESP32 DevKit C V4` немає юзер-LED на GPIO2 (лише power-LED) — потрібен зовнішній LED (див. граблі).

3. Локальний PlatformIO-проєкт (`blink/`): board `esp32dev`, framework `arduino`. Код у `src/main.cpp`. Build (✓).
   _Перевірка:_ у терміналі `SUCCESS`; з'явились `.pio/build/esp32dev/firmware.elf` і `firmware.bin`.

4. Хост-компілятор для чистого C — WSL:
   ```powershell
   wsl --install            # PowerShell від адміна, потім reboot
   ```
   ```bash
   sudo apt update && sudo apt install build-essential gdb
   gcc --version
   ```
   _Перевірка:_ `gcc --version` друкує версію.

5. Хост-C вправа (`host-c/reg.c`) — прогін ланцюга вручну:
   ```bash
   gcc -E reg.c -o reg.i                    # препроцесор
   gcc -S -Wall -Wextra reg.c -o reg.s      # → асемблер (-Wall -Wextra: перша лінія проти UB)
   gcc -c -Wall -Wextra reg.c -o reg.o      # → об'єктник (не запускається)
   gcc reg.o   -o reg                       # → виконуваний (лінкування)
   ./reg                                    # запуск (asserts впадуть, якщо біти не ті)
   nm reg.o                   # символи: T (код), b (.bss), U (дірка, напр. putchar)
   file reg.o                 # "relocatable" — має дірки
   file reg                   # "executable" — дірки заповнено
   ```
   _Перевірка:_ `./reg` друкує очікувані бітові патерни; `nm` показує функції; `U putchar` — це та сама «дірка».

6. Wokwi for VS Code (локальний емулятор): встановити розширення → `F1 → Wokwi: Request a new License`.
   Додати в `blink/` файли `wokwi.toml` і `diagram.json` (схему скопіювати з браузерного проєкту).
   _Перевірка:_ Build → `F1 → Wokwi: Start Simulator` → LED блимає у вікні VS Code.

7. Дебаг: у `wokwi.toml` додати `gdbServerPort = 3333`. Конфіг дебагера тримати **не** в
   `blink/.vscode/launch.json` (його PlatformIO регенерує й затирає ручні правки), а в
   `embedded.code-workspace` → секція `"launch"`: один `cppdbg` з `name: "Wokwi GDB — m0-blink"`,
   `miDebuggerPath` → xtensa-gdb із `.platformio/packages`, `miDebuggerServerAddress: localhost:3333`,
   `program` → `.elf` (у multi-root шлях через `${workspaceFolder:m0 · blink (ESP32)}`). Відкрити проєкт через
   *Open Workspace from File* → в «Run and Debug» вибрати `Wokwi GDB — m0-blink`. Потрібне розширення `ms-vscode.cpptools`.
   Build → `F1 → Wokwi: Start Simulator and Wait for Debugger` → `F5`.
   _Перевірка:_ виконання спиняється на breakpoint; працюють F10/F11; у VARIABLES видно змінні.

## Артефакти цього модуля
- `blink/` — PlatformIO ESP32-проєкт (blink) з конфігами Wokwi й дебагу.
- `host-c/reg.c` — чиста C-вправа: set/clear/toggle біта в «регістрі» + прогін тулчейну; перевірка через `assert`, збірка з `-Wall -Wextra`.

## Граблі / лог факапів
- **Wokwi: «не блимає, просто горить червоним»** → на `ESP32 DevKit C V4` немає
  юзер-LED на GPIO2; червоний — це power-LED (припаяний до +5V). Фікс: додати
  зовнішній `wokwi-led` + резистор 220 Ω на GPIO2, або підтвердити роботу циклу через `Serial.println`.
- **Перший Build «висить» довго** → це разове завантаження тулчейну з мережі. Не перебивати.
- **Wokwi у VS Code: «ELF file not found»** → не зроблено Build перед запуском, або шлях у `wokwi.toml` не збігається з теками `.pio`.
- **Дебаг: «Unable to start debugging»** → невірний `miDebuggerPath`, або порт у `launch.json` ≠ `gdbServerPort` у `wokwi.toml`, або симулятор запущено не в режимі «…and Wait for Debugger».
- **Дебаг: «unable to find a matching CMSIS-DAP device» + «Target disconnected»** → запущено
  *PlatformIO: Debug* замість дебагу через Wokwi. PIO для `esp32dev` за замовчуванням стартує OpenOCD
  і шукає **фізичний** JTAG/SWD-адаптер (`debug_tool = cmsis-dap`), якого в emulator-first немає → GDB
  обриває зв'язок. Фікс: дебажити через GDB-сервер Wokwi (`F1 → Wokwi: Start Simulator and Wait for Debugger`,
  далі `F5` на конфізі з `miDebuggerServerAddress: localhost:3333`), а не *PlatformIO: Debug*. Тут вузол
  `GDB ↔ OpenOCD ↔ адаптер ↔ чіп` цілком підмінено симулятором — апаратний дебагер не задіюється.
- **Доданий `Wokwi GDB` зникає після Build / зміни `platformio.ini`** → PlatformIO регенерує
  `blink/.vscode/launch.json` (шапка «AUTOMATICALLY GENERATED… DO NOT MODIFY») і затирає ручні
  конфіги. Фікс: тримати `Wokwi GDB` не в проєктному `launch.json`, а в `embedded.code-workspace`
  → секція `"launch"` — цей файл PlatformIO не чіпає, тож конфіг переживає будь-який rebuild.
  Умова: відкривати проєкт через *Open Workspace from File*, інакше workspace-конфіги не видно в дропдауні.
- **IntelliSense червонить `#include`, а Build = SUCCESS** → лінтер відстає; `PlatformIO: Rebuild IntelliSense Index`.

## Чекліст критерію переходу
- [x] blink на емуляторі (браузер + локальний Wokwi)
- [x] чиста C-вправа збирається й запускається на хості (`reg.c`)
- [x] можу пояснити кожну ланку тулчейну (прогнав `.c→.i→.s→.o→exe` руками + дебаг + `nm`)

## Джерела
- docs.platformio.org — встановлення PlatformIO IDE
- docs.wokwi.com — Wokwi for VS Code: getting started, project config, debugging