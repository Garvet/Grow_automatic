#include "LoRa.h"



// modes
#define MODE_SLEEP 0x00 // Спящий режим
#define MODE_STDBY 0x01 // Режим ожидания
#define MODE_FSTX 0x02  // Синтез частот TX
#define MODE_TX 0x03    // Передача пакета
#define MODE_FSRX 0x04  // Синтез частот RX
#define MODE_RX_CONTINUOUS 0x05 // Непрерывное получение
#define MODE_RX_SINGLE 0x06 // Единичное получение
#define MODE_CAD 0x07 // Обнаружение активности канала

// PaDac
#define RF_PADAC_20DBM_ON  0x07
#define RF_PADAC_20DBM_OFF 0x04

// DetectionThreshold
#define DT_SF6 0x0C
#define DT_SF7_12 0x0A
// DetectionOptimize
#define DO_SF6 0x05
#define DO_SF7_12 0x03
// Frequency
#define REG_FRF_MSB 0x06
#define REG_FRF_MID 0x07
#define REG_FRF_LSB 0x08


LoRa::LoRa(/* args */) {
    _spi = nullptr;
    _setting = nullptr;
}
LoRa::LoRa(uint8_t pin_reset, uint8_t spi_bus, uint8_t spi_nss, uint8_t pin_dio0, uint8_t pin_dio1, uint8_t pin_dio3) {
    init(pin_reset, spi_bus, spi_nss, pin_dio0, pin_dio1, pin_dio3);
}
LoRa::~LoRa() {
    if(_spi != nullptr)
        delete _spi;
    if(_setting != nullptr)
        delete _setting;
}
// - ----- - --------------------------------------------------------------------------------------------------------------------------------------------------------
bool LoRa::init(uint8_t pin_reset, uint8_t spi_bus, uint8_t spi_nss, uint8_t pin_dio0, uint8_t pin_dio1, uint8_t pin_dio3) {
    if((spi_bus != HSPI) && (spi_bus != VSPI) && (spi_bus != HELTEC_LORA)) {
        _init = false;
        return true;
    }
    if(spi_bus == HELTEC_LORA)
        _spi = new SPIClass(VSPI);
    else
        _spi = new SPIClass(spi_bus);
    _setting = new SPISettings(2E6, MSBFIRST, SPI_MODE0);
    // _setting = new SPISettings(8E6, MSBFIRST, SPI_MODE0);
    uint8_t sclk, miso, mosi;
    if(spi_bus == VSPI) {
        // sclk = 5; miso = 19; mosi = 27; // SS = 18;
        sclk = 18; miso = 19; mosi = 23; // SS = 5;
        if(spi_nss == 0)
            spi_nss = 5;
    }
    else if(spi_bus == HSPI) {
        sclk = 14; miso = 12; mosi = 13; // SS = 15;
        if(spi_nss == 0)
            spi_nss = 15;
    }
    else {
        sclk = 5; miso = 19; mosi = 27; spi_nss = 18;
    }
    _spi->begin(sclk, miso, mosi, spi_nss);
    _reg_field.init(_spi, _setting, spi_nss);
    _nss = spi_nss;
    _reset = pin_reset;
    _dio0 = pin_dio0;
    _dio1 = pin_dio1;
    _dio3 = pin_dio3;
	_FifoTxBaseAddr = 0;

    if (_dio0 != 0)
        pinMode(_dio0, INPUT);
    if (_dio1 != 0)
        pinMode(_dio1, INPUT);
    if (_dio3 != 0)
        pinMode(_dio3, INPUT);
        
    _init = true;
    return false;
}


uint8_t LoRa::field_set(Address_field field, uint32_t value, bool write) {
    uint8_t result;
    result = _reg_field.set_field_value(field, value);
    if(write) {
        result = _reg_field.register_write(field, true, true);
    }
    return result;
}
uint8_t LoRa::field_set(Address_field* fields, uint32_t* values, uint8_t amt, bool write) {
    uint8_t result;
    result = _reg_field.set_field_value(fields, values, amt);
    if(write) {
        result = _reg_field.register_write(fields, amt, true, true);
    }
    return result;
}

uint8_t LoRa::field_get(Address_field field, uint32_t* value, bool read) {
    return _reg_field.get_field_value(field, value, read);
}
uint8_t LoRa::field_get(Address_field fields[], uint32_t* values, uint8_t amt, bool read) {
    return _reg_field.get_field_value(fields, values, amt, read);
}


uint8_t LoRa::begin(ulong frequency, bool paboost, uint8_t signal_power, uint8_t SF, ulong SBW, uint8_t sync_word) {
    if(!_init) return 1;
    uint32_t result = 0;
    uint8_t amt;
    // настройка выходов
    pinMode(_reset, OUTPUT);
    digitalWrite(_reset, HIGH);

    // запуск модуля
    digitalWrite(_reset, LOW);
    delay(20);
    digitalWrite(_reset, HIGH);
    delay(50);
    
    // проверка версии LoRa-модуля
    amt = field_get(Version, &result);
    if ((result != 0x12) || (amt != 1)) return 2;
    // переход в режим сна/настройки
    if(mode_sleep() != 1) return 3;
    // установка частоты работы модуля
    if(set_frequency(frequency) != 3) return 4;
    // установка адресов памяти TX и RX
    if(field_set(FifoRxBaseAddr, 0x00) != 1) return 5;
    if(field_set(FifoTxBaseAddr, _FifoTxBaseAddr) != 1) return 6;
    // настройка LNA
    if(field_set(LnaBoostHf, 0x03) != 1) return 7;
    // установка автоматического AGC
    if(field_set(LowDataRateOptimize, 0) != 1) return 8;
    if(field_set(AgcAutoOn, 1) != 1) return 9;
    // установкасилы сигнала на 14 дБ
    if(set_TX_power(signal_power, paboost) != 2) return 10;
    // установка силы коэффициента распространения SF
    if(set_spreading_factor(SF) != 3) return 11;
    // установка пропускной способности
    if(set_signal_bandwidth(SBW) != 1) return 12;
    // установка кодового слова 0x4A,  0x34 - LoRaWan
    if(field_set(SyncWord, sync_word) != 1) return 13;
    // включение проверки ошибки пакета
    if(crc_enable() != 1) return 14;
    // переход в режим ожидания
    if(mode_STDBY() != 1) return 15;
    return 0;
}

void LoRa::end() {
    // переход в режим сна
    mode_sleep();
    // остановка SPI, если создавалась классом
    if (_spi != nullptr)
        _spi->end();
}


uint8_t LoRa::set_mode(uint8_t mode) {
    Address_field fields[3] = {LongRangeMode, LowFrequencyModeOn, Mode};
    uint32_t values[3] = {1, 0, mode}; 
    return field_set(fields, values, 3);
}
// Режим сна/настройки
uint8_t LoRa::mode_sleep() {
    return set_mode(MODE_SLEEP);
}
// Режим ожидания
uint8_t LoRa::mode_STDBY() {
    return set_mode(MODE_STDBY);
}
// Режим отправки
uint8_t LoRa::mode_TX(bool set_dio) {    
    if (set_dio & (_dio0 != 0))
        field_set(Dio0Mapping, 1);
    return set_mode(MODE_TX);
}
// Подготовка к отправке (настройка частот)
uint8_t LoRa::mode_FSTX() {
    return set_mode(MODE_FSTX);
}
// Режим непрерывного приёма
uint8_t LoRa::mode_RX_continuous(bool set_dio) {
    // if (_dio0 != 0)
    //     field_set(Dio0Mapping, 0);
    // if (_dio1 != 0)
    //     field_set(Dio1Mapping, 0);
    if (set_dio & ((_dio0 != 0) || (_dio1 != 0))) {
        field_set(Dio0Mapping, 0, false);
        field_set(Dio1Mapping, 0);
    }
    return set_mode(MODE_RX_CONTINUOUS);
}
// Режим единичного приёма
uint8_t LoRa::mode_RX_single(bool set_dio) {
    if (set_dio & ((_dio0 != 0) || (_dio1 != 0))) {
        field_set(Dio0Mapping, 0, false);
        field_set(Dio1Mapping, 0);
    }
    return set_mode(MODE_RX_SINGLE);
}
// Подготовка к приёму (настройка частот)
uint8_t LoRa::mode_FSRX() {
    return set_mode(MODE_FSRX);
}
// Режим проверки сети
uint8_t LoRa::mode_CAD(bool set_dio) {
    // if (_dio0 != 0)
    //     field_set(Dio0Mapping, 2);
    // if (_dio1 != 0)
    //     field_set(Dio1Mapping, 2);
    if (set_dio & ((_dio0 != 0) || (_dio1 != 0))) {
        field_set(Dio0Mapping, 2, false);
        field_set(Dio1Mapping, 2);
    }
    return set_mode(MODE_CAD);
}


// Установка силы отправляемого пакета
uint8_t LoRa::set_TX_power(uint8_t power, bool paboost, uint8_t max_power) {
    Address_field fields[4] = {PaDac, PaSelect, MaxPower, OutputPower};
    uint32_t pa_dac, pa_select;
    int16_t power_adjustment, min_power_value, max_power_value;
    if (max_power < 0x01)
        max_power = 0x01;
    else if (max_power > 0x07)
        max_power = 0x07;
    _reg_field.register_read(fields, 4);
    // Изменение бита PABOOST
    if (paboost) {
        pa_select = 1;
        min_power_value = 2;
        max_power_value = 20;
        if (power > 17)
            power_adjustment = -5;
        else
            power_adjustment = -2;
    }
    else {
        pa_select = 0;
        min_power_value = -1;
        max_power_value = 14;
        power_adjustment = 1;
    }
    // Проверка выхода силы сигнала за диапазон
    if (power < min_power_value)
        power = min_power_value;
    if (power > max_power_value)
        power = max_power_value;
    // Корректировка параметра
    power += power_adjustment;
    // Настройка флага высокого сигнала
    if (power > 17)
        pa_dac = RF_PADAC_20DBM_ON;
    else
        pa_dac = RF_PADAC_20DBM_OFF;
    // Передача настроек
    uint32_t values[4] = {pa_dac, pa_select, max_power, power};
    return field_set(fields, values, 4);
}

// Установка частоты радиосигнала
uint8_t LoRa::set_frequency(ulong frequency) {
    _frequency = frequency;
    uint32_t frf = ((uint64_t)frequency << 19) / 32000000;
    return field_set(Frf, frf);
}

// Установка силы коэффициента распространения SF
uint8_t LoRa::set_spreading_factor(uint8_t SF) {
    uint32_t detection_optimize, detection_threshold;
    if (SF < 6)
        SF = 6;
    else if (SF > 12)
        SF = 12;
    if (SF == 6) {
        detection_optimize = DO_SF6;
        detection_threshold = DT_SF6;
    }
    else {
        detection_optimize = DO_SF7_12;
        detection_threshold = DT_SF7_12;
    }
    Address_field fields[3] = {DetectionOptimize, DetectionThreshold, SpreadingFactor};
    uint32_t values[3] = {detection_optimize, detection_threshold, SF};
    return field_set(fields, values, 3);
}

// Установка пропускной способности
uint8_t LoRa::set_signal_bandwidth(ulong sbw) {
    uint8_t bw;
    if (sbw <= 7.8E3)        bw = 0;
    else if (sbw <= 10.4E3)  bw = 1;
    else if (sbw <= 15.6E3)  bw = 2;
    else if (sbw <= 20.8E3)  bw = 3;
    else if (sbw <= 31.25E3) bw = 4;
    else if (sbw <= 41.7E3)  bw = 5;
    else if (sbw <= 62.5E3)  bw = 6;
    else if (sbw <= 125E3)   bw = 7;
    else if (sbw <= 250E3)   bw = 8;
    else                     bw = 9;
    return field_set(Bw, bw);
}

// Установка длины преамбулы
uint8_t LoRa::set_preamble_length(uint length) {
    return field_set(PreambleLength, length);
}

// Установка кодового слова
uint8_t LoRa::set_sync_word(uint8_t SW) {
    return field_set(SyncWord, SW);
}

// Включение CRC на LoRa-модуле
uint8_t LoRa::crc_enable() {
    return field_set(RxPayloadCrcOn, 1);
}
// Выключение CRC на LoRa-модуле
uint8_t LoRa::crc_disable() {
    return field_set(RxPayloadCrcOn, 0);
}


// Приём пакета
class LoRa_packet LoRa::receiver_packet(uint8_t count, ulong wait, bool rssi, bool snr) {
    class LoRa_packet send_packet;
    Address_field fields[3] = {RxTimeout, RxDone, PayloadCrcError};
    Address_field flags[3] = {RxDone, ValidHeader, PayloadCrcError};
    if(count <= 1) {
        if(count == 1)
            mode_RX_single();
        uint8_t rx_done, rx_timeout, crc_err;
        uint8_t amt;
        bool signal = false;
        ulong time, start_time, read_time;
        int pin_done, pin_timeout, pin_crc_err;
        uint32_t values[3] = {0, 0, 0};
        rx_done = rx_timeout = crc_err = 0;
        if(wait == 0)
            time = 0;
        else
            time = millis();
        for(start_time = time, read_time = time; (wait == 0) || (millis() - time < wait);) {
            // Считывание каждые 10 мс.
            if((wait == 0) || (millis() - read_time > 10U)) {
                // Если работают DIO выходы, то при HIGH хотя бы на одном из них, пускаем signal
                if ((_dio0 != 0) && (_dio1 != 0)) {
                    pin_done = digitalRead(_dio0);
                    pin_timeout = digitalRead(_dio1);
                    if (_dio3 != 0) {
                        pin_crc_err = digitalRead(_dio3);
                    }
                    else {
                        pin_crc_err = LOW;
                    }
                    if ((pin_done == HIGH) || (pin_timeout == HIGH) || (pin_crc_err == HIGH)) {
                        signal = true;
                    }
                }
                // Если неработают DIO выходы, был signal или превышено время ожидания
                if (((count == 0) && (wait == 0)) || (_dio0 == 0) || (_dio1 == 0) || (millis() - start_time > 2000) || signal) {
                    amt = field_get(fields, values, 3, true);
                    if(amt == 3) {
                        rx_timeout = values[0];
                        rx_done = values[1];
                        crc_err = values[2];
                    }
                }
                if(rx_done != 0) {
                    break;
                }
                if(rx_timeout != 0) {
                    _reg_field.clear_flags(RxTimeout);
                    mode_RX_single(false);
                    rx_done = rx_timeout = crc_err = 0;
                    signal = false;
                    if(wait != 0)
                        start_time = millis();
                }
                if(wait == 0) {
                    break;
                }
            }
        }
        if((rx_done > 0) /*&& (crc_err == 0)*/) {
            _reg_field.clear_flags(RxDone);
            _reg_field.clear_flags(RxTimeout);
            send_packet = read_packet_data(crc_err, rssi, snr);
        }
        else {
            field_get(fields, values, 3, true);
            _reg_field.clear_flags(flags, 3);
            // send_packet = LoRa_packet();
        }
        if(wait != 0)
            mode_sleep();
    }
    else {
        for(int i = 0; i < count; ++i) {
            
        }
    }
    return send_packet;
}

// Содержание последнего принятого пакета
class LoRa_packet LoRa::read_packet_data(bool crc_err, bool f_rssi, bool f_snr) {
    uint32_t length, adr;
    uint8_t rssi;//, *data;
    if (f_rssi)
        rssi = packet_rssi();
    else
        rssi = 0;

    field_get(FifoRxBytesNb, &length, true);
    field_get(FifoRxCurrentAddr, &adr, true);
    field_set(FifoAddrPtr, adr);
    
    class LoRa_packet send_packet(nullptr, 0, crc_err, rssi);
    uint32_t data32 = 0;
    for(int i = 0; i < length; ++i) {
        field_get(Fifo, &data32, true);
        send_packet.add_packet_data(data32);
        // send_packet.add_packet_data((uint8_t)(data32 & 0xFF));
        // data[i] = data32 & 0xFF;
    }
    // data = new uint8_t[length];
    // uint32_t data32 = 0;
    // for(int i = 0; i < length; ++i) {
    //     field_get(Fifo, &data32, true);
    //     data[i] = data32 & 0xFF;
    // }
    // class LoRa_packet send_packet(data, length, crc_err, rssi);
    // delete[] data;
    return send_packet;
}

// RSSI последнего принятого пакета
uint8_t LoRa::packet_rssi() {
    uint32_t rssi = 0;
    field_get(PacketRssi, &rssi, true);
    if (_frequency < 868E6)
        rssi -= 164;
    else
        rssi -= 157;
    return rssi;
}
// SNR последнего принятого пакета
float LoRa::packet_snr() {
    float snr = 0;
    field_get(PacketSnr, (uint32_t*)&snr, true);
    return (snr * 0.25);
}
// Отправка пакета
bool LoRa::sender_packet(uint8_t* packet, uint8_t len, bool wait) {
    packet_begin();
    if (packet_write(packet, len))
        return true;
    if (packet_end(wait))
        return true;
    return false;
}
bool LoRa::sender_packet(const std::vector<uint8_t>& packet, bool wait) {
    packet_begin();
    if (packet_write(packet))
        return true;
    if (packet_end(wait))
        return true;
    return false;
}
// Объявление пакета
bool LoRa::packet_begin() {
    mode_FSTX();
    field_set(FifoAddrPtr, _FifoTxBaseAddr);
    _packet_length = 0;
    return true;
}
// Отправка данных в пакет buffer, size=None? (len)
bool LoRa::packet_write(uint8_t* packet, uint8_t len) {
    if (len + _packet_length > 255)
        return true;
    _packet_length += len;
    for(int i = 0; i < len; ++i) 
        field_set(Fifo, packet[i]);
    field_set(PayloadLength, _packet_length);
    return false;
}
bool LoRa::packet_write(const std::vector<uint8_t>& packet) {
    if (packet.size() + _packet_length > 255)
        return true;
    _packet_length += packet.size();
    for(int i = 0; i < packet.size(); ++i) 
        field_set(Fifo, packet[i]);
    field_set(PayloadLength, _packet_length);
    return false;
}

// Отправка пакета
bool LoRa::packet_end(ulong wait, bool sleep) {
    // _reg_field.clear_flags(TxDone);
    mode_TX();
    bool result = false;
    if(wait > 0) {
        uint32_t tx_done = 0;
        bool signal = false;
        ulong time, start_time, read_time;
        int pin_done;
        tx_done = 0;
        for(time = millis(), start_time = time, read_time = time; millis() - time < wait;) {
            // Считывание каждые 10 мс.
            if(millis() - read_time > 10) {
                // Если работают DIO выходы, то при HIGH хотя бы на одном из них, пускаем signal
                if (_dio0 != 0) {
                    pin_done = digitalRead(_dio0);
                    if (pin_done == HIGH) {
                        signal = true;
                    }
                }
                // Если неработают DIO выходы, был signal или превышено время ожидания
                if ((_dio0 == 0) || (millis() - start_time > 2000) || signal) {
                    field_get(TxDone, &tx_done, true);
                }
                if(tx_done != 0) {
                    break;
                }
            }
        }
        if(sleep && (tx_done != 0)) {
            mode_sleep();
        }
    }
    return result;
}

