# M0 · blink — ESP32 GPIO blink (Wokwi)

> **Статус:** ✅ закрито
> **Що демонструє:** перша прошивка + повний локальний цикл *правка → build → симуляція → дебаг* на ESP32, без фізичної плати.
> **Залізо:** емулятор (Wokwi); схема — ESP32 DevKit C v4 + зовнішній LED + резистор 220 Ω

## Мета
Підняти PlatformIO-проєкт під ESP32, залити blink у локальний Wokwi-симулятор
і навчитися дебажити прошивку (breakpoints / step / variables) прямо у VS Code.

## Теорія (стисло, своїми словами)
- `loop()` — вічний цикл: firmware не завершується, крутиться поки є живлення.
- `delay(500)` — **блокуючий** антипатерн: МК нічого не робить, поки чекає. ⤳ у M4 (event-driven/КА) і M7 (RTOS) замінимо на неблокуючий `millis()`/таймер.
- `digitalWrite(pin, HIGH/LOW)` = запис біта в регістр GPIO. ⤳ на регістрах ESP32 — M5.
- Дебаг прошивки = `GDB ↔ симульований чіп`; на залізі праву частину підніме OpenOCD + JTAG. ⤳ M6.
- Збірка дає `.elf` (символи, для дебагу) і `.bin` (для flash). ⤳ деталі ланцюга — у модульному README M0.

## Передумови
- VS Code + розширення: **PlatformIO IDE**, **Wokwi Simulator**, **C/C++** (саме `ms-vscode.cpptools` — потрібен для `cppdbg`-дебагу; пак `ms-vscode.cpptools-extension-pack` не треба, він у `unwantedRecommendations` воркспейсу)
- Активована безкоштовна ліцензія Wokwi (`F1 → Wokwi: Request a new License`)
- Проєкт відкривати у VS Code як окрему теку (саме `blink/`, де лежить `platformio.ini`)

## Кроки (відтворювані з нуля)
1. New Project: board `esp32dev`, framework `arduino`.
   _Перевірка:_ з'явився `platformio.ini` з `[env:esp32dev]`.
2. Код у `src/main.cpp` (GPIO2, період 500 мс).
   _Перевірка:_ файл збережено, лінтер не червонить синтаксис.
3. **Build** (✓).
   _Перевірка:_ у терміналі `SUCCESS`; з'явились `.pio/build/esp32dev/firmware.elf` і `firmware.bin`.
4. Додати `wokwi.toml` (шляхи до `.elf`/`.bin` + `gdbServerPort = 3333`) і `diagram.json`
   (плата + `wokwi-led` + `wokwi-resistor` 220 Ω; LED на GPIO2 через резистор, катод на GND).
   _Перевірка:_ `F1 → Wokwi: Start Simulator` → зовнішній LED блимає у вікні VS Code.
5. Дебаг: `.vscode/launch.json` (`cppdbg`, `miDebuggerPath` → `xtensa-esp32-elf-gdb`,
   `miDebuggerServerAddress: localhost:3333`, `program` → `.elf`).
   Build → `F1 → Wokwi: Start Simulator and Wait for Debugger` → `F5`.
   _Перевірка:_ виконання спиняється на breakpoint; F10/F11 крокують; у VARIABLES видно змінні.

## Артефакти цього підпроєкту
- `src/main.cpp` — blink-логіка (доказ робочої прошивки)
- `platformio.ini` — конфіг проєкту (board, framework; версію платформи запінено: `espressif32@6.9.0`)
- `wokwi.toml` — зв'язок симулятора з `.elf`/`.bin` + порт GDB-сервера
- `diagram.json` — віртуальна схема (плата + LED + резистор)
- `.vscode/launch.json` — конфіг дебагу (в `.gitignore`: містить машинний шлях)

## Граблі / лог факапів
- **Wokwi: горить червоним, не блимає** → на ESP32 DevKit C v4 немає юзер-LED на GPIO2
  (червоний — це power-LED, припаяний до +5V). Фікс: зовнішній LED + резистор 220 Ω на GPIO2.
- **«ELF file not found» / симулятор не стартує** → не зроблено Build перед запуском,
  або шлях у `wokwi.toml` не збігається з теками `.pio`.
- **Дебаг: «Unable to start debugging»** → невірний `miDebuggerPath`, порт у `launch.json`
  ≠ `gdbServerPort` у `wokwi.toml`, або симулятор запущено не в режимі «…and Wait for Debugger».
- **Перший Build висить довго** → разове завантаження тулчейну з мережі. Не перебивати.

## Чекліст готовності
- [x] Build = `SUCCESS`, є `.elf` і `.bin`
- [x] LED блимає в локальному Wokwi (правка → build → симуляція)
- [x] Дебаг спиняється на breakpoint, F10/F11 крокують, видно змінні

## Джерела
- docs.platformio.org — встановлення й робота з PlatformIO IDE
- docs.wokwi.com — Wokwi for VS Code: getting started, project config, debugging