#include "protocol.h"
#include <iostream>
#include <cstring>
#include <cstdlib>

using namespace std;

Protocol::Protocol(CSerial *ser) {
    this->_ser = ser;
}

Protocol::~Protocol() {}

uint8_t Protocol::get_crc8(char *data, uint16_t length, uint8_t crc8) {
    uint8_t index;

    while (length--) {
        index = crc8 ^ (*data++);
        crc8 = crc8_table[index]; 
    }
    return crc8;
}

bool Protocol::check_crc8(char *data, uint16_t length) {
    if (length <= 2 || !data) {
        cout << "NULL point encoutered or data length too short!" << endl; 
        return false;
    }
    return (get_crc8(data, length - 1, k_crc8) == data[length - 1]);
}

void Protocol::append_crc8(char *data, uint16_t length) {
    if (length <= 2 || !data) {
        cout << "NULL point encoutered or data length too short!" << endl; 
        return;
    }
    data[length - 1] = get_crc8(data, length - 1, k_crc8);
}

bool Protocol::get_header() {
    uint16_t recv = _ser->read_bytes(_buf, sizeof(header_t));
    if (recv != sizeof(header_t) ||
            !check_crc8(_buf, sizeof(header_t))) 
        return false;

    std::memcpy(&_header, _buf, sizeof(header_t));
    if (_header.irm != IRM)
        return false;

    return true;
}
