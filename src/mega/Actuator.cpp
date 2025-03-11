#include "Actuator.h"

Actuator::Actuator(int index, int forwardPin, int reversePin) {
    _index = index;
    _forwardPin = forwardPin;
    _reversePin = reversePin;
    _isDown = false;
    _moveStartTime = 0;
    _moveTimeout = 10000; // 10 секунд таймаут по умолчанию
}

void Actuator::init() {
    // Настраиваем пины как выходы
    pinMode(_forwardPin, OUTPUT);
    pinMode(_reversePin, OUTPUT);
    
    // Изначально все реле выключены (HIGH для реле = выключено, т.к. активация LOW)
    digitalWrite(_forwardPin, HIGH);
    digitalWrite(_reversePin, HIGH);
    
    Serial.print(F("Актуатор #"));
    Serial.print(_index + 1);
    Serial.println(F(" инициализирован"));
}

bool Actuator::moveDown() {
    // Сначала останавливаем актуатор для безопасности
    stop();
    delay(100); // Небольшая пауза
    
    // Для движения вниз активируем реле направления "вперед"
    // LOW для реле = включено (замыкает цепь)
    digitalWrite(_forwardPin, LOW);
    digitalWrite(_reversePin, HIGH);
    
    _isDown = true;
    _moveStartTime = millis();
    
    Serial.print(F("Актуатор #"));
    Serial.print(_index + 1);
    Serial.println(F(" движется вниз"));
    
    return true;
}

bool Actuator::moveUp() {
    // Сначала останавливаем актуатор для безопасности
    stop();
    delay(100); // Небольшая пауза
    
    // Для движения вверх активируем реле направления "назад"
    // LOW для реле = включено (замыкает цепь)
    digitalWrite(_forwardPin, HIGH);
    digitalWrite(_reversePin, LOW);
    
    _isDown = false;
    _moveStartTime = millis();
    
    Serial.print(F("Актуатор #"));
    Serial.print(_index + 1);
    Serial.println(F(" движется вверх"));
    
    return true;
}

void Actuator::stop() {
    // Выключаем оба реле для остановки актуатора
    digitalWrite(_forwardPin, HIGH);
    digitalWrite(_reversePin, HIGH);
    
    Serial.print(F("Актуатор #"));
    Serial.print(_index + 1);
    Serial.println(F(" остановлен"));
}

bool Actuator::isDown() {
    return _isDown;
}

bool Actuator::isUp() {
    return !_isDown;
}

void Actuator::update() {
    // Проверяем таймаут движения
    if (_moveStartTime > 0 && (millis() - _moveStartTime > _moveTimeout)) {
        // Если превышен таймаут, останавливаем актуатор
        stop();
        _moveStartTime = 0;
        
        Serial.print(F("Актуатор #"));
        Serial.print(_index + 1);
        Serial.println(F(" остановлен по таймауту"));
    }
    
    // Здесь также можно добавить проверку концевых выключателей, 
    // если они есть в вашей системе
}