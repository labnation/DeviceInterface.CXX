#ifndef _LABNATION_SCOPE_H
#define _LABNATION_SCOPE_H

#include "channel.h"
#include <map>

namespace labnation {

enum Coupling { AC, DC };
enum AcquisitionMode { SINGLE = 2, NORMAL = 1, AUTO = 0};
enum TriggerSource { Channel = 0, External = 1 };
enum TriggerEdge { RISING = 0, FALLING = 1, ANY = 2 };
enum TriggerMode { Edge = 0, Timeout = 1, Pulse = 2, Digital = 3 };
enum DigitalTriggerValue { L, H, R, F, X };

class TriggerValue
{

public:
  TriggerValue();
  TriggerValue(TriggerValue* t);

  TriggerMode mode = Edge;
  TriggerSource source = Channel;
  AnalogChannel channel = Channels::ChA;
  std::map<DigitalChannel*, DigitalTriggerValue> digital;
  TriggerEdge edge = RISING;
  float level;
  double pulseWidthMin = 0.0;
  double pulseWidthMax = std::numeric_limits<double>::infinity();

};

}

#endif // _LABNATION_SCOPE_H
