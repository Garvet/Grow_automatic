#include "Period_adjuster.h"

void Period_adjuster::border_recount() {
    if(number > period) {
        number = number % period;
        if(number == 0)
            number = period;
    }
    dawn = dawn % period;
}

Period_adjuster::Period_adjuster(uint8_t number, uint8_t dawn, uint8_t max_period, uint8_t period): max_period(max_period) {
    set_period(period);
    set_number(number);
    set_dawn(dawn);
}


// уменьшение длительности периода на 1 (до 0)
uint8_t Period_adjuster::decrease_number() {
    if (number > 1)
        number -= 1;
    else
        number = 0;
    return number;
}
// увеличение длительности периода на 1 (до периода)
uint8_t Period_adjuster::increase_number() {
    if (number < (period - 1))
        number += 1;
    else
        number = period;
    return number;
}
// установка длительности, на N-е значение (mod периода)
uint8_t Period_adjuster::set_number(uint8_t new_number) {
    if (new_number > period)
        number = period;
    else
        number = new_number;
    return number;
}

// смещение начала периода, на 1 раньше (mod периода)
uint8_t Period_adjuster::dawn_before() {
    if (dawn == 0)
        dawn = period - 1;
    else
        --dawn;
    return dawn;
}
// смещение начала периода, на 1 позже (mod периода)
uint8_t Period_adjuster::dawn_later() {
    ++dawn;
    dawn %= period;
    return dawn;
}
// смещение начала периода, на N-е значение (mod периода)
uint8_t Period_adjuster::set_dawn(uint8_t new_dawn) {
    return dawn = new_dawn % period;
}

// уменьшение периода
uint8_t Period_adjuster::decrease_period() {
    uint8_t coefficient = (uint8_t)(max_period / period);
    while(true) {
        if (coefficient != max_period)
            ++coefficient;
        else
            break;
        if (max_period % coefficient == 0)
            break;
    }
    period = max_period / coefficient;
    border_recount();
    return  period;
}
// увеличение периода
uint8_t Period_adjuster::increase_period() {
    uint8_t coefficient = (uint8_t)(max_period / period);
    while(true) {
        if(coefficient != 1)
            --coefficient;
        else
            break;
        if (max_period % coefficient == 0)
            break;
    }
    period = max_period / coefficient;
    border_recount();
    return  period;
}
// присвоение периоду N-о значения (делитель max_period)
uint8_t Period_adjuster::set_period(uint8_t new_period) {
    if (new_period > max_period)
        period = max_period;
    else {
        period = new_period;
        if (max_period % period != 0)
            increase_period();
    }
    border_recount();
    return  period;
}

// получение начала диапазона
uint8_t Period_adjuster::get_dawn() {
    return dawn;
}
// получение конца диапазона
uint8_t Period_adjuster::get_sunset() {
    return ((dawn + number) % period);
}
// получение длительности диапазона
uint8_t Period_adjuster::get_number() {
    return number;
}
// получение длительности вне диапазона
uint8_t Period_adjuster::get_number_out_range() {
    return (period - number);
}
// получение длительности периода
uint8_t Period_adjuster::get_period_range() {
    return period;
}
// получение максимальной длительности периода
uint8_t Period_adjuster::get_max_period_range() {
    return max_period;
}

// получение значения вне диапазона / в диапазоне
// последнее значение
bool Period_adjuster::check_work() {
    return signal;
}
// расчёт нового значения по времени
bool Period_adjuster::check_work(uint8_t time_value) {
    uint8_t sunset = get_sunset();
    uint8_t value = time_value % period;
    bool new_signal = false;
    if (dawn < sunset)
        new_signal = ((dawn <= value) && (value < sunset));
    else if (dawn > sunset)
        new_signal = ((value < sunset) || (dawn <= value));
    else if (dawn == sunset)
        new_signal = (number == period);

    if (new_signal != signal) {
        if (signal)
            last_state = -1;
        else
            last_state = 1;
        if (state == 0)
            state = last_state;
    }
    else
        last_state = 0;
    
    signal = new_signal;
    return signal;
}
// изменение значения относительно последнего считывания
int8_t Period_adjuster::last_state_change() {
    return last_state;
}
// флаг изменения значения
int8_t Period_adjuster::state_change(bool clear) {
    int8_t ret_state = state;
    if (clear)
        clear_state_change();
    return ret_state;
}
// обнулить изменение значения
void Period_adjuster::clear_state_change() {
    state = 0;
}