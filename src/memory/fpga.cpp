#include <labnation.h>
#include <labnation/memory/memory.h>

namespace labnation {

/*
 * FPGA General I2C memory
 */

FpgaMemory::FpgaMemory(SmartScopeUsb *interface, uint8_t i2c_address, const char* name, uint32_t size, bool read_only)
  : Memory(name)
  , _interface(interface)
  , _i2c_address(i2c_address)
  , _read_only(read_only)
{

  if (i2c_address != (i2c_address & 0x7f))
      throw Exception("I2c Address too large to be an I2C address: 0x%02X", i2c_address);

  char reg_name[200];
  for(uint32_t i =0; i < size; i++) {
    sprintf(reg_name, "reg[%d]", i);
    registers[i] = new Register(this, i, name);

  }
}

#define CONVERT(addr) (uint)((addr & 0xff) + (_i2c_address << 8))

void FpgaMemory::Read(uint32_t address)
{
    uint8_t data[1];
    _interface->GetControllerRegister(SmartScopeUsb::FPGA, CONVERT(address), sizeof(data), data);
    registers[address]->Set(data[0]);
    registers[address]->dirty = false;
}

void FpgaMemory::Write(uint32_t address)
{
    if (_read_only) return;

    uint8_t data[1] = { (uint8_t)registers[address]->Get() };
    _interface->SetControllerRegister(SmartScopeUsb::FPGA, CONVERT(address), sizeof(data), data);
    registers[address]->dirty = false;
}

/*
 * FPGA Settings (RAM)
 */


FpgaSettings::FpgaSettings(SmartScopeUsb *interface, uint8_t i2c_address)
  : FpgaMemory(interface, i2c_address, "FPGA Settings", 0, false) {
  #define X(name, value) registers[value] = new Register(this, value, #name);
  LIST_REG
  #undef X

}

/*
 * FPGA ROM
 */


FpgaRom::FpgaRom(SmartScopeUsb *interface, uint8_t i2c_address)
  : FpgaMemory(interface, i2c_address, "FPGA ROM", 0, true) {
  #define X(name, value) registers[value] = new Register(this, value, #name);
  LIST_ROM
  #undef X

  char reg_name[200];
  for(uint32_t i = ROM_STROBES + 1; i < ROM_STROBES + STR_LAST / 8 + 1; i++) {
    sprintf(reg_name, "STROBES %d", i - (int)ROM_STROBES);
    registers[i] = new Register(this, i, reg_name);
  }
}

/*
 * FPGA Strobes
 */

FpgaStrobes::FpgaStrobes(FpgaSettings *write_memory, FpgaRom *read_memory)
  : Memory("FPGA Strobes")
  , _write_memory(write_memory)
  , _read_memory(read_memory) {
  #define X(name, value) registers[value] = new Register(this, value, #name);
  LIST_STR
  #undef X
}

void FpgaStrobes::Write(uint32_t address) {
  _write_memory[REG_STROBE_UPDATE].Write((address << 1) + (registers[address]->Get() ? 1 : 0));
  registers[address]->dirty = false;
}

#define STROBE_TO_ROM(strobe) (ROM_STROBES + (strobe / 8))

void FpgaStrobes::Read(uint32_t address) {
  uint32_t romAddress = STROBE_TO_ROM(address);
  _read_memory->Read(romAddress);
  registers[address]->Set( ((*_read_memory)[romAddress]->Get() >> (address % 8)) & 0x01);
  registers[address]->dirty = false;
}

/*
 * ADC
 */

Adc::Adc(FpgaSettings *fpga_settings, FpgaStrobes *fpga_strobes, FpgaRom *fpga_rom)
  : Memory("MAX19506")
  , _fpga_settings(fpga_settings)
  , _fpga_strobes(fpga_strobes)
  , _fpga_rom(fpga_rom) {
  #define X(name, value) registers[value] = new Register(this, value, #name);
  LIST_ADC
  #undef X
}

void Adc::Read(uint32_t address) {

    (*_fpga_settings)[REG_SPI_ADDRESS]->Write(address + 0x80); //for a read, MSB must be 1

    //next, trigger rising edge to initiate SPI comm
    (*_fpga_strobes)[STR_INIT_SPI_TRANSFER]->Write(0);
    (*_fpga_strobes)[STR_INIT_SPI_TRANSFER]->Write(1);

    //finally read acquired value
    uint32_t acquiredVal = (*_fpga_rom)[ROM_SPI_RECEIVED_VALUE]->Read()->Get();

    registers[address]->Set(acquiredVal);
    registers[address]->dirty = false;
}

void Adc::Write(uint32_t address) {
  //first send correct address to FPGA
  (*_fpga_settings)[REG_SPI_ADDRESS]->Write(address);

  //next, send the write value to FPGA
  uint32_t valToWrite = registers[address]->Get();
  (*_fpga_settings)[REG_SPI_WRITE_VALUE]->Write(valToWrite);

  //finally, trigger rising edge
  (*_fpga_strobes)[STR_INIT_SPI_TRANSFER]->Write(0);
  (*_fpga_strobes)[STR_INIT_SPI_TRANSFER]->Write(1);
  registers[address]->dirty = false;
}

}


