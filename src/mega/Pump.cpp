#include "Pump.h"

Pump::Pump(int index, int relayPin) {
    _index = index;
    _relayPin = relayPin;
    _isRunning = false;
    _stepCounter = 0;
    _lastStepTime = 0;
}

void Pump::init() {
    // Настраиваем пин как выход
    pinMode(_relayPin, OUTPUT);
    
    // Изначально реле выключено (HIGH для реле = выключено)
    digitalWrite(_relayPin, HIGH);
    
    Serial.print(F("Насос #"));
    Serial.print(_index + 1);
    Serial.println(F(" инициализирован"));
}

void Pump::start(int speed) {
    // Активируем реле, чтобы включить насос
    // LOW для реле = включено (замыкает цепь)
    digitalWrite(_relayPin, LOW);
    _isRunning = true;
    
    // В реальном проекте здесь будет настройка скорости, если используется ШИМ или шаговый драйвер
    // Например: analogWrite(_pwmPin, speed);
    
    Serial.print(F("Насос #"));
    Serial.print(_index + 1);
    Serial.print(F(" запущен со скоростью "));
    Serial.println(speed);
}

void Pump::stop() {
    // Выключаем реле, чтобы остановить насос
    digitalWrite(_relayPin, HIGH);
    _isRunning = false;
    
    Serial.print(F("Насос #"));
    Serial.print(_index + 1);
    Serial.println(F(" остановлен"));
}

bool Pump::isRunning() {
    return _isRunning;
}

int Pump::getStepCount() {
    return _stepCounter;
}

void Pump::resetStepCount() {
    _stepCounter = 0;
    Serial.print(F("Счетчик шагов насоса #"));
    Serial.print(_index + 1);
    Serial.println(F(" сброшен"));
}

void Pump::simulateStep() {
    // Эта функция имитирует работу насоса, увеличивая счетчик шагов
    // В реальном проекте здесь будет считывание с датчика расхода
    
    // Увеличиваем счетчик только если насос работает
    if (_isRunning) {
        _stepCounter++;
    }
}

float Pump::stepsToVolume() {
    // Пересчет шагов в объем (в литрах)
    return (float)_stepCounter / STEPS_PER_LITER;
}

void Pump::update() {
    // Обновление состояния насоса
    // Вызывать в основном цикле
    
    // Если насос работает, имитируем шаги для демо
    if (_isRunning && (millis() - _lastStepTime > 10)) { // каждые 10 мс
        simulateStep();
        _lastStepTime = millis();
        
        // Выводим информацию каждые 1000 шагов
        if (_stepCounter % 1000 == 0) {
            Serial.print(F("Насос #"));
            Serial.print(_index + 1);
            Serial.print(F(" шаги: "));
            Serial.print(_stepCounter);
            Serial.print(F(", объем: "));
            Serial.print(stepsToVolume(), 3);
            Serial.println(F(" л"));
        }
    }
}