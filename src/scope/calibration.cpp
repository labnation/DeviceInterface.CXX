#include <labnation/scope/calibration.h>
#include <labnation/hardware/smartscopeusb.h>
#include <labnation/hardware/smartscope_regs.h>
#include <algorithm>

namespace labnation {

Calibration::Calibration(SmartScopeUsb *hardware_interface)
  : _hardware_interface(hardware_interface) {
  Download();
}

Calibration::~Calibration() {
  //Delete gaincalib
}

void Calibration::Download() {
  int max_read_length = 11;
  for (uint offset = 0; offset < sizeof(_rom_contents); )
  {
    int left = sizeof(_rom_contents) - offset;
    int len = std::min(max_read_length, left);
    _hardware_interface->GetControllerRegister(SmartScopeUsb::FLASH, offset, len, ((uint8_t*)(&_rom_contents)) + offset );
    offset += len;
  }

  debug("Plug count: %d", _rom_contents.plug_count);
  debug("ADC timing: %d", _rom_contents.adc_timing_value);

  int offset = 0;
  for(auto ch : AnalogChannel::list) {
    for(auto divider : VALID_DIVIDERS) {
      for(auto multiplier : VALID_MULTIPLIERS) {
        for (int i = 0; i < N_GAIN_CALIB_COEFF; i++) {
          gain_calibration[ch][divider][multiplier][i] = (double)_rom_contents.gain_calibration[offset + i];
        }
        offset += N_GAIN_CALIB_COEFF;
      }
    }
  }
  ComputeDividersMultipliers();
}

void Calibration::ComputeDividersMultipliers() {
  double* ref_cal = gain_calibration[&Channels::ChA][VALID_DIVIDERS[0]][VALID_MULTIPLIERS[0]];

  computed_multipliers[0] = 1;
  for(int i = 1; i < sizeof(VALID_MULTIPLIERS)/sizeof(VALID_MULTIPLIERS[0]); i++) {
    computed_multipliers[i] = ref_cal[0] / gain_calibration[&Channels::ChA][VALID_DIVIDERS[0]][VALID_MULTIPLIERS[i]][0];
  }

  computed_dividers[0] = 1;
  for(int i = 1; i < sizeof(VALID_DIVIDERS)/sizeof(VALID_MULTIPLIERS[0]); i++) {
    computed_dividers[i] = gain_calibration[&Channels::ChA][VALID_DIVIDERS[i]][VALID_MULTIPLIERS[0]][0] / ref_cal[0];
  }
}

}
