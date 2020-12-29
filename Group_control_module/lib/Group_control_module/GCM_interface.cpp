#include "GCM_interface.h"

GCM_interface gcm_interface;

static char network_name_[AMT_BYTES_NETWORK_NAME];
static char network_pswd_[AMT_BYTES_NETWORK_PASSWORD];
static char network_adr_ [AMT_BYTES_NETWORK_ADDRESS];
static uint8_t  len_network_name_ = 0;
static uint8_t  len_network_pswd_ = 0;
static uint8_t  len_network_adr_  = 0;
static uint32_t network_port_ = 0;
static WiFiUDP udp_;
static bool connected_ = false;

static uint8_t buffer[AMT_BYTES_BUFFER];
static uint16_t buf_len = 0;
void WiFiEvent(WiFiEvent_t event);
void connectToWiFi();
void WiFisend();

extern const uint16_t LORA_ADDRESS_BRANCH;

bool GCM_interface::set_group_control_module(Group_control_module * gcm) {
    if(gcm == nullptr)
        return true;
    gcm_ = gcm;
    return false;
}
bool GCM_interface::init_server_connect(
            std::array<char, AMT_BYTES_NETWORK_NAME> network_name, uint8_t  len_network_name,
            std::array<char, AMT_BYTES_NETWORK_PASSWORD> network_pswd, uint8_t  len_network_pswd,
            std::array<char, AMT_BYTES_NETWORK_ADDRESS> network_adr, uint8_t  len_network_adr,
            uint32_t network_port) {
    if((len_network_name > AMT_BYTES_NETWORK_NAME) ||
        (len_network_pswd > AMT_BYTES_NETWORK_PASSWORD) ||
        (len_network_adr > AMT_BYTES_NETWORK_ADDRESS)) {
        return true;
    }
    len_network_name_ = len_network_name;
    len_network_pswd_ = len_network_pswd;
    len_network_adr_  = len_network_adr;
    for(int i = 0; i < len_network_name; ++i)
        network_name_[i] = network_name[i];
    for(int i = 0; i < len_network_pswd; ++i)
        network_pswd_[i] = network_pswd[i];
    for(int i = 0; i < len_network_adr; ++i)
        network_adr_[i] = network_adr[i];
    network_port_ = network_port;
    connectToWiFi();
    return false;
}




// --- Сохранение в энергонезависимую память ---

size_t GCM_interface::get_size() {
    // // unsigned long _period; 4
    // // uint8_t _amt_component; 1
    // // Config_grow_sensor_component* _component; not save
    // size_t size = 5;
    // for(int i = 0; i < grow_sensor.component_.size(); ++i)
    //     size += grow_sensor.component_[i].get_size();
    // return size;
}
size_t GCM_interface::get_data(uint8_t *data) {
    // if(data == nullptr)
    //     return 0;
    // size_t size = 0;
    // data[size++] = (uint8_t)((grow_sensor.period_ >> 24) & 0xFF);
    // data[size++] = (uint8_t)((grow_sensor.period_ >> 16) & 0xFF);
    // data[size++] = (uint8_t)((grow_sensor.period_ >> 8) & 0xFF);
    // data[size++] = (uint8_t)(grow_sensor.period_ & 0xFF);
    // data[size++] = grow_sensor.component_.size();
    // for(int i = 0; i < grow_sensor.component_.size(); ++i)
    //     size += grow_sensor.component_[i].get_data(data+size);
    // return size;
}
size_t GCM_interface::set_data(uint8_t *data, size_t available_size) {
    // if(data == nullptr)
    //     return 0;
    // size_t size = 0;
    // grow_sensor.component_.clear();
    // grow_sensor.period_  = ((ulong)data[size++]) << 24;
    // grow_sensor.period_ |= ((ulong)data[size++]) << 16;
    // grow_sensor.period_ |= ((ulong)data[size++]) << 8;
    // grow_sensor.period_ |= ((ulong)data[size++]);
    // uint8_t amt_component = data[size++];
    // if(amt_component != 0) {
    //     grow_sensor.component_.resize(amt_component);
    //     for(int i = 0; i < COUNT_TYPE_SENSOR; ++i)
    //         id_mas_sensors[i] = 0;
    //     for(int i = 0; i < amt_component; ++i) {
    //         size += grow_sensor.component_[i].set_data(data+size, available_size-size);
    //         grow_sensor.component_[i].set_id(id_mas_sensors[grow_sensor.component_[i].get_type()]++);            
    //     }
    // }
    // return size;
}

// --- Обмен с телефоном ---

// // --- LoRa-соединение ---
// LoRa_packet GCM_interface::creat_regist_packet(const Grow_sensor &grow_sensor, LoRa_contact_data& contact_data) {
//     // Exchange_packet packet;
//     // // packet.creat_packet(16 + grow_sensor.get_count_component(), Packet_Type::SYSTEM); 
//     // // packet.packet->set_dest_adr(LORA_GLOBAL_ADDRESS);
//     // // packet.packet->set_sour_adr(LORA_GLOBAL_ADDRESS);
//     // // packet.packet->set_packet_type(Packet_Type::SYSTEM);
//     // packet_system.set_dest_adr(packet.packet_data, LORA_GLOBAL_ADDRESS);
//     // packet_system.set_sour_adr(packet.packet_data, LORA_GLOBAL_ADDRESS);
//     // packet_system.set_packet_type(packet.packet_data, Packet_Type::SYSTEM);
//     LoRa_packet packet;
//     packet_system.set_dest_adr(packet, LORA_GLOBAL_ADDRESS);
//     packet_system.set_sour_adr(packet, LORA_GLOBAL_ADDRESS);
//     packet_system.set_packet_type(packet, Packet_Type::SYSTEM);
//
//     uint8_t com = 0;
//     // uint8_t sensor_type = 0;
//     uint8_t len = grow_sensor.get_count_component();
//     uint8_t num_byte = 0;
//     uint8_t* data = new uint8_t[len + AMT_BYTES_SYSTEM_ID + 2]; // ID, Type, Length
//
//     std::array<uint8_t, AMT_BYTES_SYSTEM_ID> id = grow_sensor.get_system_id();
//     for(int i = 0; i < AMT_BYTES_SYSTEM_ID; ++i)
//         data[num_byte++] = id[i];
//    
//     // data[num_byte++] = (grow_sensor.get_system_id() >> 24) & 0xFF;
//     // data[num_byte++] = (grow_sensor.get_system_id() >> 16) & 0xFF;
//     // data[num_byte++] = (grow_sensor.get_system_id() >> 8) & 0xFF;
//     // data[num_byte++] = grow_sensor.get_system_id() & 0xFF;
//
//     data[num_byte++] = 0x01; // Type = sensors
//
//     data[num_byte++] = len; // Length = grow_sensor.get_count_component();
//     for(int i = 0; i < len; ++i)
//     // {
//         grow_sensor.get_type(i, data[num_byte++]);
//     //     grow_sensor.get_type(i, sensor_type);
//     //     data[num_byte++] = sensor_type;
//     // }
//     packet_system.set_packet_data(packet, &com, data, &len);
//     delete[] data;
//     return packet;
// }
// Grow_sensor GCM_interface::read_regist_packet(LoRa_packet& packet) {
//     // Получение длины
//     uint8_t size;
//     uint8_t err = 0;
//     err = packet_system.get_size_by_packet(packet, size);
//     if((err != 0) || (size == 0)) 
//         return Grow_sensor();
//     //Получение содержимого
//     uint8_t com = 0;
//     uint8_t len = 0;
//     uint8_t* data = nullptr;
//     data = new uint8_t[size]; // ID, Type, Length
//     err = packet_system.get_packet_data(packet, &com, data, &len);
//     if((err != 0) || (com != 0)) {
//         return Grow_sensor();
//     }
//     // Расшифровка содержимого
//     uint8_t num_byte = 0;
//    
//     std::array<uint8_t, AMT_BYTES_SYSTEM_ID> sensor_id;
//     for(int i = 0; i < AMT_BYTES_SYSTEM_ID; ++i)
//         sensor_id[i] = data[num_byte++];
//     // uint32_t sensor_id = data[num_byte++];
//     // sensor_id = (sensor_id << 8) | data[num_byte++];
//     // sensor_id = (sensor_id << 8) | data[num_byte++];
//     // sensor_id = (sensor_id << 8) | data[num_byte++];
//     // Генерация объекта
//     Grow_sensor sensors(len, &(data[num_byte]));
//     sensors.set_system_id(sensor_id);
//     delete[] data;
//     return sensors;
// }
// bool GCM_interface::check_regist_packet(LoRa_contact_data& contact_data) {
//
// }
// uint8_t GCM_interface::creat_send_data_packet(Grow_sensor &grow_sensor, LoRa_contact_data& contact_data) {
//     if(contact_data.get_signal_start_connect())
//         return 0;
//     LoRa_packet packet;
//     uint8_t err = 0;
//     uint8_t amt = grow_sensor.get_count_component();
//     uint8_t param, id;
//     uint32_t value;
//     packet_sensor.set_setting(grow_sensor.get_setting());
//     contact_data.clear_send_packet();
//     for(int i = 0; i < amt; ++i) {
//     // last_send_packet_.packet->set_dest_adr(connect_adr_);
//     // last_send_packet_.packet->set_sour_adr(my_adr_);
//         packet_sensor.set_dest_adr(packet, contact_data.get_connect_adr()); // адреса задаются при передаче в LoRa_contact_data LORA_GLOBAL_ADDRESS
//         packet_sensor.set_sour_adr(packet, contact_data.get_my_adr()); // адреса задаются при передаче в LoRa_contact_data LORA_GLOBAL_ADDRESS
//         err = grow_sensor.get_type(i, param);
//         if(err) return i;
//         err = grow_sensor.get_id(i, id);
//         if(err) return i;
//         err = grow_sensor.get_value(i, *reinterpret_cast<float*>(&value));
//         if(err) return i;
//         packet_sensor.set_packet_type(packet, Packet_Type::SENSOR);
//         err = packet_sensor.set_packet_data(packet, nullptr, &param, &id, &value);
//         if(err) return i;
//         contact_data.add_packet(std::move(packet));
//     }
//     contact_data.wait_recipient();
//     return amt;
// }
// uint8_t GCM_interface::read_send_data_packet(Grow_sensor &grow_sensor, LoRa_packet* packet, uint8_t amt) {
//     return 0; // (-) ----- (!) -----
// }

// --- Отчётность ---

void connectToWiFi() {
                                                                                                        // Serial.println("Connecting to WiFi network: " + String(ssid));

  // delete old config
  WiFi.disconnect(true);
  //register event handler
  WiFi.onEvent(WiFiEvent);
  
  //Initiate connection
  WiFi.begin(network_name_, network_pswd_);

                                                                                                        Serial.println("Waiting for WIFI connection...");
}
void WiFiEvent(WiFiEvent_t event) {
    switch(event) {
      case SYSTEM_EVENT_STA_GOT_IP:
          //When connected set 
                                                                                                        Serial.print("WiFi connected! IP address: ");
                                                                                                        Serial.println(WiFi.localIP());  
          //initializes the UDP state
          //This initializes the transfer buffer
          udp_.begin(WiFi.localIP(), network_port_);
          connected_ = true;
          break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
                                                                                                        Serial.println("WiFi lost connection");
          connected_ = false;
          break;
      default: break;
    }
                                                                                                        if(connected_) {
                                                                                                            Serial.println("True connection");
                                                                                                        }
                                                                                                        else {
                                                                                                            Serial.println("False connection");
                                                                                                        }
}
void WiFisend() {
    // udp_.begin(WiFi.localIP(), network_port_);
    udp_.beginPacket(network_adr_, network_port_);
    for(int i = 0; i < buf_len; ++i) {
        udp_.write(buffer[i]);
    }
    udp_.endPacket();
    // udp_.stop();
                                                                                                        // Serial.println();
                                                                                                        // Serial.print("|>-{");
                                                                                                        // for(int i = 0; i < buf_len; ++i) {
                                                                                                        //     if(buffer[i] < 16)
                                                                                                        //         Serial.print("0");
                                                                                                        //     Serial.print(buffer[i], 16);
                                                                                                        //     if(i < buf_len - 1)
                                                                                                        //         Serial.print(", ");
                                                                                                        // }
                                                                                                        // Serial.println("}-<|\n");
    // Serial.printf("Seconds since boot: %lu\n", millis()/1000);
}


// Регистрация в системе
uint16_t GCM_interface::report_to_server_regist_data() {
    buf_len = 0;
    for(int i = 0; i < gcm_->system_id_.size(); ++i)
        buffer[buf_len++] = gcm_->system_id_[i];

    buffer[buf_len++] = 0; // type report (0 - registration, 1 - sensor, 2 - device)

    buffer[buf_len++] = gcm_->sensors_.size() + gcm_->devices_.size(); // amt

    for(int k = 0; k < gcm_->sensors_.size(); ++k)
        for(int i = 0; i < gcm_->system_id_.size(); ++i)
            buffer[buf_len++] = gcm_->sensors_[k].get_system_id()[i];

    for(int k = 0; k < gcm_->devices_.size(); ++k)
        for(int i = 0; i < gcm_->system_id_.size(); ++i)
            buffer[buf_len++] = gcm_->devices_[k].get_system_id()[i];

    RtcDateTime rdt = gcm_->get_data_time();

    buffer[buf_len++] = rdt.Day();
    buffer[buf_len++] = rdt.Month();
    buffer[buf_len++] = (rdt.Year() >> 8) & 0xFF;
    buffer[buf_len++] = rdt.Year() & 0xFF;

    buffer[buf_len++] = rdt.Hour();
    buffer[buf_len++] = rdt.Minute();
    buffer[buf_len++] = rdt.Second();

    if(init_server)
        WiFisend();
                                                                                                        // Serial.println();
                                                                                                        // Serial.print("|>-{");
                                                                                                        // for(int i = 0; i < buf_len; ++i) {
                                                                                                        //     if(buffer[i] < 16)
                                                                                                        //         Serial.print("0");
                                                                                                        //     Serial.print(buffer[i], 16);
                                                                                                        //     if(i < buf_len - 1)
                                                                                                        //         Serial.print(", ");
                                                                                                        // }
                                                                                                        // Serial.print("}-<| ");

                                                                                                        // Serial.print(rdt.Day());
                                                                                                        // Serial.print("/");
                                                                                                        // Serial.print(rdt.Month());
                                                                                                        // Serial.print("/");
                                                                                                        // Serial.print(rdt.Year());
                                                                                                        // Serial.print("  ");
                                                                                                        // Serial.print(rdt.Hour());
                                                                                                        // Serial.print(":");
                                                                                                        // Serial.print(rdt.Minute());
                                                                                                        // Serial.print(":");
                                                                                                        // Serial.print(rdt.Second());
                                                                                                        // Serial.println(" |\n");

    // mas = [0x01, 0x05, 0x10, 0x20, 0x0a, 0x0b, 0xc0, 0xd0, 0x4a, 0xff, 0xc2, 0x00, // ID
    //        0, // type report (0 - registration, 1 - sensor, 2 - device)
    //        3, // amt
    //        0x02, 0x06, 0x11, 0x21, 0x0b, 0x0c, 0xc1, 0xd1, 0x4b, 0x00, 0xc3, 0x01, // module 1
    //        0x03, 0x07, 0x12, 0x22, 0x0c, 0x0d, 0xc2, 0xd2, 0x4c, 0x01, 0xc4, 0x02, // module 2
    //        0x04, 0x08, 0x13, 0x23, 0x0d, 0x0e, 0xc3, 0xd3, 0x4d, 0x02, 0xc5, 0x03, // module 3
    //        15, 6, 2020 >> 8, 2020 & 0xFF, // date
    //        12, 45, 20]                    // time
}
// Отправка данных датчиков и устройств
uint16_t GCM_interface::report_to_server_read_data() {
    // clear_change_value = false
    buf_len = 0;
    RtcDateTime rdt = gcm_->get_data_time();
    for(int k = 0; k < gcm_->sensors_.size(); ++k) {
        if(!gcm_->sensors_[k].get_change_value())
            continue;
        gcm_->sensors_[k].clear_change_value();


        for(int i = 0; i < gcm_->system_id_.size(); ++i)
            buffer[buf_len++] = gcm_->sensors_[k].get_system_id()[i];
        buffer[buf_len++] = 1; // type report (0 - registration, 1 - sensor, 2 - device)

        buffer[buf_len++] = gcm_->sensors_[k].get_count_component();

        uint8_t data_b;
        uint32_t data;
        for(int i = 0; i < gcm_->sensors_[k].get_count_component(); ++i) {
            gcm_->sensors_[k].get_type(i, data_b);
            buffer[buf_len++] = data_b;
            gcm_->sensors_[k].get_id(i, data_b);
            buffer[buf_len++] = data_b;
            gcm_->sensors_[k].get_value(i, data);
            buffer[buf_len++] = (data >> 24) & 0xFF;
            buffer[buf_len++] = (data >> 16) & 0xFF;
            buffer[buf_len++] = (data >>  8) & 0xFF;
            buffer[buf_len++] =  data        & 0xFF;
        }

        buffer[buf_len++] = rdt.Day();
        buffer[buf_len++] = rdt.Month();
        buffer[buf_len++] = (rdt.Year() >> 8) & 0xFF;
        buffer[buf_len++] = rdt.Year() & 0xFF;

        buffer[buf_len++] = rdt.Hour();
        buffer[buf_len++] = rdt.Minute();
        buffer[buf_len++] = rdt.Second();
        if(init_server)
            WiFisend();
        // mas = [0x02, 0x06, 0x11, 0x21, 0x0b, 0x0c, 0xc1, 0xd1, 0x4b, 0x00, 0xc3, 0x01,  # ID
        //        1,                     # type report (0 - registration, 1 - sensor, 2 - device)
        //        4,                     # amt
        //        0, 0, 10, 20,          # sensor 1
        //        1, 0, 1,               # sensor 2
        //        3, 1, 0x42, 0x12, 0x66, 0x66,   # sensor 3
        //        6, 0, 0x43, 0x66, 0x00, 0x00,   # sensor 4
        //        15, 6, 2020 >> 8, 2020 & 0xFF,  # date
        //        12, 30, 30]            # time
                                                                                                        // Serial.println();
                                                                                                        // Serial.print("|>-{");
                                                                                                        // for(int i = 0; i < buf_len; ++i) {
                                                                                                        //     if(buffer[i] < 16)
                                                                                                        //         Serial.print("0");
                                                                                                        //     Serial.print(buffer[i], 16);
                                                                                                        //     if(i < buf_len - 1)
                                                                                                        //         Serial.print(", ");

                                                                                                        //     if((i == 11) || (i == 13) || (i == 19) || (i == 25) || (i == 31))
                                                                                                        //         Serial.print("\n     ");
                                                                                                        // }
                                                                                                        // Serial.print("}-<| ");

                                                                                                        // Serial.print(rdt.Day());
                                                                                                        // Serial.print("/");
                                                                                                        // Serial.print(rdt.Month());
                                                                                                        // Serial.print("/");
                                                                                                        // Serial.print(rdt.Year());
                                                                                                        // Serial.print("  ");
                                                                                                        // Serial.print(rdt.Hour());
                                                                                                        // Serial.print(":");
                                                                                                        // Serial.print(rdt.Minute());
                                                                                                        // Serial.print(":");
                                                                                                        // Serial.print(rdt.Second());
                                                                                                        // Serial.println(" |\n");
    }
}
// Отправка данных об ошибках
uint16_t GCM_interface::report_to_server_error() {

}




// uint16_t GCM_interface::report_to_server(Grow_sensor &grow_sensor, uint8_t *buf, uint16_t size) {
//     uint16_t save_size = 0;
//     if(size < 5 + (6 * grow_sensor.get_count_component()))
//         return save_size;
//     uint8_t save_byte = 0;
//     float save_value = 0;
//     uint32_t save_component;
//     // ID
//     std::array<uint8_t, AMT_BYTES_SYSTEM_ID> sensor_id;
//     for(int i = 0; i < AMT_BYTES_SYSTEM_ID; ++i)
//         buf[save_size++] = sensor_id[i];
//     // save_component = grow_sensor.get_system_id(); 
//     // buf[save_size++] = (save_component >> 24) & 0xFF;
//     // buf[save_size++] = (save_component >> 16) & 0xFF;
//     // buf[save_size++] = (save_component >>  8) & 0xFF;
//     // buf[save_size++] =  save_component        & 0xFF;
//     // количество компонентов
//     buf[save_size++] = grow_sensor.get_count_component();
//     // состав компонентов
//     for(int i = 0; i < grow_sensor.get_count_component(); ++i) {
//         // тип
//         grow_sensor.get_type(i, save_byte);
//         buf[save_size++] = save_byte;
//         // номер
//         grow_sensor.get_id(i, save_byte);
//         buf[save_size++] = save_byte;
//         // значение
//         grow_sensor.get_value(i, save_value);
//         save_component = *reinterpret_cast<uint32_t*>(&save_value);
//         buf[save_size++] = (save_component >> 24) & 0xFF;
//         buf[save_size++] = (save_component >> 16) & 0xFF;
//         buf[save_size++] = (save_component >>  8) & 0xFF;
//         buf[save_size++] =  save_component        & 0xFF;
//     }
//     return save_size;
// }