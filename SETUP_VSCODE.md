# Розгортання й робота в VS Code (PlatformIO · Wokwi · C)

Єдиний вхідний гайд: як підняти репозиторій з нуля та як ним щодня користуватися —
збірка прошивки (PlatformIO), симуляція (Wokwi), компіляція й дебаг C на хості,
дебаг прошивки через GDB. Усі приклади — на модулі **m0** (`m0-blink/`).

> Середовище, під яке писано: **Linux** (Ubuntu/Pop!_OS), VS Code, system Python `/usr/bin/python3`.
> Репозиторій **emulator-first**: M0–M4 працюють без фізичної плати.

---

## 0. Передумови

| Інструмент | Перевірка | Якщо немає |
|---|---|---|
| Git | `git --version` | `sudo apt install git` |
| VS Code | — | з офсайту / `snap` |
| System Python 3.6+ | `/usr/bin/python3 --version` | `sudo apt install python3 python3-venv python3-pip` |
| gcc/gdb (для host-C) | `gcc --version && gdb --version` | `sudo apt install build-essential gdb` |

> **Чому саме system Python.** PlatformIO навмисно **не бере** conda/anaconda-Python.
> Якщо `which -a python3` першим показує conda — це нормально, ми явно вкажемо
> PlatformIO системний `/usr/bin/python3` (див. крок 2).

---

## 1. Клон і відкриття workspace

```bash
git clone <repo-url> embedded && cd embedded
code embedded.code-workspace        # АБО: File → Open Workspace from File…
```

⚠️ Відкривай саме **`embedded.code-workspace`** (кореневий, multi-root), а не окрему теку модуля.
Він містить усі модулі одним вікном, спільні налаштування, рекомендації розширень і GDB-конфіги.

_Перевірка:_ у Explorer видно кілька папок workspace — `embedded (корінь)`, `m0 · blink (ESP32)`, `m1 · photo-led (ESP32)`.

---

## 2. Розширення VS Code

При відкритті workspace VS Code запропонує **рекомендовані розширення** — погодься
(або Extensions → фільтр `@recommended`). Ключові:

- `platformio.platformio-ide` — збірка/прошивка/монітор
- `wokwi.wokwi-vscode` — симуляція ESP32
- `ms-vscode.cpptools` — IntelliSense/дебаг C/C++
- `marus25.cortex-debug`, `dan-c-underwood.arm` — для перенесення на ARM/STM32 (опційно; ESP32 дебажиться через cppdbg/GDB)
- `streetsidesoftware.code-spell-checker` + `…-ukrainian` — орфографія (en+uk)

> **НЕ став** `C/C++ Extension Pack` — конфліктує з PlatformIO. Він у
> `unwantedRecommendations`, але cpptools усе одно може запропонувати його
> своїм власним попапом → натисни **«Don't Show Again»** (записується глобально, більше не питатиме).

PlatformIO Python уже зашитий у workspace:
```jsonc
// embedded.code-workspace → settings
"platformio-ide.customPMPython": "/usr/bin/python3"
```
Після першого відкриття PlatformIO докачає свій Core у `~/.platformio` (хвилина-дві).

_Перевірка:_ внизу зліва з'явилась іконка 🏠 (PlatformIO Home); у статус-барі — кнопки ✓ (Build), → (Upload), 🔌 (Monitor).

---

## 3. PlatformIO — збірка прошивки (m0)

### Через VS Code
Внизу в статус-барі обери активний проєкт (`m0 · blink`) → натисни **✓ (Build)**.

### Через CLI
```bash
cd m0-blink/blink
pio run                 # якщо pio не в PATH: ~/.platformio/penv/bin/pio run
```

_Перевірка:_ у терміналі `SUCCESS`; з'явились `.pio/build/esp32dev/firmware.elf` і `firmware.bin`.
Перший білд довгий (тягне платформу `espressif32@6.9.0` + тулчейн), далі — з кешу ~5 с.

> Щоб `pio` був у PATH глобально, додай у `~/.bashrc`:
> `export PATH="$HOME/.platformio/penv/bin:$PATH"`

Корисні цілі:
```bash
pio run -t clean        # чиста перезбірка
pio run -t upload       # прошити фізичну плату (коли буде)
pio device monitor      # серійний монітор
```

---

## 4. IntelliSense для прошивки (`#include <Arduino.h>`)

Якщо під `#include <Arduino.h>` червоні squiggles, а Build при цьому `SUCCESS` — це
**лінтер**, не помилка коду. Причина: cpptools читає `c_cpp_properties.json` із `.vscode/`
**кожної папки workspace**, а PlatformIO генерує його в теку проєкту. Тому:

1. кожен firmware-проєкт у `embedded.code-workspace` доданий **окремою папкою** (це вже зроблено);
2. сам конфіг **git-ignored** (`.vscode/`), тож після клону його треба згенерувати:
   ```bash
   cd m0-blink/blink
   pio project init --ide vscode      # пише .vscode/c_cpp_properties.json
   ```
   (PlatformIO-розширення робить це й само при відкритті проєкту.)
3. `Ctrl+Shift+P` → **Developer: Reload Window**.

_Перевірка:_ squiggles зникли; працює автодоповнення `pinMode`, `digitalWrite`…; у статус-барі cpptools — конфіг **«PlatformIO»**.

---

## 5. Wokwi — симуляція

Wokwi бере вже зібраний `firmware.elf`/`firmware.bin` (шляхи в `wokwi.toml`).

1. Спершу **Build** (крок 3) — без `.elf` симуляція не стартує.
2. Перший раз: `Ctrl+Shift+P` → **Wokwi: Request a new License** (безкоштовний токен з wokwi.com, разово).
3. `Ctrl+Shift+P` → **Wokwi: Start Simulator** — відкриється `diagram.json` зі схемою.

_Перевірка:_ для m0 LED блимає у вікні симуляції.

> ⚠️ На платі `ESP32 DevKit C V4` немає юзер-LED на GPIO2 (тільки power-LED). У m0
> в `diagram.json` доданий зовнішній `wokwi-led` + резистор 220 Ω. Деталі — у
> [m0-blink/README.md](m0-blink/README.md) («Граблі»).

---

## 6. Дебаг прошивки через Wokwi GDB

Дебаг іде через **GDB-сервер симулятора** (не через фізичний JTAG/SWD-адаптер).

1. У `wokwi.toml` має бути `gdbServerPort = 3333` (у m0 вже є).
2. **Build** прошивки.
3. `Ctrl+Shift+P` → **Wokwi: Start Simulator and Wait for Debugger**.
4. Перейди в **Run and Debug** (Ctrl+Shift+D) → обери конфіг **«Wokwi GDB — m0-blink»** → `F5`.

GDB-конфіги лежать у `embedded.code-workspace` → секція `"launch"` (а **не** в
`blink/.vscode/launch.json`, який PlatformIO регенерує й затирає). Шлях до GDB
прописаний переносно: `${userHome}/.platformio/packages/toolchain-xtensa-esp32/bin/xtensa-esp32-elf-gdb`.

_Перевірка:_ виконання спиняється на breakpoint; працюють F10/F11 (step); у VARIABLES видно змінні.

> ❌ **Не** запускай *PlatformIO: Debug* — для `esp32dev` він стартує OpenOCD і шукає
> фізичний адаптер, якого в emulator-first немає → «Target disconnected». Тільки Wokwi GDB.

---

## 7. Компіляція / запуск / дебаг C на хості

Host-C трек (`m0-blink/host-c/`, пізніше `m2-c/`) не потребує плати — звичайний gcc.

### Швидкий прогін
```bash
cd m0-blink/host-c
gcc -Wall -Wextra -std=c11 reg.c -o reg && ./reg
```
_Перевірка:_ друкує очікувані бітові патерни; `assert` не падає (exit 0).

### House style — суворі прапори + санітайзери
```bash
gcc -Wall -Wextra -std=c11 -fsanitize=address,undefined -g reg.c -o reg && ./reg
```
UB — головний ворог; `-Wall -Wextra` + ASAN/UBSAN — перша лінія захисту.
Модуль m2 уже має Unity-тести під цими прапорами: `cd m2-c/frame-codec && make test`.

### Дебаг у gdb
```bash
gcc -Wall -Wextra -std=c11 -g reg.c -o reg     # -g обов'язково
gdb ./reg
# (gdb) break main → run → next/step → print <змінна> → continue
```
Або візуально у VS Code: створи `launch.json` типу `cppdbg` на зібраний бінарник
(host-C дебаг іде звичайним системним gdb, без xtensa-тулчейну).

---

## 8. Додавання нового модуля (mN) у workspace

Спільне для обох типів: новий модуль додаєш **окремою `folders`-папкою** у
`embedded.code-workspace` (інакше cpptools читатиме його під коренем без власного
конфіга) і робиш **Reload Window**. Далі — залежно від типу модуля.

### A. Firmware-модуль (PlatformIO + Wokwi)
1. Скопіюй структуру: `mN-*/blink/` з `platformio.ini` (пінь версії платформи!), `wokwi.toml`, `diagram.json`, `src/main.cpp`.
2. Додай `folders`-запис:
   ```jsonc
   { "name": "mN · <назва> (ESP32)", "path": "mN-.../blink" }
   ```
3. За потреби дебагу — додай `launch`-блок (скопіюй з m0/m1, поправ `name` і `${workspaceFolder:…}`).
4. Згенеруй IntelliSense-конфіг: `cd mN-.../blink && pio project init --ide vscode`
   (хедери Arduino/ESP-IDF лежать поза workspace — без цього `#include <Arduino.h>` червонітиме).
5. **Reload Window**.

> PlatformIO-білд новий проєкт підхопить сам; ручного редагування потребують лише
> `folders` і (за бажанням) `launch`.

### B. Host-C модуль (gcc + Makefile, без плати)
1. Скопіюй структуру: `mN-*/` з `Makefile`, `src/`, `test/` (за зразком `m2-c/frame-codec/`).
2. Додай `folders`-запис:
   ```jsonc
   { "name": "mN · C (host)", "path": "mN-..." }
   ```
3. **`pio project init` НЕ потрібен** — це не PlatformIO-проєкт. Усі хедери локальні
   (`*.h` у `src/`, `test/unity/`), тож cpptools резолвить їх рекурсивним includePath
   за замовчуванням, щойно тека стане папкою workspace.
4. **Reload Window**.

> Збірка/тести — лише через `make` (`make test`); дебаг — звичайним системним `gdb`.
> Окремий `c_cpp_properties.json` зазвичай не потрібен; додавай мінімальний (`-Isrc -Itest/unity`)
> тільки якщо squiggles на локальних `#include "…"` усе ж з'являться.

---

## Швидка шпаргалка

| Дія | Як |
|---|---|
| Відкрити проєкт | `code embedded.code-workspace` |
| Зібрати прошивку | статус-бар ✓, або `pio run` у теці проєкту |
| Запустити симуляцію | `F1 → Wokwi: Start Simulator` (після Build) |
| Дебаг прошивки | `F1 → Wokwi: Start Simulator and Wait for Debugger` → `F5` |
| Скомпілювати C | `gcc -Wall -Wextra -std=c11 reg.c -o reg && ./reg` |
| Тести host-C | `cd m2-c/frame-codec && make test` |
| Полагодити IntelliSense | `pio project init --ide vscode` + Reload Window |

Детальний наратив теорії та повний «лог факапів» — у [m0-blink/README.md](m0-blink/README.md).
