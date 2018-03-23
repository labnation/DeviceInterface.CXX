#include <labnation.h>
#include <labnation/memory/register.h>
#include <labnation/memory/memory.h>

namespace labnation {

Register::Register(void* m, uint32_t address, const char* name)
  : name(name)
  , address(address)
  , dirty(false)
  , _memory(m)
{
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
  debug("Setting reg %s to 0x%02X", name.c_str(), value);
  _value = value;
  dirty = true;
}

}
