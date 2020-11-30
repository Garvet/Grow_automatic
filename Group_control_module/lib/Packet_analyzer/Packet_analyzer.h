// #define __PACKET_ANALYZER_H__ // (-) -----
#ifndef __PACKET_ANALYZER_H__
#define __PACKET_ANALYZER_H__
#define __PACKET_ANALYZER_H__OLD // (-) -----


#include <Arduino.h>
#include <Address_field.h>
#include <LoRa_packet.h>

#define MINIMAL_PACKET_SIZE 9 //
#define PACKET_HEADER_SIZE 9
extern const uint16_t LORA_ADDRESS_BRANCH;

// #define PACKET_CONNECTION 0x00
// #define PACKET_SENSOR 0x01
// #define PACKET_DEVICE 0x02
// #define PACKET_SYSTEM 0x03

// (?) ----- использовать пространство?

// Адресация в LoRa-сети
// (!) ----- Поменяй глобальные адреса на рассчитываемые
#define LORA_GLOBAL_ADDRESS    0x01FF7FFF
#define LORA_GLOBAL_ADR_GROUP  0x1FF
#define LORA_GLOBAL_ADR_BRANCH 0x7FFF
class LoRa_address {
public:
    uint16_t group = 0xFFFF;  // Адрес группы
    uint16_t branch = 0xFFFF; // Адрес ветви
public:
    LoRa_address() = default;
    LoRa_address(const uint16_t group, const uint16_t branch);
    LoRa_address(const uint32_t adr);
    LoRa_address(const LoRa_address &adr) = default;
    ~LoRa_address() = default;

    bool global();

    friend bool operator==(const LoRa_address& left, const LoRa_address& right);
    friend bool operator!=(const LoRa_address& left, const LoRa_address& right);
    friend bool operator==(const LoRa_address& left, const uint32_t& right);
    friend bool operator!=(const LoRa_address& left, const uint32_t& right);
    friend bool operator==(const uint32_t& left, const LoRa_address& right);
    friend bool operator!=(const uint32_t& left, const LoRa_address& right);
};

// Пакеты в LoRa-сети
// Типы пакетов
enum class Packet_Type {
    CONNECTION = 0x00,
    SENSOR,
    DEVICE,
    SYSTEM
};

class Packet_analyzer {
protected:
    Address_field **field_header_; // поля шапки
    Address_field **field_packet_; // поля пакета
    uint16_t amt_field_header_; // количество полей шапки
    uint16_t amt_field_packet_; // количество полей пакета

    uint8_t setting_; // настройки передатчика (общие)

    size_t last_filled_byte = 0; // номер последнего заполненного байта пакета (не шапки)
    size_t last_read_byte = 0;  // номер последнего прочитанного байта пакета (не шапки)

    // // LoRa_packet* packet;
    // uint8_t lenght_;
    // uint8_t receiv_lenght_;
    // uint8_t send_lenght_;
    // Address_field **field_;
    // uint16_t count_field_;
    // uint16_t max_address_;
    // uint8_t setting_;
    // bool creat_packet_;
    // bool set_field(Address_field **field, uint16_t count_field);
public:
    Packet_analyzer();
    ~Packet_analyzer()=default;
    // bool select_packet(LoRa_packet* packet);
    // uint8_t get_receiv_lenght();
    // uint8_t get_send_lenght();
    
    uint16_t     get_dest_adr_group(LoRa_packet& packet);  // Адрес группы адресанта
    uint16_t     get_dest_adr_branch(LoRa_packet& packet); // Адрес ветви  адресанта
    LoRa_address get_dest_adr(LoRa_packet& packet);        // Адрес адресанта
    uint16_t     get_sour_adr_group(LoRa_packet& packet);  // Адрес группы отправителя
    uint16_t     get_sour_adr_branch(LoRa_packet& packet); // Адрес ветви  отправителя
    LoRa_address get_sour_adr(LoRa_packet& packet);        // Адрес отправителя
    uint8_t      get_packet_type(LoRa_packet& packet);     // Тип пакета
    uint16_t     get_packet_number(LoRa_packet& packet);   // Номер пакета

    bool set_dest_adr_group (LoRa_packet& packet, uint16_t adr);   // Адрес группы адресанта
    bool set_dest_adr_branch(LoRa_packet& packet, uint16_t adr);   // Адрес ветви  адресанта
    bool set_dest_adr(LoRa_packet& packet, LoRa_address adr);      // Адрес адресанта
    bool set_sour_adr_group (LoRa_packet& packet, uint16_t adr);   // Адрес группы отправителя
    bool set_sour_adr_branch(LoRa_packet& packet, uint16_t adr);   // Адрес ветви  отправителя
    bool set_sour_adr(LoRa_packet& packet, LoRa_address adr);      // Адрес отправителя
    bool set_packet_type  (LoRa_packet& packet, uint8_t pac_type); // Тип пакета
    bool set_packet_number(LoRa_packet& packet, uint16_t num);     // Номер пакета

    virtual bool set_setting(uint8_t setting=0);
    uint8_t get_setting();
};


class Packet_Connection: public Packet_analyzer {
private:
    uint8_t command_ = 0;
public:
    virtual bool set_setting(uint8_t setting=0); // установить настройки

    /// --- Запись в пакет ---
    // Установить команду
    uint8_t set_command(LoRa_packet& packet, uint8_t com);
    // Установить данные
    uint8_t set_data(LoRa_packet& packet, uint8_t *data, uint8_t len);
    // Занести в пакет параметры и данные
    uint8_t set_packet_data(LoRa_packet& packet, uint8_t *com, uint8_t *data, uint8_t *len); 

    /// --- Чтение из пакета ---
    // Получить команду
    uint8_t get_command(LoRa_packet& packet, uint8_t *com);
    // Получить данные
    uint8_t get_data(LoRa_packet& packet, uint8_t *data, uint8_t *len);
    // Получить из пакета параметры и данные
    uint8_t get_packet_data(LoRa_packet& packet, uint8_t *com, uint8_t *data, uint8_t *len); 
    
    /// --- Расчёты ---
    // Узнать объём поля данных по параметрам
    uint8_t get_size_by_data(uint8_t *com, uint8_t *len, uint8_t &size_data); 
    // Узнать объём поля данных по содержимому пакета
    uint8_t get_size_by_packet(LoRa_packet& packet, uint8_t &size_data); 
};

class Packet_Sensor: public Packet_analyzer {
private:
    // uint8_t param_; // (--) ----- исключить
public:
    virtual bool set_setting(uint8_t setting=0); // установить настройки

    /// --- Запись в пакет ---
    // занести в пакет параметры и данные
    uint8_t set_packet_data(LoRa_packet& packet, uint8_t *amt, uint8_t *param, uint8_t *num, uint32_t *data);

    /// --- Чтение из пакета ---
    // получить из пакета параметры и данные
    uint8_t get_packet_data(LoRa_packet& packet, uint8_t *amt, uint8_t *param, uint8_t *num, uint32_t *data);
    bool get_count   (LoRa_packet& packet,  uint8_t &amt); // setting[0] == 1 иначе вернёт 1
    // bool get_parametr(LoRa_packet& packet,  uint8_t &param, uint8_t *num);
    // bool get_id_component(LoRa_packet& packet, uint8_t &id, uint8_t *num, uint8_t *param); // setting[2] == 1 иначе вернёт 0
    // bool get_data    (LoRa_packet& packet,  uint32_t &data, uint8_t *num, uint8_t *param); // len = {1, 2, 4}
    
    /// --- Расчёты ---
    // узнать объём поля данных по параметрам
    uint8_t get_size_by_data(uint8_t *amt, uint8_t *param, uint8_t &size_data);
    // узнать объём поля данных по содержимому пакета
    uint8_t get_size_by_packet(LoRa_packet& packet, uint8_t *amt, uint8_t *param, uint8_t &size_data);
};

#endif // __PACKET_ANALYZER_H__







#ifndef __PACKET_ANALYZER_H__OLD
#define __PACKET_ANALYZER_H__OLD

#include <Arduino.h>
#include <Address_field.h>
#include <LoRa_packet.h>
#include <vector>

#define MINIMAL_PACKET_SIZE 9
extern const uint16_t LORA_ADDRESS_BRANCH;
#define PACKET_CONNECTION 0x00
#define PACKET_SENSOR 0x01
#define PACKET_DEVICE 0x02
#define PACKET_SYSTEM 0x03

// (!) ----- Поменяй глобальные адреса на рассчитываемые
#define LORA_GLOBAL_ADDRESS    0x01FF7FFF
#define LORA_GLOBAL_ADR_GROUP  0x1FF
#define LORA_GLOBAL_ADR_BRANCH 0x7FFF
class LoRa_address {
public:
    uint16_t group = 0xFFFF;  // Адрес группы
    uint16_t branch = 0xFFFF; // Адрес ветви
public:
    LoRa_address() = default;
    LoRa_address(const uint16_t group, const uint16_t branch);
    LoRa_address(const uint32_t adr);
    LoRa_address(const LoRa_address &adr) = default;
    ~LoRa_address() = default;

    bool global();

    friend bool operator==(const LoRa_address& left, const LoRa_address& right);
    friend bool operator!=(const LoRa_address& left, const LoRa_address& right);
    friend bool operator==(const LoRa_address& left, const uint32_t& right);
    friend bool operator!=(const LoRa_address& left, const uint32_t& right);
    friend bool operator==(const uint32_t& left, const LoRa_address& right);
    friend bool operator!=(const uint32_t& left, const LoRa_address& right);
};

class Packet_analyzer {
protected:
public: // (!)(!)(!) ----- (-) -----
    uint8_t *_packet; // указатель на пакет с данными
    uint8_t _length; // длина пакета
    uint8_t _receive_length; // указатель обработчика приёма (для работы в случае ошибки) (--) ----- исключить
    uint8_t _send_length; // указатель опработчика отправки (для работы в случае ошибки) (--) ----- исключить
    Address_field **_field; // указатель на массив полей пакета
    uint16_t _count_field; // количество полей
    uint16_t _max_address; // максимаьлный адрес регистров пакета
    uint8_t _setting; // настройки передатчика
    bool _creat_packet; // пакет был создан классом (использовался ли new), (!) не используется

    bool set_field(Address_field **field, uint16_t count_field); // присваение полей
public:
    Packet_analyzer();
    ~Packet_analyzer()=default;
    // выбор пакета для обработки
    bool select_packet(LoRa_packet& packet);
    bool select_packet(uint8_t* packet, uint8_t length);
    // получить количество пройденных байт обработчиком приёма
    uint8_t get_receive_length(); //  (--) ----- исключить
    // получить количество пройденных байт обработчиком отправки
    uint8_t get_send_length(); //  (--) ----- исключить
    
    uint16_t get_dest_adr_group();  // получить адрес группы адресанта
    uint16_t get_dest_adr_branch(); // получить адрес ветви  адресанта
    LoRa_address get_dest_adr();    // получить адрес адресанта
    uint16_t get_sour_adr_group();  // получить адрес группы отправителя
    uint16_t get_sour_adr_branch(); // получить адрес ветви  отправителя
    LoRa_address get_sour_adr();    // получить адрес отправителя
    uint8_t  get_packet_type();     // получить тип пакета
    uint16_t get_packet_number();   // получить номер пакета

    bool set_dest_adr_group(uint16_t adr);  // установить адрес группы адресанта
    bool set_dest_adr_branch(uint16_t adr); // установить адрес ветви  адресанта
    bool set_dest_adr(LoRa_address adr); // установить адрес адресанта
    bool set_sour_adr_group(uint16_t adr);  // установить адрес группы отправителя
    bool set_sour_adr_branch(uint16_t adr); // установить адрес ветви  отправителя
    bool set_sour_adr(LoRa_address adr); // установить адрес отправителя
    bool set_packet_type(uint8_t pac_type); // установить тип пакета
    bool set_packet_number(uint16_t num);   // установить номер пакета

    virtual bool set_setting(uint8_t setting=0); // установить настройки (-) ----- нет get
};


class Packet_Connection: public Packet_analyzer {
private:
    uint8_t _command; // (--) ----- исключить
public:
    virtual bool set_setting(uint8_t setting=0); // установить настройки (-) ----- нет get

    // занести в пакет параметры и данные
    uint8_t set_packet_data(uint8_t *com, uint8_t *data, uint8_t *len); 
    // получить из пакета параметры и данные
    uint8_t get_packet_data(uint8_t *com, uint8_t *data, uint8_t *len); 
    
    // узнать объём поля данных по параметрам
    uint8_t get_size_by_data(uint8_t *com, uint8_t *size_data, uint8_t *len); 
    // узнать объём поля данных по содержимому пакета
    uint8_t get_size_by_packet(uint8_t *size_data); 

    bool set_command(uint8_t com); // (-) ----- не реализовано
    bool set_data(uint8_t *data, uint8_t len); // (-) ----- не реализовано
};

class Packet_Sensor: public Packet_analyzer {
private:
    uint8_t _param; // (--) ----- исключить
public:
    virtual bool set_setting(uint8_t setting=0); // установить настройки (-) ----- нет get

    // занести в пакет параметры и данные
    uint8_t set_packet_data(uint8_t *amt, uint8_t *param, uint8_t *num, uint32_t *data);
    // получить из пакета параметры и данные
    uint8_t get_packet_data(uint8_t *amt, uint8_t *param, uint8_t *num, uint32_t *data);

    // узнать объём поля данных по параметрам
    uint8_t get_size_by_data(uint8_t *amt, uint8_t *param, uint8_t *size_data);
    // узнать объём поля данных по содержимому пакета
    uint8_t get_size_by_packet(uint8_t *amt, uint8_t *param, uint8_t *size_data);

    bool set_count(uint8_t amt); // (-) ----- не реализовано // setting[0] == 1
    bool set_parametr(uint8_t *param); // (-) ----- не реализовано
    bool set_number(uint8_t num); // (-) ----- не реализовано // setting[2] == 1
    bool set_data(uint32_t *data); // (-) ----- не реализовано // len = {2, 1, 4} 2 - PWM, 1 - On|Off, 4 - float
};

class Packet_Device: public Packet_analyzer {
private:
    uint8_t _object; // при setting[0] == 1 в пакет не запишется, но нужен для последующих параметров
    uint8_t _command;

public:
    virtual bool set_setting(uint8_t setting = 0); // установить настройки (-) ----- нет get

    // занести в пакет параметры и данные
    uint8_t set_packet_data(uint8_t *obj, uint8_t *num, uint8_t *com, uint8_t *data, uint8_t *len);
    // получить из пакета параметры и данные
    uint8_t get_packet_data(uint8_t *obj, uint8_t *num, uint8_t *com, uint8_t *data, uint8_t *len);
    
    // узнать объём поля данных по параметрам
    uint8_t get_size_by_data(uint8_t *obj, uint8_t *com, uint8_t *size_data);
    // узнать объём поля данных по содержимому пакета
    uint8_t get_size_by_packet(uint8_t *obj, uint8_t *size_data);

    bool set_object(uint8_t obj); // (-) ----- не реализовано
    bool set_number(uint8_t num); // (-) ----- не реализовано // setting[1] == 1
    bool set_command(uint8_t com); // (-) ----- не реализовано 
    bool set_data(uint8_t *data, uint8_t len); // (-) ----- не реализовано // len учитываетсятолько при комбинации obj = 0x06 и com = 0x05
};

class Packet_System: public Packet_analyzer {
private:
    uint8_t _command;
public:
    virtual bool set_setting(uint8_t setting=0); // установить настройки (-) ----- нет get

    // занести в пакет параметры и данные
    uint8_t set_packet_data(uint8_t *com, uint8_t *data, uint8_t *len);
    // получить из пакета параметры и данные
    uint8_t get_packet_data(uint8_t *com, uint8_t *data, uint8_t *len);

    // узнать объём поля данных по параметрам
    uint8_t get_size_by_data(uint8_t *size_data, uint8_t *com, uint8_t *len);
    // узнать объём поля данных по содержимому пакета
    uint8_t get_size_by_packet(uint8_t *size_data);

    bool set_command(uint8_t com); // (-) ----- не реализовано
    bool set_data(uint8_t *data, uint8_t len); // (-) ----- не реализовано
};

// класс пакетов ошибок (-) ----- не реализовано
// class Packet_Error: public Packet_analyzer {
// private:
// public:
//     virtual bool set_setting(uint8_t setting=0);
// };

#endif // __PACKET_ANALYZER_H__OLD