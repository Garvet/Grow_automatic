#ifndef __PERIOD_ADJUSTER_H__
#define __PERIOD_ADJUSTER_H__

#include <Arduino.h>
// #include <RtcDS3231.h>

class Period_adjuster {
    bool signal = false;    // false - вне диапазона, true - в диапазоне
    uint8_t number = 0;     // длительность диапазона
    uint8_t dawn = 0;       // время налача диапазона
    uint8_t max_period = 0; // максимальное значение периода
    uint8_t period = 0;     // период adjustable period

    int8_t last_state = 0;
    int8_t state = 0;
    // Пересчитывает значения для длительности и положения диапазона при изменении периода
    void border_recount();
public:
    Period_adjuster() = default;
    Period_adjuster(uint8_t number, uint8_t dawn, uint8_t max_period, uint8_t period);
    ~Period_adjuster() = default;

    // уменьшение длительности периода на 1 (до 0)
    uint8_t decrease_number();
    // увеличение длительности периода на 1 (до периода)
    uint8_t increase_number();
    // установка длительности, на N-е значение (mod периода)
    uint8_t set_number(uint8_t new_number);

    // смещение начала периода, на 1 раньше (mod периода)
    uint8_t dawn_before();
    // смещение начала периода, на 1 позже (mod периода)
    uint8_t dawn_later();
    // смещение начала периода, на N-е значение (mod периода)
    uint8_t set_dawn(uint8_t new_dawn);

    // уменьшение периода
    uint8_t decrease_period();
    // увеличение периода
    uint8_t increase_period();
    // присвоение периоду N-о значения (делитель max_period)
    uint8_t set_period(uint8_t new_period);

    // получение начала диапазона
    uint8_t get_dawn();
    // получение конца диапазона
    uint8_t get_sunset();
    // получение длительности диапазона
    uint8_t get_number();
    // получение длительности вне диапазона
    uint8_t get_number_out_range();
    // получение длительности периода
    uint8_t get_period_range();
    // получение максимальной длительности периода
    uint8_t get_max_period_range();

    // получение значения вне диапазона / в диапазоне
    bool check_work(); // последнее значение
    bool check_work(uint8_t time_value); // расчёт нового значения по времени
    int8_t last_state_change(); // изменение значения относительно последнего считывания
    int8_t state_change(bool clear = false); // флаг изменения значения
    void clear_state_change(); // обнулить изменение значения
} typedef Period_adjuster;


#endif // __PERIOD_ADJUSTER_H__