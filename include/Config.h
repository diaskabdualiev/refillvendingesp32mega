// Config.h

#ifndef CONFIG_H
#define CONFIG_H

// Версия системы
#define VERSION "1.0.0"

// Константы для WiFi
#define WIFI_SSID "Nurda"
#define WIFI_PASSWORD "diasdias"

// Константы для Firebase
#define FIREBASE_HOST "https://diastest-d6240-default-rtdb.europe-west1.firebasedatabase.app/"
#define FIREBASE_AUTH "AIzaSyCdZvy2cIOA-ULtQoiGMY25nBVod-CZ6rY"

// Пины для соединения ESP32 с Arduino Mega
#define ESP32_RX_PIN 16  // RX пин ESP32 (подключается к TX Arduino Mega)
#define ESP32_TX_PIN 17  // TX пин ESP32 (подключается к RX Arduino Mega)

// Определения для LCD дисплея
#define LCD_ADDR 0x27    // Адрес I2C LCD дисплея
#define LCD_COLS 20      // Количество колонок LCD
#define LCD_ROWS 4       // Количество строк LCD

// Константы для статусов устройства
#define STATUS_ZERO_BALANCE 0   // нет баланса
#define STATUS_READY 1          // готов к работе
#define STATUS_PROGRESS 2       // процесс разлива

// Определения для кнопок
#define BUTTON1 1
#define BUTTON2 2
#define BUTTON3 3
#define BUTTON4 4
#define BUTTON5 5
#define BUTTON6 6
#define BUTTON7 7
#define START_BUTTON 8
#define STOP_BUTTON 9

// Пины Arduino Mega
// Первый релейный модуль (пины 2-9)
const int RELAY_MODULE_1_PINS[] = {2, 3, 4, 5, 6, 7, 8, 9};
// Второй релейный модуль (пины 10-15)
const int RELAY_MODULE_2_PINS[] = {10, 11, 12, 13, 14, 15};

// Константы для насосов и актуаторов
#define NUM_PUMPS 7                // Количество насосов
#define NUM_ACTUATORS 7            // Количество актуаторов

// Пины для насосов (используем первый релейный модуль)
const int PUMP_PINS[] = {RELAY_MODULE_1_PINS[0], RELAY_MODULE_1_PINS[1], 
                         RELAY_MODULE_1_PINS[2], RELAY_MODULE_1_PINS[3], 
                         RELAY_MODULE_1_PINS[4], RELAY_MODULE_1_PINS[5], 
                         RELAY_MODULE_1_PINS[6]};

// Пины для релейного управления актуаторами (используем второй релейный модуль)
// Каждый актуатор использует два реле для изменения полярности (H-мост)
// Подключаем два реле для каждого актуатора:
const int ACTUATOR_RELAY_FWD[] = {RELAY_MODULE_2_PINS[0], RELAY_MODULE_2_PINS[2], RELAY_MODULE_2_PINS[4]};
const int ACTUATOR_RELAY_REV[] = {RELAY_MODULE_2_PINS[1], RELAY_MODULE_2_PINS[3], RELAY_MODULE_2_PINS[5]};

// Пин для пищалки

// Константы для шаговых двигателей и насосов
#define STEPS_PER_ML 100    // Количество шагов на 1 мл (настройте по калибровке)
#define MAX_SPEED 13000     // Максимальная скорость шагового двигателя
#define STEPS_PER_LITER 577500  // Шагов на 1 литр

// Интервалы для таймеров (в миллисекундах)
#define POLLING_INTERVAL 500     // Интервал опроса шагов
#define QR_UPDATE_INTERVAL 3000  // Интервал обновления QR

// Команды для обмена между ESP32 и Arduino Mega
#define CMD_BUTTON_PRESSED "BUTTON_PRESSED:"
#define CMD_STEP_COUNT "STEP_COUNT:"
#define CMD_STEPPER_START "STEPPER_START:"
#define CMD_STEPPER_STOP "STEPPER_STOP"
#define CMD_BUZZER "BUZZER:"
#define CMD_GET_STEP "GET_STEP:"
#define CMD_RESET_STEP "RESET_STEP:"
#define CMD_SET_LEDS "SET_LEDS:"
#define CMD_ARDUINO_READY "ARDUINO_READY"

#endif // CONFIG_H