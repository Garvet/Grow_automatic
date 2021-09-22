#include "build_data.h"

#define SERIAL_LOG_OUTPUT

#include <Arduino.h>

#include <GCM_interface.h>
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
// #define PIN_LED   GPIO_NUM_2
#define PIN_RESET GPIO_NUM_33
#define PIN_DIO0  GPIO_NUM_27
#define PIN_DIO1  GPIO_NUM_26
// #define PIN_DIO3  GPIO_NUM_25
#define SPI_NSS   GPIO_NUM_32
#define SPI_BUS   VSPI
#endif // ~(defined( WIFI_LoRa_32 ) || defined( WIFI_LoRa_32_V2 ))

#if defined (BUILD_TESTING_CODE_409)
#define VERSION_MODULE 1
uint32_t led1_time = millis();
uint32_t led2_time = millis();
uint32_t led3_time = millis();
bool led1_state = false;
bool led2_state = false;
bool led3_state = false;
uint32_t change_button1_time = millis();
uint32_t change_button2_time = millis();
uint32_t change_button3_time = millis();
uint32_t change_button4_time = millis();
bool button1_state = false;
bool button2_state = false;
bool button3_state = false;
bool button4_state = false;
#endif

#if ( VERSION_MODULE == 1 ) 

#define RTC_CLK   GPIO_NUM_5
#define RTC_DAT   GPIO_NUM_4
#define RTC_RST   GPIO_NUM_2
#define LCD_RESET GPIO_NUM_13
#define BUTTON1   GPIO_NUM_15
#define BUTTON2   GPIO_NUM_16
#define BUTTON3   GPIO_NUM_17
#define BUTTON4   GPIO_NUM_21
#define LED1      GPIO_NUM_14
#define LED2      GPIO_NUM_12
#define LED3      GPIO_NUM_13
#define ENCODER1  GPIO_NUM_35
#define ENCODER2  GPIO_NUM_34
#define ENCODER3  GPIO_NUM_39
#define VOLTAGE   GPIO_NUM_36

#elif ( VERSION_MODULE == 2 )

#define UART2_RX   GPIO_NUM_22 // TX in STM
#define UART2_TX   GPIO_NUM_25 // RX in STM

#define RTC_CLK GPIO_NUM_5
#define RTC_DAT GPIO_NUM_4
#define RTC_RST GPIO_NUM_2

#define BUTTON1   GPIO_NUM_15
#define BUTTON2   GPIO_NUM_16
#define BUTTON3   GPIO_NUM_17
#define BUTTON4   GPIO_NUM_21
#define ENCODER1  GPIO_NUM_35
#define ENCODER2  GPIO_NUM_34
#define ENCODER3  GPIO_NUM_39

#define VOLTAGE   GPIO_NUM_36

#endif



std::vector<uint8_t> packet = { 01, 00, 00, 01, 01, 01, 04, 11, 12, 00, 3, 1, 1, 2, 3};
LoRa_packet reg_packet;
Group_control_module __GCM__;




extern std::array<LoRa_packet_data, SIZE_LORA_PACKET_BUFFER> lora_packet_data;
void GT_print_NR_S();
void GT_print_NR_D();
void GT_print_NR();
void GT_print_R_S();
void GT_print_R_D();
void GT_print_R();
void GT_print();
void GT_print_f();
uint8_t use_type();
void use_print();
uint16_t use_sen_num();
uint16_t use_dev_num();
std::array<uint8_t, AMT_BYTES_SYSTEM_ID> use_reg_sen_num();
std::array<uint8_t, AMT_BYTES_SYSTEM_ID> use_reg_dev_num();
void first_device_control();







bool end_serial = false;
bool serial_loop_control = false;
// bool work_and_loop_control = false;
bool work_and_loop_control = true;
bool button_control = false;

void setup() {
    Serial.begin(115200);
    while(!Serial){}
    delay(2000);

#if defined (BUILD_S0_D1_60s_SPECIALIZED)
    serial_loop_control = true;
#endif
#if defined (BUILD_TESTING_CODE_409)
    serial_loop_control = true;
    button_control = false;
    // button_control = true;
#endif

#if defined(VERSION_MODULE)
    pinMode(BUTTON1, INPUT);
    pinMode(BUTTON2, INPUT);
    pinMode(BUTTON3, INPUT);
    pinMode(BUTTON4, INPUT);
#if (VERSION_MODULE == 1)
    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    pinMode(LED3, OUTPUT);
#endif
#endif

    Serial.println("Start!");
    Serial.println();
    uint8_t result;
    gcm_interface.set_group_control_module(&__GCM__);
    __GCM__.LoRa_init(PIN_RESET, SPI_BUS, SPI_NSS, PIN_DIO0, PIN_DIO1);
    result = __GCM__.begin(); // 0 - correct, other - num LoRa_error
    if(result != 0) {
        Serial.print("LoRa begin error: ");
        Serial.println(result);
        while (1) delay(10000);        
    }
    // __GCM__.set_data_time(RtcDateTime(2020, 01, 22, 17, 22, 00));
    // GT_print();

    #if defined( SET_RTC_TIME )
    RtcDateTime start_dt {SET_RTC_TIME_YEAR, SET_RTC_TIME_MONTH, SET_RTC_TIME_DAY,
                          SET_RTC_TIME_HOUR, SET_RTC_TIME_MIN,   SET_RTC_TIME_SEC};
    __GCM__.rtc_.SetDateTime(start_dt);
    #endif
    // RtcDateTime start_dt{2021, 05, 03, 19, 48, 50};
    // __GCM__.rtc_.SetDateTime(start_dt);
    __GCM__.set_data(data, 100);
    // GT_print();
    // Serial.println();

// #define AUTO_REG
#if defined (AUTO_REG) // добавляет модули в запрос регистрации
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
    
    if(1) {
        for(int i = 0; (i < __GCM__.sensors_.size()) && (i < AMT_SENSORS_ID); ++i) {
            __GCM__.sensors_[i].set_active(2);
            __GCM__.sensors_[i].set_system_id(sensors_id[i]);
        }
        for(int i = 0; (i < __GCM__.devices_.size()) && (i < AMT_DEVICES_ID); ++i) {
            __GCM__.devices_[i].set_active(2);
            __GCM__.devices_[i].set_system_id(devices_id[i]);
        }
        __GCM__.contact_data_.end_contact();
        __GCM__.set_mode(Group_control_module::GT_PROCESSING);
        end_serial = true;
        GT_print();


        #if (SEND_SERVER == 1)
        gcm_interface.init_server_connect(network_name, 10, network_pswd, 9, server_address, AMT_BYTES_NETWORK_ADDRESS, server_port);
        delay(5000);
        gcm_interface.report_to_server_regist_data();
        #endif
    }
    else {
        GT_print();
        GT_print_f();
    }
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



ulong time_interval = 3600000;
void loop() {
    if (!button_control) {        
        while(!end_serial) {
            switch (use_type()) {
            case 0: {
                use_print();
                break;
            }
            case 1: {
                bool err = true;
                std::array<uint8_t, AMT_BYTES_SYSTEM_ID> system_id; 
                uint16_t adr_new;
                adr_new = use_sen_num();
                if(adr_new == 0)
                    break;
                system_id = use_reg_sen_num();
                for(int i = 0; i < AMT_BYTES_SYSTEM_ID; ++i)
                    if(system_id[i] != 0) {
                        err = false;
                        break;
                    }
                if(err)
                    break;
                if(!__GCM__.regist_sensor(system_id, adr_new)) {
                    Serial.println("Correct");
                }
                else
                    Serial.println("Error");
                break;
            }
            case 2: {
                bool err = true;
                std::array<uint8_t, AMT_BYTES_SYSTEM_ID> system_id; 
                uint16_t adr_new;
                adr_new = use_dev_num();
                if(adr_new == 0)
                    break;
                system_id = use_reg_dev_num();
                for(int i = 0; i < AMT_BYTES_SYSTEM_ID; ++i)
                    if(system_id[i] != 0) {
                        err = false;
                        break;
                    }
                if(err)
                    break;
                if(!__GCM__.regist_device(system_id, adr_new)) {
                    Serial.println("Correct");
                }
                else
                    Serial.println("Error");
                break;
            }
            case 0xFD: {
                Serial.print("Stady = ");
                switch (__GCM__.contact_data_.get_state_contact())
                {
                case SC_DOWNTIME:
                    Serial.println("SC_DOWNTIME");
                    break;
                case SC_CONNECTION:
                    Serial.println("SC_CONNECTION");
                    break;
                case SC_EXCHANGE:
                    Serial.println("SC_EXCHANGE");
                    break;
                case SC_DISCONNECT:
                    Serial.println("SC_DISCONNECT");
                    break;
                case SC_COMPLETE:
                    Serial.println("SC_COMPLETE");
                    break;

                case SC_BROADCASTING:
                    Serial.println("SC_BROADCASTING");
                    break;
                case SC_WAITING:
                    Serial.println("SC_WAITING");
                    break;
                case SC_PACKET_ACCEPTED:
                    Serial.println("SC_PACKET_ACCEPTED");
                    break;
                case SC_REPLY_BROADCAST:
                    Serial.println("SC_REPLY_BROADCAST");
                    break;
                default:
                    break;
                }
                break;
            }
            case 0xFE:
                end_serial = true;
            case 0xFC: {
                if(__GCM__.mode_ == Group_control_module::GT_SETTING) {
                    Serial.println("Mode = GT_SETTING");
                }
                else if(__GCM__.mode_ == Group_control_module::GT_PROCESSING) {
                    Serial.println("Mode = GT_PROCESSING");
                }
                else {
                    Serial.println("Mode = ERROR!");
                }
                break;
            }
            default:
                break;
            }
        }
        
        if(end_serial || work_and_loop_control) {
            __GCM__.work_system();
            gcm_interface.report_to_server_read_data();
        }
    }
    else {
#if defined (BUILD_TESTING_CODE_409)
        static bool start_contact;
        static bool time_multiple = false; // для захода в if при первой кратности 10 мс
        start_contact = false;
        // Проверка нажатия кнопки для отправки
        if(millis() - change_button2_time > 200UL) {
            if(digitalRead(BUTTON2) != button2_state) {
                button2_state = !button2_state;
                if(button2_state) start_contact = true;
                change_button2_time = millis();
            }
        }
        if(millis() - change_button3_time > 200UL) {
            if(digitalRead(BUTTON3) != button3_state) {
                button3_state = !button3_state;
                if(button3_state) start_contact = true;
                change_button3_time = millis();
            }
        }
        if(start_contact && !led2_state) {
            // Отправка сообщения по нажатию кнопки, если не запущена система
            __GCM__.handler_devices();
            // Зажигаем диод 2 и выключаем остальные по нажатию кнопки
            led1_state = false;
            led2_state = true;
            led3_state = false;
            led2_time = millis();
        }
        if(millis() % 10 == 0) {
            if(time_multiple) {
                time_multiple = false;
                if(millis() - led1_time > 1000UL) {
                    if(led1_state == true)
                        led1_state = false;
                }
                if(millis() - led2_time > 10000UL) {
                    if(led2_state == true) {
                        led2_state = false;
                        Serial.println("<<< Error time send/receive! >>>");
                    }
                }
                if(millis() - led3_time > 1000UL) {
                    if(led3_state == true)
                        led3_state = false;
                }
            }
        }
        else {
            time_multiple = true;
        }
        digitalWrite(LED1, led1_state);
        digitalWrite(LED2, led2_state);
        digitalWrite(LED3, led3_state);
#endif
    }
}



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
        if(!lora_packet_data[i].free()) {
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
            for(int k = 0; k < AMT_BYTES_SYSTEM_ID; ++k) {
                if((__GCM__.filter_adr_[j])[k] < 16)
                    Serial.print("0");
                Serial.print((__GCM__.filter_adr_[j])[k], 16);
                if(k < AMT_BYTES_SYSTEM_ID - 1)
                    Serial.print(" ");
            }
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
            for(int k = 0; k < AMT_BYTES_SYSTEM_ID; ++k) {
                if((__GCM__.filter_adr_[j])[k] < 16)
                    Serial.print("0");
                Serial.print((__GCM__.filter_adr_[j])[k], 16);
                if(k < AMT_BYTES_SYSTEM_ID - 1)
                    Serial.print(" ");
            }
            Serial.print("}");

            if(j < __GCM__.filter_adr_.size() - 1)
                Serial.print(", ");
        }
        Serial.println("]");
    }
    Serial.println();
}
uint8_t use_type() {
    uint8_t r = -1;
    Serial.print("Write type (0-print, 1-sen, 2-dev): ");
    while(1) {
        r = Serial.read();
        if((r == '0') || (r == '1')  || (r == '2')) {
            Serial.println(r-'0');
            break;
        }
        if((r == 'S') || (r == 's')) {
            Serial.println((char)r);
            return 0xFD;
        }
        if((r == 'M') || (r == 'm')) {
            Serial.println((char)r);
            return 0xFC;
        }
        if((r == 'E') || (r == 'e')) {
            Serial.println((char)r);
            return 0xFE;
        }
        if(r == ' ')
            return r;
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
                for(int k = 0; k < AMT_BYTES_SYSTEM_ID; ++k) {
                    if((__GCM__.filter_adr_[j])[k] < 16)
                        Serial.print("0");
                    Serial.print((__GCM__.filter_adr_[j])[k], 16);
                    if(k < AMT_BYTES_SYSTEM_ID - 1)
                        Serial.print(" ");
                }
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
    return __GCM__.sensors_[r].get_address().branch; // adr.branch
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
                for(int k = 0; k < AMT_BYTES_SYSTEM_ID; ++k) {
                    if((__GCM__.filter_adr_[j])[k] < 16)
                        Serial.print("0");
                    Serial.print((__GCM__.filter_adr_[j])[k], 16);
                    if(k < AMT_BYTES_SYSTEM_ID - 1)
                        Serial.print(" ");
                }
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
    return __GCM__.devices_[r].get_address().branch; // adr.branch
}
std::array<uint8_t, AMT_BYTES_SYSTEM_ID> use_reg_sen_num() {
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
std::array<uint8_t, AMT_BYTES_SYSTEM_ID> use_reg_dev_num() {
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
void first_device_control() {
    if(serial_loop_control) {
        if(__GCM__.devices_.size()) {
            if(Serial.available() > 0) {
                uint8_t r_b = Serial.read();
                switch (r_b)
                {
                case 'm': // mode (S/W)
                case 'M': {
                    bool set_mode = false;
                    if(__GCM__.devices_[0].get_state___() == scs::State::work) {
                        __GCM__.devices_[0].set_state___(scs::State::from_work_to_stop);
                        set_mode = true;
                    }
                    else if(__GCM__.devices_[0].get_state___() == scs::State::stop) {
                        __GCM__.devices_[0].set_state___(scs::State::from_stop_to_work);
                        set_mode = true;
                    }
                    if(set_mode && !work_and_loop_control) {
                        __GCM__.work_system();
                    }
                    break;
                }
                case 't': // time: sync rtc
                case 'T': {
                    bool set_time = false;
                    if(!__GCM__.devices_[0].get_rtc_sync()) {
                        __GCM__.devices_[0].set_rtc_sync(true);
                        set_time = true;
                    }
                    if(set_time && !work_and_loop_control) {
                        __GCM__.work_system();
                    }
                    break;
                }
                case 'r': // read data
                case 'R': {
                    
                    if(!work_and_loop_control) {
                        LoRa_address connect_adr = __GCM__.contact_data_.get_my_adr();
                        connect_adr.branch = __GCM__.devices_[0].get_address().branch; // adr.branch
                        __GCM__.contact_data_.init_contact(connect_adr);
                        if(__GCM__.contact_data_.start_transfer())
                            Serial.println("Error start devices transfer");
                    }

                    break;
                }
                case '\n': // work
                    __GCM__.work_system();
                    break;
                default:
                    break;
                }
            }
        }
        else {
            static bool print_err = true;
            if(print_err) {
                Serial.println("Not device (serial_loop_control)!");
                print_err = false;
            }
        }
    }
}
