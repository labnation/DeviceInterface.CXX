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
  memory->Write(address);
}

void Register::Write(uint32_t value) {
  _value = value;
  Write();
}

Register* Register::Read() {
  memory->Read(address);
  return this;
}

uint32_t Register::Get() {
  return _value;
}

void Register::Set(uint32_t value) {
  _value = value;
  dirty = true;
}

}
