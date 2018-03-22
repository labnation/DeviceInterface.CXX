#ifndef _LABNATION_REGISTER_H
#define _LABNATION_REGISTER_H

#include <cstdint>
#include <string>


namespace labnation {

class Register {

public:
  std::string name;
  uint32_t address;

  Register(void* memory, uint32_t address, const char * name);
  void Set(uint32_t value);
  uint32_t Get();
  Register* Read();
  void Write();
  void Write(uint32_t value);
  bool dirty;
  uint32_t _value;

  #define memory static_cast<Memory*>(_memory)

private:
  void* _memory;


};

}

#endif // _LABNATION_REGISTER_H
