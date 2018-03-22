#include "serial.h"
#include "protocol.h"
#include <iostream>
#include <unistd.h>
#include <cstring>

using namespace std;

int main() {
    CSerial *ser = new CSerial("/dev/tty.usbserial-00000000", 115200);
    Protocol *proto = new Protocol(ser);
    char msg[11] = {'I', 'R', 'M', 0, 4, 0, 0, 0x12, 0, '\x01', 0};

    proto->append_crc8(msg, 7);
    printf("%hhu\n\n\n\n\n", msg[6]);
    proto->append_crc8(msg+7, 4);
    while (true) {
        //ser->write_bytes(msg, 11);
        header_t *hd;
        if ((hd = proto->get_header())) {
            cout << "header read complete" << endl;
            if (proto->get_body())
                cout << "data proccessed success" << endl;
        }
        else {
            ser->flush();
        }
        usleep(1e5);
    }
    return 0;
}
