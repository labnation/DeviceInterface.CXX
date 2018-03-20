#include <memory.h>

namespace labnation {

void Memory::write_range(uint32_t from, uint32_t until) {
  for(uint32_t i = from; i <= until; i++) {
    write(i);
  }
}

Register Memory::operator [](uint32_t address) {
  return registers[address];
}

}
