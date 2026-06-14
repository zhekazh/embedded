# sandbox-esp32-ps4 — DualShock 4 по Bluetooth + OLED

> **Статус:** 🔄 в роботі
> **Що це:** контролер **PS4 (DualShock 4)** під'єднується до ESP32 по Bluetooth, а на
> **OLED SSD1306** показується статус підключення і назви натиснутих кнопок.
> **Залізо:** ESP32 DevKit (WROOM-32), DualShock 4, OLED 0.96" SSD1306 (I2C 0x3C).

## Чому окремий проєкт (а не в `sandbox-esp32`)
DualShock 4 — це **Bluetooth Classic HID**. Бібліотека **Bluepad32** (найнадійніша під
ESP32) на Arduino їде на власному стеку **BTstack** і ставиться лише через шаблонний
проєкт **ESP-IDF + Arduino-as-component** — тобто `framework = espidf`, а не звичайний
Arduino з `lib_deps`. Через це окрема тека з іншою структурою (`main/` замість `src/`,
компоненти в `components/`).

## Схема (OLED — як у sandbox-esp32)
| OLED SSD1306 | ESP32 |
|---|---|
| VCC | 3V3 |
| GND | GND |
| SDA | GPIO21 |
| SCL | GPIO22 |

Контролер по повітрю — фізичних з'єднань не треба.

## Відтворення з нуля
Важкі апстрім-компоненти (`arduino` 64M, `btstack` 14M, `bluepad32`) **не в git**
(див. `.gitignore`) — відновлюються із шаблону:

```bash
# 1) Клон шаблону Bluepad32 (дає components/{arduino,btstack,bluepad32,...})
git clone --recursive https://github.com/ricardoquesada/esp-idf-arduino-bluepad32-template.git tmp
# Перенести з tmp/components/ сюди: arduino, btstack, bluepad32, bluepad32_arduino, cmd_*

# 2) OLED-бібліотеки як компоненти (вже в git цього репо, з нашими CMakeLists):
#    components/Adafruit_BusIO, Adafruit_GFX, Adafruit_SSD1306
#    (clone з github.com/adafruit/*, теку GFX перейменувати на Adafruit_GFX)
```

Наші правки поверх шаблону вже в git: `main/sketch.cpp`, `main/CMakeLists.txt`
(додані Adafruit-компоненти в REQUIRES), `platformio.ini` (`default_envs = esp32dev`),
CMakeLists у трьох Adafruit-компонентах.

## Збірка / прошивка / запуск
```bash
pio run                       # перша збірка довга: качає платформу + ESP-IDF тулчейн
pio run -t upload --upload-port COM3
pio device monitor            # лог Bluepad32 (Console), 115200
```
У VS Code — PlatformIO **Build / Upload / Monitor** (env `esp32dev`).

## Паринг DualShock 4
1. Прошити й увімкнути плату (на OLED — `Scanning...`).
2. На контролері затиснути **SHARE + PS** разом, поки лайтбар не почне **швидко
   двічі-блимати** (режим паринга).
3. Bluepad32 у режимі сканування **сам під'єднається** → на OLED `OK: DualShock 4`,
   а натиснуті кнопки з'являються списком (`Cross`, `Circle`, `L1`, `Up`, `PS`…).

> Ключі НЕ забуваються між перезавантаженнями — контролер пам'ятається. Якщо паринг
> капризує, у `setup()` одноразово розкоментуй `BP32.forgetBluetoothKeys();`.

## Граблі / лог факапів
> Симптом → причина → фікс.
- **Adafruit-компоненти не лінкуються** → їхні вшиті `CMakeLists.txt` мають чужі імена:
  BusIO вимагав `arduino-esp32`, а в шаблоні компонент зветься `arduino`; SSD1306 вимагав
  `Adafruit-GFX-Library`, а ми перейменували теку на `Adafruit_GFX` → виправити `REQUIRES`
  під реальні імена компонентів (= імена тек) і прибрати зайві `project()`/`cmake_minimum_required`.
- **Serial не друкує / конфлікт** → Bluepad32 тримає UART власною `Console` → не
  викликати `Serial`, дебаг через `Console.printf` (UI і так на OLED).
- **Треба перепаринговувати щоразу** → `BP32.forgetBluetoothKeys()` у setup стирає ключі
  → закоментувати, щоб контролер пам'ятався.
- **Перша збірка «висить»** → насправді качає ESP-IDF тулчейн (десятки хвилин, у
  `~/.platformio`) → дочекатись; далі збірки швидкі.
- **`kconfgen ... not found` / покручений шлях у логу** → ESP-IDF **не переварює
  не-ASCII символи у шляху проєкту**, а репо лежало в `OneDrive\Документи\…` (кирилиця)
  → збирати з ASCII-шляху (перенесли репо в `C:\dev\embedded`). Arduino-фреймворк цього
  не помічав, повний ESP-IDF — падає.
- **`esptool ... get_metavar() missing 'ctx'`** на генерації `*.bin` → у penv PlatformIO
  став занадто новий `click` (8.3.x), несумісний з esptool 5.0.0 → відкотити:
  `~/.platformio/penv/Scripts/python -m pip install "click==8.1.8"`.
- **`Could not open COM3, port is busy`** → відкритий Serial Monitor або зомбі-процес
  esptool від обірваної заливки тримає порт → закрити монітор / прибити зайві `pio`/`python`,
  тоді одна чиста заливка.

## Чекліст «працює»
- [x] `pio run` збирається (ESP-IDF + Arduino + Bluepad32 + Adafruit OLED)
- [x] прошивка заливається на плату
- [x] на OLED статус `Scanning...` → `OK: DualShock 4` після паринга
- [x] натиснуті кнопки показуються на екрані
- [x] стіки (L/R, -512..511) показуються на екрані
