#include <labnation.h>
#include <labnation/httpserver.h>

namespace labnation {
  HttpServer::HttpServer() {
    mg_mgr_init(&_manager, this);
  }

  HttpServer::~HttpServer() {
    debug("Destroying HTTP server");
    mg_mgr_free(&_manager);
  }

  void HttpServer::Handler(struct mg_connection* conn, int ev, void *p) {
    struct http_message* hm;

    HttpServer* s = (HttpServer*)conn->mgr->user_data;
    std::string response = std::string();

    switch(ev) {
      case 0:
        return;
      case MG_EV_HTTP_REQUEST:
        hm = (struct http_message *) p;

        // Match URI with route map
        for(std::map<const char *,
              std::function<ROUTE_HANDLER()>>::iterator iter = s->routes.begin();
              iter != s->routes.end(); ++iter) {
          debug("Testing route to %s", iter->first);
          if(mg_vcmp(&hm->uri, iter->first)==0) {
            response = iter->second(s, hm);
            break;
          }
        }

        //Serve response if we got any, otherwise, serve a 404
        if(response.empty()) {
          response = "We don't do that command";
          mg_send_head(conn, 404, response.length(), NULL);
          mg_printf(conn, "%s", response.c_str());
          break;
        } else {
          mg_send_head(conn, 200, response.length(), NULL);
          mg_printf(conn, "%s", response.c_str());
          break;
        }
      default:
        debug("Don't know what to do with event %d", ev);
    }
  }

  ROUTE_HANDLER(HttpServer::ServeSerial) {
    return "serial";
  }
  ROUTE_HANDLER(HttpServer::ServeUnknown) {
    return std::string(m->uri.p, m->uri.len) + " - Not supported yet";
  }

  void HttpServer::Start() {
    debug("Starting HTTP server");
    _connection = mg_bind(&_manager, _http_address, Handler);

    if (_connection == NULL) {
      throw NetException("Failed to make connection");
    }

    mg_set_protocol_http_websocket(_connection);

    info("HTTP server started");
    for (;;) {
      mg_mgr_poll(&_manager, 5000);
      //find scope...
      _scope = NULL;
    }
  }

  void HttpServer::Stop() {
    debug("Stop HTTP server");
  }
}