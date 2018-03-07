#ifndef _LABNATION_HTTPSERVER_H
#define _LABNATION_HTTPSERVER_H

#include "smartscopeusb.h"
#include "server.h"

#include <mongoose.h>

namespace labnation {
  class HttpServer {

public:
  HttpServer();
  ~HttpServer();

  void Start();
  void Stop();

private:
  static void Handler(struct mg_connection* conn, int ev, void *p);
  static Command ParseUri(mg_str* uri);

#define ROUTE_HANDLER(name) std::string name(HttpServer* s, http_message* m)

  static ROUTE_HANDLER(ServeSerial);
  static ROUTE_HANDLER(ServeUnknown);

  SmartScopeUsb* _scope = NULL;
  struct mg_mgr _manager;
  struct mg_connection* _connection;
  const char* _http_address = ":8000";

  std::map<const char*, std::function<ROUTE_HANDLER()>> routes = {
    { "/serial",      ServeSerial  },
    { "/get",         ServeUnknown },
    { "/set",         ServeUnknown },
    { "/data",        ServeUnknown },
    { "/flash_fpga",  ServeUnknown },
    { "/acquisition", ServeUnknown },
    { "/flush",       ServeUnknown },
    { "/disconnect",  ServeUnknown },
  };


};

}
#endif // _LABNATION_HTTPSERVER_H