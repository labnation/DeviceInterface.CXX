#ifndef _LABNATION_INTERFACESERVER_H
#define _LABNATION_INTERFACESERVER_H

#include "smartscope.h"
#include "smartscopeusb.h"

#ifdef DARWIN
#include <dns_sd.h>
#else
#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-common/thread-watch.h>
#include <avahi-common/simple-watch.h>
#include <avahi-client/publish.h>
#endif

#include <pthread.h>
#include <functional>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

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

class InterfaceServer {

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

public:
  InterfaceServer(SmartScopeUsb* scope);
  ~InterfaceServer();
  enum State {
    Uninitialized = 0,
    Stopped = 1,
    Stopping = 2,
    Started = 3,
    Starting = 4,
    Destroying = 5,
    Destroyed = 6
  };
  State GetState();
  void SetState(State state);
  void Start();
  void Stop();
  void Destroy();

  void ManageState();
  void DataSocketServer();
  void ControlSocketServer();

private:
  static const int ACQUISITION_PACKET_SIZE = SZ_HDR + FETCH_SIZE_MAX;
  static const int DATA_SOCKET_BUFFER_SIZE = ACQUISITION_PACKET_SIZE * 2;
  static const int HDR_SZ = 4;
  static const int BUF_SIZE = 8 * 1024;

  std::function<void(InterfaceServer*)> _stateChanged = NULL;
  const char* SERVICE_TYPE = "_sss._tcp";
  const char* REPLY_DOMAIN = "local.";
  const char* TXT_DATA_PORT = "DATA_PORT";
  // Max received = header + full acq buf
  uint8_t smartScopeBuffer[ACQUISITION_PACKET_SIZE];

  State _stateRequested = Uninitialized;
  State _state = Uninitialized;
  labnation::SmartScopeUsb* _scope = NULL;
  uint16_t _port = 0;
  uint16_t _port_data = 0;
  bool _connected = false;

  bool _disconnect_called = false;

  /* Zeroconf service registration */

#ifdef DARWIN
  DNSServiceRef _dnsService = NULL;
  static void ServiceRegistered(
      DNSServiceRef sdRef,
      DNSServiceFlags flags,
      DNSServiceErrorType errorCode,
      const char *name,
      const char *regtype,
      const char *domain,
      void       *context
  );
#else
  AvahiThreadedPoll* _avahi_poll = NULL;
  AvahiClient* _avahi_client = NULL;
  AvahiEntryGroup* _avahi_entry_group = NULL;
  static void AvahiCallback(AvahiClient *s, AvahiClientState state, void *data);
  static void AvahiGroupChanged(AvahiEntryGroup *g, AvahiEntryGroupState state, void* userdata);

#endif
  void RegisterService();
  void UnregisterService();

  /* Threads */

  pthread_t _thread_state = 0;
  pthread_t _thread_data = 0;
  pthread_t _thread_ctrl = 0;

  static void* ThreadStartManageState(void * ctx);
  static void* ThreadStartDataSocketServer(void * ctx);
  static void* ThreadStartControlSocketServer(void * ctx);

  void Disconnect();
  void CleanSocketThread(pthread_t *thread, int *listener_socket, int *socket);

  /* Network */
  int _sock_ctrl_listen = -1;
  int _sock_ctrl = -1;
  int _sock_data_listen = -1;
  int _sock_data = -1;

  static int StartServer(const char * port);

};

}

#endif // _LABNATION_INTERFACESERVER_H
