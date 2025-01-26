#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseClient.h>
#include "secrets.h"
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "var.h"
#include "FirebaseJson.h"

// ------ Ваши глобальные объекты (как у вас в коде) ------
String deviceID;
bool taskComplete = false;// получение баланса и всех остальных имен и цен для выполнения только одного раза
int account = 0;
int currentSelect = 10;
int step = 0;
int priceForLit = 0;
int oneTengeIsStep = 0;
int priceing = 0;
int oneLitForSteps = 577500;
double currentVolume = 0;
int bufferStep = 0;

const unsigned long interval = 500;  // Интервал между переключениями (мс)
unsigned long previousMillis = 0;    // Время последнего переключения
const unsigned long interval1 = 3000;  // Интервал между переключениями (мс)
unsigned long previousMillis1 = 0; 
int status = ZERO_BALANCE;


struct Names {
  String name[7];
};
struct Prices {
  int price[7];
};
struct Tanks {
  float tank[7];
};
Names names;
Prices prices;
Tanks tanks;

void asyncCB(AsyncResult &aResult);
void printResult(AsyncResult &aResult);
void getAllNameAndPrice(String jsonString);
void printAccount(String account);
void handleButtonClick(int index);
void printCurrentVolume(double volume);
double round2(double val);

DefaultNetwork network;
UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD);
FirebaseApp app;
WiFiClientSecure ssl_client;
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client, getNetwork(network));
RealtimeDatabase Database;
AsyncResult aResult_no_callback;

LiquidCrystal_I2C lcd(PCF8574_ADDR_A21_A11_A01, 4, 5, 6, 16, 11, 12, 13, 14, POSITIVE);

String inputBuffer; // буфер для приёма строк через Serial1

// ------ Предопределим паттерны SET_LEDS для удобства ------
static const char* LED_PATTERNS[] = {
  "SET_LEDS:1,0,0,0,0,0,0",  // Для кнопки 0
  "SET_LEDS:0,1,0,0,0,0,0",  // Для кнопки 1
  "SET_LEDS:0,0,1,0,0,0,0",  // Для кнопки 2
  "SET_LEDS:0,0,0,1,0,0,0",  // Для кнопки 3
  "SET_LEDS:0,0,0,0,1,0,0",  // Для кнопки 4
  "SET_LEDS:0,0,0,0,0,1,0",  // Для кнопки 5
  "SET_LEDS:0,0,0,0,0,0,1",  // Для кнопки 6
  "SET_LEDS:1,1,1,1,1,1,1",  // Например, для кнопки 7 (пример)
  // Если нужно больше вариантов – добавьте
};

// ------ Объявление функций ------
void wifiConnect();

// ------ setup ------
void setup()
{
    Serial.begin(115200);

    // Инициализируем Serial1 (укажите правильные пины и скорость!)
    // Пример: если TX = GPIO17, RX = GPIO16, скорость 115200
    Serial1.begin(115200, SERIAL_8N1, 16, 17);

    // Инициализация LCD
    while (lcd.begin(COLUMS, ROWS, LCD_5x8DOTS) != 1)
    {
      Serial.println(F("PCF8574 is not connected or wrong lcd pins."));
      delay(5000);   
    }

    // Подключаемся к WiFi
    wifiConnect();

    // Инициализация Firebase
    ssl_client.setInsecure();
    Serial.println("Initializing the app...");
    initializeApp(aClient, app, getAuth(user_auth), asyncCB, "authTask");
    Database.setSSEFilters("get,put,patch,keep-alive,cancel,auth_revoked");
    app.getApp<RealtimeDatabase>(Database);
    Database.url(DATABASE_URL);
    Database.get(aClient, "/"+deviceID, asyncCB, false, "getInitInfo");
}

void loop()
{
    unsigned long currentMillis = millis();
    // Основной цикл Firebase
    app.loop();
    Database.loop();
    if (currentMillis - previousMillis >= interval && status == PROGRESS) {
        // Запоминаем время текущего переключения
        previousMillis = currentMillis;
        Serial1.println("GET_STEP:" + String(currentSelect));
        Serial.println("Справишваем количество оборотов");
    }



    if(status == ZERO_BALANCE && currentMillis - previousMillis1 >= interval1){
      lcd.setCursor(0, 0);
      lcd.print("Please pay Kaspi QR");
      lcd.setCursor(0, 1);
      lcd.print("Click start for");
      lcd.setCursor(0, 2);
      lcd.print("the balance report");
      lcd.setCursor(0, 3);
      lcd.print("                       ");
    }

    if (app.ready() && !taskComplete){
        Database.get(aClient, "/"+deviceID, asyncCB, false, "getInitInfo");
        taskComplete = true;
    }

    // Обработка входящих данных от Mega
    if (Serial1.available() > 0)
    {
        char c = (char)Serial1.read();
        // Накопим символы в буфере, пока не встретим '\n'
        if (c == '\n')
        {
            // Удаляем \r и пробелы по краям, если есть
            inputBuffer.trim();

            // Новый префикс вместо [From Mega] BUTTON_PRESSED:
            const String prefixButton = "BUTTON_PRESSED:";
            const String prefixStepCount = "STEP_COUNT:";
            if (inputBuffer.startsWith(prefixButton))
            {
                // Вычленим номер кнопки
                String numStr = inputBuffer.substring(prefixButton.length());
                int buttonIndex = numStr.toInt();
                Serial.printf("Got button index: %d\n", buttonIndex);

                handleButtonClick(buttonIndex);
            }
            else if(inputBuffer.startsWith(prefixStepCount)){
                String stepData = inputBuffer.substring(prefixStepCount.length());
                Serial.println(stepData);
                stepData.remove(0, 3);
                Serial.println(stepData);
                int step = stepData.toInt();
                Serial.printf("Stepps in stepper: %d\n", step);
                priceForLit = prices.price[currentSelect+1];
                Serial.print("PriceForLit: ");
                Serial.println(priceForLit);
                oneTengeIsStep = oneLitForSteps/priceForLit;
                Serial.print("oneTengeIsStep: ");
                Serial.println(oneTengeIsStep);
                priceing = step / oneTengeIsStep;
                Serial.print("priceing: ");
                Serial.println(priceing);
                Serial.println(account);
                int test = bufferStep - priceing;
                if(test > 0){
                  account = bufferStep - priceing;
                  currentVolume = step / 577500.0;
                  Serial.println(currentVolume);
                  double currentVolumeRound = round2(currentVolume);
                  Serial.println(currentVolumeRound);
                  printAccount(String(account));
                  printCurrentVolume(currentVolumeRound);
                }else{
                  printAccount(String(0));
                  Serial.println("Баланс закончился");
                  Serial1.println("STEPPER_STOP");
                  status = ZERO_BALANCE;
                  account = bufferStep;
                }
            }
            else
            {
                // Если строка не соответствует ожидаемому формату
                Serial.print("Unknown command: ");
                Serial.println(inputBuffer);
            }

            // Очистим буфер
            inputBuffer = "";
        }
        else
        {
            // Если это не конец строки, накапливаем символ в буфер
            inputBuffer += c;
        }
    }

}

// ------ Ваши колбэки Firebase ------
void asyncCB(AsyncResult &aResult)
{
    printResult(aResult);
}

void printResult(AsyncResult &aResult)
{
    RealtimeDatabaseResult &RTDB = aResult.to<RealtimeDatabaseResult>();
    if (aResult.isEvent())
    {
        Firebase.printf("Event task: %s, msg: %s, code: %d\n",
                        aResult.uid().c_str(),
                        aResult.appEvent().message().c_str(),
                        aResult.appEvent().code());
    }

    if (aResult.isDebug())
    {
        Firebase.printf("Debug task: %s, msg: %s\n",
                        aResult.uid().c_str(),
                        aResult.debug().c_str());
    }

    if (aResult.isError())
    {
        Firebase.printf("Error task: %s, msg: %s, code: %d\n",
                        aResult.uid().c_str(),
                        aResult.error().message().c_str(),
                        aResult.error().code());
    }

    if (aResult.available())
    {
        Firebase.printf("task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());
        String task = aResult.uid().c_str();
        task.trim();
        if(task == "getInitInfo"){
            getAllNameAndPrice(RTDB.data());
        }
        if(task == "getBalanceTask"){
            String pay = aResult.c_str();
            Serial.printf("Payload string %s, payload int %d", pay, pay.toInt());
            account = pay.toInt();
            if(account > 0){
              Serial.println("account больше нуля");
              status = READY;
              printAccount(String(account));
              lcd.setCursor(0, 1);
              lcd.print("Please select chem");
              lcd.setCursor(0, 2);
              lcd.print("                    ");
            }
        }
    }
}

void wifiConnect()
{
  lcd.setCursor(0, 0);
  lcd.print("Connecting to WiFi");
  lcd.setCursor(0, 1);
  lcd.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int dotCount = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
      lcd.setCursor(0, 2);
      lcd.print("Status: ");
      for (int i = 0; i <= dotCount; i++)
      {
          lcd.print(".");
      }
      dotCount = (dotCount + 1) % 10; 
      delay(500);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected!");
  lcd.setCursor(0, 1);
  lcd.print("IP:");
  lcd.print(WiFi.localIP());
  delay(2000);
  lcd.clear();

  deviceID = WiFi.macAddress();
  deviceID.replace(":", "");
  lcd.setCursor(0, 0);
  lcd.print("Wifi MAC Address");
  lcd.setCursor(0, 1);
  lcd.print(deviceID);
  delay(2000);
  lcd.clear();
  Serial.println(deviceID);
  lcd.print("Balance:");
}

void getAllNameAndPrice(String jsonString){
  FirebaseJson json;
  FirebaseJsonData jsonData;

  // Парсим JSON строку
  json.setJsonData(jsonString);

  json.get(jsonData, "containers/tank1/name"); if (jsonData.success) names.name[1] = jsonData.stringValue;
  json.get(jsonData, "containers/tank2/name"); if (jsonData.success) names.name[2] = jsonData.stringValue;
  json.get(jsonData, "containers/tank3/name"); if (jsonData.success) names.name[3] = jsonData.stringValue;
  json.get(jsonData, "containers/tank4/name"); if (jsonData.success) names.name[4] = jsonData.stringValue;
  json.get(jsonData, "containers/tank5/name"); if (jsonData.success) names.name[5] = jsonData.stringValue;
  json.get(jsonData, "containers/tank6/name"); if (jsonData.success) names.name[6] = jsonData.stringValue;
  json.get(jsonData, "containers/tank7/name"); if (jsonData.success) names.name[7] = jsonData.stringValue;

  // Извлечение prices
  json.get(jsonData, "containers/tank1/price"); if (jsonData.success) prices.price[1] = jsonData.intValue;
  json.get(jsonData, "containers/tank2/price"); if (jsonData.success) prices.price[2] = jsonData.intValue;
  json.get(jsonData, "containers/tank3/price"); if (jsonData.success) prices.price[3] = jsonData.intValue;
  json.get(jsonData, "containers/tank4/price"); if (jsonData.success) prices.price[4] = jsonData.intValue;
  json.get(jsonData, "containers/tank5/price"); if (jsonData.success) prices.price[5] = jsonData.intValue;
  json.get(jsonData, "containers/tank6/price"); if (jsonData.success) prices.price[6] = jsonData.intValue;
  json.get(jsonData, "containers/tank7/price"); if (jsonData.success) prices.price[7] = jsonData.intValue;

  json.get(jsonData, "balance"); if(jsonData.success) printAccount(String(jsonData.intValue));
}

void printAccount(String account1){
  // beepBuzzer();
  lcd.setCursor(0, 0);
  lcd.print("Balance: ");
  lcd.setCursor(8, 0);
  lcd.print("              ");
  lcd.setCursor(8, 0);
  lcd.print(account1);
  account = account1.toInt();
  if(account == 0){
    status =ZERO_BALANCE;
  }
}
void printNameAndPrice(String name, int prices){
  lcd.setCursor(0, 1);
  lcd.print("                    ");
  lcd.setCursor(0, 2);
  lcd.print("                    ");
  lcd.setCursor(0, 3);
  lcd.print("                    ");
  lcd.setCursor(0, 1);
  lcd.print(name);
  lcd.setCursor(0, 2);
  lcd.print("Price:" + String(prices)+" t/l");
}

void handleButtonClick(int index){
  switch (index)
  {
    case BUTTON1:
      Serial.println("Клик на BUTTON1");
      printNameAndPrice(names.name[1], prices.price[1]);
      currentSelect = 0;
      currentVolume = 0;
      break;

    case BUTTON2:
      Serial.println("Клик на BUTTON2");
      printNameAndPrice(names.name[2], prices.price[2]);
      currentSelect = 1;
      currentVolume = 0;
      break;

    case BUTTON3:
      Serial.println("Клик на BUTTON3");
      printNameAndPrice(names.name[3], prices.price[3]);
      currentSelect = 2;
      currentVolume = 0;
      break;

    case BUTTON4:
      Serial.println("Клик на BUTTON4");
      printNameAndPrice(names.name[4], prices.price[4]);
      currentSelect = 3;
      currentVolume = 0;
      break;

    case BUTTON5:
      Serial.println("Клик на BUTTON5");
      printNameAndPrice(names.name[5], prices.price[5]);
      currentSelect = 4;
      currentVolume = 0;
      break;

    case BUTTON6:
      Serial.println("Клик на BUTTON6");
      printNameAndPrice(names.name[6], prices.price[6]);
      currentSelect = 5;
      currentVolume = 0;
      break;

    case BUTTON7:
      Serial.println("Клик на BUTTON7");
      printNameAndPrice(names.name[7], prices.price[7]);
      currentSelect = 6;
      currentVolume = 0;
      break;

    case START_BUTTON:
        // Пример кода для запуска stepper на скорости 30000 STEPPER_START:0,18000,0
        if(status == READY & currentSelect != 10){
          Serial1.println("BUZZER:200");
          Serial.println("Клик на START_BUTTON");
          //Serial1.println("STEPPER_START:" + String(currentSelect)+",13000,0");
          Serial1.println("STEPPER_START:" + String(currentSelect)+",13000,0");
          Serial1.println("RESET_STEP:0");
          status = PROGRESS;
          bufferStep = account;
        }else if (status == ZERO_BALANCE){
          Serial.println("отправка запроса на получение счета");
          Serial1.println("BUZZER:1000");
          Database.get(aClient, "/3C61056557B4/balance", asyncCB, false, "getBalanceTask");
          Database.set<int>(aClient, "/3C61056557B4/balance", 0, asyncCB, "setZeroBalanceTask");
        }
        
        break;

    case STOP_BUTTON:
        // Пример кода для остановки stepper  STEPPER_STOP
        if(status == PROGRESS){
          Serial1.println("BUZZER:100");
          Serial.println("Клик на STOP_BUTTON");
          Serial1.println("STEPPER_STOP");
          Serial1.println("GET_STEP:" + String(currentSelect));
          status = READY;
          account = bufferStep;
        }
        break;

    default:
        Serial.print("Клик на неизвестной кнопке P");
        Serial.println(index);
        break;
    }
}

void printCurrentVolume(double volume1){
  lcd.setCursor(0, 3);
  lcd.print("Current Volume: ");
  lcd.setCursor(16, 3);
  lcd.print(volume1);
}

double round2(double val) {
  return floor(val * 100.0 + 0.5) / 100.0;
}