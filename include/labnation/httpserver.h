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

  SmartScopeUsb* _scope = NULL;
  struct mg_mgr _manager;
  struct mg_connection* _connection;
  const char* _http_address = ":8000";

};

}
#endif // _LABNATION_HTTPSERVER_H