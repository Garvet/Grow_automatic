//
// Created by garvet on 07.09.2021.
//

#include "UART_controller.hpp"

namespace dtp {
    UART_controller::UART_controller(void (*uart_send)(uint8_t&),
                                     void (*uart_receive)(uint8_t&)):
            uart_send(uart_send), uart_receive(uart_receive) {
    }

    void UART_controller::begin() {
        erase();
        uart_receive(last_receive_byte);
    }

    Status UART_controller::get_send_status() const {
        return send_status;
    }

    Status UART_controller::get_receive_status() const {
        return receive_status;
    }

    Stage UART_controller::get_stage() const {
        return stage;
    }

    bool UART_controller::send_data(const uint8_t *buffer, uint8_t len) {
        if ((stage != Stage::No_transmission) || (BUFFER_SIZE < len))
            return true;
        send_cnt = 0;
        send_len = len;
        send_status = Status::Ok;
        stage = Stage::Sent_initialization_byte;
        for(int i = 0; i < send_len; ++i) {
            send_buffer[i] = buffer[i];
        }
        last_send_byte = 0xA5;
        uart_send(last_send_byte);
        return false;
    }

    uint8_t UART_controller::receive_data(uint8_t *buffer, uint8_t len) {
        if((stage != Stage::No_transmission) || (len < receive_len) || (receive_status == Status::Ok))
            return 0;
        len = receive_len;
        receive_status = Status::Ok;
        receive_cnt = 0;
        receive_len = 0;
        for(int i = 0; i < len; ++i) {
            buffer[i] = receive_buffer[i];
        }
        return len;
    }

    void UART_controller::end_send() {
        switch (stage) {
            case Stage::No_transmission:
                // Такого быть не может
                break;
            case Stage::Received_initialization_byte:
                // отправили ответ на байт инициализации, ожидаем байт длины
                uart_receive(last_receive_byte); // - wait();
                break;
            case Stage::Received_length_byte:
                // отправили ответ на байт длины, ожидаем байт подтверждения длины
                uart_receive(last_receive_byte); // - wait();
                break;
            case Stage::Receive_bytes:
                // Такого быть не может
                break;
            case Stage::Expect_trailing_byte:
                // Отправили ответ на завершающий байт
                stage = Stage::No_transmission;
                receive_status = Status::Exit;
                // (-) ----- отправить инициализирующий пакет, если ожидаем?
                break;
            case Stage::Sent_initialization_byte:
                // Отправили байт инициализации, ожидаем реакцию; отправили 0, отправляем байт инициализации
                if (last_send_byte == 0) { // - send();
                    last_send_byte = 0xA5;
                    uart_send(last_send_byte);
                }
                else { // - wait();
                    uart_receive(last_receive_byte);
                }
                break;
            case Stage::Sent_length_byte:
                // Отправили байт длины, ожидаем реакцию; отправили 0xFF, отправляем байт длины
                if (last_send_byte == 0xFF) { // - send();
                    last_send_byte = send_len;
                    uart_send(last_send_byte);
                }
                else { // - wait();
                    uart_receive(last_receive_byte);
                }
                break;
            case Stage::Sending_bytes:
                // Отправили байт, отправляем следующий или отправляем конечный
                if(send_cnt < send_len) { // - send_next();
                    last_send_byte = send_buffer[send_cnt++];
                    uart_send(last_send_byte);
                }
                else { // - send_end();
                    stage = Stage::Sent_trailing_byte;
                    last_send_byte = 0x5A;
                    uart_send(last_send_byte);
                }
                break;
            case Stage::Sent_trailing_byte:
                // Отправили конечный, ожидаем байт подтверждения
                uart_receive(last_receive_byte); // - wait();
                break;
            default:
                stage = Stage::No_transmission;
        }
    }

    void UART_controller::end_receive() {
        switch (stage) {
            case Stage::No_transmission:
                // начало отправки с другой стороны, отправить реакцию; игнор
                if(last_receive_byte == 0xA5) {
                    stage = Stage::Received_initialization_byte;
                    last_send_byte = 0xDB;
                    uart_send(last_send_byte);
                }
                else {
                    last_receive_byte = 0;
                    uart_receive(last_receive_byte); // - wait();
                }
                break;
            case Stage::Received_initialization_byte:
                // проверяю на 0, если есть длина, отправляю инверсию, если 0 ожидаю начало инициализации
                if(last_receive_byte == 0) {
                    stage = Stage::No_transmission;
                    uart_receive(last_receive_byte);
                }
                else if(last_receive_byte == 0xFF) {
                    uart_receive(last_receive_byte);
                }
                else {
                    stage = Stage::Received_length_byte;
                    receive_len = last_receive_byte;
                    last_send_byte = ((~receive_len) & 0xFF);
                    uart_send(last_send_byte);
                }
                break;
            case Stage::Received_length_byte:
                // проверяю на 0xFF, если есть инверсия длины, принимаю серию байт, если 0 ожидаю байт длины
                if(last_receive_byte == 0xFF) {
                    stage = Stage::Received_initialization_byte;
                }
                else {
                    stage = Stage::Receive_bytes;
                }
                uart_receive(last_receive_byte);
                break;
            case Stage::Receive_bytes:
                // принимаем байты, если все - то ожидаем последний байт
                receive_buffer[receive_cnt++] = last_receive_byte;
                uart_receive(last_receive_byte);
                if(receive_cnt == receive_len) {
                    receive_cnt = 0;
                    stage = Stage::Expect_trailing_byte;
                }
                break;
            case Stage::Expect_trailing_byte:
                // Принимаем завершающий байт
                if(last_receive_byte == 0x5A) {
                    last_send_byte = 0x24;
                    uart_send(last_send_byte);
                }
                else {
                    stage = Stage::No_transmission;
                    receive_status = Status::Error;
                    // (-) ----- отправить инициализирующий пакет, если ожидаем?
                }
                break;
            case Stage::Sent_initialization_byte:
                // Реакция на инициализирующий пакет
                if(last_receive_byte == 0xDB) {
                    // отправить длину
                    stage = Stage::Sent_length_byte;
                    last_send_byte = send_len;
                    uart_send(last_send_byte);
                }
                else if(last_receive_byte == 0xA5) {
                    // проверка на приоритет, если я важнее, то ничего, если нет - то отправить 0xDB
                    last_send_byte = 0xDB;
                    uart_send(last_send_byte);
                }
                else {
                    // фигня? отправить 0
                    last_send_byte = 0;
                    uart_send(last_send_byte);
                }
                break;
            case Stage::Sent_length_byte:
                // Принята инвертированная длина
                if(last_receive_byte == ((~send_len) & 0xFF)) {
                    // отправить перемешанную длину
                    stage = Stage::Sending_bytes;
                    last_send_byte = (last_receive_byte << 4 & 0xF0) | (last_receive_byte >> 4 & 0x0F);
                    uart_send(last_send_byte);
                }
                else {
                    // Ошибка, обнуляю
                    last_send_byte = 0xFF;
                    uart_send(last_send_byte);
                }
                break;
            case Stage::Sending_bytes:
                // Такого быть не может
                break;
            case Stage::Sent_trailing_byte:
                // Принимаем реакцию на завершающий байт
                if(last_receive_byte == 0x24) {
                    stage = Stage::No_transmission;
                    send_status = Status::Exit;
                }
                else {
                    stage = Stage::No_transmission;
                    send_status = Status::Error;
                }
                uart_receive(last_receive_byte);
                break;
            default:
                stage = Stage::No_transmission;
        }
    }

    void UART_controller::receive_timeout() {
        last_receive_byte = 0;
        switch (stage) {
            case Stage::No_transmission:
                stage = Stage::No_transmission;
                uart_receive(last_receive_byte);
                break;
            case Stage::Received_initialization_byte:
            case Stage::Received_length_byte:
            case Stage::Receive_bytes:
            case Stage::Expect_trailing_byte:
                stage = Stage::No_transmission;
                receive_status = Status::Error;
                uart_receive(last_receive_byte);
                break;
            case Stage::Sent_initialization_byte:
                last_send_byte = 0;
                uart_send(last_send_byte);
                break;
            case Stage::Sent_length_byte:
                last_send_byte = 0xFF;
                uart_send(last_send_byte);
                break;
            case Stage::Sending_bytes:
                // Такого быть не может
                break;
            case Stage::Sent_trailing_byte:
                stage = Stage::No_transmission;
                send_status = Status::Error;
                uart_receive(last_receive_byte);
                break;
            default:
                stage = Stage::No_transmission;
                uart_receive(last_receive_byte);
        }
    }

    Status UART_controller::clear_send_status() {
        Status result = send_status;
        send_status = Status::Ok;
        return result;
    }

    Status UART_controller::clear_receive_status() {
        Status result = receive_status;
        receive_status = Status::Ok;
        return result;
    }

    void UART_controller::erase() {
        stage = Stage::No_transmission;
        send_status = Status::Ok;
        receive_status = Status::Ok;
    }
}
