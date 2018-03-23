#include "serial.h"
#include "protocol.h"
#include <iostream>
#include <unistd.h>
#include <cstring>

using namespace std;

int main() {
#ifdef CPU_ONLY
    CSerial *ser = new CSerial("/dev/tty.usbserial-00000000", 115200);
#else
    CSerial *ser = new CSerial("/dev/ttyTHS2", 115200);
#endif
    Protocol *proto = new Protocol(ser);
    char msg[11] = {'I', 'R', 'M', 0, 4, 0, 0, 0x12, 0, '\x01', 0};

    proto->append_crc8(msg, 7);
    proto->append_crc8(msg+7, 4);
    while (true) {
        ser->write_bytes(msg, 11);
        usleep(1e5);
        if (proto->get_header() && proto->get_body())
            proto->process_body();
        proto->transmit();
        usleep(1e5);
        ser->flush();
    }
    return 0;
}
