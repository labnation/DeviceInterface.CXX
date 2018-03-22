#include <labnation.h>
#include <labnation/smartscope.h>

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

}

SmartScope::~SmartScope() {
  for(auto m : memories)
    delete(m);
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
