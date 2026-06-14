# Adafruit Bus IO — ESP-IDF компонент. У цьому шаблоні Arduino-компонент
# зветься "arduino" (не "arduino-esp32").
idf_component_register(SRCS "Adafruit_I2CDevice.cpp" "Adafruit_BusIO_Register.cpp" "Adafruit_SPIDevice.cpp" "Adafruit_GenericDevice.cpp"
                       INCLUDE_DIRS "."
                       REQUIRES arduino)
