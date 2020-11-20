#ifndef __ADDRESS_FIELD_H__
#define __ADDRESS_FIELD_H__

#include <Arduino.h>

// (!) ----- (-) ----- перенести на статическую память

uint32_t value_range(uint32_t value, uint32_t min_value=0, uint32_t max_value=0xFFFFFFFF);
uint16_t value_range(uint16_t value, uint16_t min_value=0, uint16_t max_value=0xFFFF);
uint8_t  value_range(uint8_t  value, uint8_t  min_value=0, uint8_t  max_value=0xFF);

class Register {
private:
    uint16_t address_;   // Адрес регистра
    uint8_t  bit_count_; // Количество бит
    uint8_t  bit_bias_;  // Смещение в битах
    uint32_t mask_;      // Маска регистра
public:
    Register(uint16_t address = 0, uint8_t bit_count = 8, uint8_t bit_bias = 0);
    ~Register() = default;
    // Получение полей класса
    uint16_t address();
    uint8_t  bit_count();
    uint8_t  bit_bias();
    uint32_t mask();
    // Операторы сравнения
    friend bool operator==(const Register& left, const Register& right);
    friend bool operator!=(const Register& left, const Register& right);
} typedef Register;

class Address_field {
private:
    Register *registers_;      // Регистры содержащие поле
    uint8_t  reg_count_;       // Количество регистров
    bool     reg_revers_;      // Обратное расположение регистров
    uint16_t max_address_;     // Максимальный адрес регистров
    uint32_t min_value_;       // Минимальное  возможное значение поля
    uint32_t max_value_;       // Максимальное возможное значение поля
    uint32_t *reserved_value_; // Запрещённые значения поля
    uint32_t reserv_count_;    // Количество запрещённых значений
    char     mode_;            // Режим работы ('r' - чтение,'w' - чтение/запись,'c' - чтение/обнуление)
public:
    Address_field() = default;
    Address_field(Register *registers, uint8_t reg_count=1, bool reg_revers=false, char mode='w', uint32_t min_value=0, 
                  uint32_t max_value=(-1), uint32_t *reserved_value=nullptr, uint32_t reserv_count=0);
    ~Address_field() = default;
    
    // функция инициализации класса, используется автоматически в конструкторе с параметром
    bool init_address_field(class Register *registers, uint8_t reg_count=1, bool reg_revers=false, char mode='w', uint32_t min_value=0, 
                  uint32_t max_value=(-1), uint32_t *reserved_value=nullptr, uint32_t reserv_count=0);
    
    /// Обработка поля
    // получение значения поля: передаётся адрес массива в котором ищется значение и длина массива (вернёт 0xFFFFFFFFFFFFFFFF в случае ошибки или значение в случае увспеха)
    uint32_t get_value(uint8_t *register_value, int register_count=(uint16_t(-1)));
    // установить значение:передаётся значение, адрес массива куда заносится объект и длина массива, вернёт наличие ошибки (0 - нет ошибок, 1 - есть ошибка)
    bool set_value(uint32_t value, uint8_t *register_value, int register_count=(uint16_t(-1)));

    /// Получение полей класса (значения см. выше)
    Register*get_registers();
    uint8_t  get_reg_count();
    uint16_t get_max_address();
    uint32_t get_min_value();
    uint32_t get_max_value();
    uint32_t*get_reserved_value(); 
    uint32_t get_reserv_count();
    char get_mode();
    
    /// Операторы сравнения
    friend bool operator==(const Address_field& left, const Address_field& right);
    friend bool operator!=(const Address_field& left, const Address_field& right);
    
#if defined (SERIAL_LOG_OUTPUT)
    /// вывод класса в Serial
    void print();
#endif
} typedef Address_field;

#endif // __ADDRESS_FIELD_H__