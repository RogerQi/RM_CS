#include "serial.h"

using namespace std;

int main() {
    auto ser = new CSerial("/dev/", 115200);
    return 0;
}
