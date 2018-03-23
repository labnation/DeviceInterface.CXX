#include <labnation.h>
#include <labnation/hardware/smartscopeusb.h>
#include <labnation/hardware/smartscope_const.h>
#include <labnation/memory/memory.h>

namespace labnation {

PicMemory::PicMemory(SmartScopeUsb* interface)
  : Memory("Pic memory")
  , _interface(interface)
{
  #define X(name, value) registers[value] = new Register(this, value, #name);
  LIST_PIC
  #undef X
}

void PicMemory::Read(uint32_t address) {
  uint8_t data[1];
  _interface->GetControllerRegister(SmartScopeUsb::PIC, address, 1, data);
  registers[address + 1]->Set(data[0]);
  registers[address + 1]->dirty = false;
}

void PicMemory::Write(uint32_t address) {
  uint8_t data[1] = { (uint8_t) registers[address]->_value };
  _interface->SetControllerRegister(SmartScopeUsb::PIC, address, 1, data);
  registers[address]->dirty = false;
}

}
