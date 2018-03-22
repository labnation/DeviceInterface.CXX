#include <labnation.h>
#include <labnation/scope/smartscope.h>
#include <utils.h>
#include <iostream>
#include <fstream>

namespace labnation {

SmartScope::SmartScope(SmartScopeUsb* interface) {
  debug("Making new scope");
  _hardware_interface = interface;
  for(auto ch : AnalogChannel::list)
    YOffset(ch, 0);

  pic = new PicMemory(_hardware_interface);
  memories.push_back(pic);

  fpga_settings = new FpgaSettings(_hardware_interface, FPGA_I2C_ADDRESS_SETTINGS);
  memories.push_back(fpga_settings);

  fpga_rom = new FpgaRom(_hardware_interface, FPGA_I2C_ADDRESS_ROM);
  memories.push_back(fpga_rom);

  fpga_strobes = new FpgaStrobes(fpga_settings, fpga_rom);
  memories.push_back(fpga_strobes);

  adc = new Adc(fpga_settings, fpga_strobes, fpga_rom);
  memories.push_back(adc);

  serial = new Serial::Serial(_hardware_interface->GetSerial());
  FlashFpga();

}

SmartScope::~SmartScope() {
  for(auto m : memories)
    delete(m);
  delete(serial);
}

void SmartScope::FlashFpga() {
  std::string fw_filename = Base36::Encode(serial->model, 3);
  std::transform(fw_filename.begin(), fw_filename.end(), fw_filename.begin(), ::toupper);
  fw_filename = "SmartScope_" + fw_filename + ".bin";

  std::ifstream file(fw_filename, std::ios::in|std::ios::binary|std::ios::ate);
  if (file.is_open())
  {
    int fw_size = file.tellg();
    char* fw_data = new char[fw_size];
    file.seekg (0, std::ios::beg);
    file.read(fw_data, fw_size);
    file.close();
    debug("Read fw file %s, %d bytes", fw_filename.c_str(), fw_size);
    _hardware_interface->FlashFpga(fw_size, (uint8_t*)fw_data);
    debug("FPGA fw version 0x%08X", FpgaFirmwareVersion());
    delete[] fw_data;
  } else {
    throw Exception("Unable to open file %s", fw_filename.c_str());
  }
}

uint32_t SmartScope::FpgaFirmwareVersion() {
  return
      ((*fpga_rom)[ROM_FW_GIT0]->Read()->Get())      +
      ((*fpga_rom)[ROM_FW_GIT1]->Read()->Get() << 8) +
      ((*fpga_rom)[ROM_FW_GIT2]->Read()->Get() << 16) +
      ((*fpga_rom)[ROM_FW_GIT3]->Read()->Get() << 24);
}

/*
 *  Vertical
 */

float SmartScope::YOffset(AnalogChannel* channel) {
  return yOffset[channel];
}

void SmartScope::YOffset(AnalogChannel* channel, float offset) {
  yOffset[channel] = offset;
}

}
