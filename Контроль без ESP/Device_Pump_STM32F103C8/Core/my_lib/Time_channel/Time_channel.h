#ifndef __TIME_CHANNEL_H__
#define __TIME_CHANNEL_H__

#include <stm32f1xx_hal.h>
//#include <Check_time/Check_time.h>
#include <Device_relay/Device_relay.h>

//extern RTC_HandleTypeDef hrtc; // стандартный HAL RTC

struct time_channel_t {
	int16_t time_on_signal;  /**< время включенного сигнала */
	int16_t time_off_signal; /**< время выключенного сигнала */
	int32_t time_left;       /**< Осталось до изменения сигнала */
	int32_t time_check;      /**< Время последней проверки */
	struct relay_t* relay;   /**< включаемый модуль */
    bool state;              /**< текущее состояние сигнала */
};

struct time_channel_t time_channel_init(struct relay_t* relay, int16_t time_on_signal, int16_t time_off_signal);

// изменяет состояние если нужно, возвращает факт изменения
bool check_state_time_channel(struct time_channel_t* time_channel);

// ищет время до ближайшего переключения
uint32_t find_nearest_time(struct time_channel_t* time_channels, uint8_t amt_time_channel);

#endif // __TIME_CHANNEL_H__

/*

// проверяет время, 00 - не пересекается, 01 - пересекается с включением, 10 - пересекается с выключением, 11 - и одно, и другое
uint8_t state_change_clock(struct clock_channel_t* clock_channel);

// изменяет состояние если нужно, возвращает факт изменения
bool check_state_by_RTC(struct clock_channel_t* clock_channel);

// ищет ближайший будильник среди массива исключая текущие (+-.deviation_sec сек)
RTC_TimeTypeDef find_alarm_clock(struct clock_channel_t* clock_channels, uint8_t amt_clock_channel);

*/
