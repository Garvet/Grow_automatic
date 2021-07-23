#include "System_component.hpp"

namespace scs {

    // индивидуальный номер платы
    void System_component::set_system_id(std::array<uint8_t, AMT_BYTES_ID> set_system_id) {
        system_id = set_system_id;
    }
    std::array<uint8_t, AMT_BYTES_ID> System_component::get_system_id() const {
        return system_id;
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
}
