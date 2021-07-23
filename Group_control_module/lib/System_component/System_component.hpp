#ifndef SYSTEM_COMPONENT_HPP_
#define SYSTEM_COMPONENT_HPP_

#if defined( linux )
#include "../../Arduino.h"
#else // use linux
#if defined ( ESP32 )
#include <Arduino.h>
#endif
#include <Packet_analyzer.h> // [packet_analyzer]
#endif // use linux

#include <array>

namespace scs {

    const uint8_t AMT_BYTES_ID = 12;

    enum class State { // зарегистрирован, в процессе регистрации (setting - МУГ, ЗАРЕГ и В_ПР_РЕГ - S/D)
        work,           // Работает
        stop,           // Остановлен
        setting,        // МУГ или зарегистрирован
        is_registered,  // в процессе регистрации
        not_registered, // не зарегитсрирован
        from_work_to_stop, // (-) ----- после исключу
        from_stop_to_work, // (-) ----- после исключу
        end_
    };
    
    enum class Packet_State { // (-) ----- после исключу, пока касается только сервера
        setting = 0,
        work = 1,
        stop = 2,
        end_
    };

    class System_component {
    protected:
        std::array<uint8_t, AMT_BYTES_ID> system_id{};
        State state_={State::work}; // (-) ----- здесь not_registered (work, чтобы сразу всё работало)
        LoRa_address address_{}; // [packet_analyzer]
    public:
        virtual ~System_component()=default;

        // индивидуальный номер платы
        void set_system_id(std::array<uint8_t, AMT_BYTES_ID> system_id);
        std::array<uint8_t, AMT_BYTES_ID> get_system_id() const;

        // адрес LoRa
        // установка адреса модуля
        bool set_address(LoRa_address address); // [packet_analyzer]
        // получение адреса модуля
        LoRa_address get_address() const; // [packet_analyzer]

        // состояние компонента
        bool set_state___(State);
        State get_state___() const;

        // //
        // size_t get_size();
        // size_t get_data();
        // size_t set_data();
    };
} // scs

#endif // SYSTEM_COMPONENT_HPP_