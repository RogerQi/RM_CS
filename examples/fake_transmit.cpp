#include "serial.h"
#include "protocol.h"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <ctime>
#include <stdio.h>

using namespace std;

int main() {
#ifdef CPU_ONLY
    CSerial *ser = new CSerial("/dev/tty.usbserial-00000000", 115200);
#else
    CSerial *ser = new CSerial("/dev/ttyTHS2", 115200);
#endif
    Protocol *proto = new Protocol(ser);

    uint8_t fake_tx[MAX_BUFFER_LENGTH] = {0};
    data_u *fake_body = (data_u*)(fake_tx + sizeof(header_t));
    fake_body->gimbal_control.command_id = GIMBAL_CONTROL;
    fake_body->gimbal_control.pitch_ref = 10;
    fake_body->gimbal_control.yaw_ref = 10;
    
    proto->pack_data(fake_tx, sizeof(gimbal_control_t));

    while (true) {
        cout << "Press return to transmit";
        getchar();
        ser->write_bytes(fake_tx, sizeof(header_t) + sizeof(gimbal_control_t));
    }
}
