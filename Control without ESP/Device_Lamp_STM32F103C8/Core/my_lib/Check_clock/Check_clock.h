#ifndef __CHECK_CLOCK_H__
#define __CHECK_CLOCK_H__

#include <stm32f1xx_hal.h>
#include <stdbool.h>

// (-) ----- Дописать документации кода

/**
  * @brief  Добавление секунд ко времени
  */
void clock_add_second(RTC_TimeTypeDef* corrected_time, int8_t sec);
/**
  * @brief  Проверка меньшего времени
  */
bool clock_comparison(RTC_TimeTypeDef* less, RTC_TimeTypeDef* more);
/**
  * @brief  Проверка нахождения времени в диапазоне
  */
bool clock_range_membership(RTC_TimeTypeDef* checked_time, RTC_TimeTypeDef* lower_limit, RTC_TimeTypeDef* upper_limit);
/**
  * @brief  Проверка ближайшего времени
  * @details Данная функция служит для поиска ближайшего времени
  *         к стартовому, не влючая его.
  * @param[in]  start_time: текущее время
  * @param[in]  near_time: ближнее время
  * @param[in]  distant_time: дальнее время
  * @retval возвращает истину, если near_time ближе к start_time,
  *         чем distant_time по ходу течения времени.
  */
bool search_nearest_clock(RTC_TimeTypeDef* start_time, RTC_TimeTypeDef* near_time, RTC_TimeTypeDef* distant_time);

#endif // __CHECK_CLOCK_H__
