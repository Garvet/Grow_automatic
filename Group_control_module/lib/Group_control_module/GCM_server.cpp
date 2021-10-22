#include "GCM_server.h"

#define START_WIFI_CONNECT
// #define TEST_CODE_15_10_2021

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

        AsyncWebServer server{80};
        uint8_t buffer[1024]{};
        Group_control_module *gcm=nullptr;

        // std::array<uint8_t, AMT_BYTES_SYSTEM_ID> get_system_id() const;

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
            server.on("/registered_device/get",    HTTP_GET, module_data);
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
            if(len == AMT_BYTES_SYSTEM_ID * 2) {
                std::array<uint8_t, AMT_BYTES_SYSTEM_ID> system_id = gcm->get_system_id();
                bool error_id = false;
                for (size_t i = 0; i < len; i++) {
                    uint8_t num;
                    if(('A' <= data[i]) && (data[i] <= 'F')) {
                        num = data[i] - 'A' + 10;
                    }
                    else if(('a' <= data[i]) && (data[i] <= 'f')) {
                        num = data[i] - 'a' + 10;
                    } 
                    else if(('0' <= data[i]) && (data[i] <= '9')) {
                        num = data[i] - '0';
                    }
                    else {
                        error_id = true;
                        break;
                    }
                    if(i%2 == 0) {
                        buffer[i/2] = num << 4;
                    }
                    else {
                        buffer[i/2] |= num;
                    }
                    // buffer[i] = data[i];
                }
                if(!error_id) {

                    Serial.print("ID+{");
                    for (size_t i = 0; i < AMT_BYTES_SYSTEM_ID; i++) {
                        Serial.print(buffer[i], HEX);
                    }
                    correct_id = true;
                    for(size_t i = 0; i < AMT_BYTES_SYSTEM_ID; i++) {
                        if(buffer[i] != system_id[i])
                            correct_id = false;
                    }

                    if(correct_id) {
                        Serial.print(" == ");
                    }
                    else {
                        Serial.print(" != ");
                    }

                    for (size_t i = 0; i < AMT_BYTES_SYSTEM_ID; i++) {
                        Serial.print(system_id[i], HEX);
                    }
                    Serial.println("}");
                }
            }
            if(!correct_id) {
                Serial.print("ID-{");
                for (size_t i = 0; i < len; i++) {
                    Serial.print((char)data[i]);
                }
                Serial.print(" != ");
                for (size_t i = 0; i < AMT_BYTES_SYSTEM_ID; i++) {
                    Serial.print(gcm->get_system_id()[i], HEX);
                }
                Serial.println("}");
            }
            
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
            // AsyncWebServerResponse *response =
            //     request->beginResponse(200, "text/plain", "Ok");
            // response->addHeader("Access-Control-Allow-Origin", "*");
            // response->addHeader("Access-Control-Expose-Headers", "*");
            // response->addHeader("count_of_devices", "2");
            // response->addHeader("device_ids", "3,4");
            // response->addHeader("device_names", "dim_lamps, nasosi");
            // request->send(response);
        }

        // Отправка информации о модуле | "/registered_device/get"
        void module_data(AsyncWebServerRequest *request) {
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
        // Регистрация модуля | "/registered_device/add"
        void module_registration(AsyncWebServerRequest *request) {
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
        void module_editing(AsyncWebServerRequest *request) {
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
                             size_t len, size_t index, size_t total) {
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
        void module_changing_mode(AsyncWebServerRequest *request) {
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
            //     else if (param_name == "work_mode") { // режим работы (auto, manual)
            //         String work_mode = p->value();
            //     } else if (param_name == "status") { // статус устройства (on, off)
            //         String status = p->value();
            //     } else if (param_name == "manual_value") { // значение ШИМ (0-100)
            //         String manual_value = p->value();
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
        // Отправка данных о датчиках | "/sensor_device/get"
        void module_sensor_data(AsyncWebServerRequest *request) {
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
        void mgtt_using(AsyncWebServerRequest *request) {
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
        void mgtt_edit(AsyncWebServerRequest *request) {
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
        void  system_time_get(AsyncWebServerRequest *request) {
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
