#include <labnation.h>
#include <labnation/scope/channel.h>
#include <labnation/scope/scope.h>

namespace labnation {

TriggerValue::TriggerValue() {
  for (auto ch: DigitalChannel::list) {
      debug("Setting trigger value %s to X", ch->name.c_str());
      digital[ch] = X;
    }
}

TriggerValue::TriggerValue(TriggerValue* t) : TriggerValue()
{
    mode = t->mode;
    source = t->source;
    channel = t->channel;
    digital = t->digital;
    edge = t->edge;
    level = t->level;
    pulseWidthMax = t->pulseWidthMax;
    pulseWidthMin = t->pulseWidthMin;
}

}
