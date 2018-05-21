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

    proto->run();

    while(1);

    return 0;
}
