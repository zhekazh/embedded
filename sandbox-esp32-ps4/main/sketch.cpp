// sandbox-esp32-ps4 — DualShock 4 по Bluetooth + статус і кнопки на OLED SSD1306.
//
// Bluepad32 займає UART власною консоллю (Console), тож Serial НЕ використовуємо —
// дебаг через Console.printf, а весь UI на OLED.
//   DS4  : Bluetooth Classic HID, авто-конект (Bluepad32 у режимі сканування).
//   OLED : I2C 0x3C, SDA=GPIO21, SCL=GPIO22 (як у sandbox-esp32).

#include "sdkconfig.h"

#include <Arduino.h>
#include <Bluepad32.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_W  128
#define SCREEN_H  64
#define OLED_ADDR 0x3C
#define SDA_PIN   21
#define SCL_PIN   22

Adafruit_SSD1306 oled(SCREEN_W, SCREEN_H, &Wire, -1);

// Показуємо один контролер (екран малий). Перший підключений займає слот.
ControllerPtr myController = nullptr;

// Біти dpad() з uni_gamepad.h: UP=1, DOWN=2, RIGHT=4, LEFT=8.
static const uint8_t DP_UP = 0x01, DP_DOWN = 0x02, DP_RIGHT = 0x04, DP_LEFT = 0x08;

void onConnectedController(ControllerPtr ctl) {
    if (myController == nullptr) {
        myController = ctl;
        Console.printf("Connected: %s\n", ctl->getModelName().c_str());
    } else {
        Console.println("Controller connected, but slot is busy");
    }
}

void onDisconnectedController(ControllerPtr ctl) {
    if (myController == ctl) {
        myController = nullptr;
        Console.println("Disconnected");
    }
}

// Додає назву кнопки в рядок, якщо вона натиснута.
static void appendBtn(String& s, bool pressed, const char* name) {
    if (pressed) {
        if (s.length())
            s += ' ';
        s += name;
    }
}

// Збирає рядок зі всіх натиснутих кнопок (назви як на DualShock 4).
static String pressedButtons(ControllerPtr c) {
    String s;
    appendBtn(s, c->a(), "Cross");
    appendBtn(s, c->b(), "Circle");
    appendBtn(s, c->x(), "Square");
    appendBtn(s, c->y(), "Triangle");
    appendBtn(s, c->l1(), "L1");
    appendBtn(s, c->r1(), "R1");
    appendBtn(s, c->l2(), "L2");
    appendBtn(s, c->r2(), "R2");
    appendBtn(s, c->thumbL(), "L3");
    appendBtn(s, c->thumbR(), "R3");
    uint8_t dp = c->dpad();
    appendBtn(s, dp & DP_UP, "Up");
    appendBtn(s, dp & DP_DOWN, "Down");
    appendBtn(s, dp & DP_LEFT, "Left");
    appendBtn(s, dp & DP_RIGHT, "Right");
    appendBtn(s, c->miscSelect(), "Share");
    appendBtn(s, c->miscStart(), "Options");
    appendBtn(s, c->miscSystem(), "PS");
    appendBtn(s, c->miscCapture(), "Pad");
    return s;
}

static void drawScreen() {
    oled.clearDisplay();
    oled.setTextColor(SSD1306_WHITE);
    oled.setTextSize(1);

    oled.setCursor(0, 0);
    oled.print("PS4 <-> ESP32 BT");
    oled.drawFastHLine(0, 9, SCREEN_W, SSD1306_WHITE);

    if (myController && myController->isConnected()) {
        oled.setCursor(0, 12);
        oled.print("OK: ");
        oled.println(myController->getModelName());

        // Аналогові стіки (-512..511): L = лівий, R = правий.
        oled.setCursor(0, 22);
        oled.printf("L%d,%d R%d,%d", myController->axisX(), myController->axisY(),
                    myController->axisRX(), myController->axisRY());

        // Натиснуті кнопки (з автопереносом рядка).
        oled.setCursor(0, 34);
        String b = pressedButtons(myController);
        if (b.length() == 0)
            b = "(none)";
        oled.print("Btn:");
        oled.println(b);
    } else {
        oled.setCursor(0, 22);
        oled.println("Scanning...");
        oled.setCursor(0, 42);
        oled.println("Press SHARE+PS");
        oled.println("on DualShock 4");
    }

    oled.display();
}

// Arduino setup(). Виконується на CPU 1.
void setup() {
    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(400000);

    if (!oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Console.println("SSD1306 not found at 0x3C");
        // Без екрана сенсу мало, але BT хай працює — не блокуємось.
    }
    oled.clearDisplay();
    oled.setTextColor(SSD1306_WHITE);
    oled.setTextSize(1);
    oled.setCursor(0, 0);
    oled.println("PS4 <-> ESP32 BT");
    oled.println("starting BT...");
    oled.display();

    Console.printf("Firmware: %s\n", BP32.firmwareVersion());
    BP32.setup(&onConnectedController, &onDisconnectedController);

    // НЕ забуваємо ключі -> контролер пам'ятається між перезавантаженнями.
    // Якщо паринг капризує (не конектиться) — розкоментуй на один запуск:
    // BP32.forgetBluetoothKeys();

    BP32.enableVirtualDevice(false);
}

// Arduino loop(). Виконується на CPU 1.
void loop() {
    BP32.update();
    drawScreen();
    delay(100);  // ~10 оновлень/с — для статусу й кнопок з головою
}
