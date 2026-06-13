#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- Параметри панелі ---
#define SCREEN_W   128
#define SCREEN_H   64
#define OLED_ADDR  0x3C   // адреса I2C нашого SSD1306 (з HARDWARE.md)
#define SDA_PIN    21     // дефолтні I2C-піни ESP32
#define SCL_PIN    22

// reset-піна окремого немає (-1): у I2C-модулях вона не виведена
Adafruit_SSD1306 oled(SCREEN_W, SCREEN_H, &Wire, -1);

// стан анімації — квадратик, що відбивається від країв
int x = 0, y = 40, dx = 3, dy = 2;
uint32_t frame = 0;

void setup() {
  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);          // fast-mode I2C — без цього ~10 FPS замість ~25

  if (!oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("SSD1306: не знайдено на 0x3C — перевір пайку SDA/SCL/живлення");
    while (true) delay(1000);     // далі немає сенсу
  }
  Serial.println("\n[sandbox-esp32] OLED OK");

  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
}

void loop() {
  uint32_t t0 = millis();

  oled.clearDisplay();

  // 1. Текст різного розміру
  oled.setTextSize(1);
  oled.setCursor(0, 0);
  oled.print("sandbox-esp32 OLED");

  oled.setTextSize(2);            // 2x масштаб вбудованого шрифту
  oled.setCursor(0, 12);
  oled.print("frame ");
  oled.print(frame);

  // 2. Графічні примітиви
  oled.drawRect(0, 30, 128, 12, SSD1306_WHITE);            // рамка прогрес-бара
  oled.fillRect(0, 30, (frame % 128), 12, SSD1306_WHITE);  // заповнення

  // 3. Анімація — рухомий квадратик, що відбивається
  oled.fillRect(x, y, 8, 8, SSD1306_WHITE);
  x += dx; y += dy;
  if (x <= 0 || x >= SCREEN_W - 8) dx = -dx;
  if (y <= 44 || y >= SCREEN_H - 8) dy = -dy;

  oled.display();                 // ось ТУТ весь буфер ллється по I2C

  // Раз на ~30 кадрів друкуємо реальний час кадру -> видно FPS
  uint32_t dt = millis() - t0;
  if (frame % 30 == 0) {
    Serial.printf("frame %lu: %lu ms/кадр (~%lu FPS)\n",
                  frame, dt, dt ? 1000 / dt : 999);
  }
  frame++;
}
