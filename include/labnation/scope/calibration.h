#ifndef _LABNATION_CALIBRATION_H
#define _LABNATION_CALIBRATION_H

#include <map>
#include "../hardware/smartscopeusb.h"
#include "../hardware/smartscope_const.h"
#include "channel.h"

namespace labnation {

class Calibration {

static const int N_GAIN_CALIB_COEFF = 3;
static const int N_FREQ_RESP_MAG = 16;
static const int N_FREQ_RESP_PHASES = 10;

struct __attribute__ ((__packed__)) RomContents
{
    uint32_t plug_count;
    float gain_calibration[N_GAIN_CALIB_COEFF * N_DIV * N_MUL * 2]; //calibrationSize * nDivider * nMultiplier * nChannel
    float magnitudes[N_FREQ_RESP_MAG * N_MUL * 2]; //N_FREQ_RESP_MAG * nMultiplier * nChannel
    ushort magnitudes_indices[N_FREQ_RESP_MAG * N_MUL * 2]; //N_FREQ_RESP_MAG * nMultiplier * nChannel
    float phases[N_FREQ_RESP_PHASES * N_MUL * 2]; //N_FREQ_RESP_PHASES * nMultiplier * nChannel
    ushort phases_indices[N_FREQ_RESP_PHASES * N_MUL * 2]; //N_FREQ_RESP_PHASES * nMultiplier * nChannel
    uint8_t adc_timing_value;
};

public:
  Calibration(SmartScopeUsb* hardware_interface);
  ~Calibration();
  void Download();
  std::map<AnalogChannel*, std::map<double, std::map<double, double[N_GAIN_CALIB_COEFF]>>> gain_calibration;
  double computed_dividers[N_DIV];
  double computed_multipliers[N_MUL];

private:
  SmartScopeUsb* _hardware_interface;
  RomContents _rom_contents;
  void ComputeDividersMultipliers();

};

}
#endif // _LABNATION_CALIBRATION_H
