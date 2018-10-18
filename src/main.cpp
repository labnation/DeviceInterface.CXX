#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include <signal.h>

#include <libusb-1.0/libusb.h>
#include <labnation.h>
#include <labnation/smartscopeusb.h>
#include <labnation/interfaceserver.h>

using namespace labnation;

libusb_context* usb_ctx;
libusb_device** devices;
libusb_device_descriptor desc;
SmartScopeUsb* scope;
InterfaceServer* server;

const uint8_t FPGA_I2C_ADDRESS_SETTINGS = 0x0C;
const uint8_t FPGA_I2C_ADDRESS_ROM = 0x0D;

void handle_sighup(int sig)
{
  if(server) {
    debug("SIGHUP - Stopping server");
    server->Stop();
  }
}

int main(int argc, char *argv[])
{
  signal(SIGHUP, handle_sighup);

  libusb_init(NULL);
  //libusb_set_debug(usb_ctx, LIBUSB_LOG_LEVEL_DEBUG);
  info("Starting smartscope server v%d.%d-%s", VERSION_MAJOR, VERSION_MINOR, FLAVOR);
  while(true)
  {
    int n = libusb_get_device_list(NULL, &devices);
    for(int i = 0; i < n; i++) {
      libusb_get_device_descriptor(devices[i], &desc);
      if(desc.idVendor == SmartScopeUsb::VID) {
        for(std::vector<int>::const_iterator j = SmartScopeUsb::PIDs.begin();
            j != SmartScopeUsb::PIDs.end(); ++j) {
          if(desc.idProduct == *j) {

            scope = new SmartScopeUsb(devices[i]);
            server = new InterfaceServer(scope);
            server->Start();
            while(server->GetState() != InterfaceServer::State::Destroyed) {
              std::this_thread::sleep_for(std::chrono::milliseconds(100));
              if(server->GetState() == InterfaceServer::State::Stopped)
                server->Start();
            }
            debug("Server destroyed - quitting");
            delete(server);
            delete(scope);
          }
        }
      }
    }
    libusb_free_device_list(devices, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }
  libusb_exit(NULL);
  info("Server quiting");
}
