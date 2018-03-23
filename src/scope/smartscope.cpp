#include <labnation.h>
#include <labnation/scope/smartscope.h>
#include <utils.h>
#include <iostream>
#include <fstream>
#include <set>

namespace labnation {

SmartScope::SmartScope(SmartScopeUsb* interface)
  : SuspendViewportUpdates(false)
{
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
  if(FpgaFirmwareVersion() == FPGA_VERSION_UNFLASHED)
    throw Exception("Flashing FPGA failed");

  Configure();
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
    delete[] fw_data;
  } else {
    throw Exception("Unable to open file %s", fw_filename.c_str());
  }
}

void SmartScope::Configure() {
  EnableEssentials(true);

  //Enable scope controller
  SendOverviewBuffer(false);
  for(auto* ch : AnalogChannel::list)
  {
    debug("configuring channel %s (%p)", ch->name.c_str(), ch);
    VerticalRange(ch, -1.0, 1.0);
    Coupling(ch, DC);
  }

  DigitalOutput(0);
  ViewPort(0, 10e-3);

  //Part 2: perform actual writes
  (*fpga_strobes)[STR_GLOBAL_RESET]->Write(true);
  CommitSettings();
  _hardware_interface->FlushDataPipe();

  CalibrateAdc();

  AcquisitionDepth(ACQUISITION_DEPTH_DEFAULT);
  CommitSettings();

  //Reconfigure ADC with new timing value
  ConfigureAdc();

  for(auto r : fpga_settings->registers)
    debug("FPGA reg %s @ 0x%02X = 0x%02X", r.second->name.c_str(), r.second->address, r.second->Read()->Get());
}

void SmartScope::EnableEssentials(bool enable) {
  (*fpga_strobes)[STR_ENABLE_ADC]->Set(enable);
  (*fpga_strobes)[STR_ENABLE_RAM]->Set(enable);
  (*fpga_strobes)[STR_ENABLE_NEG]->Set(enable);
  (*fpga_strobes)[STR_SCOPE_ENABLE]->Set(enable);
}

void SmartScope::CalibrateAdc() {
  warn("ADC calibration not implemented");
  //Logger.Info("Found good ADC timing value [" + AdcTimingValue + "]");
}

void SmartScope::ConfigureAdc() {
  (*adc)[ADC_SOFT_RESET]->Write(90);
  (*adc)[ADC_POWER_MANAGEMENT]->Set(4);
  (*adc)[ADC_OUTPUT_PWR_MNGMNT]->Set(0);
  (*adc)[ADC_FORMAT_PATTERN]->Set(16);
  (*adc)[ADC_DATA_CLK_TIMING]->Set(24);
  (*adc)[ADC_CHA_TERMINATION]->Set(0);
  (*adc)[ADC_POWER_MANAGEMENT]->Set(3);
  (*adc)[ADC_OUTPUT_FORMAT]->Set(0x02);
}

void SmartScope::ViewPort(double offset, double timespan) {
  warn("Viewport setting not implemented");
}

void SmartScope::CommitSettings() {
  bool acquisitionUpdateRequired = false;
  bool viewUpdateRequired = false;

  pic->Commit();
  adc->Commit();
  std::set<uint32_t> fpga_regs_written = fpga_settings->Commit();
  std::set<uint32_t> fpga_strobes_written = fpga_strobes->Commit();

  for(auto reg : fpga_regs_written) {
    if(AcquisitionRegisters.find((REG)reg) != AcquisitionRegisters.end()) {
      acquisitionUpdateRequired = true;
      debug("Acqusition update required because of reg %s", (*fpga_settings)[reg]->name.c_str());
      break;
    }
  }
  for(auto reg : fpga_regs_written) {
    if(ViewRegisters.find((REG)reg) != ViewRegisters.end()) {
      viewUpdateRequired = true;
      debug("View update required because of reg %s", (*fpga_settings)[reg]->name.c_str());
      break;
    }
  }

  if (!acquisitionUpdateRequired) {
    for(auto strobe: fpga_strobes_written) {
      if(AcquisitionStrobes.find((STR)strobe) != AcquisitionStrobes.end()) {
        acquisitionUpdateRequired = true;
        debug("Acqusition update required because of strobe %s", (*fpga_strobes)[strobe]->name.c_str());
      }
    }

  }

  if (acquisitionUpdateRequired)
      ToggleAcquisitionUpdateStrobe();
  if (viewUpdateRequired && !SuspendViewportUpdates)
      ToggleViewUpdateStrobe();
}


void SmartScope::ToggleAcquisitionUpdateStrobe()
{
    (*fpga_strobes)[STR_SCOPE_UPDATE]->Write(true);
}
void SmartScope::ToggleViewUpdateStrobe()
{
    (*fpga_strobes)[STR_VIEW_UPDATE]->Write(true);
}

bool SmartScope::SendOverviewBuffer() {
  return (*fpga_strobes)[STR_VIEW_SEND_OVERVIEW]->Get() ? true : false;
}

void SmartScope::SendOverviewBuffer(bool v) {
  (*fpga_strobes)[STR_VIEW_SEND_OVERVIEW]->Set(v);
}


uint32_t SmartScope::FpgaFirmwareVersion() {
  return
      ((*fpga_rom)[ROM_FW_GIT0]->Read()->Get())      +
      ((*fpga_rom)[ROM_FW_GIT1]->Read()->Get() << 8) +
      ((*fpga_rom)[ROM_FW_GIT2]->Read()->Get() << 16) +
      ((*fpga_rom)[ROM_FW_GIT3]->Read()->Get() << 24);
}

/*
 * Acquisition
 */

uint32_t SmartScope::AcquisitionDepthUserMaximum()
{
    return _acquisitionDepthUserMaximum;
}
void SmartScope::AcquisitionDepthUserMaximum(uint32_t value)
{
  if (value > ACQUISITION_DEPTH_MAX)
      _acquisitionDepthUserMaximum = ACQUISITION_DEPTH_MAX;
  else if (value < ACQUISITION_DEPTH_MIN)
      _acquisitionDepthUserMaximum = ACQUISITION_DEPTH_MIN;
  else
      _acquisitionDepthUserMaximum = value;

  VIEW_DECIMATION_MAX = (int)log2(_acquisitionDepthUserMaximum / OVERVIEW_BUFFER_SIZE);
}

uint32_t SmartScope::AcquisitionDepth() {
  return (uint32_t)(OVERVIEW_BUFFER_SIZE * pow(2, (*fpga_settings)[REG_ACQUISITION_DEPTH]->Get()));
}
void SmartScope::AcquisitionDepth(uint32_t depth) {
  if (depth > AcquisitionDepthUserMaximum())
      depth = AcquisitionDepthUserMaximum();
  double multiple = ceil((double)depth/ OVERVIEW_BUFFER_SIZE);
  double power = log2(multiple);
  (*fpga_settings)[REG_ACQUISITION_DEPTH]->Set((uint32_t)power);

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

void SmartScope::VerticalRange(AnalogChannel* channel, float minimum, float maximum)
{
  warn("Set vertical range not implemented");
}

Coupling SmartScope::Coupling(AnalogChannel *channel) {
  uint32_t strobe = channel == &Channels::ChA ? STR_CHA_DCCOUPLING : STR_CHB_DCCOUPLING;
  return (*fpga_strobes)[strobe]->Get() ? DC : AC;
}

void SmartScope::Coupling(AnalogChannel* ch, labnation::Coupling coupling)
{
  uint32_t strobe = STR_CHA_DCCOUPLING;
  debug("Channel in = %p", ch);
  debug("Channel A  = %p", &Channels::ChA);
  debug("Channel B  = %p", &Channels::ChB);
  //debug("A == ch? %d", );
  //debug("B == ch? %d", *ch == channels::ChB);
  //if(ch == &channels::ChB)
  //if(*ch == channels::ChA)
//    strobe = STR_CHB_DCCOUPLING;

  debug("Set DC coupling for channel %s to %d", ch->name.c_str(), coupling == DC);
  (*fpga_strobes)[strobe]->Set(coupling == DC);
}

/*
 * AUX port
 */
uint8_t SmartScope::DigitalOutput() {
  return (uint8_t)(*fpga_settings)[REG_DIGITAL_OUT]->Get();
}

void SmartScope::DigitalOutput(uint8_t value) {
  (*fpga_settings)[REG_DIGITAL_OUT]->Write(value);
}

void SmartScope::DigitalOutputVoltage(float voltage) {
  (*fpga_strobes)[STR_DOUT_3V_5V]->Write(voltage < 4.0);
}


}
