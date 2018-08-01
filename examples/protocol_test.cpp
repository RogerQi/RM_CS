#include "serial.h"
#include "protocol.h"
#include <iostream>
#include <unistd.h>
#include <cstring>

using namespace std;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cerr << "require exactly one argument for serial port" << endl;
        return 0;
    }
    CSerial *ser = new CSerial(argv[1], 115200);
    Protocol *proto = new Protocol(ser);

    uint8_t fake_tx[MAX_BUFFER_LENGTH] = {0};
    data_u *fake_body = (data_u*)(fake_tx+sizeof(header_t));
    fake_body->aim_request.command_id = AIM_REQUEST;
    fake_body->aim_request.mode = AUTOAIM;

    proto->pack_data(fake_tx, sizeof(aim_request_t));

    proto->run();
    while(1);

    return 0;
}
