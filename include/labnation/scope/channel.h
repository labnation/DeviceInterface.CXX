#ifndef _LABNATION_CHANNEL_H
#define _LABNATION_CHANNEL_H

#include <typeinfo>
#include <string>
#include <set>
#include <labnation.h>

namespace labnation {

class Channel {

  public:
    std::string name;
    uint32_t value;
    const std::type_info* datatype;
    static std::set<Channel*> list;

    Channel() {}

    Channel(std::string name, uint32_t value, const std::type_info * datatype)
    {
      this->name = name;
      this->value = value;
      this->datatype =  datatype;

      list.insert(this);
    }

};

class AnalogChannel: public Channel {
  public:
    static std::set<AnalogChannel*> list;

    AnalogChannel() {}
    AnalogChannel(std::string name, uint32_t value) : Channel(name, value, &typeid(float)) {
      list.insert(this);
      debug("Adding analogchannel %s @%p to list ", name.c_str(), this);
    }

};

class DigitalChannel: public Channel {

  public:
    static std::set<DigitalChannel*> list;

    DigitalChannel() {}
    DigitalChannel(std::string name, uint32_t value) : Channel(name, value, &typeid(bool)) {
      DigitalChannel::list.insert(this);
    }

};

class Channels {
public:
  static AnalogChannel ChA;
  static AnalogChannel ChB;
  static DigitalChannel Digi0;
  static DigitalChannel Digi1;
  static DigitalChannel Digi2;
  static DigitalChannel Digi3;
  static DigitalChannel Digi4;
  static DigitalChannel Digi5;
  static DigitalChannel Digi6;
  static DigitalChannel Digi7;
};

}
#endif // _LABNATION_CHANNEL_H
