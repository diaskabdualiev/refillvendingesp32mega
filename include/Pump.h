// Pump.h

#ifndef PUMP_H
#define PUMP_H

#include <Arduino.h>
#include "Config.h"

class Pump {
private:
    int _index;                 // Индекс насоса (0-6)
    int _relayPin;              // Пин реле для управления насосом
    bool _isRunning;            // Флаг работы насоса
    int _stepCounter;           // Счетчик шагов (для рассчета объема)
    unsigned long _lastStepTime; // Время последнего обновления шагов

public:
    /**
     * Конструктор
     * @param index: Индекс насоса (0-6)
     * @param relayPin: Пин реле для управления насосом
     */
    Pump(int index, int relayPin);
    
    /**
     * Инициализация насоса
     */
    void init();
    
    /**
     * Запуск насоса
     * @param speed: Скорость насоса (если поддерживается)
     */
    void start(int speed = MAX_SPEED);
    
    /**
     * Остановка насоса
     */
    void stop();
    
    /**
     * Проверка работы насоса
     * @return true, если насос работает
     */
    bool isRunning();
    
    /**
     * Получение текущего счетчика шагов
     * @return Количество шагов
     */
    int getStepCount();
    
    /**
     * Сброс счетчика шагов
     */
    void resetStepCount();
    
    /**
     * Имитация работы насоса с увеличением счетчика шагов
     * В реальном проекте здесь будет считывание с датчика
     */
    void simulateStep();
    
    /**
     * Пересчет шагов в объем (в литрах)
     * @return Объем в литрах
     */
    float stepsToVolume();
    
    /**
     * Обновление состояния насоса
     * Вызывать в основном цикле
     */
    void update();
};

#endif // PUMP_H