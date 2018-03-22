#ifndef _LABNATION_HARDWARE_H
#define _LABNATION_HARDWARE_H

#include <iostream>

namespace labnation {

class Hardware {
public:
    virtual std::string GetSerial() = 0;
};
}

#endif //_LABNATION_HARDWARE_H