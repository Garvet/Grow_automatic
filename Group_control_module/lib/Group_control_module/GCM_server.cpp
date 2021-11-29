#include "GCM_server.h"

#define START_WIFI_CONNECT
// #define TEST_CODE_15_10_2021

// #define SERIAL_PRINT_1
// #define SERIAL_PRINT_REG_MODULES
#define SERIAL_PRINT_MODULES
// #define SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA
// #define SERIAL_PRINT_TIME
#define SERIAL_PRINT_MODULE_EDIT_SEND_DATA
// #define SERIAL_PRINT_MODULE_EDIT_SEND_DATA_LEV_PRINT_1
#define SERIAL_PRINT_MODULE_EDIT_SEND_DATA_LEV_PRINT_2

namespace lsc {

    namespace server_GCM {

#if defined( START_WIFI_CONNECT )
        void connectToWifi() {
            Serial.println("Connecting to Wi-Fi...");
            //  "Подключаемся к WiFi..."
            Serial.println(WIFI_SSID);
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        }
        void WiFiEvent(WiFiEvent_t event) {
            Serial.printf("[WiFi-event] event: %d\n", event);
            switch (event) {
            case SYSTEM_EVENT_STA_GOT_IP:
                Serial.println("WiFi connected"); //  "Подключились к WiFi"
                Serial.println("IP address: ");   //  "IP-адрес: "
                Serial.println(WiFi.localIP());
                break;
            case SYSTEM_EVENT_STA_DISCONNECTED:
                Serial.println("WiFi lost connection");
                //  "WiFi-связь потеряна"
                // делаем так, чтобы ESP32
                // не переподключалась к MQTT
                // во время переподключения к WiFi:
                break;
            }
        }
#endif

        bool hex_to_byte(const uint8_t* where, uint8_t* whence, size_t len) {
            bool error_id = false;
            // whence[i] = (where[i] << 4) | where[i];
            for (size_t i = 0; i < len; i++) {
                uint8_t num;
                if(('A' <= where[i]) && (where[i] <= 'F')) {
                    num = where[i] - 'A' + 10;
                }
                else if(('a' <= where[i]) && (where[i] <= 'f')) {
                    num = where[i] - 'a' + 10;
                } 
                else if(('0' <= where[i]) && (where[i] <= '9')) {
                    num = where[i] - '0';
                }
                else {
                    error_id = true;
                    break;
                }
                if(i%2 == 0) {
                    whence[i/2] = num << 4;
                }
                else {
                    whence[i/2] |= num;
                }
            }
            return error_id;
        }
        bool hex_to_byte(const uint8_t* where, std::array<uint8_t, scs::AMT_BYTES_ID>& whence) {
            // static uint8_t buf_id[scs::AMT_BYTES_ID];
            static bool result;
            result = hex_to_byte(where, &whence[0], scs::AMT_BYTES_ID*2);
            // if(result){
            //     for(int i = 0; i < ; ++i)
            //         whence[i]
            // }
            return result;
        }
        uint16_t byte_to_hex(const uint8_t* where, uint8_t* whence, size_t len) {
            // whence[i*2+0] = (where[i] >> 4) & 0x0F;
            // whence[i*2+1] = (where[i] >> 0) & 0x0F;
            for (size_t i = 0; i < len; i++) {
                whence[i * 2]     = (where[i] >> 4) & 0x0F;
                whence[i * 2 + 1] =  where[i]       & 0x0F;
                if(whence[i * 2] < 10) whence[i * 2] += '0';
                else                   whence[i * 2] += 'A' - 10;
                if(whence[i * 2 + 1] < 10) whence[i * 2 + 1] += '0';
                else                       whence[i * 2 + 1] += 'A' - 10;
            }
            return len * 2;
        }
        int get_number(const uint8_t* where, uint16_t& bias, bool use_sign) {
            int num = 0;
            int8_t sign = 1;
            if(use_sign && ((where[bias] == '-') || (where[bias] == '+'))) {
                if (where[bias] == '-')
                    sign = -1;
                else
                    sign = 1;
                bias++;
            }
            while(('0' <= where[bias]) && (where[bias] <= '9')) {
                num *= 10;
                num += where[bias++] - '0';
            }
            num *= sign;
            return num;
        }
        uint16_t add_number(int number, uint8_t* whence, uint16_t& bias, bool use_sign) {
            uint16_t size = 0;
            // Знак
            if(use_sign) {
                ++size;
                if(number < 0) whence[bias++] = '-';
                else           whence[bias++] = '+';
            }
            if(number < 0)     number *= -1;
            // Получение количества десятков
            uint16_t len = 1;
            uint16_t num = number;
            while(num > 9) {num /= 10; len *= 10;};
            // Запись числа
            for(; len > 0; len /= 10) {
                whence[bias++] = number / len + '0';
                number %= len;
                ++size;
            }
            return size;
        }
        uint8_t add_module_id(const scs::System_component* where, uint8_t* whence, uint16_t& bias, bool delimiter) {
            static std::array<uint8_t, scs::AMT_BYTES_ID> id;
            static uint8_t len;
            id = where->get_system_id();
            len = scs::AMT_BYTES_ID;
            bias += byte_to_hex(&id[0], &whence[bias], len);
            if(delimiter) {
                whence[bias++] = ',';
                ++len;
            }
            return len;
        }
        uint8_t add_module_name(const scs::System_component* where, uint8_t* whence, uint16_t& bias, bool delimiter) {
            static const std::array<uint8_t, scs::AMT_BYTES_NAME> *name;
            static uint8_t len;
            name = &where->get_name();
            len = where->get_len_name();
            for(int j = 0; j < len; ++j) {
                if((*name)[j] == '\0') {
                    break;
                }
                whence[bias++] = (*name)[j];
            }
            if(len == 0) {
                whence[bias++] = '-';
                ++len;
            }
            if(delimiter) {
                whence[bias++] = ',';
                ++len;
            }
            return len;
        }

        uint8_t get_value(const uint8_t* where, uint32_t* whence, uint8_t& end_index) {
            uint8_t amount = 0;
            whence[amount] = 0;
            uint8_t start_index = end_index;
            bool end_data = false;
            while(1) {
                if(where[end_index] == '\0') {
                    if(start_index == end_index)
                        --amount;
                    ++end_index;
                    break;
                }
                else if(where[end_index] == ';') {
                    if(start_index == end_index)
                        --amount;
                    ++end_index;
                    break;
                } else if(where[end_index] == ',') {
                    ++end_index;
                    ++amount;
                    end_data = false;
                    whence[amount] = 0;
                } else if (end_data) {
                    ++end_index;
                } else if( ('0' <= where[end_index]) && (where[end_index] <= '9') ) {
                    whence[amount] = whence[amount] * 10 + where[end_index] - '0';
                    ++end_index;
                } else {
                    end_data = true;
                }
            }
            return ++amount;
        }
        uint8_t get_value(const uint8_t* where, uint32_t* whence) {
            uint8_t empty_data = 0;
            return get_value(where, whence, empty_data);
        }
        uint8_t get_time_value(const uint8_t* where, uint32_t* whence, uint8_t& end_index) {
            uint8_t amount = 0;
            whence[amount] = 0;
            uint8_t start_index = end_index;
            bool end_data = false;
            while(1) {
                if(where[end_index] == '\0') {
                    if(start_index == end_index)
                        --amount;
                    ++end_index;
                    break;
                } else if(where[end_index] == ';') {
                    if(start_index == end_index)
                        --amount;
                    ++end_index;
                    break;
                } else if(where[end_index] == ':') {
                    ++end_index;
                    whence[amount] = whence[amount] * 10;
                } else if(where[end_index] == ',') {
                    ++end_index;
                    ++amount;
                    end_data = false;
                    whence[amount] = 0;
                } else if (end_data) {
                    ++end_index;
                } else if( ('0' <= where[end_index]) && (where[end_index] <= '9') ) {
                    whence[amount] = whence[amount] * 10 + where[end_index] - '0';
                    ++end_index;
                } else {
                    end_data = true;
                }
            }
            return ++amount;
        }


        AsyncWebServer server{80};
        uint8_t buffer[1024]{};
        uint32_t input_value[25]{};
        uint16_t buf_size = 0;
        Group_control_module *gcm=nullptr;

        // std::array<uint8_t, scs::AMT_BYTES_ID> get_system_id() const;

        void init(Group_control_module &set_gcm, bool spiffs_init) {
            gcm = &set_gcm;


#if defined( START_WIFI_CONNECT )
            WiFi.onEvent(WiFiEvent); // задает то. что при подключении к wi-fi будет
            connectToWifi();         // запущена функция обратного вызова WiFiEvent(),
                                     // которая напечатает данные о WiFi подключении
#endif


            if(spiffs_init) {
                if (!SPIFFS.begin(true)) {
                    Serial.println("An Error has occurred while mounting SPIFFS");
                    //  "При монтировании SPIFFS произошла ошибка"
                    return;
                }
            }
            server.on("/",      HTTP_GET,  login_page);
            server.on("/login", HTTP_POST, [](AsyncWebServerRequest *request) {},
                      NULL, login_connection);
            server.on("/devices/unregistered/get", HTTP_GET, modules_unregistered);
            server.on("/devices/registered/get",   HTTP_GET, modules_registered);
            server.on("/registered_device/get",    HTTP_GET, module_registered_data);
            server.on("/unregistered_device/get",  HTTP_GET, module_unregistered_data);
            server.on("/registered_device/add",    HTTP_GET, module_registration);
            server.on("/registered_device/edit",   HTTP_GET, module_editing);
            server.on("/registered_device/delete", HTTP_POST, [](AsyncWebServerRequest *request) {},
                      NULL, module_removing);
            server.on("/change_device_mode",   HTTP_GET, module_changing_mode);
            server.on("/system_settings/get",  HTTP_GET, mgtt_using);
            server.on("/system_settings/edit", HTTP_GET, mgtt_edit);
            server.on("/system_time/get",      HTTP_GET, system_time_get);
            server.on("/system_time/edit",     HTTP_GET, system_time_edit);
            server.on("/sensor_device/get",    HTTP_GET, module_sensor_data);

            server.on("/refresh",              HTTP_GET, refresh_icon);
            server.on("/plus_icon",            HTTP_GET, plus_icon);
            server.on("/delete",               HTTP_GET, delete_icon);

            // server.on("/refresh.png",              HTTP_GET, refresh_icon);
            // server.on("/plus-icon.png",            HTTP_GET, plus_icon);
            // server.on("/delete.png",               HTTP_GET, delete_icon);

            server.on("/settings",             HTTP_GET, main_page);
            server.on("/style.css",            HTTP_GET, style);

            server.begin();
        }


#ifndef TEST_CODE_15_10_2021

        // Страница для входа | "/" ("login_page.html")
        void login_page(AsyncWebServerRequest *request) {
            request->send(SPIFFS, "/login_page.html", String(), false);
        }
        // Процесс входа | "/login"
        void login_connection(AsyncWebServerRequest *request, uint8_t *data,
                              size_t len, size_t index, size_t total) {
            bool correct_id = false;
            buf_size = 0;
            if(len == scs::AMT_BYTES_ID * 2) {
                std::array<uint8_t, scs::AMT_BYTES_ID> system_id = gcm->get_system_id();
                bool error_id = hex_to_byte(data, buffer, len);
                // for (size_t i = 0; i < len; i++) {
                //     uint8_t num;
                //     if(('A' <= data[i]) && (data[i] <= 'F')) {
                //         num = data[i] - 'A' + 10;
                //     }
                //     else if(('a' <= data[i]) && (data[i] <= 'f')) {
                //         num = data[i] - 'a' + 10;
                //     } 
                //     else if(('0' <= data[i]) && (data[i] <= '9')) {
                //         num = data[i] - '0';
                //     }
                //     else {
                //         error_id = true;
                //         break;
                //     }
                //     if(i%2 == 0) {
                //         buffer[i/2] = num << 4;
                //     }
                //     else {
                //         buffer[i/2] |= num;
                //     }
                //     // buffer[i] = data[i];
                // }
                if(!error_id) {

#if defined(SERIAL_PRINT_1)
                    Serial.print("ID+{");
                    for (size_t i = 0; i < scs::AMT_BYTES_ID; i++) {
                        Serial.print(buffer[i], HEX);
                    }
#endif
                    correct_id = true;
                    for(size_t i = 0; i < scs::AMT_BYTES_ID; i++) {
                        if(buffer[i] != system_id[i])
                            correct_id = false;
                    }
#if defined(SERIAL_PRINT_1)
                    if(correct_id) {
                        Serial.print(" == ");
                    }
                    else {
                        Serial.print(" != ");
                    }

                    for (size_t i = 0; i < scs::AMT_BYTES_ID; i++) {
                        Serial.print(system_id[i], HEX);
                    }
                    Serial.println("}");
#endif
                }
            }
#if defined(SERIAL_PRINT_1)
            if(!correct_id) {
                Serial.print("ID-{");
                for (size_t i = 0; i < len; i++) {
                    Serial.print((char)data[i]);
                }
                Serial.print(" != ");
                for (size_t i = 0; i < scs::AMT_BYTES_ID; i++) {
                    Serial.print(gcm->get_system_id()[i], HEX);
                }
                Serial.println("}");
            }
#endif
            
            if(correct_id) {
                AsyncWebServerResponse *response = request->beginResponse(
                    301); //код переадресации
                response->addHeader("Access-Control-Allow-Origin",
                    "*"); // заголовок, позволяющий делать запросы в одном домене
                response->addHeader("Location", "/settings"); // место переадресации
                request->send(response);
            }
            else {
                AsyncWebServerResponse *response = request->beginResponse(
                    401); // код неавторизированного пользователя
                response->addHeader("Access-Control-Allow-Origin", "*");
                request->send(response);
            }

            // String a;
            // for (size_t i = 0; i < len; i++) {
            //     a += (char)data[i];
            // }
            // Serial.println(a);
            // if (a == esp32_id) { //проверка ввода id esp32
            // } else {
            // }
            // AsyncWebServerResponse *response =
            // request->beginResponse(301); //код переадресации
            // //          response->addHeader("Access-Control-Allow-Origin", "*"); //
            // //          заголовок, позволяющий делать запросы в одном домене
            // response->addHeader("Location", "/settings"); // место переадресации
            // request->send(response);
        }

        // Список незарегестрированных модулей | "/devices/unregistered/get"
        void modules_unregistered(AsyncWebServerRequest *request) {
            AsyncWebServerResponse *response =
                request->beginResponse(200, "text/plain", "Ok");
            response->addHeader("Access-Control-Allow-Origin", "*");
            response->addHeader("Access-Control-Expose-Headers", "*");
            // Получение количества
            uint16_t amt{0};
            buf_size = 0;
            if(gcm != nullptr)
                amt = gcm->reg_devices_.size() + gcm->reg_sensors_.size();
            add_number(amt, buffer, buf_size); // Количество компонентов
            buffer[buf_size++] = '\0';
            // Отправка количества
            response->addHeader("count_of_devices", (char*)buffer);
            // Получение ID
            buf_size = 0;
            if(gcm != nullptr) {
                for(int i = 0; i < gcm->reg_devices_.size(); ++i) {
                    add_module_id(&gcm->reg_devices_[i], buffer, buf_size, true);
                }
                for(int i = 0; i < gcm->reg_sensors_.size(); ++i) {
                    add_module_id(&gcm->reg_sensors_[i], buffer, buf_size, true);
                }
                // std::array<uint8_t, scs::AMT_BYTES_ID> id;
                // for(int i = 0; i < gcm->reg_devices_.size(); ++i) {
                //     id = gcm->reg_devices_[i].get_system_id();
                //     buf_size += byte_to_hex(&id[0], &buffer[buf_size], scs::AMT_BYTES_ID);
                //     buffer[buf_size++] = ',';
                // }
                // for(int i = 0; i < gcm->reg_sensors_.size(); ++i) {
                //     id = gcm->reg_sensors_[i].get_system_id();
                //     buf_size += byte_to_hex(&id[0], &buffer[buf_size], scs::AMT_BYTES_ID);
                //     buffer[buf_size++] = ',';
                // }
                buffer[--buf_size] = '\0';
            }
            response->addHeader("device_ids", (char*)buffer);
            // response->addHeader("device_ids", "1,2");
            request->send(response);

#if defined( SERIAL_PRINT_REG_MODULES )
            Serial.println("/devices/unregistered/get");
            Serial.print("[");
            Serial.print(amt);
            Serial.print("] = {");
            Serial.print((char*)buffer);
            Serial.println("}");
#endif
            // AsyncWebServerResponse *response =
            //     request->beginResponse(200, "text/plain", "Ok");
            // response->addHeader("Access-Control-Allow-Origin", "*");
            // response->addHeader("Access-Control-Expose-Headers", "*");
            // response->addHeader("count_of_devices", "2");
            // response->addHeader("device_ids", "1,2");
            // response->addHeader("device_types", "sensor, lamp");
            // request->send(response);
        }
        // Список зарегестрированных модулей | "/devices/registered/get"
        void modules_registered(AsyncWebServerRequest *request) {
            AsyncWebServerResponse *response =
                request->beginResponse(200, "text/plain", "Ok");
            response->addHeader("Access-Control-Allow-Origin", "*");
            response->addHeader("Access-Control-Expose-Headers", "*");
            // Получение количества
            uint16_t amt{0};
            buf_size = 0;
            if(gcm != nullptr)
                amt = gcm->devices_.size() + gcm->sensors_.size();
            add_number(amt, buffer, buf_size); // Количество компонентов
            buffer[buf_size++] = '\0';
            // Отправка количества
            response->addHeader("count_of_devices", (char*)buffer);
            // Получение ID
            buf_size = 0;
            if(gcm != nullptr) {
                
                for(int i = 0; i < gcm->devices_.size(); ++i) {
                    add_module_id(&gcm->devices_[i], buffer, buf_size, true);
                }
                for(int i = 0; i < gcm->sensors_.size(); ++i) {
                    add_module_id(&gcm->sensors_[i], buffer, buf_size, true);
                }
                // std::array<uint8_t, scs::AMT_BYTES_ID> id;
                // for(int i = 0; i < gcm->devices_.size(); ++i) {
                //     id = gcm->devices_[i].get_system_id();
                //     buf_size += byte_to_hex(&id[0], &buffer[buf_size], scs::AMT_BYTES_ID);
                //     buffer[buf_size++] = ',';
                // }
                // for(int i = 0; i < gcm->sensors_.size(); ++i) {
                //     id = gcm->sensors_[i].get_system_id();
                //     buf_size += byte_to_hex(&id[0], &buffer[buf_size], scs::AMT_BYTES_ID);
                //     buffer[buf_size++] = ',';
                // }
                buffer[--buf_size] = '\0';
            }
            response->addHeader("device_ids", (char*)buffer);

#if defined( SERIAL_PRINT_MODULES )
            Serial.println("/devices/registered/get");
            Serial.print("ID[");
            Serial.print(amt);
            Serial.print("] = {");
            Serial.print((char*)buffer);
            Serial.println("}");
#endif
            // Получение имён
            buf_size = 0;
            if(gcm != nullptr) {
                for(int i = 0; i < gcm->devices_.size(); ++i) {
                    add_module_name(&gcm->devices_[i], buffer, buf_size, true);
                }
                for(int i = 0; i < gcm->sensors_.size(); ++i) {
                    add_module_name(&gcm->sensors_[i], buffer, buf_size, true);
                }


                // const std::array<uint8_t, scs::AMT_BYTES_NAME> *name;
                // for(int i = 0; i < gcm->devices_.size(); ++i) {
                //     name = &gcm->devices_[i].get_name();
                //     for(int j = 0; j < gcm->devices_[i].get_len_name(); ++j) {
                //         if((*name)[j] == '\0') {
                //             break;
                //         }
                //         buffer[buf_size++] = (*name)[j];
                //     }
                //     buffer[buf_size++] = ',';
                // }
                // for(int i = 0; i < gcm->sensors_.size(); ++i) {
                //     name = &gcm->sensors_[i].get_name();
                //     for(int j = 0; j < gcm->sensors_[i].get_len_name(); ++j) {
                //         if((*name)[j] == '\0') {
                //             break;
                //         }
                //         buffer[buf_size++] = (*name)[j];
                //     }
                //     buffer[buf_size++] = ',';
                // }
                buffer[--buf_size] = '\0';
            }
            response->addHeader("device_names", (char*)buffer);
            // uint8_t System_component::get_name(uint8_t num) const
            // uint8_t System_component::get_len_name() const
            // response->addHeader("device_names", "dim_lamps, nasosi");
            request->send(response);


#if defined( SERIAL_PRINT_MODULES )
            Serial.print("Name[");
            Serial.print(amt);
            Serial.print("] = {");
            Serial.print((char*)buffer);
            Serial.println("}");
#endif


            // AsyncWebServerResponse *response =
            //     request->beginResponse(200, "text/plain", "Ok");
            // response->addHeader("Access-Control-Allow-Origin", "*");
            // response->addHeader("Access-Control-Expose-Headers", "*");
            // response->addHeader("count_of_devices", "2");
            // response->addHeader("device_ids", "3,4");
            // response->addHeader("device_names", "dim_lamps, nasosi");
            // request->send(response);
        }

        // Отправка информации о исп. модуле | "/registered_device/get"
        void module_registered_data(AsyncWebServerRequest *request) { // (?) -----
            static std::array<uint8_t, scs::AMT_BYTES_ID> id;
            static const scs::System_component* module;
            static const Grow_device* m_device;
            static const Grow_sensor* m_sensor;
            static bool device = false;
            static uint16_t amt_components;
            static int value;

            Serial.println("/registered_device/get");
            int paramsNr = request->params();
            module = nullptr;
            // Serial.println(paramsNr);
            for (int i = 0; i < paramsNr; i++) {
                AsyncWebParameter *p = request->getParam(i);
                // Serial.print("Param name: ");
                // Serial.println(p->name());
                // Serial.print("Param value: ");
                // Serial.println(p->value());
                // Serial.println("------");
                String param_name = p->name();
                if (param_name == "device_id") {
                    String device_id = p->value();
                    hex_to_byte((uint8_t*)&device_id[0], id);
                    
                    // for (size_t i = 0; i < scs::AMT_BYTES_ID; i++) {
                    //     Serial.print(id[i], HEX);
                    // }

                    if(gcm != nullptr) {
                        int number = gcm->search_device(id);
                        if(number == -1) {
                            number = gcm->search_sensor(id);
                            if(number != -1) {
                                module = &gcm->sensors_[number];

                                m_sensor = &gcm->sensors_[number];
                                device = false;
                            }
                        }
                        else {
                            module = &gcm->devices_[number];
                            
                            m_device = &gcm->devices_[number];
                            device = true;
                        }
                        // Serial.print(" - ");
                        // Serial.println(number);
                    }
                }
            }
            // somefunction(p->value) какая то функция, которая получает информацию
            // о модуле
            AsyncWebServerResponse *response =
                request->beginResponse(200, "text/plain", "Ok");
            response->addHeader("Access-Control-Allow-Origin", "*");
            response->addHeader("Access-Control-Expose-Headers", "*");
            if(module != nullptr) {
                buf_size = 0;
                add_module_name(module, buffer, buf_size);
                buffer[buf_size++] = '\0';
                response->addHeader("device_name", (char*)buffer);

#if defined(SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA)
                Serial.print("device_name: ");
                Serial.println((char*)buffer);
#endif

                if(!device) {
                    // ----- sensors -----
                    // Добавление количества компонентов
                    buf_size = 0;
                    amt_components = m_sensor->get_count_component();
                    add_number(amt_components, buffer, buf_size);
                    buffer[buf_size++] = '\0';
                    response->addHeader("count_of_sensor_modules", (char*)buffer);

#if defined(SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA)
                Serial.print("count_of_sensor_modules: ");
                Serial.println((char*)buffer);
#endif

                    // Добавление типов модулей
                    buf_size = 0;
                    for(int i = 0; i < amt_components; ++i) {
                        value = m_sensor->get_component(i).get_type();
                        add_number(value, buffer, buf_size);
                        buffer[buf_size++] = ',';
                    }
                    buffer[--buf_size] = '\0';
                    response->addHeader("sensors_names", (char*)buffer);

#if defined(SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA)
                Serial.print("sensors_names: ");
                Serial.println((char*)buffer);
#endif
                    // Добавление id модулей
                    buf_size = 0;
                    for(int i = 0; i < amt_components; ++i) {
                        value = m_sensor->get_component(i).get_id();
                        add_number(value, buffer, buf_size);
                        buffer[buf_size++] = ',';
                    }
                    buffer[--buf_size] = '\0';
                    response->addHeader("sensors_ids", (char*)buffer);
#if defined(SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA)
                Serial.print("sensors_ids: ");
                Serial.println((char*)buffer);
#endif
                    // Добавление значений
                    buf_size = 0;
                    for(int i = 0; i < amt_components; ++i) {
                        value = m_sensor->get_component(i).get_value();
                        add_number(value, buffer, buf_size, true);
                        buffer[buf_size++] = '.';
                        value = m_sensor->get_component(i).get_value() * 1000;
                        value %= 1000;
                        add_number(value, buffer, buf_size);
                        buffer[buf_size++] = ',';
                    }
                    buffer[--buf_size] = '\0';
                    response->addHeader("sensor_indications", (char*)buffer);

#if defined(SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA)
                Serial.print("sensor_indications: ");
                Serial.println((char*)buffer);
#endif
                    // Добавление периода опроса
                    buf_size = 0;
                    value = m_sensor->get_period() / 1000;
                    if(false) {
                        add_number(value, buffer, buf_size);
                        buffer[buf_size++] = '\0';
                    }
                    else {
                        for(int i = 0; i < amt_components; ++i) {
                            add_number(value, buffer, buf_size);
                            buffer[buf_size++] = ',';
                        }
                        buffer[--buf_size] = '\0';
                    }
                    response->addHeader("sensor_values", (char*)buffer);

#if defined(SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA)
                Serial.print("sensor_values: ");
                Serial.println((char*)buffer);
#endif

                    // response->addHeader("count_of_sensor_modules",
                    //                     "2"); // кол-во сенсоров, подключенных к модулю
                    // response->addHeader("sensors_names",
                    //                     "h2o, co2"); // название модулей сенсоров
                    // response->addHeader("sensors_ids", "1, 2"); // название модулей сенсоров
                    // response->addHeader("sensor_indications",
                    //                     "0.333,0.444"); // последние показания датчиков
                    // response->addHeader("sensor_values",
                    //                     "1,120"); //Тут не знаю как правильнее 
                    //                             //сделать подстрою под вас

                    // ----- devices -----
                    response->addHeader("count_of_device_modules", "0");
                    response->addHeader("count_of_cycles_in_device_moduls", "");
                    response->addHeader("devices_names", "");
                    response->addHeader("mech_devices_ids", "");
                    response->addHeader("pwm_powers", "");
                    response->addHeader("work_mode", "auto,manual");
                    response->addHeader("status", "");
                    response->addHeader("manual_value", "");
                    response->addHeader("cycle_begin", "");
                    response->addHeader("cycle_end", "");
                    response->addHeader("turn_on", "");
                    response->addHeader("turn_off", "");
                }
                else {
                    // ----- sensors -----
                    response->addHeader("count_of_sensor_modules", "0");
                    response->addHeader("sensors_names", "");
                    response->addHeader("sensors_ids", "");
                    response->addHeader("sensor_indications", "");
                    response->addHeader("sensor_values", "");
                    // ----- devices -----
                    // Добавление количества компонентов
                    buf_size = 0;
                    amt_components = m_device->get_count_component();
                    add_number(amt_components, buffer, buf_size);
                    buffer[buf_size++] = '\0';
                    response->addHeader("count_of_device_modules", (char*)buffer);

#if defined(SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA)
                    Serial.print("count_of_device_modules: ");
                    Serial.println((char*)buffer);
#endif
                    // Добавление количества циклов
                    buf_size = 0;
                    for(int i = 0; i < amt_components; ++i) {
                        value = m_device->get_component(i).get_timer().size();
                        add_number(value, buffer, buf_size);
                        buffer[buf_size++] = ',';
                    }
                    buffer[--buf_size] = '\0';
                    response->addHeader("count_of_cycles_in_device_moduls", (char*)buffer);
#if defined(SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA)
                    Serial.print("count_of_cycles_in_device_moduls: ");
                    Serial.println((char*)buffer);
#endif
                    // // Добавление количества циклов
                    // response->addHeader("count_of_cycles_in_device_moduls",
                    //                     "3, 2"); // количество циклов ШИМ


                    // Добавление типов модулей
                    buf_size = 0;
                    for(int i = 0; i < amt_components; ++i) {
                        value = m_device->get_component(i).get_type();
                        add_number(value, buffer, buf_size);
                        buffer[buf_size++] = ',';
                    }
                    buffer[--buf_size] = '\0';
                    response->addHeader("devices_names", (char*)buffer);
#if defined(SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA)
                    Serial.print("devices_names: ");
                    Serial.println((char*)buffer);
#endif

                    buf_size = 0;
                    for(int i = 0; i < amt_components; ++i) {
                        value = m_device->get_component(i).get_id();
                        add_number(value, buffer, buf_size);
                        buffer[buf_size++] = ',';
                    }
                    buffer[--buf_size] = '\0';
                    response->addHeader("mech_devices_ids", (char*)buffer); // (-) ----- удалить
#if defined(SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA)
                    Serial.print("mech_devices_ids: ");
                    Serial.println((char*)buffer);
#endif
                    // // Добавление типов модулей
                    // response->addHeader("devices_names",
                    //                     "lamp, nasos"); // название модулей сенсоров
                    // response->addHeader("mech_devices_ids",
                    //                     "3, 4"); // название модулей сенсоров




                    // Добавление значений
                    buf_size = 0;
                    for(int i = 0; i < amt_components; ++i) {
                        for(int j = 0; j < m_device->get_component(i).get_timer().size(); ++j) {
                            value = m_device->get_component(i).get_timer()[j].get_send_value();
                            add_number(value, buffer, buf_size);
                            buffer[buf_size++] = ',';
                        }
                        buffer[--buf_size] = ';'; ++buf_size;
                    }
                    buffer[--buf_size] = '\0';
                    response->addHeader("pwm_powers", (char*)buffer);
#if defined(SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA)
                    Serial.print("pwm_powers: ");
                    Serial.println((char*)buffer);
#endif
                    // response->addHeader(
                    //     "pwm_powers", "10,20,30;10,5;10,5"); //значения ШИМ разделитель внутри
                    //                                     //каждого устройства ',' разделитель
                    //                                     //между устройствами ';'
                    
                    // Добавление периода опроса
                    static const char MODE_WORK[3][7] = {"auto", "manual", ""};
                    static const char MODE_WORK_LEN[3] = {4, 6, 0};
                    static const char STATUS_WORK[3][7] = {"on", "off", ""};
                    static const char STATUS_WORK_LEN[3] = {2, 3, 0};
                    static uint8_t number_work;

                    buf_size = 0;
                    for(int i = 0; i < amt_components; ++i) {
                        if(m_device->get_component(i).get_work_mode() == Work_mode::Auto) {
                            number_work = 0;
                        }
                        else if(m_device->get_component(i).get_work_mode() == Work_mode::Manual) {
                            number_work = 1;
                        }
                        else {
                            number_work = 2;
                            Serial.println("Ошибка: Неизместный get_work_mode()!");
                        }

                        for(int j = 0; j < MODE_WORK_LEN[number_work]; ++j) {
                            buffer[buf_size++] = MODE_WORK[number_work][j];
                        }
                        buffer[buf_size++] = ',';
                    }
                    buffer[--buf_size] = '\0';
                    response->addHeader("work_mode", (char*)buffer);
#if defined(SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA)
                    Serial.print("work_mode: ");
                    Serial.println((char*)buffer);
#endif

                    // response->addHeader("work_mode", "auto,manual,auto"); // режимы работы
                    buf_size = 0;
                    for(int i = 0; i < amt_components; ++i) {
                        if(m_device->get_component(i).get_work_mode() == Work_mode::Auto) {
                            number_work = 0;
                        }
                        else if(m_device->get_component(i).get_work_mode() == Work_mode::Manual) {
                            if(m_device->get_component(i).get_manual_work_state() == Manual_work_state::On) {
                                number_work = 0;
                            }
                            else if(m_device->get_component(i).get_manual_work_state() == Manual_work_state::Off) {
                                number_work = 1;
                            }
                            else {
                                number_work = 2;
                                Serial.println("Ошибка: Неизместный get_manual_work_state()!");
                            }
                        }
                        else {
                            number_work = 2;
                            Serial.println("Ошибка: Неизместный get_work_mode()!");
                        }

                        for(int j = 0; j < STATUS_WORK_LEN[number_work]; ++j) {
                            buffer[buf_size++] = STATUS_WORK[number_work][j];
                        }
                        buffer[buf_size++] = ',';
                    }
                    buffer[--buf_size] = '\0';
                    response->addHeader("status", (char*)buffer);
#if defined(SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA)
                    Serial.print("status: ");
                    Serial.println((char*)buffer);
#endif

                    // response->addHeader(
                    //     "status",
                    //     "on,on,on"); // Это для режимов работы если авто, то всегда включен,
                    //             // если ручной может быть выключен или включен


                    // Добавление значений
                    buf_size = 0;
                    for(int i = 0; i < amt_components; ++i) {
                        value = m_device->get_component(i).get_send_server_value();
                        add_number(value, buffer, buf_size);
                        buffer[buf_size++] = ',';
                    }
                    buffer[--buf_size] = '\0';
                    response->addHeader("manual_value", (char*)buffer);
#if defined(SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA)
                    Serial.print("manual_value: ");
                    Serial.println((char*)buffer);
#endif
                    // response->addHeader("manual_value",
                    //                     "0,10,10"); // Для авто режима значения ШИМ нам
                    //                             // всеравно, для ручного - значение


                    // Добавление характеристик периодов:
                    buf_size = 0;
                    buffer[buf_size] = '\0';
                    for(int i = 0; i < amt_components; ++i) {
                        for(int j = 0; j < m_device->get_component(i).get_timer().size(); ++j) {
                            value = m_device->get_component(i).get_timer()[j].get_start_hours();
                            if(value < 10)
                                add_number(0, buffer, buf_size);
                            add_number(value, buffer, buf_size);
                            buffer[buf_size++] = ':';
                            value = m_device->get_component(i).get_timer()[j].get_start_minutes();
                            if(value < 10)
                                add_number(0, buffer, buf_size);
                            add_number(value, buffer, buf_size);
                            buffer[buf_size++] = ',';
                        }
                        if(m_device->get_component(i).get_timer().size() != 0) {
                            buffer[--buf_size] = ';'; ++buf_size;
                        }
                        else
                            buffer[buf_size++] = ';';
                    }
                    if(buf_size != 0)
                        buffer[--buf_size] = '\0';
                    response->addHeader("cycle_begin", (char*)buffer);
#if defined(SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA)
                    Serial.print("cycle_begin: ");
                    Serial.println((char*)buffer);
#endif
                    // response->addHeader(
                    //     "cycle_begin",
                    //     "21:15,12:20,05:00;10:00,20:00;10:00,20:00"); // начало цикла ШИМ. Тут не знаю
                    //                                     // как правильнее сделать подстрою
                    //                                     // под вас
                    buf_size = 0;
                    for(int i = 0; i < amt_components; ++i) {
                        for(int j = 0; j < m_device->get_component(i).get_timer().size(); ++j) {
                            value = m_device->get_component(i).get_timer()[j].get_end_hours();
                            if(value < 10)
                                add_number(0, buffer, buf_size);
                            add_number(value, buffer, buf_size);
                            buffer[buf_size++] = ':';
                            value = m_device->get_component(i).get_timer()[j].get_end_minutes();
                            if(value < 10)
                                add_number(0, buffer, buf_size);
                            add_number(value, buffer, buf_size);
                            buffer[buf_size++] = ',';
                        }
                        if(m_device->get_component(i).get_timer().size() != 0) {
                            buffer[--buf_size] = ';'; ++buf_size;
                        }
                        else
                            buffer[buf_size++] = ';';
                    }
                    if(buf_size != 0)
                        buffer[--buf_size] = '\0';
                    response->addHeader("cycle_end", (char*)buffer);
#if defined(SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA)
                    Serial.print("cycle_end: ");
                    Serial.println((char*)buffer);
#endif
                    // response->addHeader(
                    //     "cycle_end",
                    //     "21:15,12:20,05:00;10:00,20:00;10:00,20:00"); //конец цикла ШИМ. Тут не знаю как
                    //                                     //правильнее сделать подстрою под
                    //                                     //вас
                    buf_size = 0;
                    for(int i = 0; i < amt_components; ++i) {
                        for(int j = 0; j < m_device->get_component(i).get_timer().size(); ++j) {
                            value = m_device->get_component(i).get_timer()[j].get_channel().get_duration_on();
                            add_number(value, buffer, buf_size);
                            buffer[buf_size++] = ',';
                        }
                        buffer[--buf_size] = ';'; ++buf_size;
                    }
                    buffer[--buf_size] = '\0';
                    response->addHeader("turn_on", (char*)buffer);
#if defined(SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA)
                    Serial.print("turn_on: ");
                    Serial.println((char*)buffer);
#endif
                    // response->addHeader(
                    //     "turn_on", "10,1200,1800;36000,36000;7200,7200"); //Тут не знаю как правильнее
                    //                                             //сделать подстрою под вас
                    buf_size = 0;
                    for(int i = 0; i < amt_components; ++i) {
                        for(int j = 0; j < m_device->get_component(i).get_timer().size(); ++j) {
                            value = m_device->get_component(i).get_timer()[j].get_channel().get_duration_off();
                            add_number(value, buffer, buf_size);
                            buffer[buf_size++] = ',';
                        }
                        buffer[--buf_size] = ';'; ++buf_size;
                    }
                    buffer[--buf_size] = '\0';
                    response->addHeader("turn_off", (char*)buffer);
#if defined(SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA)
                    Serial.print("turn_off: ");
                    Serial.println((char*)buffer);
#endif
                    // response->addHeader("turn_off",
                    //                     "3,300,600;7200,7200;7200,7200"); //Тут не знаю как правильнее
                    //                                             //сделать подстрою под вас






                    // response->addHeader("count_of_device_modules",
                    //                     "2"); // Это тип насосы дим лампы и т д
                    // response->addHeader("count_of_cycles_in_device_moduls",
                    //                     "3, 2"); // количество циклов ШИМ
                    // response->addHeader("devices_names",
                    //                     "lamp, nasos"); // название модулей сенсоров
                    // response->addHeader("mech_devices_ids",
                    //                     "3, 4"); // название модулей сенсоров
                    // response->addHeader(
                    //     "pwm_powers", "10,20,30;10,5"); //значения ШИМ разделитель внутри
                    //                                     //каждого устройства ',' разделитель
                    //                                     //между устройствами ';'
                    // response->addHeader("work_mode", "auto,manual"); // режимы работы
                    // response->addHeader(
                    //     "status",
                    //     "on,on"); // Это для режимов работы если авто, то всегда включен,
                    //             // если ручной может быть выключен или включен
                    // response->addHeader("manual_value",
                    //                     "0,10"); // Для авто режима значения ШИМ нам
                    //                             // всеравно, для ручного - значение
                    // response->addHeader(
                    //     "cycle_begin",
                    //     "21:15,12:20,05:00;10:00,20:00"); // начало цикла ШИМ. Тут не знаю
                    //                                     // как правильнее сделать подстрою
                    //                                     // под вас
                    // response->addHeader(
                    //     "cycle_end",
                    //     "21:15,12:20,05:00;10:00,20:00"); //конец цикла ШИМ. Тут не знаю как
                    //                                     //правильнее сделать подстрою под
                    //                                     //вас
                    // response->addHeader(
                    //     "turn_on", "10,1200,1800;36000,36000"); //Тут не знаю как правильнее
                    //                                             //сделать подстрою под вас
                    // response->addHeader("turn_off",
                    //                     "3,300,600;7200,7200"); //Тут не знаю как правильнее
                    //                                             //сделать подстрою под вас
                }
            
            }

            request->send(response);
            // Serial.println("get_info_registered_device");
            // int paramsNr = request->params();
            // Serial.println(paramsNr);
            // for (int i = 0; i < paramsNr; i++) {
            //     AsyncWebParameter *p = request->getParam(i);
            //     Serial.print("Param name: ");
            //     Serial.println(p->name());
            //     Serial.print("Param value: ");
            //     Serial.println(p->value());
            //     Serial.println("------");
            //     String param_name = p->name();
            //     if (param_name == "device_id") {
            //         String device_id = p->value();
            //         Serial.println(device_id);
            //     }
            // }
            // // somefunction(p->value) какая то функция, которая получает информацию
            // // о модуле
            // AsyncWebServerResponse *response =
            //     request->beginResponse(200, "text/plain", "Ok");
            // response->addHeader("Access-Control-Allow-Origin", "*");
            // response->addHeader("Access-Control-Expose-Headers", "*");
            // response->addHeader("device_name",
            //                     "Device"); // название модуля (я так понимаю stmки)
            // response->addHeader("count_of_sensor_modules",
            //                     "2"); // кол-во сенсоров, подключенных к модулю
            // response->addHeader("sensors_names",
            //                     "h2o, co2"); // название модулей сенсоров
            // response->addHeader("sensors_ids", "1, 2"); // название модулей сенсоров
            // response->addHeader("sensor_indications",
            //                     "0.333,0.444"); // последние показания датчиков
            // response->addHeader("sensor_values",
            //                     "1,120"); //Тут не знаю как правильнее 
            //                               //сделать подстрою под вас
            // response->addHeader("count_of_device_modules",
            //                     "2"); // Это тип насосы дим лампы и т д
            // response->addHeader("count_of_cycles_in_device_moduls",
            //                     "3, 2"); // количество циклов ШИМ
            // response->addHeader("devices_names",
            //                     "lamp, nasos"); // название модулей сенсоров
            // response->addHeader("mech_devices_ids",
            //                     "3, 4"); // название модулей сенсоров
            // response->addHeader(
            //     "pwm_powers", "10,20,30;10,5"); //значения ШИМ разделитель внутри
            //                                     //каждого устройства ',' разделитель
            //                                     //между устройствами ';'
            // response->addHeader("work_mode", "auto,manual"); // режимы работы
            // response->addHeader(
            //     "status",
            //     "on,on"); // Это для режимов работы если авто, то всегда включен,
            //               // если ручной может быть выключен или включен
            // response->addHeader("manual_value",
            //                     "0,10"); // Для авто режима значения ШИМ нам
            //                              // всеравно, для ручного - значение
            // response->addHeader(
            //     "cycle_begin",
            //     "21:15,12:20,05:00;10:00,20:00"); // начало цикла ШИМ. Тут не знаю
            //                                     // как правильнее сделать подстрою
            //                                     // под вас
            // response->addHeader(
            //     "cycle_end",
            //     "21:15,12:20,05:00;10:00,20:00"); //конец цикла ШИМ. Тут не знаю как
            //                                     //правильнее сделать подстрою под
            //                                     //вас
            // response->addHeader(
            //     "turn_on", "10,1200,1800;36000,36000"); //Тут не знаю как правильнее
            //                                             //сделать подстрою под вас
            // response->addHeader("turn_off",
            //                     "3,300,600;7200,7200"); //Тут не знаю как правильнее
            //                                             //сделать подстрою под вас
            // request->send(response);
        }
        
        // Отправка информации о рег. модуле | "/unregistered_device/get"
        void module_unregistered_data(AsyncWebServerRequest *request) { // - (-) -----
            Serial.println("- (-) ----- \"/unregistered_device/get\"");
            if(0) {
            static std::array<uint8_t, scs::AMT_BYTES_ID> id;
            static const scs::System_component* module;
            static const Grow_device* m_device;
            static const Grow_sensor* m_sensor;
            static bool device = false;
            static uint16_t amt_components;
            static int value;

            Serial.println("/registered_device/get");
            int paramsNr = request->params();
            module = nullptr;
            // Serial.println(paramsNr);
            for (int i = 0; i < paramsNr; i++) {
                AsyncWebParameter *p = request->getParam(i);
                // Serial.print("Param name: ");
                // Serial.println(p->name());
                // Serial.print("Param value: ");
                // Serial.println(p->value());
                // Serial.println("------");
                String param_name = p->name();
                if (param_name == "device_id") {
                    String device_id = p->value();
                    hex_to_byte((uint8_t*)&device_id[0], id);
                    
                    // for (size_t i = 0; i < scs::AMT_BYTES_ID; i++) {
                    //     Serial.print(id[i], HEX);
                    // }

                    if(gcm != nullptr) {
                        int number = gcm->search_device(id);
                        if(number == -1) {
                            number = gcm->search_sensor(id);
                            if(number != -1) {
                                module = &gcm->sensors_[number];

                                m_sensor = &gcm->sensors_[number];
                                device = false;
                            }
                        }
                        else {
                            module = &gcm->devices_[number];
                            
                            m_device = &gcm->devices_[number];
                            device = true;
                        }
                        // Serial.print(" - ");
                        // Serial.println(number);
                    }
                }
            }
            // somefunction(p->value) какая то функция, которая получает информацию
            // о модуле
            AsyncWebServerResponse *response =
                request->beginResponse(200, "text/plain", "Ok");
            response->addHeader("Access-Control-Allow-Origin", "*");
            response->addHeader("Access-Control-Expose-Headers", "*");
            if(module != nullptr) {
                buf_size = 0;
                add_module_name(module, buffer, buf_size);
                buffer[buf_size++] = '\0';
                response->addHeader("device_name", (char*)buffer);

#if defined(SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA)
                Serial.print("device_name: ");
                Serial.println((char*)buffer);
#endif

                if(!device) {
                    // ----- sensors -----
                    // Добавление количества компонентов
                    buf_size = 0;
                    amt_components = m_sensor->get_count_component();
                    add_number(amt_components, buffer, buf_size);
                    buffer[buf_size++] = '\0';
                    response->addHeader("count_of_sensor_modules", (char*)buffer);

#if defined(SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA)
                Serial.print("count_of_sensor_modules: ");
                Serial.println((char*)buffer);
#endif

                    // Добавление типов модулей
                    buf_size = 0;
                    for(int i = 0; i < amt_components; ++i) {
                        value = m_sensor->get_component(i).get_type();
                        add_number(value, buffer, buf_size);
                        buffer[buf_size++] = ',';
                    }
                    buffer[--buf_size] = '\0';
                    response->addHeader("sensors_names", (char*)buffer);

#if defined(SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA)
                Serial.print("sensors_names: ");
                Serial.println((char*)buffer);
#endif
                    // Добавление id модулей
                    buf_size = 0;
                    for(int i = 0; i < amt_components; ++i) {
                        value = m_sensor->get_component(i).get_id();
                        add_number(value, buffer, buf_size);
                        buffer[buf_size++] = ',';
                    }
                    buffer[--buf_size] = '\0';
                    response->addHeader("sensors_ids", (char*)buffer);
#if defined(SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA)
                Serial.print("sensors_ids: ");
                Serial.println((char*)buffer);
#endif
                    // Добавление значений
                    buf_size = 0;
                    for(int i = 0; i < amt_components; ++i) {
                        value = m_sensor->get_component(i).get_value();
                        add_number(value, buffer, buf_size, true);
                        buffer[buf_size++] = '.';
                        value = m_sensor->get_component(i).get_value() * 1000;
                        value %= 1000;
                        add_number(value, buffer, buf_size);
                        buffer[buf_size++] = ',';
                    }
                    buffer[--buf_size] = '\0';
                    response->addHeader("sensor_indications", (char*)buffer);

#if defined(SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA)
                Serial.print("sensor_indications: ");
                Serial.println((char*)buffer);
#endif
                    // Добавление периода опроса
                    buf_size = 0;
                    value = m_sensor->get_period() / 1000;
                    if(false) {
                        add_number(value, buffer, buf_size);
                        buffer[buf_size++] = '\0';
                    }
                    else {
                        for(int i = 0; i < amt_components; ++i) {
                            add_number(value, buffer, buf_size);
                            buffer[buf_size++] = ',';
                        }
                        buffer[--buf_size] = '\0';
                    }
                    response->addHeader("sensor_values", (char*)buffer);

#if defined(SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA)
                Serial.print("sensor_values: ");
                Serial.println((char*)buffer);
#endif

                    // response->addHeader("count_of_sensor_modules",
                    //                     "2"); // кол-во сенсоров, подключенных к модулю
                    // response->addHeader("sensors_names",
                    //                     "h2o, co2"); // название модулей сенсоров
                    // response->addHeader("sensors_ids", "1, 2"); // название модулей сенсоров
                    // response->addHeader("sensor_indications",
                    //                     "0.333,0.444"); // последние показания датчиков
                    // response->addHeader("sensor_values",
                    //                     "1,120"); //Тут не знаю как правильнее 
                    //                             //сделать подстрою под вас

                    // ----- devices -----
                    response->addHeader("count_of_device_modules", "0");
                    response->addHeader("count_of_cycles_in_device_moduls", "");
                    response->addHeader("devices_names", "");
                    response->addHeader("mech_devices_ids", "");
                    response->addHeader("pwm_powers", "");
                    response->addHeader("work_mode", "auto,manual");
                    response->addHeader("status", "");
                    response->addHeader("manual_value", "");
                    response->addHeader("cycle_begin", "");
                    response->addHeader("cycle_end", "");
                    response->addHeader("turn_on", "");
                    response->addHeader("turn_off", "");
                }
                else {
                    // ----- sensors -----
                    response->addHeader("count_of_sensor_modules", "0");
                    response->addHeader("sensors_names", "-, -");
                    response->addHeader("sensors_ids", "0, 0");
                    response->addHeader("sensor_indications", "0, 0");
                    response->addHeader("sensor_values", "0, 0");
                    // ----- devices -----
                    // Добавление количества компонентов
                    buf_size = 0;
                    amt_components = m_device->get_count_component();
                    add_number(amt_components, buffer, buf_size);
                    buffer[buf_size++] = '\0';
                    response->addHeader("count_of_device_modules", (char*)buffer);

#if defined(SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA)
                    Serial.print("count_of_device_modules: ");
                    Serial.println((char*)buffer);
#endif
                    // Добавление количества циклов
                    buf_size = 0;
                    for(int i = 0; i < amt_components; ++i) {
                        value = m_device->get_component(i).get_timer().size();
                        add_number(value, buffer, buf_size);
                        buffer[buf_size++] = ',';
                    }
                    buffer[--buf_size] = '\0';
                    response->addHeader("count_of_cycles_in_device_moduls", (char*)buffer);
#if defined(SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA)
                    Serial.print("count_of_cycles_in_device_moduls: ");
                    Serial.println((char*)buffer);
#endif
                    // // Добавление количества циклов
                    // response->addHeader("count_of_cycles_in_device_moduls",
                    //                     "3, 2"); // количество циклов ШИМ


                    // Добавление типов модулей
                    buf_size = 0;
                    for(int i = 0; i < amt_components; ++i) {
                        value = m_device->get_component(i).get_type();
                        add_number(value, buffer, buf_size);
                        buffer[buf_size++] = ',';
                    }
                    buffer[--buf_size] = '\0';
                    response->addHeader("devices_names", (char*)buffer);
#if defined(SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA)
                    Serial.print("devices_names: ");
                    Serial.println((char*)buffer);
#endif

                    buf_size = 0;
                    for(int i = 0; i < amt_components; ++i) {
                        value = m_device->get_component(i).get_id();
                        add_number(value, buffer, buf_size);
                        buffer[buf_size++] = ',';
                    }
                    buffer[--buf_size] = '\0';
                    response->addHeader("mech_devices_ids", (char*)buffer); // (-) ----- удалить
#if defined(SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA)
                    Serial.print("mech_devices_ids: ");
                    Serial.println((char*)buffer);
#endif
                    // // Добавление типов модулей
                    // response->addHeader("devices_names",
                    //                     "lamp, nasos"); // название модулей сенсоров
                    // response->addHeader("mech_devices_ids",
                    //                     "3, 4"); // название модулей сенсоров




                    // Добавление значений
                    buf_size = 0;
                    for(int i = 0; i < amt_components; ++i) {
                        for(int j = 0; j < m_device->get_component(i).get_timer().size(); ++j) {
                            value = m_device->get_component(i).get_timer()[j].get_send_value();
                            add_number(value, buffer, buf_size);
                            buffer[buf_size++] = ',';
                        }
                        buffer[--buf_size] = ';'; ++buf_size;
                    }
                    buffer[--buf_size] = '\0';
                    response->addHeader("pwm_powers", (char*)buffer);
#if defined(SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA)
                    Serial.print("pwm_powers: ");
                    Serial.println((char*)buffer);
#endif
                    // response->addHeader(
                    //     "pwm_powers", "10,20,30;10,5;10,5"); //значения ШИМ разделитель внутри
                    //                                     //каждого устройства ',' разделитель
                    //                                     //между устройствами ';'
                    
                    // Добавление периода опроса
                    static const char MODE_WORK[3][7] = {"auto", "manual", ""};
                    static const char MODE_WORK_LEN[3] = {4, 6, 0};
                    static const char STATUS_WORK[3][7] = {"on", "off", ""};
                    static const char STATUS_WORK_LEN[3] = {2, 3, 0};
                    static uint8_t number_work;

                    buf_size = 0;
                    for(int i = 0; i < amt_components; ++i) {
                        if(m_device->get_component(i).get_work_mode() == Work_mode::Auto) {
                            number_work = 0;
                        }
                        else if(m_device->get_component(i).get_work_mode() == Work_mode::Manual) {
                            number_work = 1;
                        }
                        else {
                            number_work = 2;
                            Serial.println("Ошибка: Неизместный get_work_mode()!");
                        }

                        for(int j = 0; j < MODE_WORK_LEN[number_work]; ++j) {
                            buffer[buf_size++] = MODE_WORK[number_work][j];
                        }
                        buffer[buf_size++] = ',';
                    }
                    buffer[--buf_size] = '\0';
                    response->addHeader("work_mode", (char*)buffer);
#if defined(SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA)
                    Serial.print("work_mode: ");
                    Serial.println((char*)buffer);
#endif

                    // response->addHeader("work_mode", "auto,manual,auto"); // режимы работы
                    buf_size = 0;
                    for(int i = 0; i < amt_components; ++i) {
                        if(m_device->get_component(i).get_work_mode() == Work_mode::Auto) {
                            number_work = 0;
                        }
                        else if(m_device->get_component(i).get_work_mode() == Work_mode::Manual) {
                            if(m_device->get_component(i).get_manual_work_state() == Manual_work_state::On) {
                                number_work = 0;
                            }
                            else if(m_device->get_component(i).get_manual_work_state() == Manual_work_state::Off) {
                                number_work = 1;
                            }
                            else {
                                number_work = 2;
                                Serial.println("Ошибка: Неизместный get_manual_work_state()!");
                            }
                        }
                        else {
                            number_work = 2;
                            Serial.println("Ошибка: Неизместный get_work_mode()!");
                        }

                        for(int j = 0; j < STATUS_WORK_LEN[number_work]; ++j) {
                            buffer[buf_size++] = STATUS_WORK[number_work][j];
                        }
                        buffer[buf_size++] = ',';
                    }
                    buffer[--buf_size] = '\0';
                    response->addHeader("status", (char*)buffer);
#if defined(SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA)
                    Serial.print("status: ");
                    Serial.println((char*)buffer);
#endif

                    // response->addHeader(
                    //     "status",
                    //     "on,on,on"); // Это для режимов работы если авто, то всегда включен,
                    //             // если ручной может быть выключен или включен


                    // Добавление значений
                    buf_size = 0;
                    for(int i = 0; i < amt_components; ++i) {
                        value = m_device->get_component(i).get_send_server_value();
                        add_number(value, buffer, buf_size);
                        buffer[buf_size++] = ',';
                    }
                    buffer[--buf_size] = '\0';
                    response->addHeader("manual_value", (char*)buffer);
#if defined(SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA)
                    Serial.print("manual_value: ");
                    Serial.println((char*)buffer);
#endif
                    // response->addHeader("manual_value",
                    //                     "0,10,10"); // Для авто режима значения ШИМ нам
                    //                             // всеравно, для ручного - значение


                    // Добавление характеристик периодов:
                    buf_size = 0;
                    buffer[buf_size] = '\0';
                    for(int i = 0; i < amt_components; ++i) {
                        for(int j = 0; j < m_device->get_component(i).get_timer().size(); ++j) {
                            value = m_device->get_component(i).get_timer()[j].get_start_hours();
                            if(value < 10)
                                add_number(0, buffer, buf_size);
                            add_number(value, buffer, buf_size);
                            buffer[buf_size++] = ':';
                            value = m_device->get_component(i).get_timer()[j].get_start_minutes();
                            if(value < 10)
                                add_number(0, buffer, buf_size);
                            add_number(value, buffer, buf_size);
                            buffer[buf_size++] = ',';
                        }
                        if(m_device->get_component(i).get_timer().size() != 0) {
                            buffer[--buf_size] = ';'; ++buf_size;
                        }
                        else
                            buffer[buf_size++] = ';';
                    }
                    if(buf_size != 0)
                        buffer[--buf_size] = '\0';
                    response->addHeader("cycle_begin", (char*)buffer);
#if defined(SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA)
                    Serial.print("cycle_begin: ");
                    Serial.println((char*)buffer);
#endif
                    // response->addHeader(
                    //     "cycle_begin",
                    //     "21:15,12:20,05:00;10:00,20:00;10:00,20:00"); // начало цикла ШИМ. Тут не знаю
                    //                                     // как правильнее сделать подстрою
                    //                                     // под вас
                    buf_size = 0;
                    for(int i = 0; i < amt_components; ++i) {
                        for(int j = 0; j < m_device->get_component(i).get_timer().size(); ++j) {
                            value = m_device->get_component(i).get_timer()[j].get_end_hours();
                            if(value < 10)
                                add_number(0, buffer, buf_size);
                            add_number(value, buffer, buf_size);
                            buffer[buf_size++] = ':';
                            value = m_device->get_component(i).get_timer()[j].get_end_minutes();
                            if(value < 10)
                                add_number(0, buffer, buf_size);
                            add_number(value, buffer, buf_size);
                            buffer[buf_size++] = ',';
                        }
                        if(m_device->get_component(i).get_timer().size() != 0) {
                            buffer[--buf_size] = ';'; ++buf_size;
                        }
                        else
                            buffer[buf_size++] = ';';
                    }
                    if(buf_size != 0)
                        buffer[--buf_size] = '\0';
                    response->addHeader("cycle_end", (char*)buffer);
#if defined(SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA)
                    Serial.print("cycle_end: ");
                    Serial.println((char*)buffer);
#endif
                    // response->addHeader(
                    //     "cycle_end",
                    //     "21:15,12:20,05:00;10:00,20:00;10:00,20:00"); //конец цикла ШИМ. Тут не знаю как
                    //                                     //правильнее сделать подстрою под
                    //                                     //вас
                    buf_size = 0;
                    for(int i = 0; i < amt_components; ++i) {
                        for(int j = 0; j < m_device->get_component(i).get_timer().size(); ++j) {
                            value = m_device->get_component(i).get_timer()[j].get_channel().get_duration_on();
                            add_number(value, buffer, buf_size);
                            buffer[buf_size++] = ',';
                        }
                        buffer[--buf_size] = ';'; ++buf_size;
                    }
                    buffer[--buf_size] = '\0';
                    response->addHeader("turn_on", (char*)buffer);
#if defined(SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA)
                    Serial.print("turn_on: ");
                    Serial.println((char*)buffer);
#endif
                    // response->addHeader(
                    //     "turn_on", "10,1200,1800;36000,36000;7200,7200"); //Тут не знаю как правильнее
                    //                                             //сделать подстрою под вас
                    buf_size = 0;
                    for(int i = 0; i < amt_components; ++i) {
                        for(int j = 0; j < m_device->get_component(i).get_timer().size(); ++j) {
                            value = m_device->get_component(i).get_timer()[j].get_channel().get_duration_off();
                            add_number(value, buffer, buf_size);
                            buffer[buf_size++] = ',';
                        }
                        buffer[--buf_size] = ';'; ++buf_size;
                    }
                    buffer[--buf_size] = '\0';
                    response->addHeader("turn_off", (char*)buffer);
#if defined(SERIAL_PRINT_MODULE_COMPONENT_SEND_DATA)
                    Serial.print("turn_off: ");
                    Serial.println((char*)buffer);
#endif
                    // response->addHeader("turn_off",
                    //                     "3,300,600;7200,7200;7200,7200"); //Тут не знаю как правильнее
                    //                                             //сделать подстрою под вас






                    // response->addHeader("count_of_device_modules",
                    //                     "2"); // Это тип насосы дим лампы и т д
                    // response->addHeader("count_of_cycles_in_device_moduls",
                    //                     "3, 2"); // количество циклов ШИМ
                    // response->addHeader("devices_names",
                    //                     "lamp, nasos"); // название модулей сенсоров
                    // response->addHeader("mech_devices_ids",
                    //                     "3, 4"); // название модулей сенсоров
                    // response->addHeader(
                    //     "pwm_powers", "10,20,30;10,5"); //значения ШИМ разделитель внутри
                    //                                     //каждого устройства ',' разделитель
                    //                                     //между устройствами ';'
                    // response->addHeader("work_mode", "auto,manual"); // режимы работы
                    // response->addHeader(
                    //     "status",
                    //     "on,on"); // Это для режимов работы если авто, то всегда включен,
                    //             // если ручной может быть выключен или включен
                    // response->addHeader("manual_value",
                    //                     "0,10"); // Для авто режима значения ШИМ нам
                    //                             // всеравно, для ручного - значение
                    // response->addHeader(
                    //     "cycle_begin",
                    //     "21:15,12:20,05:00;10:00,20:00"); // начало цикла ШИМ. Тут не знаю
                    //                                     // как правильнее сделать подстрою
                    //                                     // под вас
                    // response->addHeader(
                    //     "cycle_end",
                    //     "21:15,12:20,05:00;10:00,20:00"); //конец цикла ШИМ. Тут не знаю как
                    //                                     //правильнее сделать подстрою под
                    //                                     //вас
                    // response->addHeader(
                    //     "turn_on", "10,1200,1800;36000,36000"); //Тут не знаю как правильнее
                    //                                             //сделать подстрою под вас
                    // response->addHeader("turn_off",
                    //                     "3,300,600;7200,7200"); //Тут не знаю как правильнее
                    //                                             //сделать подстрою под вас
                }
            
            }

            request->send(response);
            // Serial.println("get_info_registered_device");
            // int paramsNr = request->params();
            // Serial.println(paramsNr);
            // for (int i = 0; i < paramsNr; i++) {
            //     AsyncWebParameter *p = request->getParam(i);
            //     Serial.print("Param name: ");
            //     Serial.println(p->name());
            //     Serial.print("Param value: ");
            //     Serial.println(p->value());
            //     Serial.println("------");
            //     String param_name = p->name();
            //     if (param_name == "device_id") {
            //         String device_id = p->value();
            //         Serial.println(device_id);
            //     }
            // }
            // // somefunction(p->value) какая то функция, которая получает информацию
            // // о модуле
            // AsyncWebServerResponse *response =
            //     request->beginResponse(200, "text/plain", "Ok");
            // response->addHeader("Access-Control-Allow-Origin", "*");
            // response->addHeader("Access-Control-Expose-Headers", "*");
            // response->addHeader("device_name",
            //                     "Device"); // название модуля (я так понимаю stmки)
            // response->addHeader("count_of_sensor_modules",
            //                     "2"); // кол-во сенсоров, подключенных к модулю
            // response->addHeader("sensors_names",
            //                     "h2o, co2"); // название модулей сенсоров
            // response->addHeader("sensors_ids", "1, 2"); // название модулей сенсоров
            // response->addHeader("sensor_indications",
            //                     "0.333,0.444"); // последние показания датчиков
            // response->addHeader("sensor_values",
            //                     "1,120"); //Тут не знаю как правильнее 
            //                               //сделать подстрою под вас
            // response->addHeader("count_of_device_modules",
            //                     "2"); // Это тип насосы дим лампы и т д
            // response->addHeader("count_of_cycles_in_device_moduls",
            //                     "3, 2"); // количество циклов ШИМ
            // response->addHeader("devices_names",
            //                     "lamp, nasos"); // название модулей сенсоров
            // response->addHeader("mech_devices_ids",
            //                     "3, 4"); // название модулей сенсоров
            // response->addHeader(
            //     "pwm_powers", "10,20,30;10,5"); //значения ШИМ разделитель внутри
            //                                     //каждого устройства ',' разделитель
            //                                     //между устройствами ';'
            // response->addHeader("work_mode", "auto,manual"); // режимы работы
            // response->addHeader(
            //     "status",
            //     "on,on"); // Это для режимов работы если авто, то всегда включен,
            //               // если ручной может быть выключен или включен
            // response->addHeader("manual_value",
            //                     "0,10"); // Для авто режима значения ШИМ нам
            //                              // всеравно, для ручного - значение
            // response->addHeader(
            //     "cycle_begin",
            //     "21:15,12:20,05:00;10:00,20:00"); // начало цикла ШИМ. Тут не знаю
            //                                     // как правильнее сделать подстрою
            //                                     // под вас
            // response->addHeader(
            //     "cycle_end",
            //     "21:15,12:20,05:00;10:00,20:00"); //конец цикла ШИМ. Тут не знаю как
            //                                     //правильнее сделать подстрою под
            //                                     //вас
            // response->addHeader(
            //     "turn_on", "10,1200,1800;36000,36000"); //Тут не знаю как правильнее
            //                                             //сделать подстрою под вас
            // response->addHeader("turn_off",
            //                     "3,300,600;7200,7200"); //Тут не знаю как правильнее
            //                                             //сделать подстрою под вас
            // request->send(response);
            }
            else {
                Serial.println("/unregistered_device/get");
                int paramsNr = request->params();
                Serial.println(paramsNr);

                for (int i = 0; i < paramsNr; i++) {
                    AsyncWebParameter *p = request->getParam(i);
                    Serial.print("Param name: ");
                    Serial.println(p->name());
                    Serial.print("Param value: ");
                    Serial.println(p->value());
                    Serial.println("------");
                    String param_name = p->name();
                    if (param_name == "device_id") {
                        String device_id = p->value();
                        Serial.println(device_id);
                    }
                }

                // somefunction(p->value) какая то функция, которая получает информацию
                // о модуле
                AsyncWebServerResponse *response =
                    request->beginResponse(200, "text/plain", "Ok");
                response->addHeader("Access-Control-Allow-Origin", "*");
                response->addHeader("Access-Control-Expose-Headers", "*");
                response->addHeader("count_of_sensor_modules",
                                    "2"); // кол-во сенсоров, подключенных к модулю
                response->addHeader("sensors_names",
                                    "3,0,1,2,3,4,5,6,7,8"); // название модулей сенсоров
                response->addHeader(
                    "sensors_ids", "0,0,3,4,5,6,7,8,9,10"); // название модулей сенсоров
                response->addHeader("sensor_indications",
                                    "0.333,0.444,0.555,0.555,0.555,0.555,0.555,0.555,0."
                                    "555,0.555"); // последние показания датчиков
                response->addHeader(
                    "sensor_values",
                    "1,120,1,3,4,5,6,7,8,8"); //Тут не знаю как правильнее сделать
                                            //подстрою под вас
                response->addHeader("count_of_device_modules",
                                    "2"); // Это тип насосы дим лампы и т д
                response->addHeader("count_of_cycles_in_device_moduls",
                                    "1, 2, 4, 5, 6"); // количество циклов ШИМ
                response->addHeader("devices_names",
                                    "2,3,4,0,5"); // название модулей сенсоров
                response->addHeader("mech_devices_ids",
                                    "3,4,2,1,7"); // название модулей сенсоров
                response->addHeader(
                    "pwm_powers",
                    "10;10,5;1,2,3,4;1,2,3,4,5;1,2,3,4,5,6"); //значения ШИМ разделитель
                                                            //внутри каждого
                                                            //устройства ','
                                                            //разделитель между
                                                            //устройствами ';'
                response->addHeader("work_mode",
                                    "auto,manual,manual,manual,auto"); // режимы работы
                response->addHeader(
                    "status", "on,on,on,on,off"); // Это для режимов работы если авто,
                                                // то всегда включен, если ручной
                                                // может быть выключен или включен
                response->addHeader("manual_value",
                                    "0,10,2,3,1"); // Для авто режима значения ШИМ нам
                                                // всеравно, для ручного - значение
                response->addHeader("cycle_begin",
                                "21:15,12:20,05:00;10:00,20:00;10:00,20:00,10:00,20:00;"
                                "10:00,20:00,10:00,20:00,10:00;10:00,20:00,10:00,20:00,"
                                "10:00,20:00"); // начало цикла ШИМ. Тут не знаю как
                                                // правильнее сделать подстрою под вас
                response->addHeader("cycle_end",
                                "21:15,12:20,05:00;10:00,20:00;10:00,20:00,10:00,20:00;"
                                "10:00,20:00,10:00,20:00,10:00;10:00,20:00,10:00,20:00,"
                                "10:00,20:00"); //конец цикла ШИМ. Тут не знаю как
                                                //правильнее сделать подстрою под вас
                response->addHeader("turn_on",
                                    "10,1200,1800;36000,36000;36000,36000,36000,36000;"
                                    "36000,36000,36000,36000,36000;36000,36000,36000,"
                                    "36000,36000,36000"); //Тут не знаю как правильнее
                                                        //сделать подстрою под вас
                response->addHeader("turn_off",
                    "3,300,600;7200,7200;300,600,300,600;300,600,300,600,300;300,600,"
                    "300,600,300,600"); //Тут не знаю как правильнее сделать подстрою
                                        //под вас
                request->send(response);
            }
        }
        
        // Регистрация модуля | "/registered_device/add"
        void module_registration(AsyncWebServerRequest *request) { // - (-) -----
            Serial.println("- (-) ----- \"/registered_device/add\"");
            Serial.println("get_info_registered_device");
            int paramsNr = request->params();
            Serial.println(paramsNr);
            for (int i = 0; i < paramsNr; i++) {
                AsyncWebParameter *p = request->getParam(i);
                Serial.print("Param name: ");
                Serial.println(p->name());
                Serial.print("Param value: ");
                Serial.println(p->value());
                Serial.println("------");
                String param_name = p->name();
                if (param_name == "device_id") { // id всего модуля
                    String device_id = p->value();
                }
                else if (param_name == "count_of_sensor_modules") { // кол-во сенсоров, подключенных
                                                                    // к модулю '2'
                    String count_of_sensor_modules = p->value();
                } else if (param_name == "sensors_ids") { // id сенсоров, подключенных
                                                            // к модулю '1,2'
                    String sensors_ids = p->value();
                } else if (param_name ==
                            "sensors_values") { // периоды снятия показаний с датчиков
                                                // '1600,2000'
                    String sensors_values = p->value();
                } else if (param_name == "mech_devices_ids") { // // кол-во насосов,
                                                                // дим ламп и т д '2'
                    String mech_devices_ids = p->value();
                } else if (param_name ==
                            "pwm_powers") { // // значения ШИМ '10,20,30;10,5'
                    String pwm_powers = p->value();
                } else if (param_name ==
                            "cycle_begin") { // // начало цикла ШИМ
                                            // '21:15,12:20,05:00;10:00,20:00'
                    String cycle_begin = p->value();
                } else if (param_name ==
                            "cycle_end") { // // конец цикла ШИМ
                                            // '21:15,12:20,05:00;10:00,20:00'
                    String cycle_end = p->value();
                } else if (param_name == "turn_on") { // // время включения цикла ШИМ
                                                        // '600,300,3;7200,7200'
                    String turn_on = p->value();
                } else if (param_name == "turn_off") { // // время включения цикла ШИМ
                                                        // '600,300,3;7200,7200'
                    String turn_off = p->value();
                }
            }
            // somefunction(p->value) какая то функция, которая получает информацию
            // о модуле
            AsyncWebServerResponse *response =
                request->beginResponse(200, "text/plain", "Ok");
            response->addHeader("Access-Control-Allow-Origin", "*");
            response->addHeader("Access-Control-Expose-Headers", "*");
            request->send(response);


            // Serial.println("get_info_registered_device");
            // int paramsNr = request->params();
            // Serial.println(paramsNr);
            // for (int i = 0; i < paramsNr; i++) {
            //     AsyncWebParameter *p = request->getParam(i);
            //     Serial.print("Param name: ");
            //     Serial.println(p->name());
            //     Serial.print("Param value: ");
            //     Serial.println(p->value());
            //     Serial.println("------");
            //     String param_name = p->name();
            //     if (param_name == "device_id") { // id всего модуля
            //         String device_id = p->value();
            //     }
            //     else if (param_name == "count_of_sensor_modules") { // кол-во сенсоров, подключенных
            //                                                         // к модулю '2'
            //         String count_of_sensor_modules = p->value();
            //     } else if (param_name == "sensors_ids") { // id сенсоров, подключенных
            //                                                 // к модулю '1,2'
            //         String sensors_ids = p->value();
            //     } else if (param_name ==
            //                 "sensors_values") { // периоды снятия показаний с датчиков
            //                                     // '1600,2000'
            //         String sensors_values = p->value();
            //     } else if (param_name == "mech_devices_ids") { // // кол-во насосов,
            //                                                     // дим ламп и т д '2'
            //         String mech_devices_ids = p->value();
            //     } else if (param_name ==
            //                 "pwm_powers") { // // значения ШИМ '10,20,30;10,5'
            //         String pwm_powers = p->value();
            //     } else if (param_name ==
            //                 "cycle_begin") { // // начало цикла ШИМ
            //                                 // '21:15,12:20,05:00;10:00,20:00'
            //         String cycle_begin = p->value();
            //     } else if (param_name ==
            //                 "cycle_end") { // // конец цикла ШИМ
            //                                 // '21:15,12:20,05:00;10:00,20:00'
            //         String cycle_end = p->value();
            //     } else if (param_name == "turn_on") { // // время включения цикла ШИМ
            //                                             // '600,300,3;7200,7200'
            //         String turn_on = p->value();
            //     } else if (param_name == "turn_off") { // // время включения цикла ШИМ
            //                                             // '600,300,3;7200,7200'
            //         String turn_off = p->value();
            //     }
            // }
            // // somefunction(p->value) какая то функция, которая получает информацию
            // // о модуле
            // AsyncWebServerResponse *response =
            //     request->beginResponse(200, "text/plain", "Ok");
            // response->addHeader("Access-Control-Allow-Origin", "*");
            // response->addHeader("Access-Control-Expose-Headers", "*");
            // request->send(response);
        }
        // Изменение модуля | "/registered_device/edit"
        void module_editing(AsyncWebServerRequest *request) { // (+?) -----
            static std::array<uint8_t, scs::AMT_BYTES_ID> id;
            static const scs::System_component* module;
            static Grow_device* m_device;
            static Grow_sensor* m_sensor;
            static bool device = false;
            static uint16_t amt_counter;
            static uint16_t amt_components;
            static uint8_t type_component;
            static int value;
            static bool error;
            static std::vector<dtc::Grow_timer> channel_data[25];

            module = nullptr;
            amt_counter = 0;
            amt_components = 0;
            error = false;
            int paramsNr = request->params();
#if defined(SERIAL_PRINT_MODULE_EDIT_SEND_DATA)
            Serial.println("  (+?) ----- \"/registered_device/edit\"");
            Serial.print("paramsNr = ");
            Serial.println(paramsNr);
#endif
#if defined(SERIAL_PRINT_MODULE_EDIT_SEND_DATA_LEV_PRINT_1)
            for (int i = 0; i < paramsNr; i++) {
                AsyncWebParameter *p = request->getParam(i);
                Serial.print("Param name: ");
                Serial.println(p->name());
                Serial.print("Param value: ");
                Serial.println(p->value());
                Serial.println("------");
            }
#endif

            // Просмотр всех параметров
            for (int i = 0; i < paramsNr; i++) {
                AsyncWebParameter *p = request->getParam(i);
#if defined(SERIAL_PRINT_MODULE_EDIT_SEND_DATA_LEV_PRINT_2)
                Serial.print("Param name: ");
                Serial.print(p->name());
#endif
                String param_name = p->name();
                if (param_name == "device_id") { // id всего модуля
                    String device_id = p->value();
                    hex_to_byte((uint8_t*)&device_id[0], id);
                    if(gcm != nullptr) {
                        int number = gcm->search_device(id);
                        if(number == -1) {
                            number = gcm->search_sensor(id);
                            if(number != -1) {
                                module = &gcm->sensors_[number];

                                m_sensor = &gcm->sensors_[number];
                                device = false;
                            }
                            else {
                                error = true;
                            }
                        }
                        else {
                            module = &gcm->devices_[number];
                            
                            m_device = &gcm->devices_[number];
                            device = true;
                        }
                    }
                    else {
                        error = true;
                    }
#if defined(SERIAL_PRINT_MODULE_EDIT_SEND_DATA_LEV_PRINT_2)
                    Serial.println();
                    Serial.print("Param value: ");
                    Serial.println(p->value());
                    Serial.println("------");
                    if(error) {
                        Serial.println("Error!!!");
                    }
#endif
                    if(error) {
                        break;
                    }
                }
                else {
                    if(module == nullptr) {
                        // (-) ----- обработка ситуации "не первого заголовка"
                    }
                    else if (param_name == "count_of_sensor_modules") {          // (+) -----
                        // кол-во сенсоров, подключенных к модулю
                        String count_of_sensor_modules = p->value();
                        if(!device) {
                            amt_components = count_of_sensor_modules[0] - '0'; // (--) -----
                            if(m_sensor->get_count_component() != amt_components) {
                                error = true;
                            }
                        }
                    }
                    else if (param_name == "sensors_names") {                    // (+) -----
                        // Типы сенсоров, подключенных к модулю
                        String sensors_names = p->value();
                        if(!device && (amt_components != 0)) {
                            amt_counter = get_value((uint8_t*)&sensors_names[0], input_value);
                            if(amt_counter == amt_components) {
                                for(int i = 0; i < amt_counter; ++i) {
                                    if(!m_sensor->get_type(i, type_component)) {
                                        if(input_value[i] != type_component)
                                            error = true;
                                    }
                                    else {
                                        error = true;
                                    }
                                }
                            }
                        }
                    }
                    else if (param_name == "sensors_ids") {                      // (+) -----
                        // id сенсоров, подключенных к модулю
                        String sensors_ids = p->value();
                        if(!device && (amt_components != 0)) {
                            amt_counter = get_value((uint8_t*)&sensors_ids[0], input_value);
                            if(amt_counter == amt_components) {
                                for(int i = 0; i < amt_counter; ++i) {
                                    if(!m_sensor->get_id(i, type_component)) {
                                        if(input_value[i] != type_component)
                                            error = true;
                                    }
                                    else {
                                        error = true;
                                    }
                                }
                            }
                            else {
                                error = true;
                            }
                        }
                    }
                    else if (param_name == "sensors_values") {                   // (+) -----
                        // периоды снятия показаний с датчиков, '1600,2000'
                        String sensors_values = p->value();
                        if(!device && (amt_components != 0)) {
                            amt_counter = get_value((uint8_t*)&sensors_values[0], input_value);
                            if(amt_counter == amt_components) {
                                for(int i = 0; i < amt_counter; ++i) {
                                    if(input_value[0] > input_value[i])
                                        input_value[0] = input_value[i];
                                }
                                Serial.print(input_value[0]);
                                m_sensor->set_period(input_value[0] * 1000);
                            }
                            else {
                                error = true;
                            }
                        }

                    }
                    else if (param_name == "count_of_device_modules") {          // (+) -----
                        // кол-во устройств, подключенных к модулю
                        String count_of_device_modules = p->value();
                        if(device) {
                            amt_components = count_of_device_modules[0] - '0'; // (--) -----
                            if(m_device->get_count_component() != amt_components) {
                                // Serial.print("     ~");
                                // Serial.print(m_device->get_count_component());
                                // Serial.print(" != ");
                                // Serial.print(amt_components);
                                // Serial.print("~");
                                // // error = true;
                                // amt_components = m_device->get_count_component();

                                error = true;
                            }
                        }
                    }
                    else if (param_name == "count_of_cycles_in_device_moduls") { // (+?) -----
                        // Количество циклов, подключенных к модулю
                        String count_cycles = p->value();
                        if(device && (amt_components != 0)) {
                            amt_counter = get_value((uint8_t*)&count_cycles[0], input_value);
                            if(amt_counter == amt_components) {
                                for(int i = 0; i < amt_counter; ++i) {
                                    channel_data[i] = m_device->get_component(i).get_timer();
                                    while(channel_data[i].size() != input_value[i]) {
                                        if(channel_data[i].size() < input_value[i]) {
                                            channel_data[i].push_back(dtc::Grow_timer{});
                                        }
                                        else {
                                            channel_data[i].pop_back();
                                        }
                                    }
                                }
                            }
                            else {
                                // Serial.print("     ~");
                                // Serial.print(amt_components);
                                // Serial.print(" != ");
                                // Serial.print(amt_counter);
                                // Serial.print("~");
                                error = true;
                            }
                        }
                    }
                    else if (param_name == "devices_names") {                    // (+) -----
                        // Типы устройств, подключенных к модулю
                        String devices_names = p->value();
                        // // (--) -----
                        if(device && (amt_components != 0)) {
                            amt_counter = get_value((uint8_t*)&devices_names[0], input_value);
                            if(amt_counter == amt_components) {
                                for(int i = 0; i < amt_counter; ++i) {
                                    if(!m_device->get_type(i, type_component)) {
                                        if(input_value[i] != type_component)
                                            error = true;
                                    }
                                    else {
                                        error = true;
                                    }
                                }
                            }
                            else {
                                error = true;
                            }
                        }
                    }
                    else if (param_name == "mech_devices_names") {               // - (--) -----
                        // Типы устройств, подключенных к модулю
                        String sensors_ids = p->value();
                        // // (--) -----
                    }
                    else if (param_name == "mech_devices_ids") {                 // (+) -----
                        // id устройств (насосов, дим ламп и т.д.)
                        String mech_devices_ids = p->value();
                        // // (+?) -----
                        if(device && (amt_components != 0)) {
                            amt_counter = get_value((uint8_t*)&mech_devices_ids[0], input_value);
                            if(amt_counter == amt_components) {
                                for(int i = 0; i < amt_counter; ++i) {
                                    if(!m_device->get_id(i, type_component)) {
                                        if(input_value[i] != type_component)
                                            error = true;
                                    }
                                    else {
                                        error = true;
                                    }
                                }
                            }
                            else {
                                error = true;
                            }
                        }
                    }
                    else if (param_name == "pwm_powers") {                       // (+-) -----
                        // значения ШИМ '10,20,30;10,5'
                        String pwm_powers = p->value();
                        uint8_t iter = 0;
                        uint8_t index = 0;
                        dtc::Time_channel channel;
                        // TC.OFF // (--) -----
                        if(device && (amt_components != 0)) {
                            while((index < pwm_powers.length()) && (iter < amt_components) ) {
                                amt_counter = get_value((uint8_t*)&pwm_powers[0], input_value, index);
                                for(int j = 0; j < amt_counter; ++j) {
                                    channel_data[iter][j].set_send_value(input_value[j]);
                                }
                                ++iter;
                            }
                        }
                    }
                    else if (param_name == "cycle_begin") {                      // (+?) -----
                        // начало цикла ШИМ - '21:15,12:20,05:00;10:00,20:00'
                        String cycle_begin = p->value();
                        uint8_t iter = 0;
                        uint8_t index = 0;
                        // GT.S // (--) -----
                        if(device && (amt_components != 0)) {
                            while((index < cycle_begin.length()) && (iter < amt_components) ) {
                                amt_counter = get_time_value((uint8_t*)&cycle_begin[0], input_value, index);
                                for(int j = 0; j < amt_counter; ++j) {
                                    channel_data[iter][j].set_start_hours(input_value[j] / 1000);
                                    channel_data[iter][j].set_start_minutes(input_value[j] % 1000);
                                    channel_data[iter][j].set_start_seconds(0);
                                }
                                ++iter;
                            }
                        }
                    }
                    else if (param_name == "cycle_end") {                        // (+?) -----
                        // конец цикла ШИМ - '21:15,12:20,05:00;10:00,20:00'
                        String cycle_end = p->value();
                        uint8_t iter = 0;
                        uint8_t index = 0;
                        // GT.E // (--) -----
                        if(device && (amt_components != 0)) {
                            while((index < cycle_end.length()) && (iter < amt_components) ) {
                                amt_counter = get_time_value((uint8_t*)&cycle_end[0], input_value, index);
                                for(int j = 0; j < amt_counter; ++j) {
                                    channel_data[iter][j].set_end_hours(input_value[j] / 1000);
                                    channel_data[iter][j].set_end_minutes(input_value[j] % 1000);
                                    channel_data[iter][j].set_end_seconds(0);
                                }
                                ++iter;
                            }
                        }
                    }
                    else if (param_name == "turn_on") {                          // (+?) -----
                        // время включения цикла ШИМ
                        // '600,300,3;7200,7200'
                        String turn_on = p->value();
                        uint8_t iter = 0;
                        uint8_t index = 0;
                        dtc::Time_channel channel;
                        // TC.ON // (--) -----
                        if(device && (amt_components != 0)) {
                            while((index < turn_on.length()) && (iter < amt_components) ) {
                                amt_counter = get_value((uint8_t*)&turn_on[0], input_value, index);
                                for(int j = 0; j < amt_counter; ++j) {
                                    channel = channel_data[iter][j].get_channel();
                                    channel.set_duration_on(input_value[j]);
                                    channel_data[iter][j].bind_channel(channel);
                                }
                                ++iter;
                            }
                        }
                    }
                    else if (param_name == "turn_off") {                         // (+?) -----
                        // время включения цикла ШИМ
                        // '600,300,3;7200,7200'
                        String turn_off = p->value();
                        uint8_t iter = 0;
                        uint8_t index = 0;
                        dtc::Time_channel channel;
                        // TC.OFF // (--) -----
                        if(device && (amt_components != 0)) {
                            while((index < turn_off.length()) && (iter < amt_components) ) {
                                amt_counter = get_value((uint8_t*)&turn_off[0], input_value, index);
                                for(int j = 0; j < amt_counter; ++j) {
                                    channel = channel_data[iter][j].get_channel();
                                    channel.set_duration_off(input_value[j]);
                                    channel_data[iter][j].bind_channel(channel);
                                }
                                ++iter;
                            }
                        }
                    }
                    else {
#if defined(SERIAL_PRINT_MODULE_EDIT_SEND_DATA_LEV_PRINT_2)
                        Serial.print(" -!-");
#endif
                    }
#if defined(SERIAL_PRINT_MODULE_EDIT_SEND_DATA_LEV_PRINT_2)
                    Serial.println();
                    Serial.print("Param value: ");
                    Serial.println(p->value());
                    Serial.println("------");
#endif
                }
                if(error) {
#if defined(SERIAL_PRINT_MODULE_EDIT_SEND_DATA_LEV_PRINT_2)
                    Serial.println("Error!!!");
#endif
                    break;
                }
            }
            if(!error) {
                if(device) {
                    for(int i = 0; i < amt_components; ++i) {
                        m_device->component_[i].set_timer(channel_data[i]);
                    }
                    m_device->set_setting_change_period(true);
                }

#if defined(SERIAL_PRINT_MODULE_EDIT_SEND_DATA)
                // Вывод ID
                if(!device) {
                    char mas[25];
                    byte_to_hex((uint8_t*)&(m_sensor->get_system_id()[0]), (uint8_t*)mas, 12);
                    mas[24] = '\0';
                    Serial.print("<<< ");
                    Serial.print(mas);
                    Serial.println(" >>>");
                }
                if(device) {
                    char mas[25];
                    byte_to_hex((uint8_t*)&(m_device->get_system_id()[0]), (uint8_t*)mas, 12);
                    mas[24] = '\0';
                    Serial.print("<<< ");
                    Serial.print(mas);
                    Serial.println(" >>>");
                }
                // Вывод характеристик датчиков
                if(!device) {
                    // m_sensor->set_period(input_value[0] * 1000);
                    amt_components = m_sensor->get_count_component();
                    Serial.print("count_of_sensor_modules: ");
                    Serial.println(amt_components);


                    Serial.print("sensor_type: "); // sensors_names
                    for(int i = 0; i < amt_components; ++i) {
                        Serial.print((uint8_t)m_sensor->get_type()[i]);
                        if(i < amt_components - 1)
                            Serial.print(",");
                    }
                    Serial.println(); 

                    Serial.print("sensor_id: "); // sensors_ids
                    for(int i = 0; i < amt_components; ++i) {
                        Serial.print((uint8_t)m_sensor->get_id()[i]);
                        if(i < amt_components - 1)
                            Serial.print(",");
                    }
                    Serial.println(); 

                    Serial.print("sensor_time: "); // sensors_values
                    Serial.print((uint32_t)m_sensor->get_period() / 1000);
                    Serial.println(" sec"); 


                }
                // Вывод характеристик устройств
                if(device) {
                    amt_components = m_device->get_count_component();
                    Serial.print("count_of_device_modules: ");
                    Serial.println(amt_components);

                    Serial.print("count_cycles: "); // count_of_cycles_in_device_moduls
                    for(int i = 0; i < amt_components; ++i) {
                        Serial.print(m_device->component_[i].get_timer().size());
                        if(i < amt_components - 1)
                            Serial.print(",");
                    }
                    Serial.println(); 

                    Serial.print("device_type: "); // devices_names
                    for(int i = 0; i < amt_components; ++i) {
                        Serial.print((uint8_t)m_device->component_[i].get_type());
                        if(i < amt_components - 1)
                            Serial.print(",");
                    }
                    Serial.println(); 

                    Serial.print("device_id: "); // mech_devices_ids
                    for(int i = 0; i < amt_components; ++i) {
                        Serial.print((uint8_t)m_device->component_[i].get_id());
                        if(i < amt_components - 1)
                            Serial.print(",");
                    }
                    Serial.println(); 


                    Serial.print("pwm_powers: "); // pwm_powers
                    for(int i = 0; i < amt_components; ++i) {
                        for(int j = 0; j < m_device->component_[i].get_timer().size(); ++j) {
                            Serial.print(m_device->component_[i].get_timer()[j].get_send_value());
                            if(j < m_device->component_[i].get_timer().size() - 1)
                                Serial.print(",");
                        }
                        if(i < amt_components - 1)
                            Serial.print(";");
                    }
                    Serial.println(); 

                    Serial.print("cycle_begin: "); // cycle_begin
                    for(int i = 0; i < amt_components; ++i) {
                        for(int j = 0; j < m_device->component_[i].get_timer().size(); ++j) {

                            if(m_device->component_[i].get_timer()[j].get_start_hours() < 10)
                                Serial.print("0");
                            Serial.print(m_device->component_[i].get_timer()[j].get_start_hours());
                            Serial.print(":");
                            if(m_device->component_[i].get_timer()[j].get_start_minutes() < 10)
                                Serial.print("0");
                            Serial.print(m_device->component_[i].get_timer()[j].get_start_minutes());

                            if(j < m_device->component_[i].get_timer().size() - 1)
                                Serial.print(",");
                        }
                        if(i < amt_components - 1)
                            Serial.print(";");
                    }
                    Serial.println(); 

                    Serial.print("cycle_end: "); // cycle_end
                    for(int i = 0; i < amt_components; ++i) {
                        for(int j = 0; j < m_device->component_[i].get_timer().size(); ++j) {
                            
                            if(m_device->component_[i].get_timer()[j].get_end_hours() < 10)
                                Serial.print("0");
                            Serial.print(m_device->component_[i].get_timer()[j].get_end_hours());
                            Serial.print(":");
                            if(m_device->component_[i].get_timer()[j].get_end_minutes() < 10)
                                Serial.print("0");
                            Serial.print(m_device->component_[i].get_timer()[j].get_end_minutes());

                            if(j < m_device->component_[i].get_timer().size() - 1)
                                Serial.print(",");
                        }
                        if(i < amt_components - 1)
                            Serial.print(";");
                    }
                    Serial.println(); 

                    Serial.print("turn_on: "); // turn_on
                    for(int i = 0; i < amt_components; ++i) {
                        for(int j = 0; j < m_device->component_[i].get_timer().size(); ++j) {
                            Serial.print(m_device->component_[i].get_timer()[j].get_channel().get_duration_on());
                            if(j < m_device->component_[i].get_timer().size() - 1)
                                Serial.print(",");
                        }
                        if(i < amt_components - 1)
                            Serial.print(";");
                    }
                    Serial.println(); 

                    Serial.print("turn_off: "); // turn_off
                    for(int i = 0; i < amt_components; ++i) {
                        for(int j = 0; j < m_device->component_[i].get_timer().size(); ++j) {
                            Serial.print(m_device->component_[i].get_timer()[j].get_channel().get_duration_off());
                            if(j < m_device->component_[i].get_timer().size() - 1)
                                Serial.print(",");
                        }
                        if(i < amt_components - 1)
                            Serial.print(";");
                    }
                    Serial.println(); 
#endif
                }
            }
            // somefunction(p->value) какая то функция, которая получает информацию
            // о модуле
            AsyncWebServerResponse *response =
                request->beginResponse(200, "text/plain", "Ok");
            response->addHeader("Access-Control-Allow-Origin", "*");
            response->addHeader("Access-Control-Expose-Headers", "*");
            request->send(response);

            // Serial.println("get_info_registered_device");
            // int paramsNr = request->params();
            // Serial.println(paramsNr);
            // for (int i = 0; i < paramsNr; i++) {
            //     AsyncWebParameter *p = request->getParam(i);
            //     Serial.print("Param name: ");
            //     Serial.println(p->name());
            //     Serial.print("Param value: ");
            //     Serial.println(p->value());
            //     Serial.println("------");
            //     String param_name = p->name();
            //     if (param_name == "device_id") { // id всего модуля
            //         String device_id = p->value();
            //     }
            //     else if (param_name ==
            //             "count_of_sensor_modules") { // кол-во сенсоров, подключенных
            //                                             // к модулю '2'
            //         String count_of_sensor_modules = p->value();
            //     } else if (param_name == "sensors_ids") { // id сенсоров, подключенных
            //                                                 // к модулю '1,2'
            //         String sensors_ids = p->value();
            //     } else if (param_name ==
            //                 "sensors_values") { // периоды снятия показаний с датчиков
            //                                     // '1600,2000'
            //         String sensors_values = p->value();
            //     } else if (param_name == "mech_devices_ids") { // // кол-во насосов,
            //                                                     // дим ламп и т д '2'
            //         String mech_devices_ids = p->value();
            //     } else if (param_name ==
            //                 "pwm_powers") { // // значения ШИМ '10,20,30;10,5'
            //         String pwm_powers = p->value();
            //     } else if (param_name ==
            //                 "cycle_begin") { // // начало цикла ШИМ
            //                                 // '21:15,12:20,05:00;10:00,20:00'
            //         String cycle_begin = p->value();
            //     } else if (param_name ==
            //                 "cycle_end") { // // конец цикла ШИМ
            //                                 // '21:15,12:20,05:00;10:00,20:00'
            //         String cycle_end = p->value();
            //     } else if (param_name == "turn_on") { // // время включения цикла ШИМ
            //                                             // '600,300,3;7200,7200'
            //         String turn_on = p->value();
            //     } else if (param_name == "turn_off") { // // время включения цикла ШИМ
            //                                             // '600,300,3;7200,7200'
            //         String turn_off = p->value();
            //     }
            // }
            // // somefunction(p->value) какая то функция, которая получает информацию
            // // о модуле
            // AsyncWebServerResponse *response =
            //     request->beginResponse(200, "text/plain", "Ok");
            // response->addHeader("Access-Control-Allow-Origin", "*");
            // response->addHeader("Access-Control-Expose-Headers", "*");
            // request->send(response);
        }
        // Удаление модуля | "/registered_device/delete"
        void module_removing(AsyncWebServerRequest *request, uint8_t *data,
                             size_t len, size_t index, size_t total) { // - (-) -----
            Serial.println("- (-) ----- \"/registered_device/delete\"");
            // String device_id;
            // for (size_t i = 0; i < len; i++) {
            //     device_id += (char)data[i];
            // }
            // Serial.println(device_id);
            // AsyncWebServerResponse *response =
            //     request->beginResponse(301); //код переадресации
            // response->addHeader(
            //     "Access-Control-Allow-Origin",
            //     "*"); // заголовок, позволяющий делать запросы в одном домене
            // response->addHeader("Location",
            //                     "/settings"); // место переадресации
            // request->send(response);
        }
        // Отправка данных об устройствах | "/change_device_mode"
        void module_changing_mode(AsyncWebServerRequest *request) { // (-) -----
            Serial.println("   (-) ----- \"/change_device_mode\"");
            Serial.println("get_info_registered_device");
            int paramsNr = request->params();
            Serial.println(paramsNr);
            for (int i = 0; i < paramsNr; i++) {
                AsyncWebParameter *p = request->getParam(i);
                Serial.print("Param name: ");
                Serial.println(p->name());
                Serial.print("Param value: ");
                Serial.println(p->value());
                Serial.println("------");
                String param_name = p->name();
                if (param_name == "device_id") { // id всего модуля
                    String device_id = p->value();
                }
                else if (param_name == "work_mode") { // режим работы (auto, manual)
                    String work_mode = p->value();
                } else if (param_name == "status") { // статус устройства (on, off)
                    String status = p->value();
                } else if (param_name == "manual_value") { // значение ШИМ (0-100)
                    String manual_value = p->value();
                }
            }
            // somefunction(p->value) какая то функция, которая получает информацию
            // о модуле
            AsyncWebServerResponse *response =
                request->beginResponse(200, "text/plain", "Ok");
            response->addHeader("Access-Control-Allow-Origin", "*");
            response->addHeader("Access-Control-Expose-Headers", "*");
            request->send(response);
      }
        // Отправка данных о датчиках | "/sensor_device/get"
        void module_sensor_data(AsyncWebServerRequest *request) { // - - (-) -----
            int paramsNr = request->params();
            Serial.println("--(-) ----- \"/sensor_device/get\"");
            // Serial.println("----- ===== ----- ===== ----- ===== -----");
            // Serial.println("/sensor_device/get");
            // Serial.println("===== ----- ===== ----- ===== ----- =====");
            Serial.println(paramsNr);
            for (int i = 0; i < paramsNr; i++) {
                AsyncWebParameter *p = request->getParam(i);
                Serial.print("Param name: ");
                Serial.println(p->name());
                Serial.print("Param value: ");
                Serial.println(p->value());
                Serial.println("------");
                String param_name = p->name();
                if (param_name == "sensor_id") {
                    String sensor_id = p->value();
                }
            }
            // somefunction(p->value) какая то функция, которая получает информацию
            // о cенсоре
            AsyncWebServerResponse *response =
                request->beginResponse(200, "text/plain", "Ok");
            response->addHeader("Access-Control-Allow-Origin", "*");
            response->addHeader("Access-Control-Expose-Headers", "*");
            response->addHeader("sensor_indications", "132");
            request->send(response);
            // Serial.println("get_info_registered_device");
            // int paramsNr = request->params();
            // Serial.println(paramsNr);
            // for (int i = 0; i < paramsNr; i++) {
            //     AsyncWebParameter *p = request->getParam(i);
            //     Serial.print("Param name: ");
            //     Serial.println(p->name());
            //     Serial.print("Param value: ");
            //     Serial.println(p->value());
            //     Serial.println("------");
            //     String param_name = p->name();
            //     if (param_name == "sensor_id") { // имя wi-fi сети
            //         String sensor_id = p->value();
            //     }
            // }
            // // somefunction(p->value) какая то функция, которая получает информацию
            // // о cенсоре
            // AsyncWebServerResponse *response =
            //     request->beginResponse(200, "text/plain", "Ok");
            // response->addHeader("Access-Control-Allow-Origin", "*");
            // response->addHeader("Access-Control-Expose-Headers", "*");
            // response->addHeader("sensor_indications", "132");
            // request->send(response);
        }

        // Установка настроек для соединения с сервером | "/system_settings/get"
        void mgtt_using(AsyncWebServerRequest *request) { // - (-) -----
            Serial.println("- (-) ----- \"/system_settings/get\"");
            // Serial.println("system_settings/get");
            // // somefunction(p->value) какая то функция, которая получает
            // // информацию о модуле
            // AsyncWebServerResponse *response =
            //     request->beginResponse(200, "text/plain", "Ok");
            // response->addHeader("Access-Control-Allow-Origin", "*");
            // response->addHeader("Access-Control-Expose-Headers", "*");
            // response->addHeader("wifi_name", "123");
            // response->addHeader("wifi_pass", "123");
            // response->addHeader("broker_ip", "192.168.1.1");
            // response->addHeader("broker_port", "1883");
            // response->addHeader("broker_username", "MqttClient");
            // response->addHeader("broker_pass", "123123");
            // response->addHeader("sending_frequency", "120");
            // request->send(response);
        }
        // Изменение настроек для соединения с сервером | "/system_settings/edit"
        void mgtt_edit(AsyncWebServerRequest *request) { // - (-) -----
            Serial.println("- (-) ----- \"/system_settings/edit\"");
            // Serial.println("get_info_registered_device");
            // int paramsNr = request->params();
            // Serial.println(paramsNr);
            // for (int i = 0; i < paramsNr; i++) {
            //     AsyncWebParameter *p = request->getParam(i);
            //     Serial.print("Param name: ");
            //     Serial.println(p->name());
            //     Serial.print("Param value: ");
            //     Serial.println(p->value());
            //     Serial.println("------");
            //     String param_name = p->name();
            //     if (param_name == "wifi_name") { // имя wi-fi сети
            //         String wifi_name = p->value();
            //     } else if (param_name == "wifi_pass") { // пароль wi-fi сети
            //         String wifi_pass = p->value();
            //     } else if (param_name == "broker_ip") { // пароль wi-fi сети
            //         String broker_ip = p->value();
            //     } else if (param_name == "broker_port") { // пароль wi-fi сети
            //         String broker_port = p->value();
            //     } else if (param_name == "broker_username") { // пароль wi-fi сети
            //         String broker_username = p->value();
            //     } else if (param_name == "broker_pass") { // пароль wi-fi сети
            //         String broker_pass = p->value();
            //     } else if (param_name == "sending_frequency") { // пароль wi-fi сети
            //         String sending_frequency = p->value();
            //     }
            // }
            // // somefunction(p->value) какая то функция, которая получает информацию
            // // о модуле
            // AsyncWebServerResponse *response =
            //     request->beginResponse(200, "text/plain", "Ok");
            // response->addHeader("Access-Control-Allow-Origin", "*");
            // response->addHeader("Access-Control-Expose-Headers", "*");
            // request->send(response);
        }

        // Системное время - отправка на телефон | "/system_time/get"
        void system_time_get(AsyncWebServerRequest *request) {
            // somefunction(p->value) какая то функция, которая получает
            // информацию о модуле
            AsyncWebServerResponse *response =
                request->beginResponse(200, "text/plain", "Ok");
            response->addHeader("Access-Control-Allow-Origin", "*");
            response->addHeader("Access-Control-Expose-Headers", "*");
            // Получение времени
            buf_size = 0;
            RtcDateTime datetime;
            if(gcm != nullptr)
                datetime = gcm->get_date_time();
            // Установка времени
            add_number(datetime.Year(), buffer, buf_size); // Год
            buffer[buf_size++] = '-';
            add_number(datetime.Month(), buffer, buf_size); // Месяц
            buffer[buf_size++] = '-';
            add_number(datetime.Day(), buffer, buf_size); // День
            buffer[buf_size++] = 'T';
            add_number(datetime.Hour(), buffer, buf_size); // Часы
            buffer[buf_size++] = ':';
            add_number(datetime.Minute(), buffer, buf_size); // Минуты
            buffer[buf_size++] = '\0';
            // Отправка времени
            response->addHeader("system_datetime", (char*)buffer);
            request->send(response);

#if defined(SERIAL_PRINT_TIME)
            Serial.println("system_time/get");
            Serial.println((char*)buffer);
#endif
            // Serial.println("system_time/get");
            // // somefunction(p->value) какая то функция, которая получает
            // // информацию о модуле
            // AsyncWebServerResponse *response =
            //     request->beginResponse(200, "text/plain", "Ok");
            // response->addHeader("Access-Control-Allow-Origin", "*");
            // response->addHeader("Access-Control-Expose-Headers", "*");
            // response->addHeader("system_datetime", "2021-10-21T21:30");
            // request->send(response);
        }
        // Системное время - установка с телефона | "/system_time/edit"
        void system_time_edit(AsyncWebServerRequest *request) {
            int paramsNr = request->params();
            bool set_time = false;
            for (int i = 0; i < paramsNr; i++) {
                AsyncWebParameter *p = request->getParam(i);
                String param_name = p->name();
                if (param_name == "system_datetime") { // Установка времени
                    String system_datetime = p->value();
                    uint8_t *buf = (uint8_t*)&system_datetime[0];
                    uint16_t bias = 0;
                    // Получение времени из пакета
                    uint16_t year;
                    uint8_t month, hour, day, minute;
                    year = get_number(buf, bias);
                    if(buf[bias++] != '-') break;
                    month = get_number(buf, bias);
                    if(buf[bias++] != '-') break;
                    day = get_number(buf, bias);
                    if(buf[bias++] != 'T') break;
                    hour = get_number(buf, bias);
                    if(buf[bias++] != ':') break;
                    minute = get_number(buf, bias);
                    set_time = true;
                    // Установка времени в RTC
                    RtcDateTime datetime{year, month, day, hour, minute, 0};
                    if(gcm != nullptr) {
                        gcm->set_date_time(datetime);
                    }
#if defined(SERIAL_PRINT_TIME)
                    Serial.print("system_datetime");
                    Serial.print(year);
                    Serial.print('-');
                    Serial.print(mount);
                    Serial.print('-');
                    Serial.print(day);
                    Serial.print("   ");
                    Serial.print(hour);
                    Serial.print(':');
                    Serial.print(minute);
                    Serial.println();
#endif
                }
            }
            // somefunction(p->value) какая то функция, которая получает информацию
            // о модуле
            AsyncWebServerResponse *response;
            if(set_time)
                response = request->beginResponse(200, "text/plain", "Ok");
            else
                response = request->beginResponse(200, "text/plain", "Error");
            response->addHeader("Access-Control-Allow-Origin", "*");
            response->addHeader("Access-Control-Expose-Headers", "*");
            request->send(response);

            // Serial.println("get_info_registered_device");
            // int paramsNr = request->params();
            // Serial.println(paramsNr);
            // for (int i = 0; i < paramsNr; i++) {
            //     AsyncWebParameter *p = request->getParam(i);
            //     Serial.print("Param name: ");
            //     Serial.println(p->name());
            //     Serial.print("Param value: ");
            //     Serial.println(p->value());
            //     Serial.println("------");
            //     String param_name = p->name();
            //     if (param_name == "system_datetime") { // имя wi-fi сети
            //         String system_datetime = p->value();
            //     }
            // }
            // // somefunction(p->value) какая то функция, которая получает информацию
            // // о модуле
            // AsyncWebServerResponse *response =
            //     request->beginResponse(200, "text/plain", "Ok");
            // response->addHeader("Access-Control-Allow-Origin", "*");
            // response->addHeader("Access-Control-Expose-Headers", "*");
            // request->send(response);
        }

        // Главная страница | "/settings" ("main_page.html")
        void main_page(AsyncWebServerRequest *request) {
            AsyncWebServerResponse *response = request->beginResponse(
                SPIFFS, "/main_page.html", String(), false);
            response->addHeader("Access-Control-Allow-Origin", "*");
            request->send(response);
        }
        // Иконка обновления | "/refresh.png" ("refresh.png")
        void refresh_icon(AsyncWebServerRequest *request) {
            request->send(SPIFFS, "/refresh.png", "image/png");
        }
        // Иконка delete | "/delete.png" ("delete.png")
        void delete_icon(AsyncWebServerRequest *request) {
            request->send(SPIFFS, "/delete.png", "image/png");
        }
        // Иконка '+' | "/plus-icon.png" ("plus-icon.png")
        void plus_icon(AsyncWebServerRequest *request) {
            request->send(SPIFFS, "/plus-icon.png", "image/png");
        }
        // Страница стиля | "/style.css" ("style.css")
        void style(AsyncWebServerRequest *request) {
            request->send(SPIFFS, "/style.css", "text/css");
        }

#endif

#ifdef TEST_CODE_15_10_2021

        // Страница для входа | "/" ("login_page.html")
        void login_page(AsyncWebServerRequest *request) {
            request->send(SPIFFS, "/login_page.html", String(), false);
        }
        // Процесс входа | "/login"
        void login_connection(AsyncWebServerRequest *request, uint8_t *data,
                              size_t len, size_t index, size_t total) {
            String a;
            for (size_t i = 0; i < len; i++) {
                a += (char)data[i];
            }

            gcm->

            Serial.println(a);
            if (a == esp32_id) { //проверка ввода id esp32
                AsyncWebServerResponse *response =
                    request->beginResponse(301); //код переадресации
                response->addHeader(
                    "Access-Control-Allow-Origin",
                    "*"); // заголовок, позволяющий делать запросы в одном домене
                response->addHeader("Location", "/settings"); // место переадресации
                request->send(response);
            } else {
                AsyncWebServerResponse *response = request->beginResponse(
                    401); // код неавторизированного пользователя
                response->addHeader("Access-Control-Allow-Origin", "*");
                request->send(response);
            }
            AsyncWebServerResponse *response =
            request->beginResponse(301); //код переадресации
            //          response->addHeader("Access-Control-Allow-Origin", "*"); //
            //          заголовок, позволяющий делать запросы в одном домене
            response->addHeader("Location", "/settings"); // место переадресации
            request->send(response);
        }

        // Список незарегестрированных модулей | "/devices/unregistered/get"
        void modules_unregistered(AsyncWebServerRequest *request) { // получение незарегистрированных модулей
            AsyncWebServerResponse *response =
                request->beginResponse(200, "text/plain", "Ok");
            response->addHeader("Access-Control-Allow-Origin", "*");
            response->addHeader("Access-Control-Expose-Headers", "*");
            response->addHeader("count_of_devices", "2");
            response->addHeader("device_ids", "1,2");
            response->addHeader("device_types", "sensor, lamp");
            request->send(response);
        }
        // Список зарегестрированных модулей | "/devices/registered/get"
        void modules_registered(AsyncWebServerRequest *request) { // получение зарегистрированных модулей
            AsyncWebServerResponse *response =
                request->beginResponse(200, "text/plain", "Ok");
            response->addHeader("Access-Control-Allow-Origin", "*");
            response->addHeader("Access-Control-Expose-Headers", "*");
            response->addHeader("count_of_devices", "2");
            response->addHeader("device_ids", "3,4");
            response->addHeader("device_names", "dim_lamps, nasosi");
            request->send(response);
        }

        // Отправка информации о модуле | "/registered_device/get"
        void module_data(AsyncWebServerRequest *request) { // получение инфы об одном зарегистрированном модуле
            Serial.println("get_info_registered_device");
            int paramsNr = request->params();
            Serial.println(paramsNr);

            for (int i = 0; i < paramsNr; i++) {
                AsyncWebParameter *p = request->getParam(i);
                Serial.print("Param name: ");
                Serial.println(p->name());
                Serial.print("Param value: ");
                Serial.println(p->value());
                Serial.println("------");
                String param_name = p->name();
                if (param_name == "device_id") {
                    String device_id = p->value();
                    Serial.println(device_id);
                }
            }

            // somefunction(p->value) какая то функция, которая получает информацию
            // о модуле
            AsyncWebServerResponse *response =
                request->beginResponse(200, "text/plain", "Ok");
            response->addHeader("Access-Control-Allow-Origin", "*");
            response->addHeader("Access-Control-Expose-Headers", "*");
            response->addHeader("device_name",
                                "Device"); // название модуля (я так понимаю stmки)
            response->addHeader("count_of_sensor_modules",
                                "2"); // кол-во сенсоров, подключенных к модулю
            response->addHeader("sensors_names",
                                "h2o, co2"); // название модулей сенсоров
            response->addHeader("sensors_ids", "1, 2"); // название модулей сенсоров
            response->addHeader("sensor_indications",
                                "0.333,0.444"); // последние показания датчиков
            response->addHeader("sensor_values",
                                "1,120"); //Тут не знаю как правильнее 
                                          //сделать подстрою под вас
            response->addHeader("count_of_device_modules",
                                "2"); // Это тип насосы дим лампы и т д
            response->addHeader("count_of_cycles_in_device_moduls",
                                "3, 2"); // количество циклов ШИМ
            response->addHeader("devices_names",
                                "lamp, nasos"); // название модулей сенсоров
            response->addHeader("mech_devices_ids",
                                "3, 4"); // название модулей сенсоров
            response->addHeader(
                "pwm_powers", "10,20,30;10,5"); //значения ШИМ разделитель внутри
                                                //каждого устройства ',' разделитель
                                                //между устройствами ';'
            response->addHeader("work_mode", "auto,manual"); // режимы работы
            response->addHeader(
                "status",
                "on,on"); // Это для режимов работы если авто, то всегда включен,
                          // если ручной может быть выключен или включен
            response->addHeader("manual_value",
                                "0,10"); // Для авто режима значения ШИМ нам
                                         // всеравно, для ручного - значение
            response->addHeader(
                "cycle_begin",
                "21:15,12:20,05:00;10:00,20:00"); // начало цикла ШИМ. Тут не знаю
                                                // как правильнее сделать подстрою
                                                // под вас
            response->addHeader(
                "cycle_end",
                "21:15,12:20,05:00;10:00,20:00"); //конец цикла ШИМ. Тут не знаю как
                                                //правильнее сделать подстрою под
                                                //вас
            response->addHeader(
                "turn_on", "10,1200,1800;36000,36000"); //Тут не знаю как правильнее
                                                        //сделать подстрою под вас
            response->addHeader("turn_off",
                                "3,300,600;7200,7200"); //Тут не знаю как правильнее
                                                        //сделать подстрою под вас
            request->send(response);

        }
        // Регистрация модуля | "/registered_device/add"
        void module_registration(AsyncWebServerRequest *request) { // Регистрация, редактирование нового устройства
            Serial.println("get_info_registered_device");
            int paramsNr = request->params();
            Serial.println(paramsNr);

            for (int i = 0; i < paramsNr; i++) {
                AsyncWebParameter *p = request->getParam(i);
                Serial.print("Param name: ");
                Serial.println(p->name());
                Serial.print("Param value: ");
                Serial.println(p->value());
                Serial.println("------");
                String param_name = p->name();
                if (param_name == "device_id") { // id всего модуля
                    String device_id = p->value();
                }

                else if (param_name == "count_of_sensor_modules") { // кол-во сенсоров, подключенных
                                                                    // к модулю '2'
                    String count_of_sensor_modules = p->value();
                } else if (param_name == "sensors_ids") { // id сенсоров, подключенных
                                                            // к модулю '1,2'
                    String sensors_ids = p->value();
                } else if (param_name ==
                            "sensors_values") { // периоды снятия показаний с датчиков
                                                // '1600,2000'
                    String sensors_values = p->value();
                } else if (param_name == "mech_devices_ids") { // // кол-во насосов,
                                                                // дим ламп и т д '2'
                    String mech_devices_ids = p->value();
                } else if (param_name ==
                            "pwm_powers") { // // значения ШИМ '10,20,30;10,5'
                    String pwm_powers = p->value();
                } else if (param_name ==
                            "cycle_begin") { // // начало цикла ШИМ
                                            // '21:15,12:20,05:00;10:00,20:00'
                    String cycle_begin = p->value();
                } else if (param_name ==
                            "cycle_end") { // // конец цикла ШИМ
                                            // '21:15,12:20,05:00;10:00,20:00'
                    String cycle_end = p->value();
                } else if (param_name == "turn_on") { // // время включения цикла ШИМ
                                                        // '600,300,3;7200,7200'

                    String turn_on = p->value();
                } else if (param_name == "turn_off") { // // время включения цикла ШИМ
                                                        // '600,300,3;7200,7200'

                    String turn_off = p->value();
                }
            }
            // somefunction(p->value) какая то функция, которая получает информацию
            // о модуле
            AsyncWebServerResponse *response =
                request->beginResponse(200, "text/plain", "Ok");
            response->addHeader("Access-Control-Allow-Origin", "*");
            response->addHeader("Access-Control-Expose-Headers", "*");
            request->send(response);
        }
        // Изменение модуля | "/registered_device/edit"
        void module_editing(AsyncWebServerRequest *request) { // Редактирование нового устройства
            Serial.println("get_info_registered_device");
            int paramsNr = request->params();
            Serial.println(paramsNr);

            for (int i = 0; i < paramsNr; i++) {
                AsyncWebParameter *p = request->getParam(i);
                Serial.print("Param name: ");
                Serial.println(p->name());
                Serial.print("Param value: ");
                Serial.println(p->value());
                Serial.println("------");
                String param_name = p->name();
                if (param_name == "device_id") { // id всего модуля
                    String device_id = p->value();
                }

                else if (param_name ==
                        "count_of_sensor_modules") { // кол-во сенсоров, подключенных
                                                        // к модулю '2'
                    String count_of_sensor_modules = p->value();
                } else if (param_name == "sensors_ids") { // id сенсоров, подключенных
                                                            // к модулю '1,2'
                    String sensors_ids = p->value();
                } else if (param_name ==
                            "sensors_values") { // периоды снятия показаний с датчиков
                                                // '1600,2000'
                    String sensors_values = p->value();
                } else if (param_name == "mech_devices_ids") { // // кол-во насосов,
                                                                // дим ламп и т д '2'
                    String mech_devices_ids = p->value();
                } else if (param_name ==
                            "pwm_powers") { // // значения ШИМ '10,20,30;10,5'
                    String pwm_powers = p->value();
                } else if (param_name ==
                            "cycle_begin") { // // начало цикла ШИМ
                                            // '21:15,12:20,05:00;10:00,20:00'
                    String cycle_begin = p->value();
                } else if (param_name ==
                            "cycle_end") { // // конец цикла ШИМ
                                            // '21:15,12:20,05:00;10:00,20:00'
                    String cycle_end = p->value();
                } else if (param_name == "turn_on") { // // время включения цикла ШИМ
                                                        // '600,300,3;7200,7200'

                    String turn_on = p->value();
                } else if (param_name == "turn_off") { // // время включения цикла ШИМ
                                                        // '600,300,3;7200,7200'
                    String turn_off = p->value();
                }
            }
            // somefunction(p->value) какая то функция, которая получает информацию
            // о модуле
            AsyncWebServerResponse *response =
                request->beginResponse(200, "text/plain", "Ok");
            response->addHeader("Access-Control-Allow-Origin", "*");
            response->addHeader("Access-Control-Expose-Headers", "*");
            request->send(response);
        }
        // Удаление модуля | "/registered_device/delete"
        void module_removing(AsyncWebServerRequest *request, uint8_t *data,
                             size_t len, size_t index, size_t total) {
            String device_id;
            for (size_t i = 0; i < len; i++) {
                device_id += (char)data[i];
            }
            Serial.println(device_id);
            AsyncWebServerResponse *response =
                request->beginResponse(301); //код переадресации
            response->addHeader(
                "Access-Control-Allow-Origin",
                "*"); // заголовок, позволяющий делать запросы в одном домене
            response->addHeader("Location",
                                "/settings"); // место переадресации
            request->send(response);
        }
        // Отправка данных об устройствах | "/change_device_mode"
        void module_changing_mode(AsyncWebServerRequest *request) { // Изменение режима работы (насосы, лампы)
            Serial.println("get_info_registered_device");
            int paramsNr = request->params();
            Serial.println(paramsNr);
            for (int i = 0; i < paramsNr; i++) {
                AsyncWebParameter *p = request->getParam(i);
                Serial.print("Param name: ");
                Serial.println(p->name());
                Serial.print("Param value: ");
                Serial.println(p->value());
                Serial.println("------");
                String param_name = p->name();
                if (param_name == "device_id") { // id всего модуля
                    String device_id = p->value();
                }

                else if (param_name == "work_mode") { // режим работы (auto, manual)
                    String work_mode = p->value();
                } else if (param_name == "status") { // статус устройства (on, off)
                    String status = p->value();
                } else if (param_name == "manual_value") { // значение ШИМ (0-100)
                    String manual_value = p->value();
                }
            }
            // somefunction(p->value) какая то функция, которая получает информацию
            // о модуле
            AsyncWebServerResponse *response =
                request->beginResponse(200, "text/plain", "Ok");
            response->addHeader("Access-Control-Allow-Origin", "*");
            response->addHeader("Access-Control-Expose-Headers", "*");
            request->send(response);
      }
        // Отправка данных о датчиках | "/sensor_device/get"
        void module_sensor_data(AsyncWebServerRequest *request) {
            Serial.println("get_info_registered_device");
            int paramsNr = request->params();
            Serial.println(paramsNr);
            for (int i = 0; i < paramsNr; i++) {
                AsyncWebParameter *p = request->getParam(i);
                Serial.print("Param name: ");
                Serial.println(p->name());
                Serial.print("Param value: ");
                Serial.println(p->value());
                Serial.println("------");
                String param_name = p->name();
                if (param_name == "sensor_id") { // имя wi-fi сети
                    String sensor_id = p->value();
                }
            }
            // somefunction(p->value) какая то функция, которая получает информацию
            // о cенсоре
            AsyncWebServerResponse *response =
                request->beginResponse(200, "text/plain", "Ok");
            response->addHeader("Access-Control-Allow-Origin", "*");
            response->addHeader("Access-Control-Expose-Headers", "*");
            response->addHeader("sensor_indications", "132");
            request->send(response);
        }

        // Установка настроек для соединения с сервером | "/system_settings/get"
        void mgtt_using(AsyncWebServerRequest *request) { // Изменение режима работы (насосы, лампы)
            Serial.println("system_settings/get");
            // somefunction(p->value) какая то функция, которая получает
            // информацию о модуле
            AsyncWebServerResponse *response =
                request->beginResponse(200, "text/plain", "Ok");
            response->addHeader("Access-Control-Allow-Origin", "*");
            response->addHeader("Access-Control-Expose-Headers", "*");
            response->addHeader("wifi_name", "123");
            response->addHeader("wifi_pass", "123");
            response->addHeader("broker_ip", "192.168.1.1");
            response->addHeader("broker_port", "1883");
            response->addHeader("broker_username", "MqttClient");
            response->addHeader("broker_pass", "123123");
            response->addHeader("sending_frequency", "120");
            request->send(response);
        }
        // Изменение настроек для соединения с сервером | "/system_settings/edit"
        void mgtt_edit(AsyncWebServerRequest *request) { // Редактирование нового устройства
            Serial.println("get_info_registered_device");
            int paramsNr = request->params();
            Serial.println(paramsNr);
            for (int i = 0; i < paramsNr; i++) {
                AsyncWebParameter *p = request->getParam(i);
                Serial.print("Param name: ");
                Serial.println(p->name());
                Serial.print("Param value: ");
                Serial.println(p->value());
                Serial.println("------");
                String param_name = p->name();
                if (param_name == "wifi_name") { // имя wi-fi сети
                    String wifi_name = p->value();
                } else if (param_name == "wifi_pass") { // пароль wi-fi сети
                    String wifi_pass = p->value();
                } else if (param_name == "broker_ip") { // пароль wi-fi сети
                    String broker_ip = p->value();
                } else if (param_name == "broker_port") { // пароль wi-fi сети
                    String broker_port = p->value();
                } else if (param_name == "broker_username") { // пароль wi-fi сети
                    String broker_username = p->value();
                } else if (param_name == "broker_pass") { // пароль wi-fi сети
                    String broker_pass = p->value();
                } else if (param_name == "sending_frequency") { // пароль wi-fi сети
                    String sending_frequency = p->value();
                }
            }
            // somefunction(p->value) какая то функция, которая получает информацию
            // о модуле
            AsyncWebServerResponse *response =
                request->beginResponse(200, "text/plain", "Ok");
            response->addHeader("Access-Control-Allow-Origin", "*");
            response->addHeader("Access-Control-Expose-Headers", "*");
            request->send(response);
        }

        // Системное время - отправка на телефон | "/system_time/get"
        void  system_time_get(AsyncWebServerRequest *request) { // Изменение режима работы (насосы, лампы)
            Serial.println("system_time/get");

            // somefunction(p->value) какая то функция, которая получает
            // информацию о модуле
            AsyncWebServerResponse *response =
                request->beginResponse(200, "text/plain", "Ok");
            response->addHeader("Access-Control-Allow-Origin", "*");
            response->addHeader("Access-Control-Expose-Headers", "*");
            response->addHeader("system_datetime", "2021-10-21T21:30");
            request->send(response);

        }
        // Системное время - установка с телефона | "/system_time/edit"
        void system_time_edit(AsyncWebServerRequest *request) { // Редактирование нового устройства
            Serial.println("get_info_registered_device");
            int paramsNr = request->params();
            Serial.println(paramsNr);
            for (int i = 0; i < paramsNr; i++) {
                AsyncWebParameter *p = request->getParam(i);
                Serial.print("Param name: ");
                Serial.println(p->name());
                Serial.print("Param value: ");
                Serial.println(p->value());
                Serial.println("------");
                String param_name = p->name();
                if (param_name == "system_datetime") { // имя wi-fi сети
                    String system_datetime = p->value();
                }
            }
            // somefunction(p->value) какая то функция, которая получает информацию
            // о модуле
            AsyncWebServerResponse *response =
                request->beginResponse(200, "text/plain", "Ok");
            response->addHeader("Access-Control-Allow-Origin", "*");
            response->addHeader("Access-Control-Expose-Headers", "*");
            request->send(response);
        }

        // Главная страница | "/settings" ("main_page.html")
        void main_page(AsyncWebServerRequest *request) { // основная страница
            AsyncWebServerResponse *response = request->beginResponse(
                SPIFFS, "/main_page.html", String(), false);
            response->addHeader("Access-Control-Allow-Origin", "*");
            request->send(response);
        }
        // Иконка обновления | "/refresh" ("refresh.png")
        void refresh_icon(AsyncWebServerRequest *request) {
            request->send(SPIFFS, "/refresh.png", "image/png");
        }
        // Иконка '+' | "/plus-icon" ("plus-icon.png")
        void plus_icon(AsyncWebServerRequest *request) {
            request->send(SPIFFS, "/plus-icon.png", "image/png");
        }
        // Страница стиля | "/style.css" ("style.css")
        void style(AsyncWebServerRequest *request) {
            request->send(SPIFFS, "/style.css", "text/css");
        }

#endif

    }
}
