#include <labnation/memory/memory.h>
#include <labnation.h>

namespace labnation {

Memory::~Memory() {
  debug("Destroying memory %s", name.c_str());
  for(auto r : registers) {
    debug("Destroying register %s", r.second->name.c_str());
    delete(r.second);
  }
}

void Memory::WriteRange(uint from, uint until) {
  for(uint32_t i = from; i <= until; i++) {
    Write(i);
  }
}

Register* Memory::operator [](uint address) {
  return registers[address];
}

void Memory::Write(uint address) {
  throw Exception("Write not implemented for generic memory");
}

void Memory::Read(uint address) {
  throw Exception("Read not implemented for generic memory");
}

bool Memory::Commit() {
  std::map<uint, Register*>::iterator it =
      std::find_if(registers.begin(), registers.end(),
        [](const std::pair<uint, Register*> p) { return p.second->dirty; });
  return false;
}

}
