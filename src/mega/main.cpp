// src/mega/main.cpp
#include <Arduino.h>
#include "Config.h"
#include "Actuator.h"
#include "Pump.h"
#include "CommandHandler.h"
const int BUZZER_PIN = 19;

// Объявление массивов для объектов
Actuator* actuators[NUM_ACTUATORS];
Pump* pumps[NUM_PUMPS];

// Обработчик команд
CommandHandler* commandHandler;

// Пин для пищалки

// Буфер для приёма строк через Serial
String inputBuffer = "";
bool commandComplete = false;

// Функция для обработки входящих команд
void processCommand(String command) {
  Serial.print(F("Получена команда: "));
  Serial.println(command);
  
  // Запуск шагового двигателя: STEPPER_START:motor_index,speed,direction
  if (command.startsWith("STEPPER_START:")) {
    String params = command.substring(14);
    int firstComma = params.indexOf(',');
    int secondComma = params.indexOf(',', firstComma + 1);
    
    if (firstComma > 0 && secondComma > firstComma) {
      int motorIndex = params.substring(0, firstComma).toInt();
      int speed = params.substring(firstComma + 1, secondComma).toInt();
      int direction = params.substring(secondComma + 1).toInt();
      
      // Убедимся, что индекс в правильном диапазоне
      if (motorIndex >= 0 && motorIndex < NUM_PUMPS) {
        // Опускаем актуатор сначала
        actuators[motorIndex]->moveDown();
        delay(3000); // Ждём, пока актуатор опустится
        
        // Запускаем насос
        pumps[motorIndex]->start(speed);
        
        // Звуковой сигнал
        tone(BUZZER_PIN, 1000, 200);
        
        Serial.print(F("Запущен насос "));
        Serial.println(motorIndex);
      }
    }
  }
  
  // Остановка шагового двигателя: STEPPER_STOP
  else if (command.equals("STEPPER_STOP")) {
    // Останавливаем все насосы и поднимаем все актуаторы
    for (int i = 0; i < NUM_PUMPS; i++) {
      pumps[i]->stop();
      actuators[i]->moveUp();
    }
    
    // Звуковой сигнал
    tone(BUZZER_PIN, 800, 200);
    
    Serial.println(F("Все насосы остановлены"));
  }
  
  // Звуковой сигнал: BUZZER:duration
  else if (command.startsWith("BUZZER:")) {
    int duration = command.substring(7).toInt();
    tone(BUZZER_PIN, 1000, duration);
  }
  
  // Запрос о количестве шагов: GET_STEP:motor_index
  else if (command.startsWith("GET_STEP:")) {
    int motorIndex = command.substring(9).toInt();
    if (motorIndex >= 0 && motorIndex < NUM_PUMPS) {
      int steps = pumps[motorIndex]->getStepCount();
      
      // Отправляем ответ на ESP32
      Serial.print("STEP_COUNT:P");
      Serial.print(motorIndex);
      Serial.println(steps);
    }
  }
  
  // Сбросить счётчик шагов: RESET_STEP:motor_index
  else if (command.startsWith("RESET_STEP:")) {
    int motorIndex = command.substring(11).toInt();
    if (motorIndex >= 0 && motorIndex < NUM_PUMPS) {
      pumps[motorIndex]->resetStepCount();
    }
  }
}

void setup() {
  // Инициализация Serial для отладки и связи с ESP32
  Serial.begin(115200);
  
  Serial.println(F("Инициализация устройства..."));
  
  // Инициализация пина пищалки
  pinMode(BUZZER_PIN, OUTPUT);
  noTone(BUZZER_PIN);
  
  // Создание и инициализация актуаторов
  for (int i = 0; i < NUM_ACTUATORS; i++) {
    // Используем два реле для управления каждым актуатором (H-мост)
    if (i < 3) { // У нас только 3 актуатора могут быть сконфигурированы (по количеству пар реле)
      actuators[i] = new Actuator(i, ACTUATOR_RELAY_FWD[i], ACTUATOR_RELAY_REV[i]);
      actuators[i]->init();
    } else {
      actuators[i] = nullptr; // Остальные устанавливаем в nullptr
    }
  }
  
  // Создание и инициализация насосов
  for (int i = 0; i < NUM_PUMPS; i++) {
    pumps[i] = new Pump(i, PUMP_PINS[i]);
    pumps[i]->init();
  }
  
  // Звуковой сигнал при завершении инициализации
  tone(BUZZER_PIN, 1000, 100);
  delay(200);
  tone(BUZZER_PIN, 1200, 100);
  
  // Сигнализируем ESP32, что Arduino готова к работе
  Serial.println("ARDUINO_READY");
}

void loop() {
  // Чтение команд от ESP32
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    
    if (inChar == '\n') {
      commandComplete = true;
      break;
    }
    
    inputBuffer += inChar;
  }
  
  if (commandComplete) {
    processCommand(inputBuffer);
    inputBuffer = "";
    commandComplete = false;
  }
  
  // Обновление состояния актуаторов и насосов
  for (int i = 0; i < NUM_ACTUATORS; i++) {
    if (actuators[i] != nullptr) {
      actuators[i]->update();
    }
  }
  
  for (int i = 0; i < NUM_PUMPS; i++) {
    pumps[i]->update();
  }
}