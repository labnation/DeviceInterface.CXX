#ifndef _LABNATION_CHANNEL_H
#define _LABNATION_CHANNEL_H

#include <typeinfo>
#include <string>
#include <vector>

namespace labnation {

class Channel {

public:
  std::string name;
  uint32_t value;
  const std::type_info* datatype;
  static std::vector<Channel> list;

  Channel(std::string name, uint32_t value, const std::type_info * datatype)
  {
    this->name = name;
    this->value = value;
    this->datatype =  datatype;

    list.push_back(*this);
  }
};

class AnalogChannel: Channel {

public:
  static std::vector<AnalogChannel> list;

  AnalogChannel(std::string name, uint32_t value) : Channel(name, value, &typeid(float)) {
    list.push_back(*this);
  }

};

class DigitalChannel: Channel {

public:
  static std::vector<DigitalChannel> list;

  DigitalChannel(std::string name, uint32_t value) : Channel(name, value, &typeid(bool)) {
    DigitalChannel::list.push_back(*this);
  }

};

namespace channels {
  static AnalogChannel ChA = AnalogChannel("A", 0);
  static DigitalChannel Digi0 = DigitalChannel("0", 0);
  static DigitalChannel Digi1 = DigitalChannel("1", 1);
  static DigitalChannel Digi2 = DigitalChannel("2", 2);
  static DigitalChannel Digi3 = DigitalChannel("3", 3);
  static DigitalChannel Digi4 = DigitalChannel("4", 4);
  static DigitalChannel Digi5 = DigitalChannel("5", 5);
  static DigitalChannel Digi6 = DigitalChannel("6", 6);
  static DigitalChannel Digi7 = DigitalChannel("7", 7);
}

}
#endif // _LABNATION_CHANNEL_H
