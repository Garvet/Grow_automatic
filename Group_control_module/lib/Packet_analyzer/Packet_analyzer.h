#ifndef __PACKET_ANALYZER_H__
#define __PACKET_ANALYZER_H__

#include <Arduino.h>
#include <Address_field.h>
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
    uint8_t *_packet; // указатель на пакет с данными
    uint8_t _lenght; // длина пакета
    uint8_t _receive_length; // указатель обработчика приёма (для работы в случае ошибки) (--) ----- исключить
    uint8_t _send_lenght; // указатель опработчика отправки (для работы в случае ошибки) (--) ----- исключить
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
    bool select_packet(uint8_t* packet, uint8_t lenght);
    // получить количество пройденных байт обработчиком приёма
    uint8_t get_receive_length(); //  (--) ----- исключить
    // получить количество пройденных байт обработчиком отправки
    uint8_t get_send_lenght(); //  (--) ----- исключить
    
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

#endif // __PACKET_ANALYZER_H__