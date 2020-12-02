#define SERIAL_LOG_OUTPUT
#define TEST_FUN
#if defined ( TEST_FUN )


#include <Arduino.h>

#include <Exchange_packet.h>
#include <Group_control_module.h>
// #include <Grow_sensor.h>

#define TYPE_DEVICE_ONE

#if defined( TYPE_DEVICE_ONE )
#define MY_ADDRESS LoRa_address(2,0)
#define CON_ADDRESS LoRa_address(2,1)
#else
#define MY_ADDRESS LoRa_address(2,1)
#define CON_ADDRESS LoRa_address(2,0)
#endif

#if defined( WIFI_LoRa_32 ) 
// HELTEC_LORA_V1
#define PIN_LED   GPIO_NUM_25
#define PIN_RESET GPIO_NUM_14
#define PIN_DIO0  GPIO_NUM_26
#define PIN_DIO1  GPIO_NUM_33
#define SPI_NSS   GPIO_NUM_18
#define SPI_BUS   HELTEC_LORA
#elif defined( WIFI_LoRa_32_V2 ) // WIFI_LoRa_32
// HELTEC_LORA_V2 (работают криво, т.е. не работают)
#define PIN_LED   GPIO_NUM_25
#define PIN_RESET GPIO_NUM_14
#define PIN_DIO0  GPIO_NUM_34
#define PIN_DIO1  GPIO_NUM_35
#define PIN_IRQ   GPIO_NUM_26
#define SPI_NSS   GPIO_NUM_18
#define SPI_BUS   HELTEC_LORA
#elif ~(defined( WIFI_LoRa_32 ) || defined( WIFI_LoRa_32_V2 )) // WIFI_LoRa_32_V2
// OTHER ESP
#define PIN_LED   GPIO_NUM_2
#define PIN_RESET GPIO_NUM_33
#define PIN_DIO0  GPIO_NUM_27
#define PIN_DIO1  GPIO_NUM_26
// #define PIN_DIO3  GPIO_NUM_25
#define SPI_NSS   GPIO_NUM_32
#define SPI_BUS   VSPI
#endif // ~(defined( WIFI_LoRa_32 ) || defined( WIFI_LoRa_32_V2 ))

std::vector<uint8_t> packet = { 01, 00, 00, 01, 01, 01, 04, 11, 12, 00, 3, 1, 1, 2, 3};
LoRa_packet reg_packet;
Group_control_module __GCM__;
uint8_t data[100] = {0x00, 0x01, 0x00, 0x04, 0x0F, 0x00, 0x02, 0x00, 0x03, 0xD0, 
                     0x93, 0xD1, 0x80, 0xD1, 0x83, 0xD0, 0xBF, 0xD0, 0xBF, 0xD0, 
                     0xB0, 0x20, 0x31, 0x00, 0x00, 0x09, 0xFB, 0xF1, 0x02, 0x04, 
                     0x03, 0x00, 0x00, 0x00, 0x00, 0x02, 0x06, 0x06, 0x01, 0x05, 
                     0x02, 0x01, 0x10, 0x07, 0x02, 0x05, 0x02, 0x01, 0x10, 0x07, 
                     0x03, 0x01, 0x06, 0x05, 0x02, 0x01, 0x05, 0x02, 0x01, 0x10, 
                     0x07};

extern std::array<LoRa_packet_data, SIZE_LORA_PACKET_BUFFER> lora_packet_data;
void GT_print_NR_S() {
    if(__GCM__.sensors_.size() != 0) {
        Serial.println(" ----- ----- sensors ----- -----");
        for(int i = 0; i < __GCM__.sensors_.size(); ++i) {
            Serial.print(" = (");
            Serial.print(i);
            Serial.println(") =");
            __GCM__.sensors_[i].print();
        }
        Serial.println(" ----- ----- --- --- ----- -----\n\n");
    }
    else {
        Serial.println("No sensors \n");
    }
}
void GT_print_NR_D() {
    if(__GCM__.devices_.size() != 0) {
        Serial.println(" ----- ----- devices ----- -----");
        for(int i = 0; i < __GCM__.devices_.size(); ++i) {
            Serial.print(" = (");
            Serial.print(i);
            Serial.println(") =");
            __GCM__.devices_[i].print();
        }
        Serial.println(" ----- ----- --- --- ----- -----\n\n");
    }
    else {
        Serial.println("No devices \n");
    }
    Serial.println();
}
void GT_print_NR() {
    GT_print_NR_S();
    GT_print_NR_D();
    Serial.println();
}
void GT_print_R_S() {
    if(__GCM__.reg_sensors_.size() != 0) {
        Serial.println(" ----- ----- reg sensors ----- -----");
        for(int i = 0; i < __GCM__.reg_sensors_.size(); ++i) {
            Serial.print(" = (");
            Serial.print(i);
            Serial.println(") =");
            __GCM__.reg_sensors_[i].print();
        }
        Serial.println(" ----- ----- --- --- --- ----- -----\n\n");
    }
    else {
        Serial.println("No reg sensors \n");
    }
}
void GT_print_R_D() {
    if(__GCM__.reg_devices_.size() != 0) {
        Serial.println(" ----- ----- reg devices ----- -----");
        for(int i = 0; i < __GCM__.reg_devices_.size(); ++i) {
            Serial.print(" = (");
            Serial.print(i);
            Serial.println(") =");
            __GCM__.reg_devices_[i].print();
        }
        Serial.println(" ----- ----- --- --- --- ----- -----\n\n");
    }
    else {
        Serial.println("No reg devices \n");
    }
}
void GT_print_R() {
    GT_print_R_S();
    GT_print_R_D();
    Serial.println();
}
void GT_print() {
    GT_print_NR();
    GT_print_R();
    
    int amt = 0;
    int num[SIZE_LORA_PACKET_BUFFER];
    for(int i = 0; i < SIZE_LORA_PACKET_BUFFER; ++i) {
        if(!lora_packet_data[i].free_object_) {
            num[amt] = i;
            ++amt;
        }
    }
    Serial.print("amt = ");
    Serial.print(amt);
    Serial.print(" (");
    for(int i = 0; i < amt; ++i) {
        Serial.print(num[i]);
        if(i < amt - 1)
            Serial.print(", ");
    }
    Serial.println(")");
}
void GT_print_f() {
    Serial.println("Filters:");
    for(int i = 0; i < __GCM__.sensors_.size(); ++i) {
        Serial.print(" (");
        Serial.print(i);
        Serial.print(") - [");
        __GCM__.filter_sensors(__GCM__.sensors_[i]);
        for(int j = 0; j < __GCM__.filter_adr_.size(); ++j) {
            Serial.print("{");
            if((__GCM__.filter_adr_[j] >> 8) < 16)
                Serial.print("0");
            Serial.print((__GCM__.filter_adr_[j] >> 8), 16);
            Serial.print(" ");
            if((__GCM__.filter_adr_[j] & 0xFF) < 16)
                Serial.print("0");
            Serial.print((__GCM__.filter_adr_[j] & 0xFF), 16);
            Serial.print("}");

            if(j < __GCM__.filter_adr_.size() - 1)
                Serial.print(", ");
        }
        Serial.println("]");
    }
    Serial.println();
    for(int i = 0; i < __GCM__.devices_.size(); ++i) {
        Serial.print(" (");
        Serial.print(i);
        Serial.print(") - [");
        __GCM__.filter_devices(__GCM__.devices_[i]);
        for(int j = 0; j < __GCM__.filter_adr_.size(); ++j) {
            Serial.print("{");
            if((__GCM__.filter_adr_[j] >> 8) < 16)
                Serial.print("0");
            Serial.print((__GCM__.filter_adr_[j] >> 8), 16);
            Serial.print(" ");
            if((__GCM__.filter_adr_[j] & 0xFF) < 16)
                Serial.print("0");
            Serial.print((__GCM__.filter_adr_[j] & 0xFF), 16);
            Serial.print("}");

            if(j < __GCM__.filter_adr_.size() - 1)
                Serial.print(", ");
        }
        Serial.println("]");
    }
    Serial.println();
}


void setup() {
    Serial.begin(115200);
    while(!Serial){}
    delay(2000);
    Serial.println("Start!");
    Serial.println();
    uint8_t result;
    __GCM__.LoRa_init(PIN_RESET, SPI_BUS, SPI_NSS, PIN_DIO0, PIN_DIO1);
    result = __GCM__.begin(); // 0 - correct, other - num LoRa_error
    if(result != 0) {
        Serial.print("LoRa begin error: ");
        Serial.println(result);
        while (1) delay(10000);        
    }
    // GT_print();
    __GCM__.set_data(data, 100);
    // GT_print();
    // Serial.println();

// #define AUTO_REG
#if defined (AUTO_REG)
    packet = { 01, 00, 00, 01, 03, 02, 04, 11, 12, 00, 2, 1, 4, 3};
    reg_packet.set_packet(packet);
    if(__GCM__.add_reg_module(reg_packet))
        Serial.println("Err");
    packet = { 01, 00, 00, 01, 01, 02, 04, 11, 12, 00, 2, 1, 4, 3};
    reg_packet.set_packet(packet);
    if(__GCM__.add_reg_module(reg_packet))
        Serial.println("Err");
    packet = { 01, 00, 00, 01, 02, 01, 04, 11, 12, 00, 2, 1, 6, 6, 3, 3, 3, 1};
    reg_packet.set_packet(packet);
    if(__GCM__.add_reg_module(reg_packet))
        Serial.println("Err");

    packet = { 01, 00, 00, 01, 02, 03, 04, 11, 12, 00, 1, 2, 5};
    reg_packet.set_packet(packet);
    if(__GCM__.add_reg_module(reg_packet))
        Serial.println("Err");
    packet = { 01, 00, 00, 01, 02, 07, 04, 11, 12, 00, 1, 2, 5};
    reg_packet.set_packet(packet);
    if(__GCM__.add_reg_module(reg_packet))
        Serial.println("Err");
    packet = { 01, 00, 00, 01, 07, 03, 04, 11, 12, 00, 1, 2, 5};
    reg_packet.set_packet(packet);
    if(__GCM__.add_reg_module(reg_packet))
        Serial.println("Err");
    packet = { 01, 00, 00, 01, 03, 01, 04, 11, 12, 00, 2, 2, 5, 3, 4, 2};
    reg_packet.set_packet(packet);
    if(__GCM__.add_reg_module(reg_packet))
        Serial.println("Err");
#endif
    GT_print();
    GT_print_f();
    


    /*
    {
        uint8_t size = 0;
        static_cast<Packet_System*>(reg_packet.packet)->get_size_by_packet(&size);
        Serial.print("Data = ");
        Serial.println(size);
        uint8_t *data = new uint8_t[size];
        uint8_t com = 0xFF;
        uint8_t len = 0xFF;
        static_cast<Packet_System*>(reg_packet.packet)->get_packet_data(&com, data, &len);

        Serial.print("Com = ");
        Serial.println(com);

        Serial.print("Len = ");
        Serial.println(len);

        Serial.print("Data = [");
        for(int i = 0; i < size; ++i) {
            Serial.print(data[i]);

            if(i < (size - 1))
                Serial.print(", ");
        }
        Serial.println("]\n\n");
    }

    {
        uint8_t err = 0;
        uint8_t com = 0;
        uint8_t len = 6;
        uint8_t size_data = 0;
        uint8_t data[10] = {1, 5, 4, 3, 2, 3, 0, 0, 0, 0};

        err = static_cast<Packet_System*>(reg_packet.packet)->get_size_by_data(&size_data, &com, &len);
        if(err) {
            Serial.print("err1 = ");
            Serial.println(err);
        }
        Serial.print("size = ");
        Serial.println(10 + size_data);
        reg_packet.creat_packet(10 + size_data, 4);

        err = static_cast<Packet_System*>(reg_packet.packet)->set_packet_data(&com, data, &len);
        if(err) {
            Serial.print("err2 = ");
            Serial.println(err);
        }

        std::vector<uint8_t> pack = reg_packet.get_packet();

        Serial.print("pack = [");
        for(int i = 0; i < pack.size(); ++i) {
            Serial.print(pack[i]);

            if(i < (pack.size() - 1))
                Serial.print(", ");
        }
        Serial.println("]\n\n");
    }
    

    {
        uint8_t size = 0;
        static_cast<Packet_System*>(reg_packet.packet)->get_size_by_packet(&size);
        Serial.print("Data = ");
        Serial.println(size);
        uint8_t *data = new uint8_t[size];
        uint8_t com = 0xFF;
        uint8_t len = 0xFF;
        static_cast<Packet_System*>(reg_packet.packet)->get_packet_data(&com, data, &len);

        Serial.print("Com = ");
        Serial.println(com);

        Serial.print("Len = ");
        Serial.println(len);

        Serial.print("Data = [");
        for(int i = 0; i < size; ++i) {
            Serial.print(data[i]);

            if(i < (size - 1))
                Serial.print(", ");
        }
        Serial.println("]\n\n");
    }
    */
    
}


int8_t use_type() {
    uint8_t r = -1;
    Serial.print("Write type (0-print, 1-sen, 2-dev): ");
    while(1) {
        r = Serial.read();
        if((r == '0') || (r == '1')  || (r == '2')) {
            Serial.println(r-'0');
            break;
        }
    }
    r -= '0';
    return r;
}
void use_print() {
    uint8_t r = -1;
    Serial.print("Write print (0-print all, 1-sen, 2-dev, 3-r_sen, 4-r_dev, 5-all_sen, 6-all_dev): ");
    while(1) {
        r = Serial.read();
        if(('0' <= r) && (r <= '6')) {
            Serial.println(r-'0');
            switch (r) {
            case '0':
                GT_print();
                GT_print_f();
                break;
            case '1':
                GT_print_NR_S();
                break;
            case '2':
                GT_print_NR_D();
                break;
            case '3':
                GT_print_R_S();
                break;
            case '4':
                GT_print_R_D();
                break;
            case '5':
                GT_print_NR_S();
                GT_print_R_S();
                break;
            case '6':
                GT_print_NR_D();
                GT_print_R_D();
                break;
            default:
                break;
            }
            break;
        }
    }
}
uint16_t use_sen_num() {
    uint8_t r = -1;
    GT_print_NR_S();
    Serial.print("Write num sensor: ");
    while(1) {
        r = Serial.read();
        r -= '0';
        if(r < __GCM__.sensors_.size()) {
            Serial.println(r);
            __GCM__.sensors_[r].print();
            __GCM__.filter_sensors(__GCM__.sensors_[r]);

            Serial.print("F = [");
            for(int j = 0; j < __GCM__.filter_adr_.size(); ++j) {
                Serial.print("{");
                if((__GCM__.filter_adr_[j] >> 8) < 16)
                    Serial.print("0");
                Serial.print((__GCM__.filter_adr_[j] >> 8), 16);
                Serial.print(" ");
                if((__GCM__.filter_adr_[j] & 0xFF) < 16)
                    Serial.print("0");
                Serial.print((__GCM__.filter_adr_[j] & 0xFF), 16);
                Serial.print("}");
                if(j < __GCM__.filter_adr_.size() - 1)
                    Serial.print(", ");
            }
            Serial.println("]");
            break;
        }
    }
    if(__GCM__.filter_adr_.size() == 0) {
        Serial.println ("Filter empty");
        return 0;
    }
    return __GCM__.sensors_[r].get_address();
}
uint16_t use_dev_num() {
    uint8_t r = -1;
    GT_print_NR_D();
    Serial.print("Write num device:");
    while(1) {
        r = Serial.read();
        r -= '0';
        if(r < __GCM__.devices_.size()) {
            Serial.println(r);
            __GCM__.devices_[r].print();
            __GCM__.filter_devices(__GCM__.devices_[r]);
            Serial.print("F = [");
            for(int j = 0; j < __GCM__.filter_adr_.size(); ++j) {
                Serial.print("{");
                if((__GCM__.filter_adr_[j] >> 8) < 16)
                    Serial.print("0");
                Serial.print((__GCM__.filter_adr_[j] >> 8), 16);
                Serial.print(" ");
                if((__GCM__.filter_adr_[j] & 0xFF) < 16)
                    Serial.print("0");
                Serial.print((__GCM__.filter_adr_[j] & 0xFF), 16);
                Serial.print("}");
                if(j < __GCM__.filter_adr_.size() - 1)
                    Serial.print(", ");
            }
            Serial.println("]");
            break;
        }
    }
    if(__GCM__.filter_adr_.size() == 0) {
        Serial.println ("Filter empty");
        return 0;
    }
    return __GCM__.devices_[r].get_address();
}
uint16_t use_reg_sen_num() {
    uint8_t r = -1;
    Serial.print("Write num reg sensor:");
    while(1) {
        r = Serial.read();
        r -= '0';
        if(r < __GCM__.filter_adr_.size()) {
            Serial.println(r);
            break;
        }
    }
    return __GCM__.filter_adr_[r];
}
uint16_t use_reg_dev_num() {
    uint8_t r = -1;
    Serial.print("Write num reg device:");
    while(1) {
        r = Serial.read();
        r -= '0';
        if(r < __GCM__.filter_adr_.size()) {
            Serial.println(r);
            break;
        }
    }
    return __GCM__.filter_adr_[r];
}

void loop() {
    // int8_t result;
    // result = use_type();
    switch (use_type()) {
    case 0: {
        use_print();
        break;
    }
    case 1: {
        uint16_t adr_old, adr_new;
        adr_old = use_sen_num();
        if(adr_old == 0)
            break;
        adr_new = use_reg_sen_num();
        if(adr_new == 0)
            break;
        if(!__GCM__.regist_sensor(adr_new, adr_old))
            Serial.println("Correct");
        else
            Serial.println("Error");
        break;
    }
    case 2: {
        uint16_t adr_old, adr_new;
        adr_old = use_dev_num();
        if(adr_old == 0)
            break;
        adr_new = use_reg_dev_num();
        if(adr_new == 0)
            break;
        if(!__GCM__.regist_device(adr_new, adr_old)) {
            Serial.println("Correct");
        }
        else
            Serial.println("Error");
        break;
    }
    default:
        break;
    }
    // __GCM__.filter_sensors(__GCM__.sensors_[0]);
    // __GCM__.regist_sensor(__GCM__.filter_adr_[0]);
}









#else // TEST_FUN


// #define GET_BLUETOOTH_DATA
#if defined( GET_BLUETOOTH_DATA )




#include <Arduino.h>

#include "BluetoothSerial.h" // заголовочный файл для последовательного Bluetooth будет добавлен по умолчанию в Arduino
#include <Config_system.h>

// const char *copu_str = """{0#\"␓@C??0 1\"#\"1\"#\"1\"{1#\"1\"#\"45\"{2#\"Air_temperature\"#\"1\"}{2#\"Air_humidity\"#\"1\"}}{1#\"1\"#\"20\"{2#\"Illumination_level\"#\"2\"}}{3#\"3\"{4#\"Phytolamp_PWM\"#\"hour\"#\"1\"#\"15\"#\"7\"#\"1\"}}}""";
// char const_str[300];

BluetoothSerial SerialBT;
char inc[1000];
char nameSerial[] = "GreenHouse";
Config_system conf;

void setup() {
    Serial.begin(115200); // Запускаем последовательный монитор со скоростью 115200 /
    SerialBT.begin(nameSerial); // Задаем имя вашего устройства Bluetooth
    Serial.println("Start!");

    // for(int i = 0; i < 255; ++i) {
    //     const_str[i] = copu_str[i];
    // }
    // conf.set_data(const_str);
    // conf.print();
}

void loop()
{
    if (SerialBT.available())//если есть что считывать - то заходим в тело условия
    {
        delay(10);
        int amt = 0;
        int16_t read = SerialBT.read();
        while(read != -1)
        {
            inc[amt] = read;
            ++amt;
            read = SerialBT.read();
        }
        inc[999] = '\0';
        // Serial.println(inc);
        conf.set_data(&(inc[0]));
        conf.print();
        // Serial.print("кол-во пришедших символов = ");
        // Serial.println(amt);
    }
}






#else

#include <Arduino.h>
#include <LoRa_contact_data.h>
// #include <Group_control_module.h>
#include <vector>

#define LoRa_CHANNEL 8
#define BAND    (433100E3 + (LoRa_CHANNEL * 25E3))
#define SYNC_WORD 0x4A // 0x34
#define LoRa_SF   11
#define LoRa_SBW  125E3 // 500E3 250E3 125E3 60E3

#define TYPE_DEVICE_ONE
#define LORA_USE_INTERRUPT
#define SEND_DEVICE_PACKETS
#define CONTACT_TRANSFER

#if defined( TYPE_DEVICE_ONE )
#define MY_ADDRESS LoRa_address(2,0)
#define CON_ADDRESS LoRa_address(2,1)
#else
#define MY_ADDRESS LoRa_address(2,1)
#define CON_ADDRESS LoRa_address(2,0)
#endif

#if defined( WIFI_LoRa_32 ) 
// HELTEC_LORA_V1
#define PIN_LED   GPIO_NUM_25
#define PIN_RESET GPIO_NUM_14
#define PIN_DIO0  GPIO_NUM_26
#define PIN_DIO1  GPIO_NUM_33
#define SPI_NSS   GPIO_NUM_18
#define SPI_BUS   HELTEC_LORA
#elif defined( WIFI_LoRa_32_V2 ) // WIFI_LoRa_32
// HELTEC_LORA_V2 (работают криво, т.е. не работают)
#define PIN_LED   GPIO_NUM_25
#define PIN_RESET GPIO_NUM_14
#define PIN_DIO0  GPIO_NUM_34
#define PIN_DIO1  GPIO_NUM_35
#define PIN_IRQ   GPIO_NUM_26
#define SPI_NSS   GPIO_NUM_18
#define SPI_BUS   HELTEC_LORA
#elif ~(defined( WIFI_LoRa_32 ) || defined( WIFI_LoRa_32_V2 )) // WIFI_LoRa_32_V2
// OTHER ESP
#define PIN_LED   GPIO_NUM_2
#define PIN_RESET GPIO_NUM_33
#define PIN_DIO0  GPIO_NUM_27
#define PIN_DIO1  GPIO_NUM_26
// #define PIN_DIO3  GPIO_NUM_25
#define SPI_NSS   GPIO_NUM_32
#define SPI_BUS   VSPI
#endif // ~(defined( WIFI_LoRa_32 ) || defined( WIFI_LoRa_32_V2 ))


void lora_pin_interrupt();
void start_connect();
void handler_packet(std::vector<std::vector<uint8_t>> packets);
bool work_system();

uint32_t time1, time2;

LoRa_contact_data contact_data;
void setup() {
    Serial.begin(115200);
    while(!Serial){}
    delay(2000);
    Serial.println("Start!");

    contact_data.set_my_adr(MY_ADDRESS);
    contact_data.init_lora_module(PIN_RESET, SPI_BUS, SPI_NSS, PIN_DIO0, PIN_DIO1);
    // contact_data.begin_lora_module(BAND, true);

    // uint8_t begin_lora_module(ulong frequency, bool paboost=false, uint8_t signal_power=14, uint8_t SF=11, ulong SBW=125E3, uint8_t sync_word=0x4A);
    contact_data.begin_lora_module(BAND, true, 14, LoRa_SF, LoRa_SBW); // def
    // contact_data.begin_lora_module(BAND, true, 14, 7, 60E3); // min
    // contact_data.begin_lora_module(BAND, true, 14, 7, 125E3); // st
    // contact_data.begin_lora_module(BAND, true, 14, 7, 250E3); // 250
    // contact_data.begin_lora_module(BAND, true, 14, 7, 500E3); // max
    start_connect();

#if defined( WIFI_LoRa_32_V2 ) 
    pinMode(PIN_IRQ, INPUT);
#endif // WIFI_LoRa_32_V2

#if defined( LORA_USE_INTERRUPT ) 
    attachInterrupt(digitalPinToInterrupt(PIN_DIO0), lora_pin_interrupt, RISING);
    attachInterrupt(digitalPinToInterrupt(PIN_DIO1), lora_pin_interrupt, RISING);
#if defined( WIFI_LoRa_32_V2 ) 
    attachInterrupt(digitalPinToInterrupt(PIN_IRQ),  lora_pin_interrupt, RISING);
#endif // WIFI_LoRa_32_V2
#endif // LORA_USE_INTERRUPT
                                                                                                    time1 = millis();
}

#if defined( CONTACT_TRANSFER )

extern void print_all_packet();

void loop() {
    if(work_system()) {
                                                                                                    time2 = millis();
        Serial.println("\n --- Contact complete! ---");
        std::vector<std::vector<uint8_t>> packets;
        packets = contact_data.get_all_packet();
        if(packets.size() > 0) {
            Serial.println("Reciever packets:");
            for(int i = 0; i < packets.size(); ++i) {
                Serial.print(i + 1);
                Serial.print(" - [");
                for(int j = 0; j < packets[i].size(); ++j) {
                    if(packets[i][j] < 16)
                        Serial.print("0");
                    Serial.print(packets[i][j], 16);
                    if(j < packets[i].size() - 1)
                        Serial.print(" ");
                }
                Serial.println("]");
            }
            handler_packet(packets);
        }
        print_all_packet();
                                                                                                    Serial.print("\nContact time = ");
                                                                                                    Serial.print(time2 - time1);
                                                                                                    Serial.print(" ms (S = ");
                                                                                                    Serial.print(time1);
                                                                                                    Serial.print(", E = ");
                                                                                                    Serial.print(time2);
                                                                                                    Serial.println(")");
        while (1) delay(100000);
    }
}


#if !(defined( LORA_USE_INTERRUPT ) )
bool work_system() {
    static uint8_t pin_state[3] = {0, 0, 0};
    pin_state[0] = digitalRead(PIN_DIO0);
    pin_state[1] = digitalRead(PIN_DIO1);
#if defined( WIFI_LoRa_32_V2 ) 
    pin_state[2] = digitalRead(PIN_IRQ);
#endif
    if((pin_state[0]) || (pin_state[1]) || (pin_state[2])) {
        uint16_t err = contact_data.work_contact_system();
        if(err != 0) {
            Serial.print("Err: ");
            Serial.println(err);
        }
    }
    return contact_data.get_signal_complete();
}
#else

bool work_system() {
    return contact_data.get_signal_complete();
}

void lora_pin_interrupt() {
    static bool in_interrupt = false;
    if(in_interrupt)
        return;
    in_interrupt = true;
    uint16_t err = contact_data.work_contact_system();
    if(err != 0) {
        Serial.print("Err: ");
        Serial.println(err);
    }
    in_interrupt = false;
}
#endif


#if defined( SEND_DEVICE_PACKETS )
void start_connect() {
#ifdef TYPE_DEVICE_ONE
    contact_data.init_contact(CON_ADDRESS);
    class Exchange_packet _packet;
    // _packet.creat_packet(11, 0x02);
    uint8_t _obj[20] = {0x01, 0x02, 0x03, 0x01, 0x02, 0x03, 0x01, 0x02, 0x03, 0x01, 0x02, 0x03, 0x01, 0x02, 0x03, 0x01, 0x02, 0x03};
    uint8_t _num[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    Serial.println("Send packets:");
    for(int i = 0; i < 5; ++i) {
        uint8_t size = 0;
        uint8_t obj = _obj[i];
        uint8_t num = _num[i];
        uint8_t com = 1;
        uint8_t err;
        err = static_cast<Packet_Device*>(_packet.packet)->get_size_by_data(&obj, &com, &size);
        if (err != 0) {
            Serial.print("error[1] = ");
            Serial.println(err);
            break;
        }
        uint8_t *data = new uint8_t[size];
        if(size == 1) { 
            if((i%2) == 0) {
                data[0] = 0;
            }
            else if ((i%2) == 1) {
                data[0] = 1;
            }
        }
        else {
            for(int j = 0; j < size; ++j)
                data[j] = j + 1;
        }
        _packet.creat_packet(size + 11, PACKET_DEVICE);
        _packet.packet->set_dest_adr(CON_ADDRESS);
        _packet.packet->set_sour_adr(MY_ADDRESS);
        _packet.packet->set_packet_type(PACKET_DEVICE);
        _packet.packet->set_packet_number(i+1);
        err = static_cast<Packet_Device*>(_packet.packet)->set_packet_data(&obj, &num, &com, data, nullptr);
        delete[] data;
        if (err != 0) {
            Serial.print("error[2] = ");
            Serial.println(err);
            break;
        }
        contact_data.add_packet(_packet);
        std::vector<uint8_t> packet = _packet.get_packet();
        Serial.print(i + 1);
        Serial.print(" - [");
        for(int j = 0; j < packet.size(); ++j) {
            if(packet[j] < 16)
                Serial.print("0");
            Serial.print(packet[j], 16);
            if(j < packet.size() - 1)
                Serial.print(" ");
        }
        Serial.println("]");
    }
    Serial.println(" --- Start transfer ---");
    if(contact_data.start_transfer())
        Serial.println("error");
#endif // + TYPE_DEVICE_ONE



#ifndef TYPE_DEVICE_ONE
    contact_data.wait_recipient();
    Serial.println(" --- Start transfer ---");
#endif // - TYPE_DEVICE_ONE
}


#else // SEND_DEVICE_PACKETS


void start_connect() {
#ifdef TYPE_DEVICE_ONE
    contact_data.init_contact(CON_ADDRESS);
    Serial.println(" --- Start transfer ---");
    if(contact_data.start_transfer())
        Serial.println("error");
#endif // + TYPE_DEVICE_ONE


#ifndef TYPE_DEVICE_ONE
    class Exchange_packet _packet;
    // _packet.creat_packet(11, 0x02);
    uint8_t _amt[3] = {0, 0, 0};
    uint8_t _param[3] = {0x01, 0x02, 0x03};
    uint8_t _num[3] = {0, 0, 0};
    float f_data[3] = {2.71, 32.11, 65.43};
    uint16_t i_data[3] = {27, 321, 654};
    bool b_data[3] = {true, false, true};
    uint8_t f_amt = 0, i_amt = 0, b_amt = 0;
    uint8_t packet_type = 0x01;
    Serial.println("Send packets:");
    for(int i = 0; i < 3; ++i) {
        uint8_t size = 0;
        uint8_t amt = _amt[i];
        uint8_t param = _param[i];
        uint8_t num = _num[i];
        uint8_t err;
        err = ((Packet_Sensor*)_packet.packet)->get_size_by_data(&amt, &param, &size);
        if (err != 0) {
            Serial.print("error[1] = ");
            Serial.println(err);
            break;
        }
        uint32_t *data;
        if((size % 4) == 0) {
            data = new uint32_t[size / 4];
            data[0] = *((uint32_t*)(&f_data[f_amt++]));
            // data[0] = static_cast<uint32_t*>(&f_data[f_amt++]); // (?) -----
        }
        else if(size == 2) {
            data = new uint32_t[size / 2];
            data[0] = i_data[i_amt++];
        }
        else {
            data = new uint32_t[size];
            data[0] = b_data[b_amt++];
        }
        _packet.creat_packet(size + 11, packet_type);
        _packet.packet->set_dest_adr(CON_ADDRESS);
        _packet.packet->set_sour_adr(MY_ADDRESS);
        _packet.packet->set_packet_type(packet_type);
        _packet.packet->set_packet_number(i+1);
        err = ((Packet_Sensor*)_packet.packet)->set_packet_data(&amt, &param, &num, data);
        delete[] data;
        if (err != 0) {
            Serial.print("error[2] = ");
            Serial.println(err);
            break;
        }
        contact_data.add_packet(_packet);
        std::vector<uint8_t> packet = _packet.get_packet();
        Serial.print(i + 1);
        Serial.print(" - [");
        for(int j = 0; j < packet.size(); ++j) {
            if(packet[j] < 16)
                Serial.print("0");
            Serial.print(packet[j], 16);
            if(j < packet.size() - 1)
                Serial.print(" ");
        }
        Serial.println("]");
    }
    Serial.println(" --- Start transfer ---");
    contact_data.wait_recipient();
#endif // - TYPE_DEVICE_ONE
}
#endif // SEND_DEVICE_PACKETS


void handler_packet(std::vector<std::vector<uint8_t>> packets) {
    Exchange_packet packet_processing;
#if defined( TYPE_DEVICE_ONE )
    // обработка - датчик
    float value;
    uint8_t amt, size;
    uint8_t *id, *param;
    uint32_t *data; 
    Serial.println("Sensors: ");
    for(int i = 0; i < packets.size(); ++i) {
        value = 0.0;
        amt = size = 0;
        id = param = nullptr;
        data = nullptr; 

        packet_processing.set_packet(packets[i]);
        static_cast<Packet_Sensor*>(packet_processing.packet)->get_size_by_packet(&amt, nullptr, &size);
        id = new uint8_t[amt];
        param = new uint8_t[amt];
        data = new uint32_t[amt];
        for(int k = 0; k < amt; ++k) {
            id[k] = 0;
            param[k] = 0;
            data[k] = 0;
        }
        static_cast<Packet_Sensor*>(packet_processing.packet)->get_packet_data(&amt, param, id, data);
        
        Serial.print("(");
        Serial.print(i);
        Serial.print("): ");
        for(int j = 0; j < amt; ++j) {
            if((9 < packets.size()) && (i < 10))
                Serial.print(" ");
            Serial.print("type = ");
            Serial.print(param[j]);
            Serial.print(" id = ");
            Serial.print(id[j]);
            Serial.print(" data = ");
            if(param[j] == 0x00 || (param[j] == 0x01)) {
                Serial.println(data[j]);
            }
            else {
                value = *((float*)&(data[j]));
                Serial.println(value);
            }
            if(j < amt - 1)
                Serial.print("     ");
        }
        delete[] id;
        delete[] param;
        delete[] data;
    }
#else // TYPE_DEVICE_ONE
    // обработка - устройство
    // uint32_t value;
    uint8_t size = 0;
    uint8_t object = 0;
    uint8_t id = 0;
    uint8_t com = 0;
    uint8_t *data = nullptr; 
    Serial.println("Devices: ");
    for(int i = 0; i < packets.size(); ++i) {
        size = object = id = com = 0;
        packet_processing.set_packet(packets[i]);

        static_cast<Packet_Device*>(packet_processing.packet)->get_size_by_packet(nullptr, &size);
        if(size != 0)
            data = new uint8_t[size];
        static_cast<Packet_Device*>(packet_processing.packet)->get_packet_data(&object, &id, &com, data, nullptr);

        Serial.print("(");
        Serial.print(i);
        Serial.print("): ");
        if((9 < packets.size()) && (i < 10))
            Serial.print(" ");

        Serial.print("object = ");
        Serial.print(object);
        Serial.print(" id = ");
        Serial.print(id);
        Serial.print(" command = ");
        Serial.print(com);
        if(size != 0) {
            Serial.print(" data = ");
            for(int j = 0; j < size; ++j) {
                if(data[i] < 16)
                    Serial.print("0");
                Serial.print(data[i], 16);
                Serial.print(" ");
            }
        }
        Serial.println();

        if(data != nullptr) {
            delete[] data;
            data = nullptr;
        }
    }    
#endif // - TYPE_DEVICE_ONE
}

#else  // CONTACT_TRANSFER


void loop() {
    work_system();
}


#if defined( SEND_DEVICE_PACKETS )
void start_connect() {
#if defined( TYPE_DEVICE_ONE )
    class Exchange_packet _packet;
    uint8_t _obj[3] = {0x01, 0x02, 0x03};
    uint8_t _num[3] = {0, 0, 0};
    Serial.println("Send packets:");
    for(int i = 0; i < 3; ++i) {
        uint8_t size = 0;
        uint8_t obj = _obj[i];
        uint8_t num = _num[i];
        uint8_t com = 1;
        uint8_t err;
        err = ((Packet_Device*)_packet.packet)->get_size_by_data(&obj, &com, &size);
        if (err != 0) {
            Serial.print("error[1] = ");
            Serial.println(err);
            break;
        }
        uint8_t *data = new uint8_t[size];
        if(size == 1) { 
            if((i%2) == 0) {
                data[0] = 0;
            }
            else if ((i%2) == 1) {
                data[0] = 1;
            }
        }
        else {
            for(int j = 0; j < size; ++j)
                data[j] = j + 1;
        }
        _packet.creat_packet(size + 11, 0x02);
        _packet.packet->set_dest_adr(CON_ADDRESS);
        _packet.packet->set_sour_adr(MY_ADDRESS);
        _packet.packet->set_packet_type(0x02);
        _packet.packet->set_packet_number(i+1);
        err = ((Packet_Device*)_packet.packet)->set_packet_data(&obj, &num, &com, data, nullptr);
        delete[] data;
        if (err != 0) {
            Serial.print("error[2] = ");
            Serial.println(err);
            break;
        }
        contact_data.add_packet(_packet);
        std::vector<uint8_t> packet = _packet.get_packet();
        Serial.print(i + 1);
        Serial.print(" - [");
        for(int j = 0; j < packet.size(); ++j) {
            if(packet[j] < 16)
                Serial.print("0");
            Serial.print(packet[j], 16);
            if(j < packet.size() - 1)
                Serial.print(" ");
        }
        Serial.println("]");
    }
    Serial.println(" --- Start broadcast send ---");
    contact_data.broadcast_send();
    
#else // TYPE_DEVICE_ONE
    contact_data.broadcast_receive(CON_ADDRESS);
    Serial.println(" --- Start broadcast receive ---");
#endif // TYPE_DEVICE_ONE
}

static uint16_t num = 3;
bool work_system() {
#if defined( TYPE_DEVICE_ONE )

    if(contact_data.get_signal_complete()) {
        Serial.print("\n --- Broadcast complete! --- (");
        Serial.print(num);
        Serial.print(" - ");
        Serial.print(num+3);
        Serial.println(")");
        // // //
                    contact_data.broadcast_receive();
                    int i = 0;
                    while (contact_data.get_state_contact() != SC_PACKET_ACCEPTED) {
                        delay(1);
                        ++i;
                        if(i > 5000)
                            break;
                    }
                    contact_data.add_packet({1, 0, 1, 1, 0, 0, 1, 2, 2, 3, 2, 3});
                    ++num;
                    if(i <= 5000) {
                        contact_data.add_packet({1, 0, 1, 1, 0, 0, 1, 2, 2, 3, 3, 4});
                        contact_data.add_packet({1, 0, 1, 1, 0, 0, 1, 2, 2, 3, 4, 5});
                        num += 2;
                    }
                    contact_data.broadcast_send();
        // // //
        // while (1) delay(100000);
    }
    
#else // TYPE_DEVICE_ONE

    if(contact_data.get_state_contact() == SC_PACKET_ACCEPTED) {
        std::vector<std::vector<uint8_t>> packets;
        packets = contact_data.get_all_packet();
        Serial.println("Reciever packets:");
        for(int i = 0; i < packets.size(); ++i) {
            Serial.print(i + 1);
            Serial.print(" - [");
            for(int j = 0; j < packets[i].size(); ++j) {
                if(packets[i][j] < 16)
                    Serial.print("0");
                Serial.print(packets[i][j], 16);
                if(j < packets[i].size() - 1)
                    Serial.print(" ");
            }
            Serial.println("]");
        }
        Serial.println();
    }
#endif // TYPE_DEVICE_ONE
    return false;
}

void lora_pin_interrupt() {
    static bool in_interrupt = false;
    if(in_interrupt)
        return;
    in_interrupt = true;
    uint16_t err = contact_data.work_contact_system();
    if(err != 0) {
        Serial.print("Err: ");
        Serial.println(err);
    }
    in_interrupt = false;
}

#endif // SEND_DEVICE_PACKETS


#endif // CONTACT_TRANSFER








/*

В функции инициализвации прерываний третий параметр – это режим активации прерывания, они бывают следующего вида:
- LOW – прерывание будет запущено, если на контакте будет значение «LOW»
- HIGH – прерывание будет запущено, если на контакте будет значение «HIGH»
- CHANGE – прерывание будет запущено, если значение на контакте изменится (например, с «LOW» на «HIGH» или с «HIGH» на «LOW»)
- FALLING – прерывание будет запущено, если значение на контакте изменится с «HIGH» на «LOW»
- RISING – прерывание будет запущено, если значение на контакте изменится с «LOW» на «HIGH»






// #define TYPE_DEVICE_ONE

// #define HELTEC_LORA_V1
#define HELTEC_LORA_V2

#ifndef HELTEC_LORA_V1
#ifndef HELTEC_LORA_V2
#define PIN_LED   GPIO_NUM_2
#define PIN_RESET GPIO_NUM_33
#define PIN_DIO0  GPIO_NUM_27
#define PIN_DIO1  GPIO_NUM_26
// #define PIN_DIO3  GPIO_NUM_25
#define SPI_NSS   GPIO_NUM_32
#define SPI_BUS   VSPI
#define MY_ADDRESS LoRa_address(2,0)
#define CON_ADDRESS LoRa_address(2,1)
#endif // - HELTEC_LORA_V2
#endif // - HELTEC_LORA_V1

#ifdef HELTEC_LORA_V1
#define PIN_LED   GPIO_NUM_25
#define PIN_RESET GPIO_NUM_14
#define PIN_DIO0  GPIO_NUM_26
#define PIN_DIO1  GPIO_NUM_33
#define SPI_NSS   GPIO_NUM_18
#define SPI_BUS   HELTEC_LORA
#define MY_ADDRESS LoRa_address(2,1)
#define CON_ADDRESS LoRa_address(2,0)
#endif // + HELTEC_LORA_V1

#ifndef HELTEC_LORA_V1
#ifdef HELTEC_LORA_V2
#define PIN_LED   GPIO_NUM_25
#define PIN_RESET GPIO_NUM_14
#define PIN_DIO0  GPIO_NUM_34
#define PIN_DIO1  GPIO_NUM_35
#define SPI_NSS   GPIO_NUM_18
#define SPI_BUS   HELTEC_LORA
#define MY_ADDRESS LoRa_address(2,1)
#define CON_ADDRESS LoRa_address(2,0)
#endif // + HELTEC_LORA_V2
#endif // - HELTEC_LORA_V1























#define RECIEVE_DEVICE_PACKETS
#ifdef RECIEVE_DEVICE_PACKETS
LoRa_contact_data contact_data;
void setup() {
    Serial.begin(115200);
    while(!Serial){}
    delay(2000);
    Serial.println("Start!");
    
    contact_data.set_my_adr(MY_ADDRESS);
    contact_data.init_lora_module(PIN_RESET, SPI_BUS, SPI_NSS, PIN_DIO0, PIN_DIO1);
    contact_data.begin_lora_module(BAND, true);

#ifdef TYPE_DEVICE_ONE
    contact_data.init_contact(CON_ADDRESS);

    class Exchange_packet _packet;
    // _packet.creat_packet(11, 0x02);
    uint8_t _obj[3] = {0x01, 0x02, 0x03};
    uint8_t _num[3] = {0, 0, 0};
    Serial.println("Send packets:");
    for(int i = 0; i < 3; ++i) {
        uint8_t size = 0;
        uint8_t obj = _obj[i];
        uint8_t num = _num[i];
        uint8_t com = 1;
        uint8_t err;
        err = ((Packet_Device*)_packet.packet)->get_size_by_data(&obj, &com, &size);
        if (err != 0) {
            Serial.print("error[1] = ");
            Serial.println(err);
            break;
        }
        uint8_t *data = new uint8_t[size];
        if(size == 1) { 
            if((i%2) == 0) {
                data[0] = 0;
            }
            else if ((i%2) == 1) {
                data[0] = 1;
            }
        }
        else {
            for(int j = 0; j < size; ++j)
                data[j] = j + 1;
        }
        _packet.creat_packet(size + 11, 0x02);
        _packet.packet->set_dest_adr(CON_ADDRESS);
        _packet.packet->set_sour_adr(MY_ADDRESS);
        _packet.packet->set_packet_type(0x02);
        _packet.packet->set_packet_number(i+1);
        err = ((Packet_Device*)_packet.packet)->set_packet_data(&obj, &num, &com, data, nullptr);
        delete[] data;
        if (err != 0) {
            Serial.print("error[2] = ");
            Serial.println(err);
            break;
        }
        contact_data.add_packet(_packet);

        std::vector<uint8_t> packet = _packet.get_packet();
        Serial.print(i + 1);
        Serial.print(" - [");
        for(int j = 0; j < packet.size(); ++j) {
            if(packet[j] < 16)
                Serial.print("0");
            Serial.print(packet[j], 16);
            if(j < packet.size() - 1)
                Serial.print(" ");
        }
        Serial.println("]");
    }
    Serial.println(" --- Start transfer ---");
    if(contact_data.start_transfer())
        Serial.println("error");
#endif // + TYPE_DEVICE_ONE
#ifndef TYPE_DEVICE_ONE
    contact_data.wait_recipient();
    Serial.println(" --- Start transfer ---");
#endif // - TYPE_DEVICE_ONE

}

uint8_t last_pin_state[2] = {0, 0};
void loop() {
    uint8_t pin_state[2];
    pin_state[0] = digitalRead(PIN_DIO0);
    pin_state[1] = digitalRead(PIN_DIO1);
    // if(((pin_state[0] != last_pin_state[0])&&(pin_state[0])) || 
    //    ((pin_state[1] != last_pin_state[1])&&(pin_state[1]))) {
    if((pin_state[0]) || (pin_state[1])) {
        uint16_t err = contact_data.work_contact_system();
        if(err != 0) {
            Serial.print("Err: ");
            Serial.println(err);
        }
    }
    if(contact_data.get_signal_complete()) {
        Serial.println("\n --- Contact complete! ---");
        std::vector<std::vector<uint8_t>> packets;
        packets = contact_data.get_all_packet();
        if(packets.size() > 0) {
            Serial.println("Reciever packets:");
            for(int i = 0; i < packets.size(); ++i) {
                Serial.print(i + 1);
                Serial.print(" - [");
                for(int j = 0; j < packets[i].size(); ++j) {
                    if(packets[i][j] < 16)
                        Serial.print("0");
                    Serial.print(packets[i][j], 16);
                    if(j < packets[i].size() - 1)
                        Serial.print(" ");
                }
                Serial.println("]");
            }
        }
        while (1) delay(100000);
    }
    // delay(10);
    // last_pin_state[0] = pin_state[0];
    // last_pin_state[1] = pin_state[1];
}
#endif






#ifndef RECIEVE_DEVICE_PACKETS
LoRa_contact_data contact_data;
void setup() {
    Serial.begin(115200);
    while(!Serial){}
    delay(2000);
    Serial.println("Start!");
    
    contact_data.set_my_adr(MY_ADDRESS);
    contact_data.init_lora_module(PIN_RESET, SPI_BUS, SPI_NSS, PIN_DIO0, PIN_DIO1);
    contact_data.begin_lora_module(BAND);

#ifdef TYPE_DEVICE_ONE
    contact_data.init_contact(CON_ADDRESS);
    Serial.println(" --- Start transfer ---");
    if(contact_data.start_transfer())
        Serial.println("error");
#endif // + TYPE_DEVICE_ONE
#ifndef TYPE_DEVICE_ONE

    class Exchange_packet _packet;
    // _packet.creat_packet(11, 0x02);
    uint8_t _amt[3] = {0, 0, 0};
    uint8_t _param[3] = {0x01, 0x02, 0x03};
    uint8_t _num[3] = {0, 0, 0};
    float f_data[3] = {2.71, 32.11, 65.43};
    uint16_t i_data[3] = {27, 321, 654};
    bool b_data[3] = {true, false, true};
    uint8_t f_amt = 0, i_amt = 0, b_amt = 0;
    uint8_t packet_type = 0x01;
    Serial.println("Send packets:");
    for(int i = 0; i < 3; ++i) {
        uint8_t size = 0;
        uint8_t amt = _amt[i];
        uint8_t param = _param[i];
        uint8_t num = _num[i];
        uint8_t err;
        err = ((Packet_Sensor*)_packet.packet)->get_size_by_data(&amt, &param, &size);
        if (err != 0) {
            Serial.print("error[1] = ");
            Serial.println(err);
            break;
        }
        uint32_t *data;
        if((size % 4) != 0) {
            data = new uint32_t[size / 4];
            data[0] = *((uint32_t*)(&f_data[f_amt++]));
        }
        else if(size == 2) {
            data = new uint32_t[size / 2];
            data[0] = i_data[i_amt++];
        }
        else {
            data = new uint32_t[size];
            data[0] = b_data[b_amt++];
        }
        _packet.creat_packet(size + 11, packet_type);
        _packet.packet->set_dest_adr(CON_ADDRESS);
        _packet.packet->set_sour_adr(MY_ADDRESS);
        _packet.packet->set_packet_type(packet_type);
        _packet.packet->set_packet_number(i+1);
        err = ((Packet_Sensor*)_packet.packet)->set_packet_data(&amt, &param, &num, data);
        delete[] data;
        if (err != 0) {
            Serial.print("error[2] = ");
            Serial.println(err);
            break;
        }
        contact_data.add_packet(_packet);

        std::vector<uint8_t> packet = _packet.get_packet();
        Serial.print(i + 1);
        Serial.print(" - [");
        for(int j = 0; j < packet.size(); ++j) {
            if(packet[j] < 16)
                Serial.print("0");
            Serial.print(packet[j], 16);
            if(j < packet.size() - 1)
                Serial.print(" ");
        }
        Serial.println("]");
    }
    contact_data.wait_recipient();
    Serial.println(" --- Start transfer ---");
#endif // - TYPE_DEVICE_ONE
}

uint8_t last_pin_state[2] = {0, 0};
void loop() {
    uint8_t pin_state[2];
    pin_state[0] = digitalRead(PIN_DIO0);
    pin_state[1] = digitalRead(PIN_DIO1);
    // if(((pin_state[0] != last_pin_state[0])&&(pin_state[0])) || 
    //    ((pin_state[1] != last_pin_state[1])&&(pin_state[1]))) {
    if((pin_state[0]) || (pin_state[1])) {
        uint16_t err = contact_data.work_contact_system();
        if(err != 0) {
            Serial.print("Err: ");
            Serial.println(err);
        }
    }
    if(contact_data.get_signal_complete()) {
        Serial.println("\n --- Contact complete! ---");
        std::vector<std::vector<uint8_t>> packets;
        packets = contact_data.get_all_packet();
        if(packets.size() > 0) {
            Serial.println("Reciever packets:");
            for(int i = 0; i < packets.size(); ++i) {
                Serial.print(i + 1);
                Serial.print(" - [");
                for(int j = 0; j < packets[i].size(); ++j) {
                    if(packets[i][j] < 16)
                        Serial.print("0");
                    Serial.print(packets[i][j], 16);
                    if(j < packets[i].size() - 1)
                        Serial.print(" ");
                }
                Serial.println("]");
            }
        }
        while (1) delay(100000);
    }
    // delay(10);
    // last_pin_state[0] = pin_state[0];
    // last_pin_state[1] = pin_state[1];
}
#endif
*/



#endif // GET_BLUETOOTH_DATA

#endif // TEST_FUN