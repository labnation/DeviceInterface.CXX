#include <memory/register.h>

namespace labnation {

Register::Register(Memory m, uint32_t address, string name) {
  this->memory = m;
  this->address = address;
  this->name = name;
  this->dirty = false;
}

void Register::write() {
  _value = value;
  memory.write(address);
}

void Register::write(uint32_t value) {
  _value = value;
  write();
}

Register Register::read() {
  memory.read(address);
  return this;
}

uint32_t Register::get() {
  return _value;
}

void Register::set(uint32_t value) {
  _value = value;
}

}
