#include "serial.h"
#include <iostream>
#include <time.h>
#include <unistd.h>

using namespace std;

int main() {
    CSerial *ser = new CSerial("/dev/tty.usbserial-00000000", 115200);
    int w_len, r_len;
    char buf[30];
    while (true) {
        w_len = ser->write_bytes("hello\n", 6);
        cout << w_len << " bytes sent" << endl;
        if (ser->bytes_available()) {
            r_len = ser->read_bytes(buf, sizeof(buf)*sizeof(char));
            buf[r_len] = 0;
            cout << r_len << " bytes read:\n" << buf;
        }
        else {
            cout << "no bytes available\n";
        }
        usleep(1e5);
    }
    return 0;
}
