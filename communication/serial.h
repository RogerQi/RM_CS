#ifndef _SERIAL_H_
#define _SERIAL_H_

#include <termios.h>
#include <string>

/**
 * generic serial communication class for C++
 */
class CSerial {
public:
    /** 
     * @brief constructor of a CSerial instance
     * @param port      directory to a serial port in the file system
     * @param baudrate  communication baudrate
     * @param data_bits data bits - [5, 6, 7, 8]
     * @param stop_bits stop bits - [1, 2]
     * @param parity    parity option - ['n', 'o', 'e', 's']
     *                      'n': none parity
     *                      'o': odd parity
     *                      'e': even parity
     *                      's': space parity
     */
    CSerial(std::string port, int baudrate, int data_bits, int stop_bits, char parity);

    /**
     * @brief destructor of a CSerial instance
     */
    ~CSerial();

    /**
     * @brief change baudrate
     * @param baudrate
     * @return true if baudrate set successfully, otherwise false
     */
    bool set_baudrate(int baudrate);

    /**
     * @brief change parity option
     * @param parity parity option - ['n', 'o', 'e', 's']
     *          'n': none parity
     *          'o': odd parity
     *          'e': even parity
     *          's': space parity
     * @return true if parity option set successfully, otherwise false
     */
    bool set_parity(char parity);

    /**
     * @brief change data bits
     * @param data_bits number of data bits - [5, 6, 7, 8]
     * @return true if data bits set sucessfully, otherwise false
     */
    bool set_data_bits(int data_bits);

    /**
     * @brief change stop bits
     * @param stop_bits number of stop bits - [1, 2]
     * @return true if stop bits set sucessfully, otherwise false
     */
    bool set_stop_bits(int stop_bits);

    int write(char* data, size_t length);

private:
    int _fd;
    struct termios _termios_options;
    struct termios _old_termios_options;
};

#endif
