#include <labnation.h>

namespace labnation {

Exception::Exception(const char* message, ...) {
  char msg[255];
  va_list vl;
  va_start(vl, message);
  std::vsprintf(msg, message, vl);
  va_end(vl);
  _message = std::string(msg);
}

}
