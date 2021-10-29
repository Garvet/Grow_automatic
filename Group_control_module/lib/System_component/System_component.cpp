#include "System_component.hpp"

namespace scs {

    // индивидуальный номер платы
    void System_component::set_system_id(std::array<uint8_t, AMT_BYTES_ID> set_system_id) {
        system_id = set_system_id;
    }
    std::array<uint8_t, AMT_BYTES_ID> System_component::get_system_id() const {
        return system_id;
    }

    // имя платы
    bool System_component::set_name(std::array<uint8_t, AMT_BYTES_NAME> set_system_name, uint8_t len) {
        return set_name(&set_system_name[0], len);
    }
    bool System_component::set_name(const uint8_t *set_system_name, uint8_t len) {
        if(AMT_BYTES_NAME <= len)
            return true;
        for(int i = 0; i < len; ++i) {
            system_name[i] = set_system_name[i];
        }
        len_name = len;
        return false;
    }
    bool System_component::set_name(const char *set_system_name, uint8_t len) {
        return set_name((uint8_t*)set_system_name, len);
    }
    const std::array<uint8_t, AMT_BYTES_NAME>& System_component::get_name() const {
        return system_name;
    }
    uint8_t System_component::get_name(uint8_t num) const {
        // return (num < len_name) * system_name[num];
        if(num < len_name)
            return system_name[num];
        return 0;
    }
    uint8_t System_component::get_len_name() const {
        return len_name;
    }

    // адрес LoRa
    // установка адреса модуля
    bool System_component::set_address(LoRa_address address) {
        if((address.branch >= (1 << LORA_ADDRESS_BRANCH)) || (address.branch == 0))
            return true;
        address_ = address;
        return false;
    }
    // получение адреса модуля
    LoRa_address System_component::get_address() const {
        return address_;
    }

    // состояние компонента
    bool System_component::set_state___(State new_state) {
        if (new_state < State::end_) {
            state_ = new_state;
            return false;
        }
        return true;
    }
    State System_component::get_state___() const {
        return state_;
    }


    // (-) ----- (!) ----- \/ \/ \/ КОСТЫЛЬ
    bool System_component::set_send_server_value(uint16_t val) {
        if(4095 < val)
            return true;
        send_server_value = val;
        return false;
    }
    uint16_t System_component::get_send_server_value() {
        return send_server_value;
    }
    void System_component::clear_send_server_value() {
        send_server_value = 0xFFFF;
    }
    // (-) ----- (!) ----- /\ /\ /\ КОСТЫЛЬ
}
