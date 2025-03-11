// CommandHandler.h
#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <Arduino.h>
#include "Config.h"
#include "Actuator.h"
#include "Pump.h"

class CommandHandler {
private:
    String _inputBuffer;              // Буфер для приема команд
    bool _commandComplete;            // Флаг завершения команды
    Actuator** _actuators;            // Массив указателей на актуаторы
    Pump** _pumps;                    // Массив указателей на насосы
    int _selectedPump;                // Выбранный насос
    bool _dispensingActive;           // Флаг активного дозирования
    
    /**
     * Обработка команды
     * @param command: Полученная команда
     */
    void processCommand(String command);
    
    /**
     * Запуск процесса дозирования
     * @param motorIndex: Индекс насоса
     * @param speed: Скорость
     * @param direction: Направление (0 - вперед, 1 - назад)
     */
    void startDispensing(int motorIndex, int speed, int direction);
    
    /**
     * Остановка процесса дозирования
     */
    void stopDispensing();
    
    /**
     * Генерация звукового сигнала
     * @param duration: Продолжительность сигнала в мс
     */
    void beep(int duration);
    
    /**
     * Установка светодиодов
     * @param ledValues: Строка с состояниями LED (пример: "1,0,0,0,0,0,0")
     */
    void setLeds(String ledValues);

public:
    /**
     * Конструктор
     * @param actuators: Массив указателей на актуаторы
     * @param pumps: Массив указателей на насосы
     */
    CommandHandler(Actuator** actuators, Pump** pumps);
    
    /**
     * Инициализация обработчика команд
     */
    void init();
    
    /**
     * Чтение команд из Serial
     * Вызывать в основном цикле
     */
    void readCommands();
    
    /**
     * Отправка данных о количестве шагов
     * @param motorIndex: Индекс насоса
     */
    void sendStepCount(int motorIndex);
    
    /**
     * Отправка сообщения о нажатии кнопки
     * @param buttonIndex: Индекс кнопки
     */
    void sendButtonPressed(int buttonIndex);
    
    /**
     * Обновление состояния дозирования
     * Вызывать в основном цикле
     */
    void updateDispensing();
    
    /**
     * Проверка, активно ли дозирование
     * @return true, если идет процесс дозирования
     */
    bool isDispensingActive();
    
    /**
     * Получение индекса выбранного насоса
     * @return Индекс выбранного насоса или -1, если не выбран
     */
    int getSelectedPump();
};

#endif // COMMAND_HANDLER_H