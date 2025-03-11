#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <FirebaseESP32.h>
#include <ArduinoJson.h>
#include "Config.h"

// Глобальные переменные
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);

String deviceID;
int status = STATUS_ZERO_BALANCE;
int account = 0;
int currentSelect = -1;
double currentVolume = 0;
int bufferStep = 0;
unsigned long dispensingStartTime = 0;
unsigned long lastStepCheck = 0;
unsigned long lastFirebaseCheck = 0;

struct Chemical {
  String name;
  int price;
  bool available;
};

Chemical chemicals[7];

// Прототипы функций
void connectToWiFi();
void initFirebase();
void fetchChemicalsData();
void updateLCD();
void processArduinoCommand(String command);
void checkForNewOrders();
void processNewOrder(String orderData);
void startDispensing(int chemicalId, int volumeMl);
void stopDispensing();
double stepsToVolume(int steps);
int volumeToSteps(int volumeMl);

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, ESP32_RX_PIN, ESP32_TX_PIN);
  
  Wire.begin();
  lcd.begin(LCD_COLS, LCD_ROWS);
  lcd.backlight();
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Система EcoTrend");
  lcd.setCursor(0, 1);
  lcd.print("Инициализация...");
  
  connectToWiFi();
  
  // Получаем MAC-адрес как ID устройства
  deviceID = WiFi.macAddress();
  deviceID.replace(":", "");
  
  lcd.setCursor(0, 2);
  lcd.print("ID устройства:");
  lcd.setCursor(0, 3);
  lcd.print(deviceID);
  delay(2000);
  
  initFirebase();
  fetchChemicalsData();
  
  // Отображаем начальный экран
  updateLCD();
}

void loop() {
  // Проверка входящих команд от Arduino
  if (Serial2.available()) {
    String command = Serial2.readStringUntil('\n');
    processArduinoCommand(command);
  }
  
  // Периодическая проверка данных в Firebase
  unsigned long currentMillis = millis();
  
  // Проверяем Firebase каждые 10 секунд
  if (currentMillis - lastFirebaseCheck >= 10000) {
    lastFirebaseCheck = currentMillis;
    
    if (Firebase.ready()) {
      // Проверяем наличие новых заказов
      checkForNewOrders();
    }
  }
  
  // Если идет процесс разлива, опрашиваем количество шагов
  if (status == STATUS_PROGRESS && currentMillis - lastStepCheck >= 500) {
    lastStepCheck = currentMillis;
    
    Serial2.println("GET_STEP:" + String(currentSelect));
  }
  
  // Если WiFi отключен, пытаемся переподключиться
  if (WiFi.status() != WL_CONNECTED) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi отключен");
    lcd.setCursor(0, 1);
    lcd.print("Переподключение...");
    
    WiFi.reconnect();
    delay(5000); // Ждем 5 секунд
  }
}

void connectToWiFi() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Подключение к WiFi");
  lcd.setCursor(0, 1);
  lcd.print(WIFI_SSID);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int attempts = 0;
  
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    lcd.setCursor(attempts % 16, 2);
    lcd.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi подключен!");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP().toString());
    delay(2000);
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Ошибка подключения");
    lcd.setCursor(0, 1);
    lcd.print("WiFi");
    delay(2000);
  }
}

void initFirebase() {
  // Настройка Firebase
  config.database_url = FIREBASE_HOST;
  config.api_key = FIREBASE_AUTH;
  
  auth.user.email = "your-email@gmail.com";
  auth.user.password = "your-password";
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Firebase Инициализ.");
  
  // Проверка подключения к Firebase
  delay(2000); // Даем время для подключения
  
  if (Firebase.ready()) {
    lcd.setCursor(0, 2);
    lcd.print("Firebase Готов!");
  } else {
    lcd.setCursor(0, 2);
    lcd.print("Firebase Ошибка!");
  }
  delay(2000);
}

void fetchChemicalsData() {
  if (Firebase.ready()) {
    String path = "/" + deviceID;
    
    if (Firebase.get(fbdo, path.c_str())) {
      // Используем ArduinoJson для правильного парсинга
      DynamicJsonDocument doc(4096);
      deserializeJson(doc, fbdo.stringData());
      
      // Получаем баланс
      if (doc.containsKey("balance")) {
        account = doc["balance"].as<int>();
        
        if (account > 0) {
          status = STATUS_READY;
        } else {
          status = STATUS_ZERO_BALANCE;
        }
      }
      
      // Получаем данные для каждого химиката
      if (doc.containsKey("containers")) {
        for (int i = 0; i < 7; i++) {
          String tankKey = "tank" + String(i+1);
          
          if (doc["containers"].containsKey(tankKey)) {
            chemicals[i].name = doc["containers"][tankKey]["name"].as<String>();
            chemicals[i].price = doc["containers"][tankKey]["price"].as<int>();
            // По умолчанию считаем доступным
            chemicals[i].available = true;
          }
        }
      }
      
      // Обновляем экран
      updateLCD();
    }
  }
}

void updateLCD() {
  lcd.clear();
  
  switch (status) {
    case STATUS_ZERO_BALANCE:
      lcd.setCursor(0, 0);
      lcd.print("Баланс: 0");
      lcd.setCursor(0, 1);
      lcd.print("Пополните баланс");
      lcd.setCursor(0, 2);
      lcd.print("через Kaspi QR");
      break;
      
    case STATUS_READY:
      lcd.setCursor(0, 0);
      lcd.print("Баланс: ");
      lcd.print(account);
      lcd.setCursor(0, 1);
      lcd.print("Готов к работе");
      lcd.setCursor(0, 2);
      lcd.print("Выберите в Firebase");
      break;
      
    case STATUS_PROGRESS:
      lcd.setCursor(0, 0);
      lcd.print("Розлив: ");
      lcd.print(chemicals[currentSelect].name);
      lcd.setCursor(0, 1);
      lcd.print("Баланс: ");
      lcd.print(account);
      lcd.setCursor(0, 2);
      lcd.print("Объем: ");
      lcd.print(currentVolume, 2);
      lcd.print(" л");
      lcd.setCursor(0, 3);
      lcd.print("Идет розлив...");
      break;
  }
}

void processArduinoCommand(String command) {
  Serial.print("От Arduino: ");
  Serial.println(command);
  
  if (command.startsWith("STEP_COUNT:")) {
    // Формат: STEP_COUNT:P<номер_насоса><число_шагов>
    int pPos = command.indexOf('P');
    if (pPos > 0) {
      int motorIndex = command.substring(pPos+1, pPos+2).toInt();
      int steps = command.substring(pPos+2).toInt();
      
      // Проверяем, что это шаги для текущего насоса
      if (status == STATUS_PROGRESS && motorIndex == currentSelect) {
        currentVolume = stepsToVolume(steps);
        updateLCD();
        
        // Пересчитываем стоимость и обновляем баланс
        int cost = (int)(currentVolume * chemicals[currentSelect].price);
        int remainingBalance = bufferStep - cost;
        
        if (remainingBalance > 0) {
          account = remainingBalance;
        } else {
          // Баланс закончился, останавливаем насос
          stopDispensing();
          status = STATUS_ZERO_BALANCE;
          account = 0;
          
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Баланс закончился!");
          lcd.setCursor(0, 1);
          lcd.print("Операция прервана");
          delay(3000);
        }
      }
    }
  }
  else if (command.startsWith("ACTUATOR_STATUS:")) {
    // Обработка статуса актуатора
    int commaPos = command.indexOf(',');
    if (commaPos > 0) {
      int actuatorIndex = command.substring(16, commaPos).toInt();
      String actuatorStatus = command.substring(commaPos + 1);
      
      if (actuatorStatus == "DOWN" && actuatorIndex == currentSelect) {
        // Актуатор опустился, запускаем насос
        Serial2.println("STEPPER_START:" + String(currentSelect) + ",13000,0");
        Serial2.println("RESET_STEP:" + String(currentSelect));
      }
      else if (actuatorStatus == "UP") {
        // Актуатор поднялся, операция завершена
        if (status == STATUS_PROGRESS) {
          status = STATUS_READY;
          
          // Обновляем баланс в Firebase
          if (Firebase.ready()) {
            Firebase.setInt(fbdo, ("/" + deviceID + "/balance").c_str(), account);
          }
          
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Розлив завершен!");
          lcd.setCursor(0, 1);
          lcd.print("Объем: ");
          lcd.print(currentVolume, 2);
          lcd.print(" л");
          delay(3000);
          
          updateLCD();
        }
      }
    }
  }
  else if (command == "ARDUINO_READY") {
    Serial.println("Arduino Mega готова");
  }
}

void checkForNewOrders() {
  String path = "/" + deviceID + "/orders/current";
  
  if (Firebase.get(fbdo, path.c_str())) {
    // Парсим JSON данные
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, fbdo.stringData());
    
    // Проверяем статус заказа
    if (doc.containsKey("status") && doc["status"] == "new") {
      // Новый заказ
      processNewOrder(fbdo.stringData());
    }
  }
}

void processNewOrder(String orderData) {
  // Парсим данные заказа
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, orderData);
  
  // Получаем данные заказа
  int chemicalId = doc["chemical_id"];
  int volumeMl = doc["volume_ml"];
  String paymentStatus = doc["payment_status"];
  
  if (paymentStatus == "paid" && account > 0) {
    // Заказ оплачен и есть баланс, начинаем розлив
    startDispensing(chemicalId, volumeMl);
    
    // Обновляем статус заказа
    FirebaseJson json;
    json.add("status", "processing");
    Firebase.updateNode(fbdo, ("/" + deviceID + "/orders/current").c_str(), json);

  }
}

void startDispensing(int chemicalId, int volumeMl) {
  if (chemicalId < 0 || chemicalId >= 7 || !chemicals[chemicalId].available) {
    return;
  }
  
  currentSelect = chemicalId;
  bufferStep = account;
  currentVolume = 0;
  status = STATUS_PROGRESS;
  
  // Опускаем актуатор
  Serial2.println("ACTUATOR_DOWN:" + String(chemicalId));
  
  updateLCD();
}

void stopDispensing() {
  // Останавливаем насос
  Serial2.println("STEPPER_STOP");
  
  // Поднимаем актуатор
  Serial2.println("ACTUATOR_UP:" + String(currentSelect));
}

double stepsToVolume(int steps) {
  // Преобразуем шаги в объем (литры)
  return (double)steps / STEPS_PER_LITER;
}

int volumeToSteps(int volumeMl) {
  // Преобразуем объем (мл) в шаги
  return volumeMl * (STEPS_PER_LITER / 1000);
}