#define EMPTY
#if defined(EMPTY)

#include <Arduino.h>

#include <LoRa_packet.h>
#include <Packet_analyzer.h>
#include <LoRa.h>

#include <BluetoothSerial.h>

#include <WiFi.h>
#include <FS.h>
#include <WiFiUdp.h>

#include <Grow_sensor.h>
#include <Grow_sensor_interface.h>

LoRa lora;


// Sensors data
#define AMT_MODULES 3
LoRa_address adr_sensors[AMT_MODULES];
bool connect[AMT_MODULES];
bool start_report[AMT_MODULES];
bool server_report[AMT_MODULES];
// float value[AMT_MODULES];
uint16_t last_number[AMT_MODULES];

Grow_sensor sensor[AMT_MODULES];

enum Type_sensor gs_c = Air_temperature;
// Grow_sensor_interface grow_sensor_interface;

// WiFi data
const char * networkName = "a202";
const char * networkPswd = "gamma113";
const char * udpAddress  = "192.168.1.62"; // "192.168.1.56"; //"192.168.0.255"; 0123456789
const int udpPort = 3333;
bool connected = false;
WiFiUDP udp;
uint8_t send_buffer[256];
uint16_t send_size = 0;

// BlueTooth data

#define MAS_LEN 200 // максимальная длина принятых данных
uint8_t receive_data[MAS_LEN]; // полученные байты
uint8_t receive_len = 0; // количество полученных байт
bool error = false; // ошибка (не использую)
BluetoothSerial SerialBT;
int inc;
char nameSerial[] = "GreenHouse";




#define MY_ADDRESS LoRa_address(2,0)

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


#define BAND    43325E4
#define LoRa_SF   8
#define LoRa_SBW  250E3
#define SYNC_WORD 0x4A


// LoRa_address dest_adr;
LoRa_address sour_adr;
// Packet_Type packet_type;
uint16_t packet_num;
#define MAX_AMT 3
uint8_t g_amt;
uint8_t g_param[MAX_AMT] = {0, 0, 0};
uint8_t g_id[MAX_AMT] = {0, 0, 0};
uint32_t g_data[MAX_AMT] = {0, 0, 0};
float g_value = 0.0;

// void print_packet(const LoRa_packet &packet) {
//     Serial.print("{");
//     Serial.print(packet.get_len());
//     Serial.print("} = [");
//     for(int i = 0; i < packet.get_len(); ++i) {
//         if(packet[i] < 16)
//             Serial.print("0");
//         Serial.print(packet[i], 16);
//         if(i < packet.get_len()-1)
//             Serial.print(", ");
//     }
//     Serial.println("]");
// }

void lora_pin_interrupt() {
    // uint8_t error = 0;
    /// --- Приём пакета ---
    uint8_t pin0 = digitalRead(PIN_IRQ);
    // uint8_t pin0 = digitalRead(PIN_DIO0);
    uint8_t pin1 = digitalRead(PIN_DIO1);
    class LoRa_packet packet = lora.receiver_packet(0, 0);

    lora.mode_sleep();
    delay(5);
    lora.receiver_packet(1, 0);

    if(!((packet.get_len() < MINIMAL_PACKET_SIZE) || (packet.get_crc_error()))) {
        // Serial.println();
        // Serial.print("{");
        // Serial.print(pin0);
        // Serial.print(pin1);
        // Serial.print("} ");
        // print_packet(packet);
        /// --- Обработка пакета ---
                                                                        // Serial.println("Packet:");
        // dest_adr = packet_analyzer.get_dest_adr(packet);
        if(packet_analyzer.get_dest_adr(packet) != LoRa_address(2, 0))
            return;
                                                                        // Serial.print("DA = ");
                                                                        // Serial.print(dest_adr.group, 16);
                                                                        // Serial.print(".");
                                                                        // Serial.println(dest_adr.branch, 16);
        sour_adr = packet_analyzer.get_sour_adr(packet);
                                                                        // Serial.print("SA = ");
                                                                        // Serial.print(sour_adr.group, 16);
                                                                        // Serial.print(".");
                                                                        // Serial.println(sour_adr.branch, 16);
        // packet_type = packet_analyzer.get_packet_type(packet);
        if(packet_analyzer.get_packet_type(packet) != Packet_Type::SENSOR)
            return;
                                                                        // Serial.print("T  = ");
                                                                        // Serial.println((uint8_t)packet_type, 16);
        packet_num = packet_analyzer.get_packet_number(packet);
                                                                        // Serial.print("№  = ");
                                                                        // Serial.println(packet_num, 16);
                                                                        // print_packet(packet);

        // uint8_t size = 0xFF;
        // Serial.println("\nPacket_Sensor:\n");
        if(packet_sensor.get_packet_data(packet, &g_amt, g_param, g_id, g_data) == 0) {
            for(int i = 0; i < AMT_MODULES; ++i) {
                if(sour_adr.branch != adr_sensors[i].branch)
                    continue;
                // Serial.print("Report: ");
                // Serial.println(sour_adr.branch);
                if(!connect[i])
                    continue;
                // Serial.print("Connect: ");
                // Serial.println(sour_adr.branch);
                // value[i] = *((float*)&g_data[0]);
                g_value = *((float*)&g_data[0]);
                if((0 < g_value) && (g_value < 4000)) {
                    start_report[i] = true;
                    server_report[i] = true;
                                                                        Serial.print("SA = ");
                                                                        Serial.print(sour_adr.group, 16);
                                                                        Serial.print(".");
                                                                        Serial.print(sour_adr.branch, 16);
                                                                        Serial.print("     g_value = ");
                                                                        Serial.print(g_value);
                                                                        Serial.print("     i = ");
                                                                        Serial.println(i);
                    sensor[i].set_value(0, g_value);
                    // sensor[i].set_value({*((float*)&g_data[0])});
                    last_number[i] = packet_num;
                }
            }
        }
    }
    // else {
    //     Serial.print("{");
    //     Serial.print(pin0);
    //     Serial.print(pin1);
    //     Serial.print("} ");
    // }
}



void WiFiEvent(WiFiEvent_t event);
void connectToWiFi(const char * ssid, const char * pwd) {
  Serial.println("Connecting to WiFi network: " + String(ssid));

  // delete old config
  WiFi.disconnect(true);
  //register event handler
  WiFi.onEvent(WiFiEvent);
  
  //Initiate connection
  WiFi.begin(ssid, pwd);

  Serial.println("Waiting for WIFI connection...");
}
void WiFiEvent(WiFiEvent_t event) {
    switch(event) {
      case SYSTEM_EVENT_STA_GOT_IP:
          //When connected set 
          Serial.print("WiFi connected! IP address: ");
          Serial.println(WiFi.localIP());  
          //initializes the UDP state
          //This initializes the transfer buffer
          udp.begin(WiFi.localIP(),udpPort);
          connected = true;
          break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
          Serial.println("WiFi lost connection");
          connected = false;
          break;
      default: break;
    }
    if(connected) {
        Serial.println("True connection");
    }
    else {
        Serial.println("False connection");
    }
}
void WiFisend(uint8_t *buf, uint16_t size) {
    udp.beginPacket(udpAddress,udpPort);
    for(int i = 0; i < size; ++i) {
        udp.write(buf[i]);
    }
    udp.endPacket();

    Serial.println();
    Serial.print("|>-{");
    for(int i = 0; i < size; ++i) {
        if(buf[i] < 16)
            Serial.print("0");
        Serial.print(buf[i], 16);
        if(i < size - 1)
            Serial.print(", ");
    }
    Serial.println("}-<|\n");
    // Serial.printf("Seconds since boot: %lu\n", millis()/1000);
}


// функция вывода в терминал в 2-х видах true - в шестнадцатиричной, при false - в десятичной
void packet_print(bool hexadecimal) {
    Serial.print("[");
    for(int i = 0; i < receive_len; ++i) {
        if(hexadecimal)
            Serial.print(receive_data[i], 16);
        else
            Serial.print(receive_data[i]);
        if(i < receive_len - 1)
            Serial.print(", ");
    }
    Serial.print("] - кол-во пришедших символов = ");
    Serial.print(receive_len);
    Serial.print(" ");
}
void bluetooth_work() {

    if (SerialBT.available())//если есть что считывать - то заходим в тело условия
    {
        delay(10); // жду, чтобы не успел считать раньше, чем дойдут остальные
        error = false;
        while (SerialBT.available()) {
            if(receive_len == (MAS_LEN-1)) { // проверяю на переполнение, в целом массив можно увеличить
                error = true;
                break;
            }
            receive_data[receive_len++] = SerialBT.read(); // записываю полученные байты в массив
        }
        inc = receive_data[0];
        {
            // вывожу полученный код пакета
            Serial.print("{");
            Serial.print(inc);
            Serial.println("}");
            // пакет запроса 
            packet_print(true); // вывод в 16-й
            Serial.println();
            // packet_print(false); // вывод в 10-й
            // Serial.println();
        }
        // обрабатываю пакет в зависимости от (код - 100)
        switch (inc - 100) {
        case 2: // запрос имени МК модуля
        {
            Serial.println("name controller gets");
            int l = sizeof(nameSerial);
            uint8_t chars[l];
            for(int i = 0; i < l;i++)
            {
                chars[i] = (uint8_t)nameSerial[i];//переношу имя МК из одного массива char в другой uint8_t массив
            }
            SerialBT.write(chars, l);//отправляем имя МК
            break;
        }
        case 3: // Передача конфигурации
        {
            // продолжение вывода запроса, в ASKII с игнорированием переносов 
            // for(int i = 0; i < receive_len; ++i)
            //     if((receive_data[i] != '\n')&&(receive_data[i]!='\r')) {
            //         Serial.print((char)(receive_data[i]));
            //         Serial.print(" ");
            //     }
            // Serial.println("\n");
            // ответ
            uint8_t chars[1];
            chars[0] = 103;
            SerialBT.write(chars, 1);//отправляем имя МК
            break;
        }
        case 4: // - не используется - 
        {
            // ответ
            uint8_t chars[1];
            chars[0] = 103;
            SerialBT.write(chars, 1);//отправляем имя МК
            break;
        }
        case 5: // - не используется - 
        {
            // ответ
            uint8_t chars[1];
            chars[0] = 103;
            SerialBT.write(chars, 1);//отправляем имя МК
            break;
        }
        case 6: // Запрос регистрирующихся датчиков
        {
            // ответ
            uint8_t num = 0;
            uint8_t amt = 0;
            uint8_t chars[8];
            chars[num++] = 106; // код
            num++; // amt

            for(int k = 0; k < AMT_MODULES; ++k) {
                if(connect[k])
                    continue;
                ++amt;

                uint16_t id = sensor[k].get_system_id();
                chars[num++] = id & 0xFF;
                chars[num++] = (id >> 8)  & 0xFF;
            }
            chars[1] = amt; // кол-во

            // if(receive_data[1] == 45) { // первый байт периода равен 45
            //     chars[1] = 3; // кол-во
            //     // 5387
            //     chars[2] = 11;
            //     chars[3] = 21;
            //     // 10527
            //     chars[4] = 31;
            //     chars[5] = 41;
            //     // 3860
            //     chars[6] = 20;
            //     chars[7] = 15;
            // }
            // else if(receive_data[1] == 20) { // первый байт периода равен 20
            //     chars[1] = 2; // кол-во
            //     // 5390
            //     chars[2] = 14;
            //     chars[3] = 21;
            //     // 10530
            //     chars[4] = 34;
            //     chars[5] = 41;
            //     // // 3865
            //     // chars[6] = 20;
            //     // chars[7] = 15;
            // }
            // else {
            //     chars[1] = 1; // кол-во
            //     // 3865
            //     chars[6] = 20;
            //     chars[7] = 15;
            // }
            SerialBT.write(chars, (2 + (chars[1]*2))); // в chars[1] содержится количество адресов
            break;
        }
        case 7: // - не используется - 
        {
            // ответ
            uint8_t chars[1];
            chars[0] = 103;
            SerialBT.write(chars, 1);//отправляем имя МК
            break;
            break;
        }
        case 8: // Запрос регистрирующихся устройств
        {
            // ответ
            uint8_t chars[10];
            chars[0] = 108; // код
            chars[1] = 0; // кол-во
            SerialBT.write(chars, (2 + (chars[1]*2)));
            break;
        }
        case 9: // ? регистрация ?
        {
            // ответ
            uint8_t chars[8];
            uint8_t num = 0;
            chars[num++] = 109;
            if(receive_len == 1) { // Отстутствуют адреса
                // нет адресов
                Serial.println("Err send: 1");
                chars[1] = 1;
                SerialBT.write(chars, 2);
            }
            else {
                --receive_len;
                for(int k = 0; k < (receive_len/4) ; ++k) {
                    chars[num++] = 0;
                }
                SerialBT.write(chars, num);


                uint16_t recieve_num = 1;
                for(int k = 0; k < (receive_len/4) ; ++k) {
                    uint32_t recieve_id = 0;

                    recieve_id = recieve_id | receive_data[recieve_num++];
                    recieve_id = recieve_id | (((uint32_t)receive_data[recieve_num++]) << 8);
                    recieve_id = recieve_id | (((uint32_t)receive_data[recieve_num++]) << 16);
                    recieve_id = recieve_id | (((uint32_t)receive_data[recieve_num++]) << 24);

                    for(int i = 0; i < AMT_MODULES; ++i) {
                        if(recieve_id != sensor[i].get_system_id())
                            continue;
                        connect[i] = true;
                    }
                    ++recieve_num;
                }
            }
            break;
        }
        case 10: // ? обновление данных ?
        {
            
            // ответ
            uint8_t chars[20];
            uint8_t num = 0;
            chars[num++] = 110;


            for(int i = 0; i < AMT_MODULES; ++i) {
                if(!connect[i])
                    continue;

                float t_value;
                if(!start_report[i]) {
                    t_value = 0.0;
                }
                else {
                    sensor[i].get_value(0, t_value);
                }
                Serial.print("i = ");
                Serial.print(i);
                Serial.print("   t_value = ");
                Serial.println(t_value);
                uint32_t float_data = 0; 
                float_data = *reinterpret_cast<uint32_t*>(&t_value);

                chars[num++] = 0x01;
                chars[num++] = 0x06;
                // chars[num++] = (float_data >> 24) & 0xFF;
                // chars[num++] = (float_data >> 16) & 0xFF;
                // chars[num++] = (float_data >>  8) & 0xFF;
                // chars[num++] =  float_data        & 0xFF;
                chars[num++] =  float_data        & 0xFF;
                chars[num++] = (float_data >>  8) & 0xFF;
                chars[num++] = (float_data >> 16) & 0xFF;
                chars[num++] = (float_data >> 24) & 0xFF;

            }
            
            // uint32_t float_data = 0; 
            // float_data = *reinterpret_cast<uint32_t*>(&value);

            // // ответ
            // uint8_t chars[15];
            // chars[0] = 110;
            // chars[1] = 0x01;
            // chars[2] = 0x04;
            // chars[3] =  float_data        & 0xFF;
            // chars[4] = (float_data >>  8) & 0xFF;
            // chars[5] = (float_data >> 16) & 0xFF;
            // chars[6] = (float_data >> 24) & 0xFF;

            // chars[3] = (float_data >> 24) & 0xFF;
            // chars[4] = (float_data >> 16) & 0xFF;
            // chars[5] = (float_data >>  8) & 0xFF;
            // chars[6] =  float_data        & 0xFF;

            // chars[3] = 0;
            // chars[4] = 0;
            // chars[5] = 177;
            // chars[6] = 66;

            SerialBT.write(chars, num);
            // chars[7] = 0x02;
            // chars[8] = 0x02;
            // chars[9] = 17;
            // chars[10] = 66;
            // chars[11] = 0x02;
            // chars[12] = 0x03;
            // chars[13] = 1;
            // SerialBT.write(chars, 14);



            break;
        }
        default: // иное (ошибка)
            error = true;
            Serial.print("Error code!");
            packet_print(true);
            Serial.println();
            packet_print(false);
            Serial.println();
            break;
        }
        receive_len = 0; // обнуляю данные о пакете
    }
}

void setup() {
    Serial.begin(115200);
    while(!Serial) {}
    delay(1000);


    for(int i = 0; i < AMT_MODULES; ++i) {
        adr_sensors[i].group = 2;
        adr_sensors[i].branch = 1+i;
        connect[i] = false;
        start_report[i] = false;
        server_report[i] = false;
        sensor[i] = Grow_sensor(1, &gs_c);
        sensor[i].set_system_id(i+1);
        sensor[i].set_value({-273.0});
        // value[i] = -273.0;
        last_number[i] = 0;
    }
        connect[0] = true;
        connect[1] = true;

    pinMode(LED, OUTPUT);

    uint8_t err = lora.init(PIN_RESET, SPI_BUS, SPI_NSS, PIN_DIO0, PIN_DIO1);
    if(err != 0)
        while(1){
            delay(1000);
            digitalWrite(LED, HIGH);
            delay(1000);
            digitalWrite(LED, LOW);
        };
    err = lora.begin(BAND, true, 14, LoRa_SF, LoRa_SBW, SYNC_WORD);
    if(err != 0)
        while(1){
            delay(1000);
            digitalWrite(LED, HIGH);
            delay(1000);
            digitalWrite(LED, LOW);
        };
    
#if defined( WIFI_LoRa_32_V2 ) 
    pinMode(PIN_IRQ, INPUT);
#endif // WIFI_LoRa_32_V2

    attachInterrupt(digitalPinToInterrupt(PIN_DIO0), lora_pin_interrupt, RISING);
    attachInterrupt(digitalPinToInterrupt(PIN_DIO1), lora_pin_interrupt, RISING);
#if defined( WIFI_LoRa_32_V2 ) 
    attachInterrupt(digitalPinToInterrupt(PIN_IRQ),  lora_pin_interrupt, RISING);
#endif // WIFI_LoRa_32_V2

    // connectToWiFi(networkName, networkPswd);
    SerialBT.begin(nameSerial); // Задаем имя вашего устройства Bluetooth
    Serial.println("Bluetooth Device is Ready to Pair");  // По готовности сообщаем, что устройство готово к сопряжению


    lora.mode_sleep();
    delay(5);
    lora.receiver_packet(1, 0);

    /*
    if(0) {
        LoRa_packet packet1;
        uint8_t err = 0;

        // --- Packet_analyzer ---
        Serial.println();

        print_packet(packet1);
        packet_analyzer.set_packet_number(packet1, 0x0102);
        print_packet(packet1);
        packet_analyzer.set_packet_type(packet1, 0x04);
        packet_analyzer.set_dest_adr(packet1, LoRa_address(0x102030));
        packet_analyzer.set_sour_adr_branch(packet1, 0x444);
        packet_analyzer.set_sour_adr_group(packet1, (0x55<<1));
        print_packet(packet1);
        LoRa_address d_adr;
        LoRa_address s_adr;
        Packet_Type type;
        uint16_t num;
        Serial.println("Packet:");
        d_adr = packet_analyzer.get_dest_adr(packet1);
        Serial.print("DA = ");
        Serial.print(d_adr.group, 16);
        Serial.print(".");
        Serial.println(d_adr.branch, 16);
        s_adr = packet_analyzer.get_sour_adr(packet1);
        Serial.print("SA = ");
        Serial.print(s_adr.group, 16);
        Serial.print(".");
        Serial.println(s_adr.branch, 16);
        type = packet_analyzer.get_packet_type(packet1);
        Serial.print("T  = ");
        Serial.println((uint8_t)type, 16);
        num = packet_analyzer.get_packet_number(packet1);
        Serial.print("№  = ");
        Serial.println(num, 16);
        print_packet(packet1);

        Serial.println();
        // --- Packet_Connection ---
        Serial.println();


        bool one_connect = true;
        for(int i = 0; (i < 0x0C+2); ++i) {
            LoRa_packet packet2 = packet1;
            if(!one_connect) {
                Serial.print(" ----- ");
                Serial.print(i);
                Serial.println(" ----- ");
            }
            uint8_t s_data[5] = {0x05, 0x06, 0x07, 0x08, 0x09};
            err = packet_connection.set_command(packet2, i);
            if(err) Serial.println("~packet_connection.set_command()");
            err = packet_connection.set_data(packet2, s_data, 2);
            if(err) Serial.println("~packet_connection.set_data()");
            
            print_packet(packet2);

            uint8_t size = 0xFF;
            uint8_t com = 0xFF;
            uint8_t data[3] = {0, 0, 0};
            uint8_t len = 0xFF;
            Serial.println("Packet_Connection:");
            err = packet_connection.get_command(packet2, &com);
            if(err) Serial.print("~");
            Serial.print("Com = ");
            Serial.println(com, 16);
            err = packet_connection.get_size_by_packet(packet2, size);
            if(err) Serial.print("~");
            Serial.print("Size = ");
            Serial.println(size, 16);
            err = packet_connection.get_data(packet2, data, &len);
            if(err) Serial.print("~");
            Serial.print("Data{");
            Serial.print(len);
            Serial.print("} = [");
            for(int i = 0; i < size; ++i) {
                Serial.print(data[i], 16);
                if(i < size - 1)
                Serial.print(", ");
            }
            Serial.println("]");
            if(one_connect)
                break;
        }

        Serial.println();
        Serial.println();
        // --- Packet_Sensor ---
        Serial.println();
        Serial.println();


        one_connect = false;
        for(int s = 0; s < 2; ++s) {
            if(s == 0)
                err = packet_sensor.set_setting(0);
            if(s == 1)
                err = packet_sensor.set_setting(1);
                // err = packet_sensor.set_setting(5);

            if(err)
                Serial.println("~packet_sensor.set_setting");

            uint8_t s_amt = 5;
            for(int i = 0; (i < s_amt); ++i) {
                LoRa_packet packet2 = packet1;
                if(!one_connect) {
                    Serial.print(" ----- ");
                    Serial.print(i);
                    Serial.println(" ----- ");
                }
                uint8_t s_param[10] = {2, 3, 4, 5, 6, 7, 8, 9, 0, 1};
                uint8_t s_id[10] =    {1, 7, 2, 5, 3, 6, 0, 8, 9, 4};
                uint32_t s_data[10] = {0x12341234, 0x23452345, 0x34563456, 0x45674567, 0x56785678, 0x67896789, 0x78907890, 0x89018901, 0x0234, 0x01};

                uint8_t bias = i % s_amt;
                for(int k = 0; k < bias; ++k) {
                    uint32_t buf[3];
                    uint8_t num = 0; 

                    buf[0] = s_param[num];
                    buf[1] = s_id[num];
                    buf[2] = s_data[num];
                    for(int cycle = 0; cycle < s_amt - 1; ++cycle) {
                        s_param[num] = s_param[num + 1]; 
                        s_id[num]    = s_id[num + 1];
                        s_data[num]  = s_data[num + 1];
                        
                        ++num;
                    }
                    // s_param[num] = s_param[num + 1]; ++num;
                    s_param[num] = buf[0];
                    s_id[num]    = buf[1];
                    s_data[num]  = buf[2];

                    // buf = s_id[num];
                    // s_id[num] = s_id[num + 1]; ++num;
                    // s_id[num] = s_id[num + 1]; ++num;
                    // s_id[num] = buf;

                    // buf = s_data[num];
                    // s_data[num] = s_data[num + 1]; ++num;
                    // s_data[num] = s_data[num + 1]; ++num;
                    // s_data[num] = buf;
                }
                
                err = packet_sensor.set_packet_data(packet2, &s_amt, s_param, s_id, s_data);
                if(err) Serial.println("~packet_connection.set_packet_data()");

                print_packet(packet2);

                uint8_t size = 0xFF;
                uint8_t g_amt = 10;
                uint8_t g_param[g_amt] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
                uint8_t g_id[g_amt] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
                uint32_t g_data[g_amt] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


                Serial.println("\nPacket_Sensor:\n");

                err = packet_sensor.get_size_by_packet(packet2, &g_amt, g_param, size);
                if(err) Serial.println("   ~packet_connection.get_size_by_packet()");

                Serial.print("   g_amt = ");
                Serial.print(g_amt);
                Serial.print(" [");
                for(int k = 0; k < g_amt; ++k) {
                    Serial.print("{ p=");
                    Serial.print(g_param[k]);
                    Serial.print(" }");
                    if(k < g_amt - 1)
                        Serial.print(",  ");
                }
                Serial.println("]");
                Serial.print("   Size = ");
                Serial.println(size);

                Serial.println();

                err = packet_sensor.get_packet_data(packet2, &g_amt, g_param, g_id, g_data);
                if(err) Serial.println("   ~packet_connection.get_packet_data()");

                Serial.print("   g_amt = ");
                Serial.print(g_amt);
                Serial.print(" [");
                for(int k = 0; k < g_amt; ++k) {
                    Serial.print("{ p=");
                    Serial.print(g_param[k]);
                    Serial.print(" | id=");
                    Serial.print(g_id[k]);
                    Serial.print(" | d=");
                    Serial.print(g_data[k],16);
                    Serial.print(" }");
                    if(k < g_amt - 1)
                        Serial.print(",  ");
                }
                Serial.println("]");
                Serial.println();
            }
            Serial.println();
            Serial.println();
        }

        Serial.println();
    }
    else {
        uint8_t error = 0;
        LoRa_packet packet;

        LoRa_address dest_adr;
        LoRa_address sour_adr;
        Packet_Type packet_type;
        uint16_t packet_num;
                                                                        Serial.println("Packet:");
        dest_adr = packet_analyzer.get_dest_adr(packet);
                                                                        Serial.print("DA = ");
                                                                        Serial.print(dest_adr.group, 16);
                                                                        Serial.print(".");
                                                                        Serial.println(dest_adr.branch, 16);
        sour_adr = packet_analyzer.get_sour_adr(packet);
                                                                        Serial.print("SA = ");
                                                                        Serial.print(sour_adr.group, 16);
                                                                        Serial.print(".");
                                                                        Serial.println(sour_adr.branch, 16);
        packet_type = packet_analyzer.get_packet_type(packet);
                                                                        Serial.print("T  = ");
                                                                        Serial.println((uint8_t)packet_type, 16);
        packet_num = packet_analyzer.get_packet_number(packet);
                                                                        Serial.print("№  = ");
                                                                        Serial.println(packet_num, 16);
                                                                        print_packet(packet);

        uint8_t size = 0xFF;
        uint8_t g_amt, max_amt = 3;
        uint8_t g_param[max_amt] = {0, 0, 0};
        uint8_t g_id[max_amt] = {0, 0, 0};
        uint32_t g_data[max_amt] = {0, 0, 0};

        // Serial.println("\nPacket_Sensor:\n");

        error = packet_sensor.get_packet_data(packet, &g_amt, g_param, g_id, g_data);
        if(error) 
            Serial.println("   ~packet_connection.get_packet_data()");
        else {
            for(int i = 0; i < AMT_MODULES; ++i) {
                if(sour_adr != adr_sensors[i])
                    continue;
                if(!connect[i])
                    break;
                start_report[i] = true;
                server_report[i] = true;
                value[i] = *((float*)&g_data[i]);
            }
        }
    }


    Serial.println();
    */
}
void loop() {
    // bluetooth_work();

    // for(int i = 0; i < AMT_MODULES; ++i) {
    //     if((!connect[i]) || (!start_report[i]) || (!server_report[i]))
    //         continue;
    //     send_size = grow_sensor_interface.report_to_server(sensor[i], send_buffer);
    //     WiFisend(send_buffer, send_size);
    //     server_report[i] = false;
    // }
}

#else 

#define EMPTY
#ifdef EMPTY
#include <Arduino.h>


void setup() {
    // put your setup code here, to run once:
}
void loop() {
    // put your main code here, to run repeatedly:
}


#endif

#ifndef EMPTY
#include <Arduino.h>
void setup() {
    Serial.begin(115200);
    while(!Serial) {}
    delay(5000);
    pinMode(GPIO_NUM_4, INPUT);
    pinMode(GPIO_NUM_5, INPUT);
}
void my_sleep() { while(Serial.read()!='\n')delay(100); }
void loop() {
    int p4, p5;
    p4 = digitalRead(GPIO_NUM_4);
    p5 = analogRead(GPIO_NUM_5);
    Serial.print("Input at pin 4 = ");
    Serial.println(p4);
    Serial.print("Input at pin 5 = ");
    Serial.println(p5);

    delay(1000);

    while(Serial.read()!='\n')delay(500);


    static unsigned long timepoint = millis();
	if (millis() - timepoint > 1000U) { // ожидание 1с
        // ожидающие операции
		timepoint = millis();
    }
    // операции вне ожидания
}
#endif

#endif