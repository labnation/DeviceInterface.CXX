#ifndef _LABNATION_HARDWARE_H
#define _LABNATION_HARDWARE_H

#include <iostream>

namespace labnation {

using namespace std;

class Hardware {
public:
    virtual string GetSerial();
};
}

#endif //_LABNATION_HARDWARE_H