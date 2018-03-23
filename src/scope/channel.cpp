#include <labnation/scope/channel.h>
#include <vector>

namespace labnation {

std::set<Channel*>        Channel::list;
std::set<DigitalChannel*> DigitalChannel::list;
std::set<AnalogChannel*>  AnalogChannel::list;

AnalogChannel Channels::ChA = AnalogChannel("A", 0);
AnalogChannel Channels::ChB = AnalogChannel("B", 1);
DigitalChannel Channels::Digi0 = DigitalChannel("0", 0);
DigitalChannel Channels::Digi1 = DigitalChannel("1", 1);
DigitalChannel Channels::Digi2 = DigitalChannel("2", 2);
DigitalChannel Channels::Digi3 = DigitalChannel("3", 3);
DigitalChannel Channels::Digi4 = DigitalChannel("4", 4);
DigitalChannel Channels::Digi5 = DigitalChannel("5", 5);
DigitalChannel Channels::Digi6 = DigitalChannel("6", 6);
DigitalChannel Channels::Digi7 = DigitalChannel("7", 7);

}
