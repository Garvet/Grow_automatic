#ifndef __GROW_DEVICE_H__
#define __GROW_DEVICE_H__

#define SERIAL_LOG_OUTPUT

#include <Arduino.h>
#include <vector>
#include <Grow_device_component.h>

#include <Wire.h>
#include <RtcDateTime.h>

// (!) ----- (-) ----- перенести на статическую память
// (-) ----- перенести работу модуля на удалёнку и добавить период опроса

class Grow_device {
private:
    // (-) ----- период добавь
    uint32_t system_id_;
    uint16_t address_; // адрес устройства
    std::vector<Grow_device_component> component_; // вектор компонентов устройств (не как плата, а как механический модуль)

    unsigned long read_time_; // время последнего считывания
    unsigned long end_time_; // время последней проверки для считывания
    unsigned long period_; // период считывания в мс
    bool readout_signal_; // флаг готовности считывания

    uint8_t active_;  // состояние активации 0 - не активен, 1 - в процессе регистрации, 2 - зарегистрирован // (-) ----- обнулить значения при false запретить функции обработки
    bool change_value_; // флаг изменённого значения

    uint8_t setting_; // настройски (для LoRa-протокола)
public:
    Grow_device() = default;
    Grow_device(uint8_t amt_component, enum Type_device* type_sensor);
    Grow_device(uint8_t amt_component, uint8_t* type_sensor);
    Grow_device(std::vector<enum Type_device> type_device);
    ~Grow_device() = default;

    // установка индивидуального номера платы
    void set_system_id(uint32_t system_id_);
    // получение индивидуального номера платы
    uint32_t get_system_id();

    // установка состояния активности
    void set_active(uint8_t active);
    // получение состояния активности
    uint8_t get_active(); 

    // получение флага об изменённом состоянии)
    bool get_change_value();
    // сброс флага изменённого состояния
    void clear_change_value(); 

    // установка адреса модуля (ветви, без группы)
    bool set_address(uint16_t address); 
    // получение адреса модуля (ветви, без группы)
    uint16_t get_address(); 

    // Установка настроек LoRa-передачи
    void set_setting(uint8_t setting); 
    // Получение настроек LoRa-передачи
    uint8_t get_setting(); 

    // получение типа компонента (передаётся в result), если ошибка возврат true
    bool get_type(uint8_t num, enum Type_device &result); 
    // получение вектора типов компонентов
    std::vector<enum Type_device> get_type();
    // получение id компонента (передаётся в result | не путать с ID платы, этот номер для количества повторений), если ошибка возврат true
    bool get_id(uint8_t num, uint8_t &result); 
    // получение вектора id компонентов
    std::vector<uint8_t> get_id();

    // Установить период опроса модуля
    void set_period(unsigned long period);
    // Получить период опроса модуля
    unsigned long get_period();  

    // Проверка необходимости считывания 
    bool check_time(unsigned long time);
    // Обновление времени считывания
    void update();
    // Получение сигнала готовности к считыванию
    bool read_signal(bool clear=false);

    // Установить значение считанного состояния, если ошибка возврат true
    bool set_signal(uint8_t num, float value);
    // Получить значение считанного состояния, если ошибка возврат true
    bool get_signal(uint8_t num, float &result);
    // Установить значения считанных состояний, если ошибка возврат true
    bool set_signal(std::vector<float> value);
    // Получить вектор значений считанных состояний
    std::vector<float> get_signal();

    // Установить значение считанного показателя, если ошибка возврат true
    bool set_value(uint8_t num, float value);
    // Получить значение считанного показателя, если ошибка возврат true
    bool get_value(uint8_t num, float &result);
    // Установить значения считанных показателей, если ошибка возврат true
    bool set_value(std::vector<float> value);
    // Получить вектор значений считанных показателей
    std::vector<float> get_value();








    // (-) ----- Period_adjuster -> (Time_Channel | Grow_Timer), (-) ----- исключить Period_type 
    // настройка компонента, если ошибка возврат true 
    bool set_channel_setting(uint8_t num, Period_adjuster period, enum Period_type period_type); 
    // настройка всех компонентов, если ошибка возврат true
    bool set_channel_setting(std::vector<Period_adjuster> period, std::vector<enum Period_type> period_type);
    
    // получение настройки типа времени компонента (передаётся в result), если ошибка возврат true
    bool get_period_type(uint8_t num, enum Period_type &result); 
    // получение настройки работы всех компонентов
    std::vector<Period_adjuster> get_channel_setting();

    // получение настройки типов времени всех компонентов
    std::vector<enum Period_type> get_period_type(); // (-)












    // получение настройки работы компонента (передаётся в result), если ошибка возврат true
    bool get_period(uint8_t num, Period_adjuster &result); 

    /// Проверка времени 
    // Без обновления, если 0, то все устройства в тех же состояниях
    int8_t check_time(bool clear = false);
    // С обновлением в единицах (ч, м, или с, переданых из вне, м.б. некорректно, если компоненты отталкиваются от разных единиц), если 0, то все устройства в тех же состояниях
    int8_t check_time(uint8_t time, bool clear = false);
    // С обновлением по значению времени, если 0, то все устройства в тех же состояниях
    int8_t check_time(RtcDateTime date_time, bool clear = false);

    // Установить значение сигнала в рабочем состоянии для определённого компонента, если ошибка возврат true
    bool set_value(uint8_t num, uint16_t value);

    // Получить значение сигнала определённого компонента, если ошибка возврат true
    bool get_value(uint8_t num, uint16_t &result);
    // Получить наличие сигнала хоть у одного компонента
    bool get_signal();
    // Получить наличие сигнала у определённого компонента, если ошибка false (т.е. нет сигнала)
    bool get_signal(uint8_t num);
    //  Получить наличие изменения сигнала хоть у одного компонента
    bool get_state_change();
    // Получить изменения сигнала у определённого компонента, если ошибка возврат true
    bool get_state_change(uint8_t num, int8_t &result);











    // Получить количество компонентов
    uint8_t get_count_component();

    // Получить определённый компонент
    Grow_device_component get_component(uint8_t num);
    // Получить вектор компонентов
    std::vector<Grow_device_component> get_component();

    // Проверить совподает ли содержимое модулей, без учёта настроек (для отфильтровывания среди неподходящих)
    bool filter(Grow_device &device); // (-) ----- убрать привязку к порядку

    /// Функции превращения в байтовую строку (-) ----- доработать под код Дениса (-) ----- добавить генерацию пакетов (возможно другой класс)
    // Получение размера строки
    size_t get_size();
    // Заполение массива байтов, возврат количество байт (должен совпадать с размером строки)
    size_t get_data(uint8_t *data);
    // Заполнение объекта класса по байтовой строке, возврат количество использованных байт
    size_t set_data(uint8_t *data, size_t available_size);

#if defined (SERIAL_LOG_OUTPUT)
    /// вывод класса в Serial
    void print();
#endif
};


#endif // __GROW_DEVICE_H__