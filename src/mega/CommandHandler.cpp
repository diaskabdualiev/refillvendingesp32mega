#include "CommandHandler.h"
const int BUZZER_PIN = 19;

CommandHandler::CommandHandler(Actuator** actuators, Pump** pumps) {
    _actuators = actuators;
    _pumps = pumps;
    _inputBuffer = "";
    _commandComplete = false;
    _selectedPump = -1;
    _dispensingActive = false;
}

void CommandHandler::init() {
    Serial.println(F("Инициализация обработчика команд..."));
}

void CommandHandler::beep(int duration) {
    // Пример реализации звукового сигнала
    tone(BUZZER_PIN, 1000, duration);
}

void CommandHandler::readCommands() {
    while (Serial.available()) {
        char inChar = (char)Serial.read();
        
        if (inChar == '\n') {
            _commandComplete = true;
            break;
        }
        
        _inputBuffer += inChar;
    }
    
    if (_commandComplete) {
        processCommand(_inputBuffer);
        _inputBuffer = "";
        _commandComplete = false;
    }
}

void CommandHandler::processCommand(String command) {
    Serial.print(F("Получена команда: "));
    Serial.println(command);
    
    // Запуск шагового двигателя: STEPPER_START:motor_index,speed,direction
    if (command.startsWith(CMD_STEPPER_START)) {
        String params = command.substring(strlen(CMD_STEPPER_START));
        int firstComma = params.indexOf(',');
        int secondComma = params.indexOf(',', firstComma + 1);
        
        if (firstComma > 0 && secondComma > firstComma) {
            int motorIndex = params.substring(0, firstComma).toInt();
            int speed = params.substring(firstComma + 1, secondComma).toInt();
            int direction = params.substring(secondComma + 1).toInt();
            
            startDispensing(motorIndex, speed, direction);
        }
    }
    
    // Остановка шагового двигателя: STEPPER_STOP
    else if (command.equals(CMD_STEPPER_STOP)) {
        stopDispensing();
    }
    
    // Звуковой сигнал: BUZZER:duration
    else if (command.startsWith(CMD_BUZZER)) {
        int duration = command.substring(strlen(CMD_BUZZER)).toInt();
        beep(duration);
    }
    
    // Запрос о количестве шагов: GET_STEP:motor_index
    else if (command.startsWith(CMD_GET_STEP)) {
        int motorIndex = command.substring(strlen(CMD_GET_STEP)).toInt();
        if (motorIndex >= 0 && motorIndex < NUM_PUMPS) {
            sendStepCount(motorIndex);
        }
    }
    
    // Сбросить счётчик шагов: RESET_STEP:motor_index
    else if (command.startsWith(CMD_RESET_STEP)) {
        int motorIndex = command.substring(strlen(CMD_RESET_STEP)).toInt();
        if (motorIndex >= 0 && motorIndex < NUM_PUMPS) {
            _pumps[motorIndex]->resetStepCount();
        }
    }
    
    // Установка светодиодов: SET_LEDS:1,0,0,0,0,0,0
    else if (command.startsWith(CMD_SET_LEDS)) {
        String ledValues = command.substring(strlen(CMD_SET_LEDS));
        setLeds(ledValues);
    }
}

void CommandHandler::startDispensing(int motorIndex, int speed, int direction) {
    if (motorIndex >= 0 && motorIndex < NUM_PUMPS) {
        // Остановить предыдущий процесс
        if (_dispensingActive) {
            stopDispensing();
        }
        
        _selectedPump = motorIndex;
        _dispensingActive = true;
        
        // Опускаем актуатор (если существует)
        if (_actuators[motorIndex] != nullptr) {
            _actuators[motorIndex]->moveDown();
            // Ждем, пока актуатор опустится (можно реализовать асинхронно)
            delay(3000);
        }
        
        // Запускаем насос
        _pumps[motorIndex]->start(speed);
        
        Serial.print(F("Запущен процесс дозирования для насоса #"));
        Serial.println(motorIndex + 1);
    }
}

void CommandHandler::stopDispensing() {
    if (_dispensingActive && _selectedPump >= 0) {
        // Останавливаем насос
        _pumps[_selectedPump]->stop();
        
        // Поднимаем актуатор (если существует)
        if (_actuators[_selectedPump] != nullptr) {
            _actuators[_selectedPump]->moveUp();
        }
        
        Serial.print(F("Остановлен процесс дозирования для насоса #"));
        Serial.println(_selectedPump + 1);
        
        _dispensingActive = false;
        // Не сбрасываем _selectedPump, чтобы можно было получить индекс после остановки
    }
}


void CommandHandler::setLeds(String ledValues) {
    // Пример реализации, если у вас есть светодиоды для индикации
    Serial.print(F("Установка светодиодов: "));
    Serial.println(ledValues);
    
    // Здесь можно добавить код для установки состояния светодиодов
}

void CommandHandler::sendStepCount(int motorIndex) {
    if (motorIndex >= 0 && motorIndex < NUM_PUMPS) {
        int steps = _pumps[motorIndex]->getStepCount();
        
        // Отправляем ответ на ESP32
        Serial.print(CMD_STEP_COUNT);
        Serial.print("P");
        Serial.print(motorIndex);
        Serial.println(steps);
    }
}

void CommandHandler::sendButtonPressed(int buttonIndex) {
    Serial.print(CMD_BUTTON_PRESSED);
    Serial.println(buttonIndex);
}

void CommandHandler::updateDispensing() {
    if (_dispensingActive && _selectedPump >= 0) {
        // Можно добавить дополнительную логику для мониторинга процесса
    }
}

bool CommandHandler::isDispensingActive() {
    return _dispensingActive;
}

int CommandHandler::getSelectedPump() {
    return _selectedPump;
}