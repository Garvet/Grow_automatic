#include <Arduino.h>
#include "ESPAsyncWebServer.h" //библиотека веб сервера esp
#include "SPIFFS.h" //библиотека файловой системы SPIFFS
#include "WiFi.h" //библиотека для использования Wi-Fi на esp
#define WIFI_SSID "a202"         //имя Wi-Fi сети
#define WIFI_PASSWORD "gamma113" //пароль Wi-Fi сети

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
  server.on("/get_unregistered_devices", HTTP_GET,
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
  server.on("/get_registered_devices", HTTP_GET,
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
      "/get_info_registered_device", HTTP_GET,
      [](AsyncWebServerRequest
             *request) { // получение инфы об одном зарегистрированном модуле
        Serial.println("get_info_registered_device");
        if (request->hasHeader("device_id")) {
          AsyncWebHeader *h = request->getHeader("device_id");
          Serial.printf("device_id: %s\n", h->value().c_str());
        }
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
        response->addHeader("sensor_indications",
                            "0.333,0.444"); // последние показания датчиков
        response->addHeader("sensor_values",
                            "1 secs,2 mins"); //Тут не знаю как правильнее
                                              //сделать подстрою под вас
        response->addHeader("count_of_device_modules",
                            "2"); // Это тип насосы дим лампы и т д
        response->addHeader("count_of_cycles_in_device_moduls",
                            "3, 2"); // количество циклов ШИМ
        response->addHeader(
            "pwm_powers", "10,20,30;10,5"); //значения ШИМ разделитель внутри
                                            //каждого устройства ',' разделитель
                                            //между устройствами ';'
        response->addHeader("work_mode", "auto, auto"); // режимы работы
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

  // URL для подключения к mqtt брокеру
  server.on("/Connecttomqtt", HTTP_GET, [](AsyncWebServerRequest *request) {
    a = 1;
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  // URL для отключения от mqtt брокера
  server.on("/Disconnectmqtt", HTTP_GET, [](AsyncWebServerRequest *request) {
    a = 0;
    Serial.println("Disconnected from mqtt");
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  // URL для подписки на топик (еще не реализовано)
  server.on("/Subscribe", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", String(), false);
  });

  // URL для отписки от топика (еще не реализовано)
  server.on("/Unsubscribe", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", String(), false);
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
