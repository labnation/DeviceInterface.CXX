#include <labnation/channel.h>
#include <vector>

namespace labnation {

std::vector<Channel*>        Channel::list;
std::vector<DigitalChannel*> DigitalChannel::list;
std::vector<AnalogChannel*>  AnalogChannel::list;

}
