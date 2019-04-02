#include <labnation.h>
#include <labnation/scope/serial.h>
#include <utils.h>
#include <time.h>
#include <string>
#include <map>

namespace labnation {

#define X(name, value) { value, #name },
std::map<int, const char*> PlantMap = { LIST_PLANT };
std::map<int, const char*> ModelMap = { LIST_MODEL };
#undef X

Serial::Serial(std::string serial) {
  source = serial;

  plant = (Plant)Base36::Decode(serial.substr(0, 2));
  year = 2010 + std::stoi(serial.substr(2, 1));
  week = std::stoi(serial.substr(3, 2));
  number= Base36::Decode(serial.substr(5, 3));
  model = (Model)Base36::Decode(serial.substr(8, 3));
}

void Serial::Explain(Serial s) {
  info("Serial %s: ", s.source.c_str());
  info("   Plant    %s [%d]", PlantMap[s.plant], s.plant);
  info("   Year     %4d", s.year);
  info("   Week     %d", s.week);
  info("   Number   %lu", s.number);
  info("   Model    %s [%d]", ModelMap[s.model], s.model);
}

}
