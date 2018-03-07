#ifndef _LABNATION_SERVER_H
#define _LABNATION_SERVER_H

namespace labnation {
  class NetException: public std::exception {
  private:
      std::string _message;
  public:
      explicit NetException(const char* message, ...);
      virtual const char* what() const throw() {
          return _message.c_str();
      }
  };

}

enum Command : uint8_t {
  SERIAL = 13,
  GET = 24,
  SET = 25,
  DATA = 26,
  PIC_FW_VERSION = 27,
  FLASH_FPGA = 36,
  ACQUISITION = 52,
  FLUSH = 14,
  DATA_PORT = 42,
  DISCONNECT = 15,
  UNDEFINED = 0xff,
};

struct __attribute__ ((__packed__)) Message {
  uint32_t length;
  Command cmd;
  uint8_t data[];
};

struct __attribute__ ((__packed__)) ControllerMessage {
  labnation::SmartScopeUsb::Controller ctrl;
  uint16_t addr;
  uint16_t len;
  uint8_t data[];
};


#endif // _LABNATION_SERVER_H