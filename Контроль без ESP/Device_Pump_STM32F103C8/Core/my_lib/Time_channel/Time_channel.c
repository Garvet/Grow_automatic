#include <Time_channel/Time_channel.h>



struct time_channel_t time_channel_init(struct relay_t* relay, int16_t time_on_signal, int16_t time_off_signal) {
	struct time_channel_t time_channel;
	time_channel.time_on_signal = time_on_signal;
	time_channel.time_off_signal = time_off_signal;
	time_channel.time_left = time_off_signal * 1000;
	time_channel.time_check = HAL_GetTick();
	time_channel.relay = relay;
	time_channel.state = false;
	return time_channel;
}

// изменяет состояние если нужно, возвращает факт изменения
bool check_state_time_channel(struct time_channel_t* time_channel) {
	bool state_changed = false;
	uint32_t last_check = time_channel->time_check;
	time_channel->time_check = HAL_GetTick();
	uint32_t time_passed = time_channel->time_check - last_check;
//	time_channel->time_check += time_passed;
	if(time_channel->time_left <= time_passed) {
		state_changed = true;
	}
	while(time_channel->time_left <= time_passed) {
		time_passed -= time_channel->time_left;
		if(time_channel->state) {
			time_channel->state = false;
			time_channel->time_left = time_channel->time_off_signal * 1000;
		}
		else {
			time_channel->state = true;
			time_channel->time_left = time_channel->time_on_signal * 1000;
		}
	}
	time_channel->time_left -= time_passed;
	if((time_channel->relay != NULL) && state_changed)
		relay_set_state(time_channel->relay, time_channel->state);
	return state_changed;
}

// ищет время до ближайшего переключения
uint32_t find_nearest_time(struct time_channel_t* time_channels, uint8_t amt_time_channel) {
	uint32_t time_msec = -1;
	for(int i = 0; i < amt_time_channel; ++i)
		if(time_channels[i].time_left < time_msec)
			time_msec = time_channels[i].time_left;
	return time_msec;
}
