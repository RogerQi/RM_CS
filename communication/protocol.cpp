#include "protocol.h"
#include <iostream>
#include <iomanip>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

using namespace std;

Protocol::Protocol(CSerial *ser) {
    this->_ser = ser;

    _rx_header = (header_t*)_rxbuf;
    _rx_body = (data_u*)(_rxbuf + sizeof(header_t));
    _rx_body->idle_msg.command_id = IDLE_MSG;

    _tx_header = (header_t*)_txbuf;
    _tx_header->sof = TX2_SOF;
    _tx_body = (data_u*)(_txbuf + sizeof(header_t));
}

Protocol::~Protocol() {}

uint8_t Protocol::get_crc8(void *ptr, uint16_t length, uint8_t crc8) {
    uint8_t index;
    uint8_t *data = (uint8_t*)ptr;

    while (length--) {
        index = crc8 ^ *data++;
        crc8 = crc8_table[index];
    }
    return crc8;
}

uint16_t Protocol::get_crc16(void *ptr, uint16_t length, uint16_t crc16) {
    uint8_t index;
    uint8_t *data = (uint8_t*)ptr;

    while (length--) {
        index = (crc16 ^ (uint16_t)*data++) & 0x00ff; 
        crc16 = (crc16 >> 8) ^ crc16_table[index];
    }
    return crc16;
}

bool Protocol::check_crc8(void *ptr, uint16_t length) {
    uint8_t *data = (uint8_t*)ptr;

    if (length <= 2 || !data) {
        cout << "NULL point encoutered or data length too short!" << endl;
        return false;
    }
    return (get_crc8(ptr, length - 1, k_crc8) == data[length - 1]);
}

bool Protocol::check_crc16(void *ptr, uint16_t length) {
    uint8_t *data = (uint8_t*)ptr;
    uint16_t crc16 = *(uint16_t*)(data + length - LEN_CRC16);

    if (length <= 2 || !data) {
        cout << "NULL point encoutered or data length too short!" << endl;
        return false;
    }
    return (get_crc16(ptr, length - 2, k_crc16) == crc16);
}

void Protocol::append_crc8(void *ptr, uint16_t length) {
    uint8_t *data = (uint8_t*)ptr;

    if (length <= 2 || !data) {
        cout << "NULL point encoutered or data length too short!" << endl;
        return;
    }
    data[length - 1] = get_crc8(ptr, length - 1, k_crc8);
}

void Protocol::append_crc16(void *ptr, uint16_t length) {
    uint8_t *data = (uint8_t*)ptr;
    uint16_t *crc16 = (uint16_t*)(data + length - LEN_CRC16);

    if (length <= 2 || !data) {
        cout << "NULL point encoutered or data length too short!" << endl;
        return;
    }
    *crc16 = get_crc16(ptr, length - 2, k_crc16);
}

header_t* Protocol::get_header() {
    if (_ser->bytes_available() < sizeof(header_t))
        return NULL;
    uint16_t recv = _ser->read_bytes(_rx_header, sizeof(header_t));
#ifdef DEBUG
    if (recv) {
        cout << "head received:" << endl;
        for (size_t i = 0; i < recv; i++)
            printf("%hhu ", _rxbuf[i]);
        cout << endl;
    }
    else {
        cout << "no header received" << endl;
    }
#endif
    if(_rx_header->sof != TX2_SOF ||
            !check_crc8(_rx_header, sizeof(header_t))) {
        _ser->flush();
        return NULL;
    }

    return _rx_header;
}

void Protocol::pack_data(uint8_t *data, uint16_t length) {
    header_t *tx_hd = (header_t*)data;
    tx_hd->sof = TX2_SOF;
    tx_hd->seq = 0x00;
    tx_hd->data_length = length - LEN_CRC16 - LEN_CMD;
    append_crc8(data, sizeof(header_t));
    append_crc16(data, sizeof(header_t) + length);
}

void Protocol::pack_data(uint16_t length) {
    pack_data(_txbuf, length);
}

data_u* Protocol::get_body() {
    uint16_t body_length = _rx_header->data_length + LEN_CMD + LEN_CRC16;
    while (_ser->bytes_available() < body_length) {}
    uint16_t recv = _ser->read_bytes(_rx_body, body_length);
#ifdef DEBUG
    if (recv) {
        cout << "body received:" << endl;
        for (size_t i = 0; i < recv; i++)
            printf("%hhu ", _rxbuf[i + sizeof(header_t)]);
        cout << endl;
    }
    else {
        cout << "no body received" << endl;
    }
#endif
    if (!check_crc16(_rxbuf, sizeof(header_t) + body_length)) {
        _ser->flush();
        return NULL;
    }

    return (data_u*)(_rxbuf + sizeof(header_t));
}

void Protocol::process_body() {
    switch (_rx_body->aim_request.command_id) {
        case IDLE_MSG:
            break;
        case AIM_REQUEST:
            if (_rx_body->aim_request.mode == RUNE) {
                // TODO: not implemented
            }
            else if (_rx_body->aim_request.mode == AUTOAIM) {
                _tx_body->gimbal_control.command_id = GIMBAL_CONTROL;
                // TODO: change those hard coded value to values read from perception libs
                _tx_body->gimbal_control.pitch_ref = 9;
                _tx_body->gimbal_control.yaw_ref = 10;
                pack_data(sizeof(gimbal_control_t));
            }
            else
                cout << "Body data meaningless!" << endl;
            break;
        case FOUR_INT16:
            _tx_body->idle_msg.command_id = IDLE_MSG;
            cout << _rx_body->custom_int16s.x << endl;
            break;
        default:
            cout << "Message Not Implemented Yet!" << endl;
    }
}

void Protocol::transmit() {
    if (_rx_body->aim_request.command_id != IDLE_MSG) {
        uint16_t data_length = sizeof(header_t) + _tx_header->data_length + LEN_CRC16 + LEN_CMD;
        _ser->write_bytes(_txbuf, data_length);
#ifdef DEBUG
        cout << "data transmitted:" << endl;
        for (size_t i = 0; i < data_length; i++)
            printf("%hhu ", _txbuf[i]);
        printf("\n");
#endif
    }
}

void Protocol::main_process() {
    while (1) {
        if (this->get_header() && this->get_body()) {
            this->process_body();
            this->transmit();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(PROTOCOL_SLEEP_TIME));
    }
}

void Protocol::run() {
    std::thread t(&Protocol::main_process, this);
    t.detach();
}
