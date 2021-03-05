#include "build_data.h"

#if defined( BUILD_S2_D3 )
uint8_t data[100] = {0x00, 0x01, // Address
                     0x00, 0x04, // Channel
                     0x0F,       // Name.size()
                     0x00, 0x02, // Sensors.size()
                     0x00, 0x03, // Devices.size()
                     0xD0, 0x93, 0xD1, 0x80, 0xD1, 0x83, 0xD0, 0xBF, 0xD0, 0xBF, 0xD0, 0xB0, 0x20, 0x31, 0x00, // Name
                     // Sensor[0]
                     0x00, 0x09, 0xFB, 0xF1, // period
                     0x02,                   // component.size()
                     0x04, 0x03,             // component.type[0-1]
                     // Sensor[1]
                     0x00, 0x00, 0x00, 0x00, // period
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
#elif defined( BUILD_S2_D0 )
uint8_t data[100] = {0x00, 0x01, // Address
                     0x00, 0x04, // Channel
                     0x0F,       // Name.size()
                     0x00, 0x02, // Sensors.size() //  0x00, 0x03, // Sensors.size()
                     0x00, 0x00, // Devices.size()
                     0xD0, 0x93, 0xD1, 0x80, 0xD1, 0x83, 0xD0, 0xBF, 0xD0, 0xBF, 0xD0, 0xB0, 0x20, 0x31, 0x00, // Name
                     // Sensor[0]
                     0x00, 0x00, 0x20, 0x00, // period
                     0x01,                   // component.size()
                     0x06,                   // component.type[0-1]
                     // Sensor[1]
                     0x00, 0x00, 0x2F, 0x10, // period
                     0x01,                   // component.size()
                     0x06,                   // component.type[0-1]
                     };
#elif defined( BUILD_S1_D0_30s )
uint8_t data[100] = {0x00, 0x01, // Address
                     0x00, 0x04, // Channel
                     0x0F,       // Name.size()
                     0x00, 0x01, // Sensors.size() //  0x00, 0x03, // Sensors.size()
                     0x00, 0x00, // Devices.size()
                     0xD0, 0x93, 0xD1, 0x80, 0xD1, 0x83, 0xD0, 0xBF, 0xD0, 0xBF, 0xD0, 0xB0, 0x20, 0x31, 0x00, // Name
                     // Sensor[0]
                     0x00, 0x00, 0x75, 0x30, // period
                     0x03,                   // component.size()
                     0x03, 0x04, 0x06,                   // component.type[0-1]
                    };
#elif defined( BUILD_S1_D0_8s2 )
uint8_t data[100] = {0x00, 0x01, // Address
                     0x00, 0x04, // Channel
                     0x0F,       // Name.size()
                     0x00, 0x01, // Sensors.size() //  0x00, 0x03, // Sensors.size()
                     0x00, 0x00, // Devices.size()
                     0xD0, 0x93, 0xD1, 0x80, 0xD1, 0x83, 0xD0, 0xBF, 0xD0, 0xBF, 0xD0, 0xB0, 0x20, 0x31, 0x00, // Name
                     // Sensor[0]
                     0x00, 0x00, 0x20, 0x30, // period
                     0x03,                   // component.size()
                     0x03, 0x04, 0x06,                   // component.type[0-1]
                    };
#elif defined( BUILD_S6_D0_60s )
uint8_t data[100] = {0x00, 0x01, // Address
                     0x00, 0x04, // Channel
                     0x0F,       // Name.size()
                     0x00, 0x06, // Sensors.size() //  0x00, 0x03, // Sensors.size()
                     0x00, 0x00, // Devices.size()
                     0xD0, 0x93, 0xD1, 0x80, 0xD1, 0x83, 0xD0, 0xBF, 0xD0, 0xBF, 0xD0, 0xB0, 0x20, 0x31, 0x00, // Name
                     // Sensor[0]
                     0x00, 0x00, 0xEA, 0x60, // period
                     0x03,                   // component.size()
                     0x03, 0x04, 0x06,       // component.type[0-1]
                     // Sensor[1]
                     0x00, 0x00, 0xEA, 0x60, // period
                     0x03,                   // component.size()
                     0x03, 0x04, 0x06,       // component.type[0-1]
                     // Sensor[2]
                     0x00, 0x00, 0xEA, 0x60, // period
                     0x03,                   // component.size()
                     0x03, 0x04, 0x06,       // component.type[0-1]
                     // Sensor[3]
                     0x00, 0x00, 0xEA, 0x60, // period
                     0x03,                   // component.size()
                     0x03, 0x04, 0x06,       // component.type[0-1]
                     // Sensor[4]
                     0x00, 0x00, 0xEA, 0x60, // period
                     0x03,                   // component.size()
                     0x03, 0x04, 0x06,       // component.type[0-1]
                     // Sensor[5]
                     0x00, 0x00, 0xEA, 0x60, // period
                     0x03,                   // component.size()
                     0x03, 0x04, 0x06,       // component.type[0-1]
                    };
#elif defined( BUILD_S3_D0_60s )
uint8_t data[100] = {0x00, 0x01, // Address
                     0x00, 0x04, // Channel
                     0x0F,       // Name.size()
                     0x00, 0x03, // Sensors.size() //  0x00, 0x03, // Sensors.size()
                     0x00, 0x00, // Devices.size()
                     0xD0, 0x93, 0xD1, 0x80, 0xD1, 0x83, 0xD0, 0xBF, 0xD0, 0xBF, 0xD0, 0xB0, 0x20, 0x31, 0x00, // Name
                     // Sensor[0]
                     0x00, 0x00, 0xEA, 0x60, // period
                     0x03,                   // component.size()
                     0x03, 0x04, 0x06,       // component.type[0-1]
                     // Sensor[1]
                     0x00, 0x00, 0xEA, 0x60, // period
                     0x03,                   // component.size()
                     0x03, 0x04, 0x06,       // component.type[0-1]
                     // Sensor[2]
                     0x00, 0x00, 0xEA, 0x60, // period
                     0x03,                   // component.size()
                     0x03, 0x04, 0x06,       // component.type[0-1]
                    };
#elif defined( BUILD_S3_D0_10s )
uint8_t data[100] = {0x00, 0x01, // Address
                     0x00, 0x04, // Channel
                     0x0F,       // Name.size()
                     0x00, 0x03, // Sensors.size() //  0x00, 0x03, // Sensors.size()
                     0x00, 0x00, // Devices.size()
                     0xD0, 0x93, 0xD1, 0x80, 0xD1, 0x83, 0xD0, 0xBF, 0xD0, 0xBF, 0xD0, 0xB0, 0x20, 0x31, 0x00, // Name
                     // Sensor[0]
                     0x00, 0x00, 0x2A, 0x60, // period
                     0x03,                   // component.size()
                     0x03, 0x04, 0x06,       // component.type[0-1]
                     // Sensor[1]
                     0x00, 0x00, 0x2A, 0x60, // period
                     0x03,                   // component.size()
                     0x03, 0x04, 0x06,       // component.type[0-1]
                     // Sensor[2]
                     0x00, 0x00, 0x2A, 0x60, // period
                     0x03,                   // component.size()
                     0x03, 0x04, 0x06,       // component.type[0-1]
                    };
#else  // BUILD_S0_D0
uint8_t data[100] = {0x00, 0x01, // Address
                     0x00, 0x04, // Channel
                     0x0F,       // Name.size()
                     0x00, 0x00, // Sensors.size() //  0x00, 0x03, // Sensors.size()
                     0x00, 0x00, // Devices.size()
                     0xD0, 0x93, 0xD1, 0x80, 0xD1, 0x83, 0xD0, 0xBF, 0xD0, 0xBF, 0xD0, 0xB0, 0x20, 0x31, 0x00, // Name
                    };
#endif