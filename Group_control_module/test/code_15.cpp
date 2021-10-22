#include "ESPAsyncWebServer.h" //библиотека веб сервера esp
#include "SPIFFS.h" //библиотека файловой системы SPIFFS
#include "WiFi.h" //библиотека для использования Wi-Fi на esp
#define WIFI_SSID "Green_Pro"    //имя Wi-Fi сети
#define WIFI_PASSWORD "12341234" //пароль Wi-Fi сети

#define esp32_id "123" //Номер платы esp32

// Создаем экземпляр класса «AsyncWebServer»
// под названием «server» и задаем ему номер порта «80»:
AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);
  WiFi.onEvent(WiFiEvent); //задает то. что при подключении к wi-fi будет
                           //запущена функция обратного вызова WiFiEvent(),
                           //которая напечатает данные о WiFi подключении
  connectToWifi();
  // Инициализируем фалойвую систему SPIFFS:
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    //  "При монтировании SPIFFS произошла ошибка"
    return;
  }

  // URL для корневой страницы веб-сервера:
  Serial.println(WiFi.localIP());
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/login_page.html", String(), false);
  });

  server.on(
      "/login", // эндпоинт логина
      HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
      [](AsyncWebServerRequest *request, uint8_t *data, size_t len,
         size_t index, size_t total) {
        String a;
        for (size_t i = 0; i < len; i++) {
          a += (char)data[i];
        }

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
          request->send(response);
        }
      });
  server.on("/devices/unregistered/get", HTTP_GET,
            [](AsyncWebServerRequest
                   *request) { // получение незарегистрированных модулей
              AsyncWebServerResponse *response =
                  request->beginResponse(200, "text/plain", "Ok");
              response->addHeader("Access-Control-Allow-Origin", "*");
              response->addHeader("Access-Control-Expose-Headers", "*");
              response->addHeader("count_of_devices", "2");
              response->addHeader("device_ids", "1,2");
              response->addHeader("device_types", "sensor, lamp");
              request->send(response);
            });
  server.on("/devices/registered/get", HTTP_GET,
            [](AsyncWebServerRequest
                   *request) { // получение зарегистрированных модулей
              AsyncWebServerResponse *response =
                  request->beginResponse(200, "text/plain", "Ok");
              response->addHeader("Access-Control-Allow-Origin", "*");
              response->addHeader("Access-Control-Expose-Headers", "*");
              response->addHeader("count_of_devices", "2");
              response->addHeader("device_ids", "3,4");
              response->addHeader("device_names", "dim_lamps, nasosi");
              request->send(response);
            });

  server.on(
      "/registered_device/get", HTTP_GET,
      [](AsyncWebServerRequest
             *request) { // получение инфы об одном зарегистрированном модуле
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
                            "1 secs,2 mins"); //Тут не знаю как правильнее
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
            "turn_on",
            "10 secs,20 mins,30 mins;10 hours,10 hours"); //Тут не знаю как
                                                          //правильнее сделать
                                                          //подстрою под вас
        response->addHeader(
            "turn_off",
            "3 secs,5 mins,10 mins;2 hours,2 hours"); //Тут не знаю как
                                                      //правильнее сделать
                                                      //подстрою под вас
        request->send(response);

      });

  server.on(
      "/registered_device/add", HTTP_GET,
      [](AsyncWebServerRequest
             *request) { // Регистрация, редактирование нового устройства
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

      });

  server.on(
      "/registered_device/edit", HTTP_GET,
      [](AsyncWebServerRequest *request) { // Редактирование нового устройства
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

      });

  server.on("/registered_device/delete", HTTP_POST,
            [](AsyncWebServerRequest *request) {}, NULL,
            [](AsyncWebServerRequest *request, uint8_t *data, size_t len,
               size_t index, size_t total) {
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
            });

  server.on(
      "/change_device_mode", HTTP_GET,
      [](AsyncWebServerRequest
             *request) { // Изменение режима работы (насосы, лампы)
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

      });

  server.on("/settings", HTTP_GET,
            [](AsyncWebServerRequest *request) { // основная страница
              AsyncWebServerResponse *response = request->beginResponse(
                  SPIFFS, "/main_page.html", String(), false);
              response->addHeader("Access-Control-Allow-Origin", "*");
              request->send(response);
            });
  server.on("/register_device", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(200);
    //    const auto& header = request->getHeader("test");
    //    Serial.println(header);

    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });
  // URL для файла «style.css»:
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });

  server.begin();
}

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

void loop() {}