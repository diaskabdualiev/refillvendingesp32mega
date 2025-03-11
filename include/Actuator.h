// Actuator.h
#ifndef ACTUATOR_H
#define ACTUATOR_H

#include <Arduino.h>
#include "Config.h"

class Actuator {
private:
    int _index;                // Индекс актуатора (0-6)
    int _forwardPin;           // Пин реле для движения вперед
    int _reversePin;           // Пин реле для движения назад
    bool _isDown;              // Флаг: актуатор опущен
    unsigned long _moveStartTime;  // Время начала движения
    unsigned long _moveTimeout;    // Таймаут движения (мс)

public:
    /**
     * Конструктор
     * @param index: Индекс актуатора (0-6)
     * @param forwardPin: Пин реле для движения вперед
     * @param reversePin: Пин реле для движения назад
     */
    Actuator(int index, int forwardPin, int reversePin);
    
    /**
     * Инициализация актуатора
     */
    void init();
    
    /**
     * Опустить актуатор
     * @return true, если операция начата успешно
     */
    bool moveDown();
    
    /**
     * Поднять актуатор
     * @return true, если операция начата успешно
     */
    bool moveUp();
    
    /**
     * Остановить актуатор
     */
    void stop();
    
    /**
     * Проверка, находится ли актуатор в нижнем положении
     * @return true, если актуатор опущен
     */
    bool isDown();
    
    /**
     * Проверка, находится ли актуатор в верхнем положении
     * @return true, если актуатор поднят
     */
    bool isUp();
    
    /**
     * Обновление состояния актуатора
     * Вызывать в основном цикле для контроля таймаутов
     */
    void update();
};

#endif // ACTUATOR_H