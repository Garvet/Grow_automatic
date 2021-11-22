#ifndef TIME_CONTROL_H_
#define TIME_CONTROL_H_

#include <Arduino.h>
#include <algorithm>
#include <vector>

#define GROW_TIMER_ONE_CHANNEL

// Devices time control
namespace dtc {
    class Grow_timer;

    // Контроль устройства во времени
    class Time_channel {
        uint32_t duration_on{0};  // Длительность положительного сигнала
        uint32_t duration_off{0}; // Длительность отрицательного сигнала

        bool change; // Было изменено состояние любого из верхних полей

        bool state; // Текущее состояние (считанное состояние) (?) -----
    public:
        // Установка длительности сигналов
        void set_duration_on(uint32_t duration);
        void set_duration_off(uint32_t duration);
        // Получение длительности сигналов
        uint32_t get_duration_on() const;
        uint32_t get_duration_off() const;

        // Работа с сигналом, связанным с изменением полей (для передачи на узел)
        bool get_change() const;
        bool reset_change();

        // Сравнение на совподение
        bool operator==(const Time_channel& channel) const;
    };

#if defined(GROW_TIMER_ONE_CHANNEL)
    // Контроль временных диапазонов
    class Grow_timer {
    public:
        // Время
        struct Time {
            uint8_t hour;
            uint8_t min;
            uint8_t sec;
        }; 
    private:
        Time_channel channel{}; // Каналы
        Time start_time{};      // Время включения
        Time end_time{};        // Время выключения

        bool change; // Было изменено состояние любого из верхних полей

        bool state; // Текущее состояние (считанное состояние) (?) -----
    public:
        Grow_timer() = default;
        ~Grow_timer() = default;
        Grow_timer(const Grow_timer& timer) = default;
        Grow_timer(Grow_timer&& timer) = default;
        Grow_timer& operator=(const Grow_timer& timer) = default;
        Grow_timer& operator=(Grow_timer&& timer) = default;

        // Начало периода
        bool set_start_time(Time time);
        bool set_start_hours(uint8_t value);
        bool set_start_minutes(uint8_t value);
        bool set_start_seconds(uint8_t value);
        Time get_start_time() const;
        uint8_t get_start_hours() const;
        uint8_t get_start_minutes() const;
        uint8_t get_start_seconds() const;
        // Конец периода
        bool set_end_time(Time time);
        bool set_end_hours(uint8_t value);
        bool set_end_minutes(uint8_t value);
        bool set_end_seconds(uint8_t value);
        Time get_end_time() const;
        uint8_t get_end_hours() const;
        uint8_t get_end_minutes() const;
        uint8_t get_end_seconds() const;

        // Привязать канал
        void bind_channel(Time_channel value);
        // Отвязать канал
        bool unbind_channel();
        // Получить данные канала
        Time_channel get_channel();
        const Time_channel& get_channel() const;

        // Работа с сигналом, связанным с изменением полей (для передачи на узел)
        bool get_change() const;
        bool get_internal_change() const;
        bool reset_change();
        bool reset_internal_change();


        // (-) ----- (!) ----- \/ \/ \/ КОСТЫЛЬ
        uint16_t send_value{100};

        // Получить флаг изменения установленного значения
        bool set_send_value(uint16_t val);
        uint16_t get_send_value() const;
        void clear_send_value();
        // (-) ----- (!) ----- /\ /\ /\ КОСТЫЛЬ
    };
#endif
#if defined(GROW_TIMER_VECTOR_CHANNEL)
    // Контроль временных диапазонов
    class Grow_timer {
    public:
        // Время
        struct Time {
            uint8_t hour;
            uint8_t min;
            uint8_t sec;
        }; 
    private:
        std::vector<Time_channel> channel{}; // Каналы
        Time start_time{}; // Время включения
        Time end_time{};   // Время выключения

        bool change; // Было изменено состояние любого из верхних полей

        bool state; // Текущее состояние (считанное состояние) (?) -----
    public:
        Grow_timer() = default;
        ~Grow_timer() = default;
        Grow_timer(const Grow_timer& timer) = default;
        Grow_timer(Grow_timer&& timer) = default;

        // Начало периода
        bool set_start_time(Time time);
        bool set_start_hours(uint8_t value);
        bool set_start_minutes(uint8_t value);
        bool set_start_seconds(uint8_t value);
        Time get_start_time() const;
        uint8_t get_start_hours() const;
        uint8_t get_start_minutes() const;
        uint8_t get_start_seconds() const;
        // Конец периода
        bool set_end_time(Time time);
        bool set_end_hours(uint8_t value);
        bool set_end_minutes(uint8_t value);
        bool set_end_seconds(uint8_t value);
        Time get_end_time() const;
        uint8_t get_end_hours() const;
        uint8_t get_end_minutes() const;
        uint8_t get_end_seconds() const;

        // Привязать канал
        bool bind_channel(Time_channel value);
        // Отвязать канал
        bool unbind_channel(Time_channel value);
        bool unbind_channel(uint8_t num);
        // Получить количество каналов
        uint8_t get_amt_channel() const;
        // Получить N-й канал
        const Time_channel* get_channel(uint8_t num) const;
        // Изменить N-й канал
        bool set_channel(Time_channel new_setting, uint8_t num);

        // Работа с сигналом, связанным с изменением полей (для передачи на узел)
        bool get_change() const;
        bool reset_change();
    };
#endif
}

#endif // TIME_CONTROL_H_
