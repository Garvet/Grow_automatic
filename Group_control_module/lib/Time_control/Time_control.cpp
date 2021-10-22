#include "Time_control.h"


// Devices time control
namespace dtc {
    // ----- Контроль устройства во времени -----

    // Установка длительности сигналов
    void Time_channel::set_duration_on(uint32_t duration) {
        duration_on = duration;
        change = true;
    }
    void Time_channel::set_duration_off(uint32_t duration) {
        duration_off = duration;
        change = true;
    }
    // Получение длительности сигналов
    uint32_t Time_channel::get_duration_on() const {
        return duration_on;
    }
    uint32_t Time_channel::get_duration_off() const {
        return duration_off;
    }
    // Работа с сигналом, связанным с изменением полей (для передачи на узел)
    bool Time_channel::get_change() const {
        return change;
    }
    bool Time_channel::reset_change() {
        if(!change)
            return false;
        change = false;
        return true;
    }

    bool Time_channel::operator==(const Time_channel& channel) const{
        return (duration_on == channel.duration_on ||
            duration_off == channel.duration_off ||
            change == channel.change ||
            state == channel.state);
    }
    


    // ----- Контроль временных диапазонов -----
    bool Grow_timer::set_start_time(Time time) {
        Time old_time = start_time;
        if(set_start_hours(time.hour) ||
           set_start_minutes(time.min) ||
           set_start_seconds(time.sec)) {
            start_time = old_time;
            return true;
        }
        return false;
    }
    bool Grow_timer::set_start_hours(uint8_t value) {
        if(23 < value)
            return true;
        start_time.hour = value;
        return false;
    }
    bool Grow_timer::set_start_minutes(uint8_t value) {
        if(59 < value)
            return true;
        start_time.min = value;
        return false;
    }
    bool Grow_timer::set_start_seconds(uint8_t value) {
        if(59 < value)
            return true;
        start_time.sec = value;
        return false;
    }
    Grow_timer::Time Grow_timer::get_start_time() const {
        return start_time;
    }
    uint8_t Grow_timer::get_start_hours() const {
        return start_time.hour;
    }
    uint8_t Grow_timer::get_start_minutes() const {
        return start_time.min;
    }
    uint8_t Grow_timer::get_start_seconds() const {
        return start_time.sec;
    }
    // Конец периода
    bool Grow_timer::set_end_time(Time time) {
        Time old_time = end_time;
        if(set_end_hours(time.hour) ||
           set_end_minutes(time.min) ||
           set_end_seconds(time.sec)) {
            end_time = old_time;
            return true;
        }
        return false;
    }
    bool Grow_timer::set_end_hours(uint8_t value) {
        if(23 < value)
            return true;
        end_time.hour = value;
        return false;
    }
    bool Grow_timer::set_end_minutes(uint8_t value) {
        if(59 < value)
            return true;
        end_time.min = value;
        return false;
    }
    bool Grow_timer::set_end_seconds(uint8_t value) {
        if(59 < value)
            return true;
        end_time.sec = value;
        return false;
    }
    Grow_timer::Time Grow_timer::get_end_time() const {
        return end_time;
    }
    uint8_t Grow_timer::get_end_hours() const {
        return end_time.hour;
    }
    uint8_t Grow_timer::get_end_minutes() const {
        return end_time.min;
    }
    uint8_t Grow_timer::get_end_seconds() const {
        return end_time.sec;
    }

    // Привязать канал
    bool Grow_timer::bind_channel(Time_channel value) {
        channel.push_back(value);
        return false;
    }
    // Отвязать канал
    bool Grow_timer::unbind_channel(Time_channel value) {
        auto it = std::find_if(channel.begin(), channel.end(),
            [value] (const Time_channel& check) {if(check == value) return true; return false;});
        if (it == channel.end())
            return true;
        channel.erase(it);
        return false;
    }
    bool Grow_timer::unbind_channel(uint8_t num) {
        if(channel.size() <= num)
            return true;
        channel.erase(channel.begin() + num);
        return false;
    }
    // Получить количество каналов
    uint8_t Grow_timer::get_amt_channel() const {
        return channel.size();
    }
    // Получить N-й канал
    const Time_channel* Grow_timer::get_channel(uint8_t num) const {
        if(channel.size() <= num)
            return nullptr;
        return &channel[num];
    }
    // Изменить N-й канал
    bool Grow_timer::set_channel(Time_channel new_setting, uint8_t num) {
        if(channel.size() <= num)
            return true;
        channel[num] = new_setting;
        return false;
    }

    // Работа с сигналом, связанным с изменением полей (для передачи на узел)
    bool Grow_timer::get_change() const {
        return change;
    }
    bool Grow_timer::reset_change() {
        if(!change)
            return false;
        change = false;
        return true;
    }
}
