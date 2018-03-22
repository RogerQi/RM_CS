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
}

Protocol::~Protocol() {}

char Protocol::get_crc8(char *data, uint16_t length, char crc8) {
    char index;

    while (length--) {
        index = crc8 ^ *data++;
        crc8 = crc8_table[(uint8_t)index]; 
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

header_t* Protocol::get_header() {
    uint16_t recv = _ser->read_bytes(_rxbuf, sizeof(header_t));
#ifdef DEBUG
    if (recv) {
        for (size_t i = 0; i < recv; i++)
            printf("%hhu ", _rxbuf[i]);
        cout << endl;
    }
#endif
    if (recv != sizeof(header_t) ||
            !check_crc8(_rxbuf, sizeof(header_t))) { 
        _ser->flush();
        return NULL;
    }
    recv_u *rec_data = (recv_u*)_rxbuf;

    if (strcmp(rec_data->header.irm, IRM)) {
        _ser->flush();
        return NULL;
    }
    return (header_t*)_rxbuf;
}

void Protocol::pack_data(char *data, uint16_t length) {
    header_t *hd = (header_t*)data;
    strcpy(hd->irm, IRM);
    hd->data_length = length;
    append_crc8(data, sizeof(header_t));
    data += sizeof(header_t);
    append_crc8(data, length);
}

bool Protocol::process_body(uint16_t length) {
    uint16_t recv = _ser->read_bytes(_rxbuf, length);
#ifdef DEBUG
    if (recv) {
        for (size_t i = 0; i < recv; i++)
            printf("%hhu ", _rxbuf[i]);
        cout << endl;
    }
#endif
    if (recv != length ||
            !check_crc8(_rxbuf, length)) {
        return false;
    }
    
    recv_u *rec_data = (recv_u*)_rxbuf;
    switch (rec_data->aim_request.command_id) {
    case AIM_REQUEST:
        if (rec_data->aim_request.mode == RUNE) {

        }
        else if (rec_data->aim_request.mode == AUTOAIM) {
            gimbal_control_t *gc = (gimbal_control_t*)(_txbuf + sizeof(header_t));
            gc->command_id = GIMBAL_CONTROL;
            gc->pitch_ref = 10;
            gc->yaw_ref = 10;
            pack_data(_txbuf, sizeof(gimbal_control_t));
        }
        else
            return false;
        break;
    default:
        return false;
    }
    return true; 
}
