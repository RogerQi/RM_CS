#include "serial.h"
#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

using namespace std;

CSerial::CSerial(string port, int baudrate, int data_bits, 
        int stop_bits, char parity) {
    _fd = open(port.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (_fd == -1)
        cout << "serial port open failed!" << endl;
    if (!tcgetattr(_fd, &_old_termios_options))
        cout << "get serial attributes error!" << endl;
    _termios_options = _old_termios_options;
    _termios_options.c_cflag |= (CLOCAL | CREAD);
    _termios_options.c_cflag &= (~CSIZE & ~CRTSCTS);
    if (!set_baudrate(baudrate))
        cout << "baudrate " << baudrate << " does not exist!" << endl;
    if (!set_data_bits(data_bits))
        cout << "failed to set " << data_bits << " data bits!" << endl;
    if (!set_stop_bits(stop_bits))
        cout << "failed to set " << stop_bits << " stop bits!" << endl;
    if (!set_parity(parity))
        cout << parity << " parity does not exist!" << endl;
}

bool CSerial::set_baudrate(int baudrate) {
    const int speed_arr[] = {B115200, B19200, B9600, B4800, B2400, B1200, B300};
    const int name_arr[] = {115200, 19200, 9600, 4800, 2400, 1200, 300};
    
    for (size_t i = 0; i < 7; i++)
        if (baudrate == name_arr[i]) {
            cfsetispeed(&_termios_options, speed_arr[i]);
            cfsetospeed(&_termios_options, speed_arr[i]);
            return true;
        }

    return false;
}

bool CSerial::set_data_bits(int data_bits) {
    switch (data_bits) {
        case 5: _termios_options.c_cflag |= CS5;
            return true;
        case 6: _termios_options.c_cflag |= CS6;
            return true;
        case 7: _termios_options.c_cflag |= CS7;
            return true;
        case 8: _termios_options.c_cflag |= CS8;
            return true;
        default: return false;
    }
}

bool CSerial::set_stop_bits(int stop_bits) {
    switch(stop_bits) {
        case 1: _termios_options.c_cflag &= ~CSTOPB;
            return true;
        case 2: _termios_options.c_cflag |= CSTOPB;
            return true;
        default: return false;
    }
}

bool CSerial::set_parity(char parity) {
    switch (parity) {
        case 'n':
        case 'N': _termios_options.c_cflag &= ~(PARENB | PARODD);
            _termios_options.c_iflag &= ~INPCK;
            return true;
        case 'o':
        case 'O': _termios_options.c_cflag |= (PARODD | PARENB);
            _termios_options.c_iflag |= INPCK;
            return true;
        case 'e':
        case 'E': _termios_options.c_cflag |= PARENB;
            _termios_options.c_cflag &= ~PARODD;
            _termios_options.c_iflag |= INPCK;
            return true;
        case 's':
        case 'S': _termios_options.c_cflag &= ~PARENB;
            _termios_options.c_cflag &= ~CSTOPB;
            return true;
        default: return false;
    }
}
