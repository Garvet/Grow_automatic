//
// Created by garvet on 07.09.2021.
//
//
//

#ifndef UART_CONTROLLER_HPP_
#define UART_CONTROLLER_HPP_

#if defined(linux)
#include "iostream"
#include "../hal.hpp"
#elif defined(ESP32)
#include <Arduino.h>
#else
#include <main.h>
#include <usart.h>
#endif

// Data transfer protocol
namespace dtp {

    const uint8_t BUFFER_SIZE = 50;

    enum class Stage {
        No_transmission=0,            // Нет передачи
        // Принимающий модуль
        Received_initialization_byte, // Принят инициализирующий байт
        Received_length_byte,         // Принят байт длины
        Receive_bytes,                // Приём байт
        Expect_trailing_byte,         // Ожидается завершающий байт
        // Отправляющий модуль
        Sent_initialization_byte,     // Отправлен инициализирующий байт
        Sent_length_byte,             // Отправлен байт длины
        Sending_bytes,                // Отправка байт
        Sent_trailing_byte,           // Отправлен завершающий байт
        // Количество компонентов
        AMT_COMPONENTS
    };

    enum class Status {
        Ok=0,      // Передача/приём завершена и обработана
        Exit,      // Передача/приём завершена и не обработана
        Error,     // Передача/приём завершена с ошибкой
        Postponed, // Передача ожидает завершения приёма
        // Количество компонентов
        AMT_COMPONENTS
    };

    class UART_controller {
        uint8_t send_buffer[BUFFER_SIZE]{};    // буффер отправки
        uint8_t send_len{0};      // отправляемая длина
        uint8_t send_cnt{0};      // отправленная количество
        uint8_t last_send_byte{}; // последний отправленный байт
        Status send_status{Status::Ok};        // статус отправки
        uint8_t receive_buffer[BUFFER_SIZE]{}; // буффер приёма
        uint8_t receive_len{0};      // ожидаемая длина
        uint8_t receive_cnt{0};      // принятое количество
    public:
        uint8_t last_receive_byte{}; // последний принятый байт
    private:
        Status receive_status{Status::Ok};     // статус приёма
        Stage stage{Stage::No_transmission};   // текущая стадия

        void (*uart_send)(uint8_t &send_byte);
        void (*uart_receive)(uint8_t &receive_byte);
    public:
        UART_controller(void (*uart_send)(uint8_t &send_byte), void (*uart_receive)(uint8_t &receive_byte));

        void begin();

        Status get_send_status() const;
        Status get_receive_status() const;
        Stage get_stage() const;

        bool send_data(const uint8_t *buffer, uint8_t len);
        uint8_t receive_data(uint8_t *buffer, uint8_t len);

        void end_send();
        void end_receive();
        void receive_timeout();

        Status clear_send_status();
        Status clear_receive_status();
        void erase();
    };
}

#endif // UART_CONTROLLER_HPP_
