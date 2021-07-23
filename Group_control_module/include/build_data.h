#include <Arduino.h>
#include "additional_functionality.hpp"
#include <array>
#include <GCM_interface.h>

extern uint8_t data[100];

// add in project "src/hidden_build_data.cpp"
extern const std::array<char, AMT_BYTES_NETWORK_NAME> network_name;
extern const std::array<char, AMT_BYTES_NETWORK_PASSWORD> network_pswd;
extern const std::array<char, AMT_BYTES_NETWORK_ADDRESS> server_address;
extern const int server_port;