#include <labnation.h>
#include <labnation/httpserver.h>

namespace labnation {
  HttpServer::HttpServer() {
    debug("Constructing HTTP server");
    mg_mgr_init(&_manager, this);
  }

  HttpServer::~HttpServer() {
    debug("Destroying HTTP server");
    mg_mgr_free(&_manager);
  }

  void HttpServer::Handler(struct mg_connection* conn, int ev, void *p) {
    struct http_message* hm;
    Command comm = UNDEFINED;
    std::string response = std::string("");

    switch(ev) {
      case 0:
        //HttpServer* s = (HttpServer*)p;
        return;
      case MG_EV_HTTP_REQUEST:
        hm = (struct http_message *) p;
        debug("HTTP request arrived");
        debug("BODY: %s", hm->message.p);

        comm = ParseUri(&hm->uri);
        switch(comm) {
          case SERIAL:
            response += "SERIAL";
            break;
          case GET:
            response += "Getting";
            break;
          case SET:
            response += "Setting";
            break;
          case DATA:
            response += "Data";
            break;
          case PIC_FW_VERSION:
            response += "PIC firmware version";
            break;
          case FLASH_FPGA:
            response += "Flashing fpga";
            break;
          case ACQUISITION:
            response += "Acqusition smth";
            break;
          case FLUSH:
            response += "Flushing USB";
            break;
          case DATA_PORT:
            response += "Data port not supported";
            break;
          case DISCONNECT:
            response += "Disconnecting";
            break;
          default:
            response += "UNKNOWN";
        }

        mg_send_head(conn, 200, response.length(), NULL);
        mg_printf(conn, "%s", response.c_str());

        break;
      default:
        debug("Don't know what to do with event %d", ev);
    }
  }

  Command HttpServer::ParseUri(mg_str* uri) {
    debug("%s", std::string(uri->p, uri->len).c_str());
    if(mg_vcmp(uri, "/serial")==0)
      return SERIAL;
    if(mg_vcmp(uri, "/get")==0)
      return GET;
    if(mg_vcmp(uri, "/set")==0)
      return SET;
    if(mg_vcmp(uri, "/data")==0)
      return DATA;
    if(mg_vcmp(uri, "/pic_fw_version")==0)
      return PIC_FW_VERSION;
    if(mg_vcmp(uri, "/flash_fpga")==0)
      return FLASH_FPGA;
    if(mg_vcmp(uri, "/acquisition")==0)
      return ACQUISITION;
    if(mg_vcmp(uri, "/flush")==0)
      return FLUSH;
    if(mg_vcmp(uri, "/data_port")==0)
      return DATA_PORT;
    if(mg_vcmp(uri, "/disconnect")==0)
      return DISCONNECT;
    return UNDEFINED;
  }


  void HttpServer::Start() {
    debug("Starting HTTP server");
    _connection = mg_bind(&_manager, _http_address, Handler);

    if (_connection == NULL) {
      throw NetException("Failed to make connection");
    }

    debug("The connection is %p", _connection);
    mg_set_protocol_http_websocket(_connection);

    for (;;) {
      mg_mgr_poll(&_manager, 1000);
      //find scope...
      _scope = NULL;
    }
  }

  void HttpServer::Stop() {
    debug("Stop HTTP server");
  }
}