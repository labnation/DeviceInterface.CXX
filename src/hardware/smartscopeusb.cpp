#include <thread>
#include <chrono>
#include <cstring>
#include <labnation.h>
#include <labnation/smartscopeusb.h>
#include <stdarg.h>

using namespace std;

namespace labnation {
    
ScopeIOException::ScopeIOException(const char* message, ...) {
  char msg[255];
  va_list vl;
  va_start(vl, message);
  std::vsprintf(msg, message, vl);
  va_end(vl);
  _message = std::string(msg);
}

const int SmartScopeUsb::VID = 0x04D8;
const std::vector<int> SmartScopeUsb::PIDs { 0x0052, 0xF4B5 };

const uint8_t EP_DATA      = LIBUSB_ENDPOINT_IN  | 0x01;
const uint8_t EP_CMD_WRITE = LIBUSB_ENDPOINT_OUT | 0x02;
const uint8_t EP_CMD_READ  = LIBUSB_ENDPOINT_IN  | 0x03;

SmartScopeUsb::SmartScopeUsb(libusb_device* d)
{
  /* Get descriptor */
  int err = libusb_get_device_descriptor(d, &_device_desc);
  if(err)
    throw ScopeIOException("Failed to set configuration [%s]",
      libusb_error_name(err));
    
  /* Open and claim */
  err = libusb_open(d, &_device);
  if(err)
    throw ScopeIOException("Failed to open usb device [%s]", 
      libusb_error_name(err));

  err = libusb_set_configuration(_device, 1);
  if(err)
    throw ScopeIOException("Failed to set configuration [%s]", 
      libusb_error_name(err));

  err = libusb_claim_interface(_device, 0);
  if(err)
    throw ScopeIOException("Failed to claim interface [%s]", 
      libusb_error_name(err));

  /* Get serial */
  libusb_get_string_descriptor_ascii(_device, _device_desc.iSerialNumber,
    _serial, SZ_SERIAL);
  debug("Got SmartScope with serial %s", _serial);
}
SmartScopeUsb::~SmartScopeUsb()
{
  Destroy();
}
bool SmartScopeUsb::IsDestroyed() 
{
  return _destroyed;
}

std::string SmartScopeUsb::GetSerial()
{
  return std::string(reinterpret_cast<char*>(_serial));
}

void SmartScopeUsb::Destroy()
{
  debug("Releasing SmartScope usb interface %s", _serial);
  libusb_release_interface(_device, 0);
  debug("Closing SmartScope usb device %s", _serial);
  libusb_close(_device);
  _destroyed = true;
}

void SmartScopeUsb::WriteControlBytes(int length, uint8_t* message)
{
  if (length > COMMAND_WRITE_ENDPOINT_SIZE)
  {
    throw ScopeIOException("USB message too long for endpoint");
  }
  WriteControlBytesBulk(length, message);
}

void SmartScopeUsb::WriteControlBytesBulk(int length, uint8_t* message)
{
  WriteControlBytesBulk(length, message, 0);
}

void SmartScopeUsb::WriteControlBytesBulk(int length, uint8_t* message, int offset)
{
  int n = 0;
  int err = libusb_bulk_transfer(_device, EP_CMD_WRITE, &message[offset], length, &n, USB_TIMEOUT_CTRL);
  if(err)
    throw ScopeIOException("Failed write control bytes [%s]", 
      libusb_error_name(err));
  if(n != length)
    throw ScopeIOException("Only wrote %d out of %d bytes", n, length);
}

void SmartScopeUsb::ReadControlBytes(int length, uint8_t* buffer, int offset = 0)
{
  int n = 0;
  int err = libusb_bulk_transfer(_device, EP_CMD_READ, &buffer[offset], length, &n, USB_TIMEOUT_CTRL);
  if(err)
    throw ScopeIOException("Failed read control bytes [%s]", 
      libusb_error_name(err));
  if(n != length)
    throw ScopeIOException("Only read %d out of %d bytes", n, length);  
}

void SmartScopeUsb::FlushDataPipe()
{
  uint8_t buffer[64];
  int n = 1;
  int time = 0;
  while(n > 0) {
    time++;
    libusb_bulk_transfer(_device, EP_DATA, buffer, 64, &n, 100);
  }
  debug("SmartScope data pipe flushed by %d reads", time);
}

void SmartScopeUsb::GetData(int length, uint8_t* buffer, int offset)
{
  int n = 0;
  int err = libusb_bulk_transfer(_device, EP_DATA, &buffer[offset], length, &n, USB_TIMEOUT_DATA);
  if(err)
    throw ScopeIOException("Failed get data [%s]", 
      libusb_error_name(err));
  if(n != length)
    throw ScopeIOException("Only received %d out of %d bytes", n, length);
}
void SmartScopeUsb::GetControllerRegister(Controller ctrl, uint address, int length, uint8_t* data)
{
  //In case of FPGA (I2C), first write address we're gonna read from to FPGA
  //FIXME: this should be handled by the PIC firmware
  if (ctrl == FPGA)
    SetControllerRegister(ctrl, address, 0, NULL);
  if (ctrl == FLASH && (address + length) > (FLASH_USER_ADDRESS_MASK + 1))
    throw ScopeIOException("Can't read flash rom beyond 0x{0:X8}", FLASH_USER_ADDRESS_MASK);

  uint8_t msg[64];
  int len = UsbCommandHeader(ctrl, READ, address, length, msg);
  WriteControlBytes(len, msg);

  //EP3 always contains 16 bytes xxx should be linked to constant
  //FIXME: use endpoint length or so, or don't pass the argument to the function
  ReadControlBytes(16, msg);

  memcpy(data, &msg[ctrl == FLASH ? 5 : 4], length);
}
void SmartScopeUsb::SetControllerRegister(Controller ctrl, uint address, int length, uint8_t* data)
{
  uint8_t msg[32];
  int msgLen = 0;
  
  if (data != NULL && length > I2C_MAX_WRITE_LENGTH) {
    uint offset = 0;
    int bytesLeft = length;

    if (ctrl != AWG)
    {
      //Chop up in smaller chunks
      while(bytesLeft > 0)
      {
      int length = bytesLeft > I2C_MAX_WRITE_LENGTH ? I2C_MAX_WRITE_LENGTH : bytesLeft;
      SetControllerRegister(ctrl, address + offset, length, &data[offset]);
      offset += length;
      bytesLeft -= length;
      }
      return;
    }

    //Begin I2C - send start condition
    //FIXME: should be variable, not literal "32"
    msgLen = UsbCommandHeader(ctrl, WRITE_BEGIN, address, 0, msg);
    WriteControlBytes(msgLen, msg);

    while (bytesLeft > 0)
    {
      int length = bytesLeft > I2C_MAX_WRITE_LENGTH_BULK ? I2C_MAX_WRITE_LENGTH_BULK : bytesLeft;
      msgLen = UsbCommandHeader(ctrl, WRITE_BODY, address, length, msg);
    
      memcpy(&msg[msgLen], &data[offset], length);
      WriteControlBytes(32, msg);
      offset += length;
      bytesLeft -= length;
    }
    msgLen = UsbCommandHeader(ctrl, WRITE_END, address, 0, msg);
    WriteControlBytes(msgLen, msg);
  } else {
    msgLen = UsbCommandHeader(ctrl, WRITE, address, length, msg);
    if(length > 0)
      memcpy(&msg[msgLen], data, length);
    WriteControlBytes(msgLen + length, msg);
  }
}
void SmartScopeUsb::SendCommand(PicCommands cmd)
{
  uint8_t toSend[2] = { HEADER_CMD_BYTE, (uint8_t)cmd };
  WriteControlBytes(sizeof(toSend), toSend);
}
void SmartScopeUsb::Reset()
{
  SendCommand(PIC_RESET);
}
uint32_t SmartScopeUsb::GetPicFirmwareVersion()
{
  SendCommand(PIC_VERSION);
  uint8_t response[16];
  ReadControlBytes(16, response);
  return (response[6] << 16) + 
    (response[5] <<  8) + 
    (response[4] <<  0);
}

void SmartScopeUsb::FlashFpga(int length, uint8_t* firmware)
{
  int packetSize = 32;
  int padding = 2048 / 8;

  //Data to send to keep clock running after all data was sent
  uint8_t dummyData[packetSize];
  memset(dummyData, 0xFF, sizeof(dummyData));

  uint16_t commands = (uint16_t)(length / packetSize + padding);

  //PIC: enter FPGA flashing mode
  uint8_t msg[4] = {
    (uint8_t)HEADER_CMD_BYTE,
    (uint8_t)PROGRAM_FPGA_START,
    (uint8_t)(commands >> 8),
    (uint8_t)(commands)
  };
  WriteControlBytes(4, msg);

  //FIXME: this sleep is found necessary on android tablets.
  /* The problem occurs when a scope is initialised the *2nd*
   * time after the app starts, i.e. after replugging it.
   * A possible explanation is that in the second run, caches
   * are hit and the time between the PROGRAM_FPGA_START command
   * and the first bitstream bytes is smaller than on the first run.
   * 
   * Indeed, if this time is smaller than the time for the INIT bit
   * (see spartan 6 ug380 fig 2.4) to rise, the first bitstream data
   * is missed and the configuration fails.
   */
  
  //FIXME: can be shorter prolly
  this_thread::sleep_for(chrono::milliseconds(1000));

  WriteControlBytesBulk(length, firmware);
  for (int j = 0; j < padding; j++)
      WriteControlBytesBulk(sizeof(dummyData), dummyData);

  SendCommand(PROGRAM_FPGA_END);
}

void SmartScopeUsb::LoadBootLoader()
{
  SendCommand(PIC_BOOTLOADER);
}

int SmartScopeUsb::GetAcquisition(int length, uint8_t* buffer)
{
  Header* hdr;
  int tries = 0;
  while(true) {
    GetData(SZ_HDR, buffer, 0);
    hdr=(Header*)buffer;

    if(hdr->magic[0] == 'L' && hdr->magic[1] == 'N')
      break;
    if(tries++ > PACKAGE_MAX)
      throw ScopeIOException("Invalid header magic 0x%02X%02X at fetch %d", hdr->magic[0], hdr->magic[1]);
  }

  if(tries > 0)
    warn("Had to try %d times before a good header came through", tries + 1);

  if (hdr->flags & TimedOut)
    return SZ_HDR;

  if (hdr->flags & IsOverview)
  {
    GetData(SZ_OVERVIEW, buffer, SZ_HDR);
    return SZ_HDR + SZ_OVERVIEW;
  }

  if (hdr->n_bursts == 0)
    throw ScopeIOException("number of bursts in this USB pacakge is 0, cannot fetch");

  int len = hdr->n_bursts * hdr->bytes_per_burst;

  if (len + SZ_HDR > length)
  {
    error("Length of packet too large (N_burst: %d, bytes per burst: %d) expect failure", hdr->n_bursts, hdr->bytes_per_burst);
    void* dummy = malloc(len);
    GetData(len, (uint8_t*)dummy, 0);
    free(dummy);
    return 0;
  }
  GetData(len, buffer, SZ_HDR);

  return SZ_HDR + len;
}


int SmartScopeUsb::UsbCommandHeader(Controller ctrl, Operation op, uint address, int length, uint8_t* buffer)
{
  //Most common, will be overridden if necessary
  buffer[0] = HEADER_CMD_BYTE;
  buffer[2] = (uint8_t)(address);
  buffer[3] = (uint8_t)(length);
  // Set operation
  switch(ctrl) {
    case PIC:
      buffer[1] = op == WRITE ? PIC_WRITE : PIC_READ;
      return 4;
    case ROM:
      buffer[1] = op == WRITE ? EEPROM_WRITE : EEPROM_READ;
      return 4;
    case FLASH:
      buffer[1] = op == WRITE ? FLASH_ROM_WRITE : FLASH_ROM_READ;
      buffer[4] = (uint8_t)(address >> 8);
      return 5;
    case FPGA:
      return UsbCommandHeaderI2c((uint8_t)((address >> 8) & 0x7F), op, (uint8_t)(address & 0xFF), length, buffer);
    case AWG:
      throw ScopeIOException("AWG IO not implemented");
      switch(op) {
        case WRITE:
          return UsbCommandHeaderI2c(FPGA_I2C_ADDRESS_AWG, op, address, length, buffer);
        case WRITE_BEGIN:
          buffer[1] =  I2C_WRITE_START;
          buffer[2] = length + 2;
          buffer[3] = FPGA_I2C_ADDRESS_AWG << 1;
          buffer[4] = address;
          return 5;
        case WRITE_BODY:
          buffer[1] = I2C_WRITE_BULK;
          return 3;
        case WRITE_END:
          buffer[1] = I2C_WRITE_STOP;
          return 3;
        case READ:
          throw ScopeIOException("Can't read out AWG");
      }
  }
  return -1;
}

int SmartScopeUsb::UsbCommandHeaderI2c(uint8_t I2cAddress, Operation op, uint address, int length, uint8_t* buffer)
{
  //Most common, will be overridden if necessary
  buffer[0] = HEADER_CMD_BYTE;
  
  switch(op) {
    case WRITE:
      buffer[1] = I2C_WRITE;
      buffer[2] = (uint8_t)(length + 2);
      buffer[3] = I2cAddress << 1;
      buffer[4] = address;
      return 5;
    case READ:
      buffer[1] = I2C_READ;
      buffer[2] = I2cAddress;
      buffer[3] = length;
      return 4;
    default:
      throw ScopeIOException("Unsupported operation for I2C Header");}
  }
    
}
