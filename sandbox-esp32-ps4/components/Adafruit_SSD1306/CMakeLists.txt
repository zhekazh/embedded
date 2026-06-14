# Adafruit SSD1306 — ESP-IDF компонент. Тека GFX перейменована на Adafruit_GFX.
idf_component_register(SRCS "Adafruit_SSD1306.cpp"
                       INCLUDE_DIRS "."
                       REQUIRES arduino Adafruit_GFX)
