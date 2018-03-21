#include <labnation.h>
#include <labnation/smartscope.h>

namespace labnation {

SmartScope::SmartScope(SmartScopeUsb* interface) {
  debug("MAking new scope");
  _hardware_interface = interface;
  for(auto ch : AnalogChannel::list)
    YOffset(ch, 0);

  pic_memory = new PicMemory(_hardware_interface);
  memories.push_back(pic_memory);
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
