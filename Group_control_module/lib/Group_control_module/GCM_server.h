#ifndef GCM_SERVER_H_
#define GCM_SERVER_H_

#include <Arduino.h>
#include <Group_control_module.h>

#include <ESPAsyncWebServer.h> //библиотека веб сервера esp
#include <SPIFFS.h> //библиотека файловой системы SPIFFS
#include <WiFi.h> //библиотека для использования Wi-Fi на esp

// #define WIFI_SSID "Green_Pro"    //имя Wi-Fi сети
// #define WIFI_PASSWORD "12341234" //пароль Wi-Fi сети
// #define WIFI_SSID "a202"         //имя Wi-Fi сети
// #define WIFI_PASSWORD "gamma113" //пароль Wi-Fi сети
#define WIFI_SSID "213_Guest"         //имя Wi-Fi сети
#define WIFI_PASSWORD "11081975" //пароль Wi-Fi сети
#define esp32_id "123" //Номер платы esp32

// creating a local server
namespace lsc {

    namespace server_GCM {

        void init(Group_control_module &gcm, bool spiffs_init=true);

        // Страница для входа
        void login_page(AsyncWebServerRequest *request);       // "/" ("login_page.html")
        // Процесс входа
        void login_connection(AsyncWebServerRequest *request, uint8_t *data,
                              size_t len, size_t index, size_t total); // "/login"

        // Список незарегестрированных модулей
        void modules_unregistered(AsyncWebServerRequest *request); // "/devices/unregistered/get"
        // Список зарегестрированных модулей
        void modules_registered(AsyncWebServerRequest *request);   // "/devices/registered/get"

        // Отправка информации о модуле
        void module_data(AsyncWebServerRequest *request);          // "/registered_device/get"
        // Регистрация модуля
        void module_registration(AsyncWebServerRequest *request);  // "/registered_device/add"
        // Изменение модуля
        void module_editing(AsyncWebServerRequest *request);       // "/registered_device/edit"
        // Удаление модуля
        void module_removing(AsyncWebServerRequest *request, uint8_t *data, size_t len,
                             size_t index, size_t total);          // "/registered_device/delete"
        // Отправка данных об устройствах
        void module_changing_mode(AsyncWebServerRequest *request); // "/change_device_mode"
        // Отправка данных о датчиках
        void module_sensor_data(AsyncWebServerRequest *request);   // "/sensor_device/get"

        // Установка настроек для соединения с сервером
        void mgtt_using(AsyncWebServerRequest *request); // "/system_settings/get"
        // Изменение настроек для соединения с сервером
        void mgtt_edit(AsyncWebServerRequest *request);  // "/system_settings/edit"

        // Системное время - отправка на телефон
        void system_time_get(AsyncWebServerRequest *request); // "/system_time/get"
        // Системное время - установка с телефона
        void system_time_edit(AsyncWebServerRequest *request); // "/system_time/edit"

        // Главная страница
        void main_page(AsyncWebServerRequest *request); // "/settings" ("main_page.html")
        // Иконка обновления
        void refresh_icon(AsyncWebServerRequest *request);  // "/refresh" ("refresh.png")
        // Иконка '+'
        void plus_icon(AsyncWebServerRequest *request);  // "/plus-icon" ("plus-icon.png")
        // Страница стиля
        void style(AsyncWebServerRequest *request);  // "/style.css" ("style.css")
        // void ?(AsyncWebServerRequest *request); // "/register_device"
    }
}

//   ; -D CREATE_SERVER=1
//   ; -D START_WiFi=1

#endif // GCM_SERVER_H_
