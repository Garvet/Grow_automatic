#include "build_data.h"

#if defined( BUILD_S2_D3 )
uint8_t data[100] = {0x00, 0x01, // Address
                     0x00, 0x04, // Channel
                     0x0F,       // Name.size()
                     0x00, 0x02, // Sensors.size()
                     0x00, 0x03, // Devices.size()
                     0xD0, 0x93, 0xD1, 0x80, 0xD1, 0x83, 0xD0, 0xBF, 0xD0, 0xBF, 0xD0, 0xB0, 0x20, 0x31, 0x00, // Name
                     // Sensor[0]
                     0x00, 0x09, 0xFB, 0xF1, // period (654321 ms)
                     0x02,                   // component.size()
                     0x04, 0x03,             // component.type[0-1]
                     // Sensor[1]
                     0x00, 0x00, 0x00, 0x00, // period (-)
                     0x02,                   // component.size()
                     0x06, 0x06,             // component.type[0-1]
                     // Devices[0]
                     0x01,                         // component.size()
                     0x05, 0x02, 0x01, 0x10, 0x07, // component[0]
                     // Devices[1]
                     0x02,                         // component.size()
                     0x05, 0x02, 0x01, 0x10, 0x07, // component[0]
                     0x03, 0x01, 0x06, 0x05, 0x02, // component[1]
                     // Devices[2]
                     0x01,                         // component.size()
                     0x05, 0x02, 0x01, 0x10, 0x07  // component[0]
                     };
// const uint16_t MAX_ERRORS_IN_ROW = 6;
#elif defined( BUILD_S2_D0 )
uint8_t data[100] = {0x00, 0x01, // Address
                     0x00, 0x04, // Channel
                     0x0F,       // Name.size()
                     0x00, 0x02, // Sensors.size() //  0x00, 0x03, // Sensors.size()
                     0x00, 0x00, // Devices.size()
                     0xD0, 0x93, 0xD1, 0x80, 0xD1, 0x83, 0xD0, 0xBF, 0xD0, 0xBF, 0xD0, 0xB0, 0x20, 0x31, 0x00, // Name
                     // Sensor[0]
                     0x00, 0x00, 0x20, 0x00, // period (8192 ms)
                     0x01,                   // component.size()
                     0x06,                   // component.type[0-1]
                     // Sensor[1]
                     0x00, 0x00, 0x2F, 0x10, // period (12048 ms)
                     0x01,                   // component.size()
                     0x06,                   // component.type[0-1]
                     };
// const uint16_t MAX_ERRORS_IN_ROW = 3;
#elif defined( BUILD_S1_D0_30s )
uint8_t data[100] = {0x00, 0x01, // Address
                     0x00, 0x04, // Channel
                     0x0F,       // Name.size()
                     0x00, 0x01, // Sensors.size() //  0x00, 0x03, // Sensors.size()
                     0x00, 0x00, // Devices.size()
                     0xD0, 0x93, 0xD1, 0x80, 0xD1, 0x83, 0xD0, 0xBF, 0xD0, 0xBF, 0xD0, 0xB0, 0x20, 0x31, 0x00, // Name
                     // Sensor[0]
                     0x00, 0x00, 0x75, 0x30, // period (30 sec)
                     0x03,                   // component.size()
                     0x03, 0x04, 0x06,                   // component.type[0-1]
                    };
// const uint16_t MAX_ERRORS_IN_ROW = 2;
#elif defined( BUILD_S1_D0_60s )
uint8_t data[100] = {0x00, 0x01, // Address
                     0x00, 0x04, // Channel
                     0x0F,       // Name.size()
                     0x00, 0x01, // Sensors.size() //  0x00, 0x03, // Sensors.size()
                     0x00, 0x00, // Devices.size()
                     0xD0, 0x93, 0xD1, 0x80, 0xD1, 0x83, 0xD0, 0xBF, 0xD0, 0xBF, 0xD0, 0xB0, 0x20, 0x31, 0x00, // Name
                     // Sensor[0]
                     0x00, 0x00, 0xEA, 0x60, // period (1 min)
                     0x03,                   // component.size()
                     0x03, 0x04, 0x06,       // component.type[0-1]
                    };
// const uint16_t MAX_ERRORS_IN_ROW = 2;
#elif defined( BUILD_S1_D0_8s2 )
uint8_t data[100] = {0x00, 0x01, // Address
                     0x00, 0x04, // Channel
                     0x0F,       // Name.size()
                     0x00, 0x01, // Sensors.size() //  0x00, 0x03, // Sensors.size()
                     0x00, 0x00, // Devices.size()
                     0xD0, 0x93, 0xD1, 0x80, 0xD1, 0x83, 0xD0, 0xBF, 0xD0, 0xBF, 0xD0, 0xB0, 0x20, 0x31, 0x00, // Name
                     // Sensor[0]
                     0x00, 0x00, 0x20, 0x30, // period (8240 ms)
                     0x03,                   // component.size()
                     0x03, 0x04, 0x06,       // component.type[0-1]
                    };
// const uint16_t MAX_ERRORS_IN_ROW = 2;
#elif defined( BUILD_S6_D0_60s )
uint8_t data[100] = {0x00, 0x01, // Address
                     0x00, 0x04, // Channel
                     0x0F,       // Name.size()
                     0x00, 0x06, // Sensors.size() //  0x00, 0x03, // Sensors.size()
                     0x00, 0x00, // Devices.size()
                     0xD0, 0x93, 0xD1, 0x80, 0xD1, 0x83, 0xD0, 0xBF, 0xD0, 0xBF, 0xD0, 0xB0, 0x20, 0x31, 0x00, // Name
                     // Sensor[0]
                     0x00, 0x00, 0xEA, 0x60, // period (1 min)
                     0x03,                   // component.size()
                     0x03, 0x04, 0x06,       // component.type[0-1]
                     // Sensor[1]
                     0x00, 0x00, 0xEA, 0x60, // period (1 min)
                     0x03,                   // component.size()
                     0x03, 0x04, 0x06,       // component.type[0-1]
                     // Sensor[2]
                     0x00, 0x00, 0xEA, 0x60, // period (1 min)
                     0x03,                   // component.size()
                     0x03, 0x04, 0x06,       // component.type[0-1]
                     // Sensor[3]
                     0x00, 0x00, 0xEA, 0x60, // period (1 min)
                     0x03,                   // component.size()
                     0x03, 0x04, 0x06,       // component.type[0-1]
                     // Sensor[4]
                     0x00, 0x00, 0xEA, 0x60, // period (1 min)
                     0x03,                   // component.size()
                     0x03, 0x04, 0x06,       // component.type[0-1]
                     // Sensor[5]
                     0x00, 0x00, 0xEA, 0x60, // period (1 min)
                     0x03,                   // component.size()
                     0x03, 0x04, 0x06,       // component.type[0-1]
                    };
// const uint16_t MAX_ERRORS_IN_ROW = 7;
#elif defined( BUILD_S3_D0_60s )
uint8_t data[100] = {0x00, 0x01, // Address
                     0x00, 0x04, // Channel
                     0x0F,       // Name.size()
                     0x00, 0x03, // Sensors.size() //  0x00, 0x03, // Sensors.size()
                     0x00, 0x00, // Devices.size()
                     0xD0, 0x93, 0xD1, 0x80, 0xD1, 0x83, 0xD0, 0xBF, 0xD0, 0xBF, 0xD0, 0xB0, 0x20, 0x31, 0x00, // Name
                     // Sensor[0]
                     0x00, 0x00, 0xEA, 0x60, // period (1 min)
                     0x03,                   // component.size()
                     0x03, 0x04, 0x06,       // component.type[0-1]
                     // Sensor[1]
                     0x00, 0x00, 0xEA, 0x60, // period (1 min)
                     0x03,                   // component.size()
                     0x03, 0x04, 0x06,       // component.type[0-1]
                     // Sensor[2]
                     0x00, 0x00, 0xEA, 0x60, // period (1 min)
                     0x03,                   // component.size()
                     0x03, 0x04, 0x06,       // component.type[0-1]
                    };
// const uint16_t MAX_ERRORS_IN_ROW = 4;
#elif defined( BUILD_S3_D0_10s )
uint8_t data[100] = {0x00, 0x01, // Address
                     0x00, 0x04, // Channel
                     0x0F,       // Name.size()
                     0x00, 0x03, // Sensors.size() //  0x00, 0x03, // Sensors.size()
                     0x00, 0x00, // Devices.size()
                     0xD0, 0x93, 0xD1, 0x80, 0xD1, 0x83, 0xD0, 0xBF, 0xD0, 0xBF, 0xD0, 0xB0, 0x20, 0x31, 0x00, // Name
                     // Sensor[0]
                     0x00, 0x00, 0x2A, 0x60, // period (10 sec)
                     0x03,                   // component.size()
                     0x03, 0x04, 0x06,       // component.type[0-1]
                     // Sensor[1]
                     0x00, 0x00, 0x2A, 0x60, // period (10 sec)
                     0x03,                   // component.size()
                     0x03, 0x04, 0x06,       // component.type[0-1]
                     // Sensor[2]
                     0x00, 0x00, 0x2A, 0x60, // period (10 sec)
                     0x03,                   // component.size()
                     0x03, 0x04, 0x06,       // component.type[0-1]
                    };
// const uint16_t MAX_ERRORS_IN_ROW = 4;
#elif defined( BUILD_S1_D0_60s_BT ) // T, H, CO2
uint8_t data[100] = {0x00, 0x02, // Address
                     0x00, 0x09, // Channel
                     0x0F,       // Name.size()
                     0x00, 0x01, // Sensors.size() //  0x00, 0x03, // Sensors.size()
                     0x00, 0x00, // Devices.size()
                     0xD0, 0x93, 0xD1, 0x80, 0xD1, 0x83, 0xD0, 0xBF, 0xD0, 0xBF, 0xD0, 0xB0, 0x20, 0x31, 0x00, // Name
                     // Sensor[0]
                     0x00, 0x00, 0xEA, 0x60, // period (1 min)
                     0x03,                   // component.size()
                     0x03, 0x04, 0x0B,       // component.type[0-1]
                    };
// const uint16_t MAX_ERRORS_IN_ROW = 2;
#elif defined( BUILD_S1_D0_15s_A3 ) // T, H, CO2
uint8_t data[100] = {0x00, 0x03, // Address
                     0x00, 0x09, // Channel
                     0x0F,       // Name.size()
                     0x00, 0x01, // Sensors.size() //  0x00, 0x03, // Sensors.size()
                     0x00, 0x00, // Devices.size()
                     0xD0, 0x93, 0xD1, 0x80, 0xD1, 0x83, 0xD0, 0xBF, 0xD0, 0xBF, 0xD0, 0xB0, 0x20, 0x31, 0x00, // Name
                     // Sensor[0]
                     0x00, 0x00, 0x3A, 0x98, // period (15 sec)
                     0x03,                   // component.size()
                     0x03, 0x04, 0x06,       // component.type[0-1]
                    };
// const uint16_t MAX_ERRORS_IN_ROW = 2;
#elif defined( BUILD_S1_D0_60s_A3 ) // T, H, CO2
uint8_t data[100] = {0x00, 0x03, // Address
                     0x00, 0x09, // Channel
                     0x0F,       // Name.size()
                     0x00, 0x01, // Sensors.size() //  0x00, 0x03, // Sensors.size()
                     0x00, 0x00, // Devices.size()
                     0xD0, 0x93, 0xD1, 0x80, 0xD1, 0x83, 0xD0, 0xBF, 0xD0, 0xBF, 0xD0, 0xB0, 0x20, 0x31, 0x00, // Name
                     // Sensor[0]
                     0x00, 0x00, 0xEA, 0x60, // period (1 min)
                     0x03,                   // component.size()
                     0x03, 0x04, 0x06,       // component.type[0-1]
                    };
// const uint16_t MAX_ERRORS_IN_ROW = 2;
#elif defined( BUILD_S2_D0_60s_A3 ) // T, H, CO2
uint8_t data[100] = {0x00, 0x03, // Address
                     0x00, 0x09, // Channel
                     0x0F,       // Name.size()
                     0x00, 0x02, // Sensors.size() //  0x00, 0x03, // Sensors.size()
                     0x00, 0x00, // Devices.size()
                     0xD0, 0x93, 0xD1, 0x80, 0xD1, 0x83, 0xD0, 0xBF, 0xD0, 0xBF, 0xD0, 0xB0, 0x20, 0x31, 0x00, // Name
                     // Sensor[0]
                     0x00, 0x00, 0xEA, 0x60, // period (1 min)
                     0x03,                   // component.size()
                     0x03, 0x04, 0x06,       // component.type[0-1]
                     // Sensor[1]
                     0x00, 0x00, 0xEA, 0x60, // period (1 min)
                     0x03,                   // component.size()
                     0x03, 0x04, 0x06,       // component.type[0-1]
                    };
// const uint16_t MAX_ERRORS_IN_ROW = 3;
#elif defined( BUILD_S0_D1_15s_A3 ) // Lamp_d1, Lamp_d2
uint8_t data[100] = {0x00, 0x03, // Address
                     0x00, 0x09, // Channel
                     0x0F,       // Name.size()
                     0x00, 0x00, // Sensors.size() //  0x00, 0x03, // Sensors.size()
                     0x00, 0x01, // Devices.size()
                     0xD0, 0x93, 0xD1, 0x80, 0xD1, 0x83, 0xD0, 0xBF, 0xD0, 0xBF, 0xD0, 0xB0, 0x20, 0x31, 0x00, // Name
                     // Devices[0]
                     0x00, 0x00, 0x3A, 0x98,       // period (15 sec)
                     0x02,                         // component.size()
                     0x04, 0x04, //0x10, 0x07, // component[0-3]
                    };
// const uint16_t MAX_ERRORS_IN_ROW = 2;
#elif defined( BUILD_S0_D1_60s_A3 ) // Lamp_d1, Lamp_d2
uint8_t data[100] = {0x00, 0x03, // Address
                     0x00, 0x09, // Channel
                     0x0F,       // Name.size()
                     0x00, 0x00, // Sensors.size() //  0x00, 0x03, // Sensors.size()
                     0x00, 0x01, // Devices.size()
                     0xD0, 0x93, 0xD1, 0x80, 0xD1, 0x83, 0xD0, 0xBF, 0xD0, 0xBF, 0xD0, 0xB0, 0x20, 0x31, 0x00, // Name
                     // Devices[0]
                     0x00, 0x00, 0xEA, 0x60,       // period (1 min)
                     0x02,                         // component.size()
                     0x04, 0x04, //0x10, 0x07, // component[0-3]
                    };
// const uint16_t MAX_ERRORS_IN_ROW = 2;


// uint8_t data[100] = {0x00, 0x01, // Address
//                      0x00, 0x04, // Channel
//                      0x0F,       // Name.size()
//                      0x00, 0x02, // Sensors.size()
//                      0x00, 0x03, // Devices.size()
//                      0xD0, 0x93, 0xD1, 0x80, 0xD1, 0x83, 0xD0, 0xBF, 0xD0, 0xBF, 0xD0, 0xB0, 0x20, 0x31, 0x00, // Name
//                      // Sensor[0]
//                      0x00, 0x09, 0xFB, 0xF1, // period
//                      0x02,                   // component.size()
//                      0x04, 0x03,             // component.type[0-1]
//                      // Sensor[1]
//                      0x00, 0x00, 0x00, 0x00, // period
//                      0x02,                   // component.size()
//                      0x06, 0x06,             // component.type[0-1]
//                      // Devices[0]
//                      0x01,                         // component.size()
//                      0x05, 0x02, 0x01, 0x10, 0x07, // component[0]
//                      // Devices[1]
//                      0x02,                         // component.size()
//                      0x05, 0x02, 0x01, 0x10, 0x07, // component[0]
//                      0x03, 0x01, 0x06, 0x05, 0x02, // component[1]
//                      // Devices[2]
//                      0x01,                         // component.size()
//                      0x05, 0x02, 0x01, 0x10, 0x07  // component[0]
//                      };
// // const uint16_t MAX_ERRORS_IN_ROW = 6;


#elif defined( BUILD_S2_D0_60s_BASEMENT )
uint8_t data[100] = {0x00, 0x01, // Address
                     0x00, 0x08, // Channel
                     0x0F,       // Name.size()
                     0x00, 0x02, // Sensors.size() //  0x00, 0x03, // Sensors.size()
                     0x00, 0x00, // Devices.size()
                     0xD0, 0x93, 0xD1, 0x80, 0xD1, 0x83, 0xD0, 0xBF, 0xD0, 0xBF, 0xD0, 0xB0, 0x20, 0x31, 0x00, // Name
                     // Sensor[0]
                     0x00, 0x00, 0xEA, 0x60, // period (1 min)
                     0x03,                   // component.size()
                     0x03, 0x04, 0x0B,       // component.type[0-1]
                     // Sensor[1]
                     0x00, 0x00, 0xEA, 0x60,       // period (1 min)
                     0x05,                         // component.size()
                     0x03, 0x04, 0x05, 0x06, 0x0B, // component.type[0-1]
                    };
// const uint16_t MAX_ERRORS_IN_ROW = 3;
#elif defined( BUILD_S2_D1_60s_BASEMENT )
uint8_t data[100] = {0x00, 0x01, // Address
                     0x00, 0x08, // Channel
                     0x0F,       // Name.size()
                     0x00, 0x02, // Sensors.size() //  0x00, 0x03, // Sensors.size()
                     0x00, 0x01, // Devices.size()
                     0xD0, 0x93, 0xD1, 0x80, 0xD1, 0x83, 0xD0, 0xBF, 0xD0, 0xBF, 0xD0, 0xB0, 0x20, 0x31, 0x00, // Name
                     // Sensor[0]
                     0x00, 0x00, 0xEA, 0x60, // period (1 min)
                     0x03,                   // component.size()
                     0x03, 0x04, 0x0B,       // component.type[0-1]
                     // Sensor[1]
                     0x00, 0x00, 0xEA, 0x60,       // period (1 min)
                     0x05,                         // component.size()
                     0x03, 0x04, 0x05, 0x06, 0x0B, // component.type[0-1]
                     // Devices[0]
                    //  0x00, 0x36, 0xEE, 0x80,       // period (60 min)
                     0x00, 0x09, 0x27, 0xC0,       // period (10 min)
                     0x02,                         // component.size()
                     0x04, 0x04,                   // component.type[0-2]
                    };
// const uint16_t MAX_ERRORS_IN_ROW = 4;
#elif defined( BUILD_S4_D1_60s_BASEMENT )
uint8_t data[100] = {0x00, 0x01, // Address
                     0x00, 0x08, // Channel
                     0x0F,       // Name.size()
                     0x00, 0x04, // Sensors.size() //  0x00, 0x03, // Sensors.size()
                     0x00, 0x01, // Devices.size()
                     0xD0, 0x93, 0xD1, 0x80, 0xD1, 0x83, 0xD0, 0xBF, 0xD0, 0xBF, 0xD0, 0xB0, 0x20, 0x31, 0x00, // Name
                     // Sensor[0]
                     0x00, 0x00, 0xEA, 0x60, // period (1 min)
                     0x03,                   // component.size()
                     0x03, 0x04, 0x0B,       // component.type[0-1]
                     // Sensor[1]
                     0x00, 0x00, 0xEA, 0x60,       // period (1 min)
                     0x05,                         // component.size()
                     0x03, 0x04, 0x05, 0x06, 0x0B, // component.type[0-1]
                     // Sensor[2]
                     0x00, 0x00, 0xEA, 0x60,       // period (1 min)
                     0x03,                         // component.size()
                     0x03, 0x04, 0x06,             // component.type[0-1]
                     // Sensor[3]
                     0x00, 0x00, 0xEA, 0x60,       // period (1 min)
                     0x03,                         // component.size()
                     0x03, 0x04, 0x06,             // component.type[0-1]
                     // Devices[0]
                     0x00, 0x09, 0x27, 0xC0,       // period (10 min)
                     0x02,                         // component.size()
                     0x04, 0x04,                   // component.type[0-2]
                    };
// const uint16_t MAX_ERRORS_IN_ROW = 6;
#elif defined( BUILD_S5_D2_60s_BASEMENT )
uint8_t data[100] = {0x00, 0x01, // Address
                     0x00, 0x08, // Channel
                     0x0F,       // Name.size()
                     0x00, 0x05, // Sensors.size() //  0x00, 0x03, // Sensors.size()
                     0x00, 0x02, // Devices.size()
                     0xD0, 0x93, 0xD1, 0x80, 0xD1, 0x83, 0xD0, 0xBF, 0xD0, 0xBF, 0xD0, 0xB0, 0x20, 0x31, 0x00, // Name
                     // Sensor[0]
                     0x00, 0x00, 0xEA, 0x60,       // period (1 min)
                     0x03,                         // component.size()
                     0x03, 0x04, 0x0B,             // component.type[0-2]
                     // Sensor[1]
                     0x00, 0x00, 0xEA, 0x60,       // period (1 min)
                     0x05,                         // component.size()
                     0x03, 0x04, 0x05, 0x06, 0x0B, // component.type[0-4]
                     // Sensor[2]
                     0x00, 0x00, 0xEA, 0x60,       // period (1 min)
                     0x03,                         // component.size()
                     0x03, 0x04, 0x06,             // component.type[0-2]
                     // Sensor[3]
                     0x00, 0x00, 0xEA, 0x60,       // period (1 min)
                     0x03,                         // component.size()
                     0x03, 0x04, 0x06,             // component.type[0-2]
                     // Sensor[4]
                     0x00, 0x00, 0xEA, 0x60,       // period (1 min)
                     0x02,                         // component.size()
                     0x03, 0x04,                   // component.type[0-1]
                     // Devices[0]
                     0x00, 0x09, 0x27, 0xC0,       // period (10 min)
                     0x02,                         // component.size()
                     0x04, 0x04,                   // component.type[0-1] // lamp (2x)
                     // Devices[1]
                     0x00, 0x00, 0x27, 0x10,       // period (10 sec)
                     0x01,                         // component.size()
                     0x03,                         // component.type[0] // PUMP
                    };
// const uint16_t MAX_ERRORS_IN_ROW = 8;
#elif defined( BUILD_S0_D1_60s_BASEMENT_VENTILATION )
uint8_t data[100] = {0x00, 0x04, // Address
                     0x00, 0x08, // Channel
                     0x0F,       // Name.size()
                     0x00, 0x00, // Sensors.size() //  0x00, 0x03, // Sensors.size()
                     0x00, 0x01, // Devices.size()
                     0xD0, 0x93, 0xD1, 0x80, 0xD1, 0x83, 0xD0, 0xBF, 0xD0, 0xBF, 0xD0, 0xB0, 0x20, 0x31, 0x00, // Name
                     // Devices[0]
                     0xFF, 0x00, 0xEA, 0x60,       // period (inf min)
                     0x02,                         // component.size()
                     0x04, 0x04,                   // component.type[0-2]
                    };
// const uint16_t MAX_ERRORS_IN_ROW = 2;
#elif defined( BUILD_S0_D1_60s_SPECIALIZED )
uint8_t data[100] = {0x00, 0x05, // Address
                     0x00, 0x08, // Channel
                     0x0F,       // Name.size()
                     0x00, 0x00, // Sensors.size() //  0x00, 0x03, // Sensors.size()
                     0x00, 0x01, // Devices.size()
                     0xD0, 0x93, 0xD1, 0x80, 0xD1, 0x83, 0xD0, 0xBF, 0xD0, 0xBF, 0xD0, 0xB0, 0x20, 0x31, 0x00, // Name
                     // Devices[0]
                     0xFF, 0x00, 0xEA, 0x60,       // period (inf min)
                     0x01,                         // component.size()
                     0x04, 0x04,                   // component.type[0-2]
                    };
// const uint16_t MAX_ERRORS_IN_ROW = 2;
#elif defined( BUILD_S0_D2_60s_OBJECT1_PL1 )
uint8_t data[100] = {0x00, 0x06, // Address
                     0x00, 0x08, // Channel
                     0x0F,       // Name.size()
                     0x00, 0x00, // Sensors.size() //  0x00, 0x03, // Sensors.size()
                     0x00, 0x02, // Devices.size()
                     0xD0, 0x93, 0xD1, 0x80, 0xD1, 0x83, 0xD0, 0xBF, 0xD0, 0xBF, 0xD0, 0xB0, 0x20, 0x31, 0x00, // Name
                     // Devices[0]
                     0x00, 0x00, 0xEA, 0x60,       // period (1 min)
                     0x03,                         // component.size()
                     0x04, 0x04, 0x04,             // component.type[0-2]
                     // Devices[0]
                     0x00, 0x00, 0xEA, 0x60,       // period (1 min)
                     0x01,                         // component.size()
                     0x04,                         // component.type[0-2]
                    };
// const uint16_t MAX_ERRORS_IN_ROW = 3;
#elif defined( BUILD_S0_D3_OBJECT1_PL_PM )
uint8_t data[100] = {0x00, 0x06, // Address
                     0x00, 0x08, // Channel
                     0x0F,       // Name.size()
                     0x00, 0x00, // Sensors.size() //  0x00, 0x03, // Sensors.size()
                     0x00, 0x03, // Devices.size()
                     0xD0, 0x93, 0xD1, 0x80, 0xD1, 0x83, 0xD0, 0xBF, 0xD0, 0xBF, 0xD0, 0xB0, 0x20, 0x31, 0x00, // Name
                     // Devices[0]
                     0x00, 0x00, 0xEA, 0x60,       // period (1 min)
                     0x03,                         // component.size()
                     0x04, 0x04, 0x04,             // component.type[0-2] // PL PL PL
                     // Devices[1]
                     0x00, 0x00, 0xEA, 0x60,       // period (1 min)
                     0x01,                         // component.size()
                     0x04,                         // component.type[0] // PL
                     // Devices[2]
                     0x00, 0x00, 0x27, 0x10,       // period (10 sec)
                     0x01,                         // component.size()
                     0x03,                         // component.type[0] // PUMP
                    };
// const uint16_t MAX_ERRORS_IN_ROW = 4;
#elif defined( BUILD_TESTING_CODE_409 )
uint8_t data[100] = {0x00, 0xEE, // Address
                     0x00, 0x08, // Channel
                     0x0F,       // Name.size()
                     0x00, 0x00, // Sensors.size() //  0x00, 0x03, // Sensors.size()
                     0x00, 0x01, // Devices.size()
                     0xD0, 0x93, 0xD1, 0x80, 0xD1, 0x83, 0xD0, 0xBF, 0xD0, 0xBF, 0xD0, 0xB0, 0x20, 0x31, 0x00, // Name
                     // Devices[0]
                     0xFF, 0x00, 0xEA, 0x60,       // period (inf min)
                     0x02,                         // component.size()
                     0x04, 0x04,                   // component.type[0-2]
                    };
// const uint16_t MAX_ERRORS_IN_ROW = 2;
#else  // BUILD_S0_D0
uint8_t data[100] = {0x00, 0x01, // Address
                     0x00, 0x04, // Channel
                     0x0F,       // Name.size()
                     0x00, 0x00, // Sensors.size() //  0x00, 0x03, // Sensors.size()
                     0x00, 0x00, // Devices.size()
                     0xD0, 0x93, 0xD1, 0x80, 0xD1, 0x83, 0xD0, 0xBF, 0xD0, 0xBF, 0xD0, 0xB0, 0x20, 0x31, 0x00, // Name
                    };
// const uint16_t MAX_ERRORS_IN_ROW = 1;
#endif