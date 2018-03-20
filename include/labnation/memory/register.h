#ifndef _LABNATION_REGISTER_H
#define _LABNATION_REGISTER_H

#include "memory.h"
using std;

namespace labnation {

class Register {

public:
  Memory memory;
  std::string name;
  uint32_t address;


  Register(Memory m, uint32_t address, string name);
  void set(uint32_t value);
  uint32_t get();
  Register read();
  void write(uint32_t value);

private:
  bool _dirty;
  uint32_t _value;

};

}

#endif // _LABNATION_REGISTER_H
