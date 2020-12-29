#ifndef __TIME_CONTROL_H__
#define __TIME_CONTROL_H__

#include <Arduino.h>

// Список всех типов временного контроля
enum Time_type {
    SEC = 0,
    MIN,
    HOUR
};

enum class Type_time_control {
    TIMER_CHANNEL=0, 
    GROW_TIMER
};

class Time_control {
private:
    // Варианты временного контроля:
    // 1) Зависимость от вресмени типа ЧЧ:ММ:СС, задаётся 4 параметрами (уровень [ч, м или с], частота, длительность, смещение [время начала])
    // 2) Зависимость от длительности, задаётся 2 параметрами (время включённого состояние и время выключенного состояния)
public:
    Time_control(/* args */);
    ~Time_control();
    
    
};


#endif // __TIME_CONTROL_H__