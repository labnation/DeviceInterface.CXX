#include <labnation.h>
#include <labnation/memory/register.h>
#include <labnation/memory/memory.h>

namespace labnation {

Register::Register() {
  throw Exception("Register empty constructor shouldn't be used");
}

Register::Register(void* m, uint32_t address, std::string name) {
  this->_memory = m;
  this->address = address;
  this->name = name;
  this->dirty = false;
}

void Register::Write() {
  debug("Writing register %s [0x%02X] (%d)", name.c_str(), address, dirty);
  //static_cast<Memory*>(memory)->write(address);
  memory->Write(address);
}

void Register::Write(uint32_t value) {
  _value = value;
  Write();
}

Register* Register::Read() {
  debug("Reading register %s [0x%02X] (%d)", name.c_str(), address, dirty);
  //static_cast<Memory*>(memory)->read(address);
  memory->Read(address);
  return this;
}

uint32_t Register::Get() {
  return _value;
}

void Register::Set(uint32_t value) {
  _value = value;
}

}
