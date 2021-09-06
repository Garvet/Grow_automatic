#include "GCM_interface.h"

GCM_interface gcm_interface;

static char network_name_[AMT_BYTES_NETWORK_NAME];
static char network_pswd_[AMT_BYTES_NETWORK_PASSWORD];
static char network_adr_ [AMT_BYTES_NETWORK_ADDRESS];
static uint8_t  len_network_name_ = 0;
static uint8_t  len_network_pswd_ = 0;
static uint8_t  len_network_adr_  = 0;
static uint32_t network_port_ = 0;
static WiFiMulti wifiMulti;
// static WiFiUDP udp_;
WiFiClient client;
static bool init_connected = false;
static bool connected = false;
static uint32_t time_connected = 0;
static const uint32_t TEMR_RECONNECTED = 5000;

// - 2021.06.11 - перезапуск при отсутствии соединения -
static const uint16_t MAX_ERRORS_IN_ROW_WIFI = 5;
static const uint16_t MAX_ERRORS_IN_ROW_CONNECT = 5;
// -----------------------------------------------------

static uint8_t buffer[AMT_BYTES_BUFFER];
static uint16_t buf_len = 0;
void WiFiEvent(WiFiEvent_t event);
void connectToWiFi();
bool WiFisend();

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
    init_connected = true;
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
#if defined( SEND_SERVER )
                                                                                                        // Serial.println("Connecting to WiFi network: " + String(ssid));

//   // delete old config
//   WiFi.disconnect(true);
//   //register event handler
//   WiFi.onEvent(WiFiEvent);
  
//   //Initiate connection
//   WiFi.begin(network_name_, network_pswd_);

//                                                                                                         Serial.println("Waiting for WIFI connection...");





    wifiMulti.addAP(network_name_, network_pswd_);
                                                                                                        Serial.print("Waiting for WIFI connection...");
    uint8_t wait_time = 3;
    connected = true;
    while (wifiMulti.run() != WL_CONNECTED) {
        if(wait_time == 0) {
                                                                                                        Serial.println("\nWiFi lost connection");
            connected = false;
            break;
        }
        delay(500);
        Serial.print(".");
        --wait_time;
    }
    if(connected) {
                                                                                                        Serial.print("\nWiFi connection, IP = ");
                                                                                                        Serial.println(WiFi.localIP());
    }
    // - 2021.06.11 - перезапуск при отсутствии соединения -
    #if defined (ERROR_REBOOT)
    static const uint16_t MAX_ERRORS_IN_ROW = MAX_ERRORS_IN_ROW_WIFI;
    static uint16_t number_errors_row = 0;
    if(connected) {
        number_errors_row = 0;
    }
    else {
        if(number_errors_row == MAX_ERRORS_IN_ROW) {
            RtcDateTime rdt = gcm_interface.gcm_->get_data_time();
            uint16_t data;
            Serial.println("\n| ----- -----");
            Serial.println("| ------- ----- -----");
            Serial.print("| ----- ");
            data = rdt.Year(); Serial.print(data); Serial.print("/");
            data = rdt.Month(); Serial.print(data); Serial.print("/");
            data = rdt.Day(); Serial.print(data); Serial.print("  ");
            data = rdt.Hour(); Serial.print(data); Serial.print(":");
            data = rdt.Minute(); Serial.print(data); Serial.print(":");
            data = rdt.Second(); Serial.print(data); Serial.println(" - {Server connect error reboot}");
            Serial.println("| ------- ----- -----");
            Serial.println("| ----- -----\n");
            esp_restart();
        }
        else
            ++number_errors_row;
    }
    #endif
    // -----------------------------------------------------





#endif
}
void WiFiEvent(WiFiEvent_t event) {
#if defined( SEND_SERVER )
    switch(event) {
      case SYSTEM_EVENT_STA_GOT_IP:
          //When connected set 
                                                                                                        Serial.print("WiFi connected! IP address: ");
                                                                                                        Serial.println(WiFi.localIP());  
          //initializes the UDP state
          //This initializes the transfer buffer
        //   udp_.begin(WiFi.localIP(), network_port_);
          connected = true;
          break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
                                                                                                        Serial.println("WiFi lost connection");
          connected = false;
          break;
      default: break;
    }
                                                                                                        if(connected) {
                                                                                                            Serial.println("True connection");
                                                                                                        }
                                                                                                        else {
                                                                                                            Serial.println("False connection");
                                                                                                        }

#endif
}


#define SEND_WAIT_ERROR 5000u

RtcDateTime rdt;
bool send_request_state(const std::array<uint8_t, scs::AMT_BYTES_ID>& system_id, bool state) { // (-) ----- костыль
#if defined( SEND_SERVER )
    static uint32_t send_wait_error = millis() - SEND_WAIT_ERROR;
    uint8_t wait_connect = 10;
    bool client_connect = true;
    bool err_send_data = true;
    
    client.stop(); 
    while (!client.connect(network_adr_, network_port_)) { // adr!!!
        if(wait_connect == 0) {
            client_connect = false;
            break;
        }
        --wait_connect;
        delay(1000);
    }
    if(client_connect) {
        buf_len = 0;

        for(int i = 0; i < scs::AMT_BYTES_ID; ++i)
            buffer[buf_len++] = system_id[i];
        buffer[buf_len++] = 2; // type report (0 - registration, 1 - sensor, 2 - device)
        buffer[buf_len++] = 1;
        buffer[buf_len++] = Signal_digital;
        buffer[buf_len++] = 0;
        buffer[buf_len++] = (uint8_t)state;

        buffer[buf_len++] = rdt.Day();
        buffer[buf_len++] = rdt.Month();
        buffer[buf_len++] = (rdt.Year() >> 8) & 0xFF;
        buffer[buf_len++] = rdt.Year() & 0xFF;

        buffer[buf_len++] = rdt.Hour();
        buffer[buf_len++] = rdt.Minute();
        buffer[buf_len++] = rdt.Second();

        client.write(buffer, buf_len);

        uint maxloops = 0;
        while (!client.available() && maxloops < 10000) {
            maxloops++;
            delay(1); //delay 1 msec
        }

        if (client.available() > 0) {
            Serial.println("Packet send correct");
            send_wait_error = millis() - SEND_WAIT_ERROR;
            err_send_data = false;
        }
        else {
            Serial.println("client.available() timed out ");
        }
    }
#endif
}
bool send_request_value(const std::array<uint8_t, scs::AMT_BYTES_ID>& system_id, uint16_t value) { // (-) ----- костыль
#if defined( SEND_SERVER )
    static uint32_t send_wait_error = millis() - SEND_WAIT_ERROR;
    uint8_t wait_connect = 10;
    bool client_connect = true;
    bool err_send_data = true;
    
    client.stop(); 
    while (!client.connect(network_adr_, network_port_)) { // adr!!!
        if(wait_connect == 0) {
            client_connect = false;
            break;
        }
        --wait_connect;
        delay(1000);
    }
    if(client_connect) {
        buf_len = 0;

        for(int i = 0; i < scs::AMT_BYTES_ID; ++i)
            buffer[buf_len++] = system_id[i];
        buffer[buf_len++] = 2; // type report (0 - registration, 1 - sensor, 2 - device)
        buffer[buf_len++] = 1;
        buffer[buf_len++] = Signal_PWM;
        buffer[buf_len++] = 0;
        buffer[buf_len++] = (value >> 8) & 0x0F;
        buffer[buf_len++] = value & 0xFF;

        buffer[buf_len++] = rdt.Day();
        buffer[buf_len++] = rdt.Month();
        buffer[buf_len++] = (rdt.Year() >> 8) & 0xFF;
        buffer[buf_len++] = rdt.Year() & 0xFF;

        buffer[buf_len++] = rdt.Hour();
        buffer[buf_len++] = rdt.Minute();
        buffer[buf_len++] = rdt.Second();

        client.write(buffer, buf_len);

        uint maxloops = 0;
        while (!client.available() && maxloops < 10000) {
            maxloops++;
            delay(1); //delay 1 msec
        }

        if (client.available() > 0) {
            Serial.println("Packet send correct");
            send_wait_error = millis() - SEND_WAIT_ERROR;
            err_send_data = false;
        }
        else {
            Serial.println("client.available() timed out ");
        }
    }
#endif
}
bool WiFisend() {
#if defined( SEND_SERVER )
    static uint32_t send_wait_error = millis() - SEND_WAIT_ERROR;
    uint8_t wait_connect = 3;
    bool client_connect = true;
    bool err_send_data = true;


    if(!connected && (millis() - time_connected > TEMR_RECONNECTED)) {
        // установить соединение
        connectToWiFi();
        time_connected = millis();
    }


    if(!connected || (millis() - send_wait_error < SEND_WAIT_ERROR)) {
        return err_send_data;
    }
    while (!client.connect(network_adr_, network_port_, 2000)) { // adr!!!
        if(wait_connect == 0) {
            client_connect = false;
            break;
        }
        --wait_connect;
        Serial.println("client.connect err");
        delay(100);
    }
    // - 2021.06.11 - перезапуск при отсутствии соединения -
    #if defined (ERROR_REBOOT)
    static const uint16_t MAX_ERRORS_IN_ROW = MAX_ERRORS_IN_ROW_CONNECT;
    static uint16_t number_errors_row = 0;
    if(wait_connect == 0) {
        if(number_errors_row == MAX_ERRORS_IN_ROW) {
            RtcDateTime rdt = gcm_interface.gcm_->get_data_time();
            uint16_t data;
            Serial.println("\n| ----- -----");
            Serial.println("| ------- ----- -----");
            Serial.print("| ----- ");
            data = rdt.Year(); Serial.print(data); Serial.print("/");
            data = rdt.Month(); Serial.print(data); Serial.print("/");
            data = rdt.Day(); Serial.print(data); Serial.print("  ");
            data = rdt.Hour(); Serial.print(data); Serial.print(":");
            data = rdt.Minute(); Serial.print(data); Serial.print(":");
            data = rdt.Second(); Serial.print(data); Serial.println(" - {Server error reboot}");
            Serial.println("| ------- ----- -----");
            Serial.println("| ----- -----\n");
            esp_restart();
        }
        else
            ++number_errors_row;
    }
    else {
        number_errors_row = 0;
    }
    #endif
    // -----------------------------------------------------
    if(client_connect) {
        client.write(buffer, buf_len);

        uint maxloops = 0;
        while (!client.available() && maxloops < 10000) {
            maxloops++;
            delay(1); //delay 1 msec
        }
        if (client.available() > 0) {
            static size_t receive_len = 0;
            static uint8_t type_packet = 0xFF; // (regist|sensor|devices|check)
            static uint8_t amount = 0; // количество устройств (ныне не более 1)
            static uint8_t type_num = 0xFF; // тип управляемого устройства (нене только "Signal_digital")
            static uint8_t new_state = 0xFF; // новое состояние (true или false = work или sleep)
            static uint16_t new_value = 0xFFFF; // новое состояние (true или false = work или sleep)
            static scs::State set_state;
            static int index_component;
            static bool is_device;
            delay(1);
            //    {-} ----- {-}
            // read back one line from the server
            receive_len = client.readBytes(buffer, AMT_BYTES_BUFFER);
            if(receive_len > 0) {
                if(buffer[0] == 1) {
                    std::array<uint8_t, scs::AMT_BYTES_ID> system_id{}; // настраиваемый модуль
                    Serial.print("Receive command: (receive_len = ");
                    Serial.print(receive_len);
                    Serial.print("; packet_len = ");
                    Serial.print((((uint16_t)buffer[1]) << 8) | ((uint16_t)buffer[2]));
                    Serial.println(")");
                    size_t receive_num = 3; // [1, len[1], len[0], ...]

                    for(int i = 0; i < scs::AMT_BYTES_ID; ++i) {
                        system_id[i] = buffer[receive_num++];
                    }
                    type_packet = buffer[receive_num++];
                    if(type_packet != 0x02) {
                        Serial.println("<<< Recive packet no device!!! >>>");
                    }
                    amount = buffer[receive_num++];
                    if(amount != 1) {
                        Serial.print("<<< amount != 1 (amount = "); Serial.print(amount); Serial.println(">>>");
                    }

                    type_num = buffer[receive_num++];
                    if(type_num == (uint8_t)Signal_digital) {
                        Serial.println("Packet type: Signal_digital");
                        receive_num++; // number = buffer[receive_num++]; // number devices in the module
                        new_state = buffer[receive_num++];
                        if((new_state != 0) && (new_state != 1)) {
                            Serial.print("<<< new_state != 0 or 1 (new_state = "); Serial.print(new_state); Serial.println(">>>");
                        }

                        index_component = gcm_interface.gcm_->search_device(system_id);
                        if(index_component != -1) {
                            is_device = true;
                        }
                        else {
                            index_component = gcm_interface.gcm_->search_sensor(system_id);
                        }

                        if(index_component == -1) {
                            Serial.println("Error ID");
                        }
                        else {
                            if(new_state == 0) {
                                if(is_device) {
                                    set_state = scs::State::from_work_to_stop;
                                }
                                else
                                    set_state = scs::State::stop;
                                // set_state = scs::State::stop; // (-) -----
                            } 
                            else if (new_state == 1) {
                                if(is_device) {
                                    set_state = scs::State::from_stop_to_work;
                                }
                                else
                                    set_state = scs::State::work;
                                // set_state = scs::State::work; // (-) -----
                            }
                            else {
                                set_state = scs::State::end_;
                            }
                            if(is_device) {
                                gcm_interface.gcm_->devices_[index_component].set_state___(set_state);
                            }
                            else {
                                gcm_interface.gcm_->sensors_[index_component].set_state___(set_state);
                            }
                        }
                                                                                                    Serial.print("ID: {");
                                                                                                    for(int i = 0; i < scs::AMT_BYTES_ID; ++i) {
                                                                                                        if(system_id[i] < 16)
                                                                                                            Serial.print("0");
                                                                                                        Serial.print(system_id[i], 16);
                                                                                                        if(i < scs::AMT_BYTES_ID - 1)
                                                                                                            Serial.print(", ");
                                                                                                    }
                                                                                                    Serial.print("} set state: ");
                                                                                                    if(new_state) {
                                                                                                        Serial.println("true");
                                                                                                    }
                                                                                                    else {
                                                                                                        Serial.println("false");
                                                                                                    }
                        // {-} ----- присвоить значение состояния ----- {-}
                        send_request_state(system_id, new_state);
                        // {-} ----- присвоить значение состояния ----- {-}
                    }
                    else if (type_num == (uint8_t)Signal_PWM) {
                        Serial.println("Packet type: Signal_PWM");
                        receive_num++; // number = buffer[receive_num++]; // number devices in the module
                        new_value = buffer[receive_num++] & 0x0F;
                        new_value = (new_value << 8) | buffer[receive_num++];

                        index_component = gcm_interface.gcm_->search_device(system_id);
                        if(index_component == -1) {
                            Serial.println("Error ID does not belong to devices");
                        }
                        else {
                            gcm_interface.gcm_->devices_[index_component].set_send_server_value(new_value);
                        }
                                                                                                    Serial.print("ID: {");
                                                                                                    for(int i = 0; i < scs::AMT_BYTES_ID; ++i) {
                                                                                                        if(system_id[i] < 16)
                                                                                                            Serial.print("0");
                                                                                                        Serial.print(system_id[i], 16);
                                                                                                        if(i < scs::AMT_BYTES_ID - 1)
                                                                                                            Serial.print(", ");
                                                                                                    }
                                                                                                    Serial.print("} set value: ");
                                                                                                    Serial.println(new_value);
                        // {-} ----- присвоить значение состояния ----- {-}
                        send_request_value(system_id, new_value);
                        // {-} ----- присвоить значение состояния ----- {-}
                    }
                    else {
                        Serial.print("<<< type_num != Signal_digital or type_num != Signal_PWM (type_num = "); Serial.print(type_num); Serial.println(">>>");
                    }

#if defined (OLD)
                    if(0) { // OLD
                        if(type_num != (uint8_t)Signal_digital) {
                            Serial.print("<<< type_num != Signal_digital (type_num = "); Serial.print(type_num); Serial.println(">>>");
                        }
                        new_state = buffer[receive_num++]; // number
                        new_state = buffer[receive_num++];
                        if((new_state != 0) && (new_state != 1)) {
                            Serial.print("<<< new_state != 0 or 1 (new_state = "); Serial.print(new_state); Serial.println(">>>");
                        }

                        index_component = gcm_interface.gcm_->search_device(system_id);
                        if(index_component != -1) {
                            is_device = true;
                        }
                        else {
                            index_component = gcm_interface.gcm_->search_sensor(system_id);
                        }

                        if(index_component == -1) {
                            Serial.println("Error ID");
                        }
                        else {
                            if(new_state == 0) {
                                if(is_device) {
                                    set_state = scs::State::from_work_to_stop;
                                }
                                else
                                    set_state = scs::State::stop;
                                set_state = scs::State::stop; // (-) -----
                            } 
                            else if (new_state == 1) {
                                if(is_device) {
                                    set_state = scs::State::from_stop_to_work;
                                }
                                else
                                    set_state = scs::State::work;
                                set_state = scs::State::work; // (-) -----
                            }
                            else {
                                set_state = scs::State::end_;
                            }
                            if(is_device) {
                                gcm_interface.gcm_->devices_[index_component].set_state___(set_state);
                            }
                            else {
                                gcm_interface.gcm_->sensors_[index_component].set_state___(set_state);
                            }
                        }
                                                                                                    Serial.print("ID: {");
                                                                                                    for(int i = 0; i < scs::AMT_BYTES_ID; ++i) {
                                                                                                        if(system_id[i] < 16)
                                                                                                            Serial.print("0");
                                                                                                        Serial.print(system_id[i], 16);
                                                                                                        if(i < scs::AMT_BYTES_ID - 1)
                                                                                                            Serial.print(", ");
                                                                                                    }
                                                                                                    Serial.print("} set state: ");
                                                                                                    if(new_state) {
                                                                                                        Serial.println("true");
                                                                                                    }
                                                                                                    else {
                                                                                                        Serial.println("false");
                                                                                                    }
                        // {-} ----- присвоить значение состояния ----- {-}
                        send_request_state(system_id, new_state);
                        // {-} ----- присвоить значение состояния ----- {-}
                    }
#endif
                }
                else {
                    Serial.print("Not command, receive: ");
                    Serial.write(buffer, receive_len);
                    Serial.println();
                    // print "recive correct"
                }
            }
            //    {-} ----- {-}
            // buf_len = client.readBytesUntil('\r', buffer, AMT_BYTES_BUFFER);
            // for(int i = 0; i < buf_len; ++i)
            //     Serial.print((char)buffer[i]);
            // // String line = client.readStringUntil('\r');
            // // Serial.print(line);
            Serial.print("\nTime send to server = ");
            Serial.print(maxloops);
            Serial.println("ms");
            send_wait_error = millis() - SEND_WAIT_ERROR;
            err_send_data = false;
            //    {-} ----- {-}
        }
        else {
            Serial.println("client.available() timed out ");
        }
    }





    client.stop(); 
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
    return err_send_data;
#endif
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

    rdt = gcm_->get_data_time();

    buffer[buf_len++] = rdt.Day();
    buffer[buf_len++] = rdt.Month();
    buffer[buf_len++] = (rdt.Year() >> 8) & 0xFF;
    buffer[buf_len++] = rdt.Year() & 0xFF;

    buffer[buf_len++] = rdt.Hour();
    buffer[buf_len++] = rdt.Minute();
    buffer[buf_len++] = rdt.Second();

    // if(init_server)
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
    // RtcDateTime rdt; // (-) ----- вернуть из global 
    for(int k = 0; k < gcm_->sensors_.size(); ++k) {
        buf_len = 0;
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

        rdt = gcm_->get_data_time();

        buffer[buf_len++] = rdt.Day();
        buffer[buf_len++] = rdt.Month();
        buffer[buf_len++] = (rdt.Year() >> 8) & 0xFF;
        buffer[buf_len++] = rdt.Year() & 0xFF;

        buffer[buf_len++] = rdt.Hour();
        buffer[buf_len++] = rdt.Minute();
        buffer[buf_len++] = rdt.Second();
        // if(init_server)
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


    for(int k = 0; k < gcm_->devices_.size(); ++k) {
        buf_len = 0;
        if(!gcm_->devices_[k].get_change_value())
            continue;
        gcm_->devices_[k].clear_change_value();


        for(int i = 0; i < gcm_->system_id_.size(); ++i)
            buffer[buf_len++] = gcm_->devices_[k].get_system_id()[i];
        buffer[buf_len++] = 2; // type report (0 - registration, 1 - sensor, 2 - device)

        buffer[buf_len++] = gcm_->devices_[k].get_count_component();

        uint8_t data_b;        
        uint8_t dev_type;
        // uint32_t data;
        uint16_t pwmv;
        int i = 0;
        for(i = 0; i < gcm_->devices_[k].get_count_component(); ++i) {
            gcm_->devices_[k].get_type(i, dev_type);
            buffer[buf_len++] = dev_type;
            gcm_->devices_[k].get_id(i, data_b);
            buffer[buf_len++] = data_b;
            gcm_->devices_[k].get_value(i, pwmv);

            if(dev_type == Signal_PWM || dev_type == Fan_PWM || dev_type == Phytolamp_PWM) {
                buffer[buf_len++] = (pwmv >>  8) & 0xFF;
                buffer[buf_len++] =  pwmv & 0xFF;
            }
            else if (dev_type == Signal_digital || dev_type == Pumping_system || dev_type == Phytolamp_digital) {
                if(pwmv)
                    buffer[buf_len++] =  1 & 0xFF;
                else
                    buffer[buf_len++] =  0 & 0xFF;
            }
            else {
                Serial.println("Error send type devices");
                break;
            }
            // buffer[buf_len++] = (data >> 24) & 0xFF;
            // buffer[buf_len++] = (data >> 16) & 0xFF;
            // buffer[buf_len++] = (data >>  8) & 0xFF;
            // buffer[buf_len++] =  data        & 0xFF;
        }
        if(i < gcm_->devices_[k].get_count_component())
            continue;

        rdt = gcm_->get_data_time();

        buffer[buf_len++] = rdt.Day();
        buffer[buf_len++] = rdt.Month();
        buffer[buf_len++] = (rdt.Year() >> 8) & 0xFF;
        buffer[buf_len++] = rdt.Year() & 0xFF;

        buffer[buf_len++] = rdt.Hour();
        buffer[buf_len++] = rdt.Minute();
        buffer[buf_len++] = rdt.Second();
        // if(init_server)
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

    // send an empty packet every second (?) -----
    static unsigned long empty_time = millis();
    if(millis() - empty_time > 5000U) {
        buf_len = 0;
        for(int i = 0; i < gcm_->get_system_id().size(); ++i)
            buffer[buf_len++] = gcm_->get_system_id()[i];
        buffer[buf_len++] = 3; // type report (0 - registration, 1 - sensor, 2 - device)
        buffer[buf_len++] = 0;
        rdt = gcm_->get_data_time();
        buffer[buf_len++] = rdt.Day();
        buffer[buf_len++] = rdt.Month();
        buffer[buf_len++] = (rdt.Year() >> 8) & 0xFF;
        buffer[buf_len++] = rdt.Year() & 0xFF;
        buffer[buf_len++] = rdt.Hour();
        buffer[buf_len++] = rdt.Minute();
        buffer[buf_len++] = rdt.Second();
        WiFisend();
        empty_time = millis();
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