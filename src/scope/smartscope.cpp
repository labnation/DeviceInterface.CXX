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
  for(auto ch : AnalogChannel::list) {
    YOffset(ch, 0);
    Scaler(ch, 10.0);
  }

  calibration = new Calibration(_hardware_interface);

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

  serial = new Serial(_hardware_interface->GetSerial());
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


double SmartScope::AcquisitionLengthMin() { return ACQUISITION_DEPTH_MIN * BASE_SAMPLE_PERIOD; }
double SmartScope::AcquisitionLengthMax() { return AcquisitionDepthUserMaximum() * BASE_SAMPLE_PERIOD * std::pow(2, INPUT_DECIMATION_MAX); }

bool SmartScope::PreferPartial() {
  return (*fpga_strobes)[STR_VIEW_SEND_PARTIAL]->Get();
}
void SmartScope::PreferPartial(bool val) {
  (*fpga_strobes)[STR_VIEW_SEND_PARTIAL]->Set(val);
}

double SmartScope::SamplePeriod() { return BASE_SAMPLE_PERIOD * std::pow(2, SubSampleRate()); }
double SmartScope::SamplePeriodCurrent() {
  //FIXME: check C# code for original purpose
  return SamplePeriod();
}

double SmartScope::AcquisitionLength() { return AcquisitionDepth() * SamplePeriod(); }
double SmartScope::AcquisitionLengthCurrent() {
  //FIXME: Check C# code for original purpose
  return AcquisitionLength();
}
void SmartScope::AcquisitionLength(double length) {
  unsigned long samples = (unsigned long)(length / BASE_SAMPLE_PERIOD);
  double ratio = (double)samples / OVERVIEW_BUFFER_SIZE;
  int acquisition_depth_pow = std::ceil(log2(ratio));

  if (acquisition_depth_pow < 0)
      acquisition_depth_pow = 0;

  if (samples > AcquisitionDepthUserMaximum())
      AcquisitionDepth(AcquisitionDepthUserMaximum());
  else
      AcquisitionDepth(OVERVIEW_BUFFER_SIZE * std::pow(2, acquisition_depth_pow));
  acquisition_depth_pow = std::log2(AcquisitionDepth() / OVERVIEW_BUFFER_SIZE);

  ratio = (double)samples / AcquisitionDepth();
  int input_decimation_pow = std::ceil(std::log2(ratio));
  if (input_decimation_pow < 0)
      input_decimation_pow = 0;
  if (input_decimation_pow > INPUT_DECIMATION_MAX)
      input_decimation_pow = INPUT_DECIMATION_MAX;
  SubSampleRate(input_decimation_pow);

  if (PreferPartial() && acquisition_depth_pow >= INPUT_DECIMATION_MIN_FOR_ROLLING_MODE && SubSampleRate() < INPUT_DECIMATION_MIN_FOR_ROLLING_MODE)
  {
      int adjustment = INPUT_DECIMATION_MIN_FOR_ROLLING_MODE - SubSampleRate();
      acquisition_depth_pow -= adjustment;
      AcquisitionDepth(OVERVIEW_BUFFER_SIZE * std::pow(2, acquisition_depth_pow));
      SubSampleRate(SubSampleRate() + adjustment);
  }
}

uint32_t SmartScope::SubSampleRate() { return (*fpga_settings)[REG_INPUT_DECIMATION]->Get(); }
void SmartScope::SubSampleRate(uint32_t rate) {
  (*fpga_settings)[REG_INPUT_DECIMATION]->Set(rate);
  //Re-apply holdoff setting to trigger recalc
  TriggerHoldOff(TriggerHoldOff());
}


uint SmartScope::AcquisitionDepth() { return OVERVIEW_BUFFER_SIZE * std::pow(2, (*fpga_settings)[REG_ACQUISITION_DEPTH]->Get()); }
void SmartScope::AcquisitionDepth(uint32_t depth) {
  if (depth > AcquisitionDepthUserMaximum())
    depth = AcquisitionDepthUserMaximum();
  double multiple = std::ceil((double)depth / OVERVIEW_BUFFER_SIZE);
  double power = std::log2(multiple);
  (*fpga_settings)[REG_ACQUISITION_DEPTH]->Set((int)power);
}

void SmartScope::ViewPort(double offset, double timespan) {
  /*                maxTimeSpan
   *            <---------------->
   *  .--------------------------,
   *  |        ||       ||       |
   *  `--------------------------`
   *  <--------><------->
   *    offset   timespan
   */
  double timespan_max= AcquisitionLengthCurrent() - offset;
  if (timespan > timespan_max)
  {
      if (timespan > AcquisitionLengthCurrent())
      {
          timespan = AcquisitionLengthCurrent();
          offset = 0;
      }
      else
      {
          //Limit offset so the timespan can fit
          offset = AcquisitionLengthCurrent() - timespan;
      }
  }

  //Decrease the number of samples till viewport sample period is larger than
  //or equal to the full sample rate
  uint samples = VIEWPORT_SAMPLES_MAX;

  int view_decimation = 0;
  while (true)
  {
      view_decimation = std::ceil(std::log2(timespan / samples / SamplePeriodCurrent()));
      if (view_decimation >= 0)
          break;
      samples /= 2;
  }

  if (view_decimation > VIEW_DECIMATION_MAX)
  {
      warn("Clipping view decimation! better decrease the sample rate!");
      view_decimation = VIEW_DECIMATION_MAX;
  }

  viewPortSamples = timespan / (SamplePeriodCurrent() * std::pow(2, view_decimation));
  int burstsLog2 = std::ceil(std::log2(std::ceil((double)viewPortSamples / SAMPLES_PER_BURST)));
  if (burstsLog2 < BURSTS_MIN)
      burstsLog2 = BURSTS_MIN;
  //Make sure these number of samples are actually available in the acquisition buffer

  (*fpga_settings)[REG_VIEW_DECIMATION]->Set(view_decimation);
  (*fpga_settings)[REG_VIEW_BURSTS]->Set(burstsLog2);

  int excess = ComputeViewportSamplesExcess(AcquisitionLengthCurrent(), SamplePeriodCurrent(), offset, (int)(SAMPLES_PER_BURST * std::pow(2, burstsLog2)), view_decimation);
  ViewPortOffset(offset, excess);
}

int SmartScope::ComputeViewportSamplesExcess(double acquisitionTimeSpan, double samplePeriod, double viewportOffset, int viewportSamples, int viewportDecimation)
{
    double viewportSamplePeriod = samplePeriod * std::pow(2, viewportDecimation);
    double endTime = viewportOffset + viewportSamples * viewportSamplePeriod;
    if (endTime > acquisitionTimeSpan)
        return (int)((endTime - acquisitionTimeSpan) / samplePeriod);
    else
        return 0;
}

void SmartScope::ViewPortOffset(double time, int samplesExcess)
{
    int samples = (int)(time / SamplePeriodCurrent()) - samplesExcess;
    if(samples < 0)
        samples = 0;
    (*fpga_settings)[REG_VIEW_OFFSET_B0]->Set((samples));
    (*fpga_settings)[REG_VIEW_OFFSET_B1]->Set((samples >> 8));
    (*fpga_settings)[REG_VIEW_OFFSET_B2]->Set((samples >> 16));

    (*fpga_settings)[REG_VIEW_EXCESS_B0]->Set((samplesExcess));
    (*fpga_settings)[REG_VIEW_EXCESS_B1]->Set((samplesExcess >> 8));
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
        break;
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

/*
 *  Vertical
 */

float SmartScope::YOffset(AnalogChannel* channel) {
  return yOffset[channel];
}

void SmartScope::YOffset(AnalogChannel* channel, float offset) {
  yOffset[channel] = offset;
}

const float VRANGE_BASE_MIN = -0.6345f; //Volt
const float VRANGE_BASE_MAX = 0.6769f; //Volt
const float VRANGE_BASE     = VRANGE_BASE_MAX - VRANGE_BASE_MIN;

void SmartScope::VerticalRange(AnalogChannel* channel, float minimum, float maximum)
{
  //Scale to hardware range
  float reqRange = (maximum - minimum) / Scaler(channel);

  //Walk through dividers/multipliers till requested range fits
  //this walk assumes it starts with the smallest range, and that range is only increasing
  int div_idx = 0;
  int mul_idx = 0;

  for (int i = 0; i < N_DIV * N_MUL; i++)
  {
      div_idx = i / N_MUL;
      mul_idx = N_MUL - 1 - (i % N_MUL);
      double curRange = VRANGE_BASE * calibration->computed_dividers[div_idx] / calibration->computed_multipliers[mul_idx];
      if (reqRange < curRange)
          break;
  }
  SetDivider(channel, VALID_DIVIDERS[div_idx]);
  SetMultiplier(channel, VALID_MULTIPLIERS[mul_idx]);
  channelSettings[channel] = calibration->gain_calibration[channel][VALID_DIVIDERS[div_idx]][VALID_MULTIPLIERS[mul_idx]];
  debug("Current channel setting for CH %s = [ %f, %f, %f ]", channel->name.c_str(),
    channelSettings[channel][0], channelSettings[channel][1], channelSettings[channel][2]);

  //Re-set yoffset
  YOffset(channel, YOffset(channel));
}

void SmartScope::SetDivider(AnalogChannel *channel, double divider) {
  for(int i = 0; i < N_DIV; i++) {
    if(VALID_DIVIDERS[i] == divider) {
      uint32_t bit_offset = (channel->value * 4);
      uint32_t mask = 0x3 << bit_offset;
      uint32_t div_mul = (*fpga_settings)[REG_DIVIDER_MULTIPLIER]->Get();
      div_mul &= ~mask;
      div_mul += i << bit_offset;
      (*fpga_settings)[REG_DIVIDER_MULTIPLIER]->Set(div_mul);
      return;
    }
  }
  throw Exception("Invalid divider %f", divider);
}

void SmartScope::SetMultiplier(AnalogChannel *channel, double multiplier) {
  for(int i = 0; i < N_MUL; i++) {
    if(VALID_MULTIPLIERS[i] == multiplier) {
      uint32_t bit_offset = (channel->value * 4);
      uint32_t mask = 0xC << bit_offset;
      uint32_t div_mul = (*fpga_settings)[REG_DIVIDER_MULTIPLIER]->Get();
      div_mul &= ~mask;
      div_mul += (i << 2) << bit_offset;
      (*fpga_settings)[REG_DIVIDER_MULTIPLIER]->Set(div_mul);
      return;
    }
  }
  throw Exception("Invalid multiplier %f", multiplier);
}

Coupling SmartScope::Coupling(AnalogChannel *channel) {
  uint32_t strobe = channel == &Channels::ChA ? STR_CHA_DCCOUPLING : STR_CHB_DCCOUPLING;
  return (*fpga_strobes)[strobe]->Get() ? DC : AC;
}

void SmartScope::Coupling(AnalogChannel* ch, labnation::Coupling coupling)
{
  uint32_t strobe = STR_CHA_DCCOUPLING;
  if(ch == &Channels::ChB)
    strobe = STR_CHB_DCCOUPLING;

  debug("Set DC coupling for channel %s to %d", ch->name.c_str(), coupling == DC);
  (*fpga_strobes)[strobe]->Set(coupling == DC);
}

float SmartScope::Scaler(AnalogChannel* channel) {
  return _scalers[channel];
}

void SmartScope::Scaler(AnalogChannel* channel, float scaler) {
  if(scaler == 0)
    throw Exception("Channel scaler cannot be 0");
  _scalers[channel] = scaler;
}

/*
 * Trigger
 */

TriggerValue SmartScope::TriggerValue() {
  return labnation::TriggerValue();
}

double SmartScope::TriggerHoldOff() {
  return _holdoff;
}
void SmartScope::TriggerHoldOff(double holdoff) {
  if (holdoff > AcquisitionLengthCurrent())
    _holdoff = AcquisitionLengthCurrent();
  else
    _holdoff = holdoff;

  uint32_t samples = _holdoff / SamplePeriod();
  samples += TriggerDelay(TriggerValue().mode, SubSampleRate());
  //FIXME: FPGA bug
  if (samples >= AcquisitionDepth())
  {
    samples = AcquisitionDepth() - 1;
  }
  //Logger.Debug(" Set trigger holdoff to " + time * 1e6 + "us or " + samples + " samples " );
  (*fpga_settings)[REG_TRIGGERHOLDOFF_B0]->Set((samples));
  (*fpga_settings)[REG_TRIGGERHOLDOFF_B1]->Set((samples >> 8));
  (*fpga_settings)[REG_TRIGGERHOLDOFF_B2]->Set((samples >> 16));
  (*fpga_settings)[REG_TRIGGERHOLDOFF_B3]->Set((samples >> 24));
}

int SmartScope::TriggerDelay(TriggerMode mode, int inputDecimation)
{
    if(mode == Digital)
        return (((int)4) >> inputDecimation);
    if (inputDecimation == 0)
        return 7;
    if (inputDecimation == 1)
        return 4;
    if (inputDecimation == 2)
        return 2;
    return 1;
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
