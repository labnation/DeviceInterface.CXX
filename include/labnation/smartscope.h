#ifndef LABNATION_SMARTSCOPE_H
#define LABNATION_SMARTSCOPE_H

#include "channel.h"
#include "memory/memory.h"
#include "memory/register.h"
#include "smartscopeusb.h"
#include "scope.h"
#include <cmath>
#include <vector>

namespace labnation {

class SmartScope {

public:
  SmartScopeUsb GetHardwareInterface();
  bool SuspendViewportUpdates;
  std::vector<Memory*> memories;

  PicMemory* pic_memory;
  /*

  public Memories.ByteMemoryEnum<REG> FpgaSettingsMemory { get; private set; }
  public Memories.ScopeFpgaRom FpgaRom { get; private set; }
  public Memories.ScopeStrobeMemory StrobeMemory { get; private set; }
  public Memories.MAX19506Memory AdcMemory { get; private set; }
  public Memories.ScopePicRegisterMemory PicMemory { get; private set; }
*/
  std::string GetSerial();
  SmartScope(SmartScopeUsb* interface);
  ~SmartScope();
  void Pause();
  void Resume();
  void SoftReset();

private:
  /*
   * Constants
   */
  static const uint8_t  FPGA_I2C_ADDRESS_SETTINGS = 0x0C;
  static const uint8_t  FPGA_I2C_ADDRESS_ROM = 0x0D;

  static const uint32_t VIEWPORT_SAMPLES_MAX = 2048;
  static const int32_t  BURSTS_MIN = 2;
  static const int32_t  BURSTS_MAX = 64;
  static const int32_t  BYTES_PER_BURST = 64;
  static const int32_t  BYTES_PER_SAMPLE = 2;
  static const int32_t  SAMPLES_PER_BURST = BYTES_PER_BURST / BYTES_PER_SAMPLE; //one byte per channel

  static const int32_t  OVERVIEW_BUFFER_SIZE = 2048;
  static const int32_t  ACQUISITION_DEPTH_DEFAULT = 512 * 1024;
  const int VIEW_DECIMATION_MAX = (int)std::log2((double)ACQUISITION_DEPTH_DEFAULT / (double)OVERVIEW_BUFFER_SIZE);

  constexpr static const double   BASE_SAMPLE_PERIOD = 10e-9; //10MHz sample rate
  static const int32_t  MAX_COMPLETION_TRIES = 1;
  //FIXME: this should be automatically parsed from VHDL
  static const int32_t  INPUT_DECIMATION_MAX_FOR_FREQUENCY_COMPENSATION = 4;
  static const int32_t  INPUT_DECIMATION_MIN_FOR_ROLLING_MODE = 7;
  static const int32_t  INPUT_DECIMATION_MAX = 9;

  /*
   * Hardware
   */
  SmartScopeUsb* _hardware_interface;
  //SmartScopeRom* _rom;
  bool _flashed = false;
  bool _deviceReady = false;
  bool _discardPreviousAcquisition = true;

  std::vector<uint8_t> adcTimingValues = { 0, 1, 2, 3, 5, 6, 7 };
  uint8_t AdcTimingValue();
  void AdcTimingValue(uint8_t value);

  void ConfigureAdc();
  void CalibrateAdc();
  bool TestAdcRamp();
  void DeconfigureAdc();
  void EnableEssentials(bool enable);
  void Configure();
  void Deconfigure();
  void Reset();
  uint32_t FpgaFirmwareVersion();
  uint32_t PicFirmwareVersion();

  /*
   * Scope settings and control
   */
  bool _acquiring = false;
  bool _stopPending = false;
  bool _awaitingTrigger = false;
  bool _armed = false;
  bool _paused = false;
  bool _acquiringWhenPaused = false;
  int32_t _acquisitionDepthUserMaximum = ACQUISITION_DEPTH_DEFAULT;

  //std::map<AnalogChannel, GainCalibration> channelSettings;
  /*
  private TriggerValue triggerValue = new TriggerValue
  {
      source = TriggerSource.Channel,
      channel = AnalogChannel.ChA,
      edge = TriggerEdge.RISING,
      mode = TriggerMode.Edge,
      level = 0.0f,
  };*/

  static const uint32_t FPGA_VERSION_UNFLASHED = 0xffffffff;

  void CommitSettings();
  void toggleAcquisitionUpdateStrobe();
  void toggleViewUpdateStrobe();

  /*
   * Vertical
   */

  static const uint8_t yOffsetMax = 200;
  static const uint8_t yOffsetMin = 10;
  std::map<AnalogChannel*, float> yOffset;
  void YOffset(AnalogChannel* channel, float offset);
  float YOffset(AnalogChannel* channel);
  float ConvertYOffsetByteToVoltage(AnalogChannel* channel, uint8_t value);

  static const double constexpr validDividers[] = { 1, 6, 36 };
  static const double constexpr validMultipliers[] = { 1.1, 2, 3 };
  std::map<AnalogChannel*, float[2]> verticalRanges;
  void VerticalRange(AnalogChannel* channel, float minimum, float maximum);
  void SetDivider(AnalogChannel* channel, double divider);
  void SetMultiplier(AnalogChannel* channel, double multiplier);

  void Coupling(AnalogChannel* channel, enum Coupling coupling);
  enum Coupling Coupling(AnalogChannel* channel);

  /*
   * Acquisition & Viewport
   */
  int viewPortSamples = 2048;
  bool ChunkyAcquisitions;
  bool HighBandwidthMode = false;

  AcquisitionMode AcquisitionMode();
  void AcquisitionMode(enum AcquisitionMode);
  bool CanRoll();
  void Rolling(bool rolling);
  bool Rolling();
  void Running(bool running);
  bool Running();
  bool SendOverviewBuffer();
  void SendOverviewBuffer(bool value);
  bool StopPending();
  bool AwaitingTrigger();
  bool Armed();

  double AcquisitionBufferTimeSpan();
  double AcquisitionLengthCurrent();
  double AcquisitionLengthMin();
  double AcquisitionLengthMax();
  uint32_t AcquisitionDepthMax();
  uint32_t InputDecimationMax();
  bool PreferPartial();
  void PreferPartial(bool pref);
  void AcquisitionLength(double length);
  double AcquisitionLength();

  uint32_t AcquisitionDepth();
  void AcquisitionDepth(uint32_t depth);

  void ViewPort(double offset, double timespan);
  void ViewPortOffset(double time, int samplesExcess);
  double ViewPortTimeSpan();
  double ViewPortOffset();
  int SubSampleRate();
  void SubSampleRate(int rate);
  double SamplePeriod();
  double SamplesToTime(uint samples);

  bool LogicAnalyserEnabled();
  void ChannelSacrificedForLogicAnalyser(AnalogChannel);

  //  DataPackageScope GetScopeData()
  //  std::map<Channel, Array> SplitAndConvert(uint8_t[] buffer, List<AnalogChannel> channels, SmartScopeHeader header, Dictionary<AnalogChannel, SmartScope.GainCalibration> channelSettings, int offset, int length);

  /*
   * Trigger
   */
  double _holdoff;
  labnation::TriggerValue TriggerValue();
  void Trigger(labnation::TriggerValue);
  void ForceTrigger();
  static int TriggerDelay(TriggerMode mode, int inputDecimation);
  double TriggerHoldOff();
  void TriggerHoldOff(double holdoff);
  void SetTriggerByte(uint8_t level);
  void UpdateTriggerPulseWidth();

  /*
   * AUX port
   */
  void DigitalOutputVoltage(float voltage);
  uint8_t DigitalOutput();
  void DigitalOutput(uint8_t);

};

}

#endif // LABNATION_SMARTSCOPE_H
