#ifndef _LABNATION_CHANNEL_H
#define _LABNATION_CHANNEL_H

#include <typeinfo>

namespace labnation {

class Channel {

public:
  std::string name;
  uint32_t value;
  std::type_info* datatype;

  Channel(std::string name, uint32_t value, std::type_info* datatype)
  {
    this->name = name;
    this->value = value;
    this->datatype = datatype;
  }
};

class AnalogChannel: Channel {

};

}
#endif // _LABNATION_CHANNEL_H
