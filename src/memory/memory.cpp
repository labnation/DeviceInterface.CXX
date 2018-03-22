#include <labnation/memory/memory.h>
#include <labnation.h>

namespace labnation {

Memory::Memory(const char* name) : name(name) {
  debug("Initialising memory %s", name);
}

Memory::~Memory() {
  for(auto r : registers) {
    delete(r.second);
  }
}

void Memory::WriteRange(uint32_t from, uint32_t until) {
  for(uint32_t i = from; i <= until; i++) {
    Write(i);
  }
}

Register* Memory::operator [](uint32_t address) {
  return registers[address];
}

void Memory::Write(uint32_t address) {
  throw Exception("Write not implemented for generic memory");
}

void Memory::Read(uint32_t address) {
  throw Exception("Read not implemented for generic memory");
}

bool Memory::Commit() {
  std::map<uint, Register*>::iterator it =
      std::find_if(registers.begin(), registers.end(),
        [](const std::pair<uint, Register*> p) { return p.second->dirty; });
  return false;
}

}
