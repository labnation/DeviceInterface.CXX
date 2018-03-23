#ifndef _LABNATION_SMARTSCOPEUSB_H
#define _LABNATION_SMARTSCOPEUSB_H

#include <libusb-1.0/libusb.h>
#include <vector>
#include "hardware.h"
#include "smartscope_const.h"

namespace labnation {

class ScopeIOException: public std::exception {
private:
    std::string _message;
public:
    explicit ScopeIOException(const char* message, ...);
    virtual const char* what() const throw() {
        return _message.c_str();
    }
};

class SmartScopeUsb : public Hardware {
public:
    const static int VID;
    const static std::vector<int> PIDs;

    enum PicCommands
    {
        PIC_VERSION = 1,
        PIC_WRITE = 2,
        PIC_READ = 3,
        PIC_RESET = 4,
        PIC_BOOTLOADER = 5,
        EEPROM_READ = 6,
        EEPROM_WRITE = 7,
        FLASH_ROM_READ = 8,
        FLASH_ROM_WRITE = 9,
        I2C_WRITE = 10,
        I2C_READ = 11,
        PROGRAM_FPGA_START = 12,
        PROGRAM_FPGA_END = 13,
        I2C_WRITE_START = 14,
        I2C_WRITE_BULK = 15,
        I2C_WRITE_STOP = 16,
    };
    enum Controller : uint8_t
    {
        PIC = 0,
        ROM = 1,
        FLASH = 2,
        FPGA = 3,
        AWG = 4
    };
    enum HeaderFlags : uint8_t
    {
        None = 0,
        Acquiring = 1,
        IsOverview = 2,
        IsLastAcquisition = 4,
        Rolling = 8,
        TimedOut = 16,
        AwaitingTrigger = 32,
        Armded = 64,
        IsFullAcqusition = 128,
    };

    struct __attribute__ ((__packed__)) Header
    {
        uint8_t magic[2];           //0-1
        uint8_t header_offset;      //2
        uint8_t bytes_per_burst;    //3
        uint16_t n_bursts;          //4-5
        uint16_t offset;            //6-7
        uint8_t unused[2];          //8-9
        HeaderFlags flags;          //10
        uint8_t acquisition_id;     //11
        uint8_t unused2[3];          //12-13-14
        uint8_t regs[N_HDR_REGS];
        uint8_t strobes[(N_HDR_STROBES + 7) / 8];
    };
    SmartScopeUsb(libusb_device* device);
    virtual ~SmartScopeUsb();
    const static uint8_t HEADER_CMD_BYTE = 0xC0; //C0 as in Command
    const static uint8_t HEADER_RESPONSE_BYTE = 0xAD; //AD as in Answer Dude
    const static int FLASH_USER_ADDRESS_MASK = 0x0FFF;
    const static uint8_t FPGA_I2C_ADDRESS_AWG = 0x0E;
    const static int I2C_MAX_WRITE_LENGTH = 27;
    const static int I2C_MAX_WRITE_LENGTH_BULK = 29;

    enum Operation { READ, WRITE, WRITE_BEGIN, WRITE_BODY, WRITE_END };

    /* hardware abstract class implementation */
    std::string GetSerial();

    /* SmartScope abstract class implementation */
    void GetControllerRegister(Controller ctrl, uint address, int length, uint8_t* data) ;
    void SetControllerRegister(Controller ctrl, uint address, int length, uint8_t* data) ;
    int GetAcquisition(int length, uint8_t* buffer);
    void GetData(int length, uint8_t* buffer, int offset) ;
    void FlushDataPipe() ;
    void Reset() ;
    void FlashFpga(int length, uint8_t* firmware);
    uint32_t GetPicFirmwareVersion();
    bool IsDestroyed() ;
    void Destroy() ;

    void WriteControlBytes(int length, uint8_t* message);
    void WriteControlBytesBulk(int length, uint8_t* message);
    void WriteControlBytesBulk(int length, uint8_t* message, int offset);
    void ReadControlBytes(int length, uint8_t* buffer, int offset);
    void LoadBootLoader();

private:

    const static int USB_TIMEOUT_CTRL = 10000;
    const static int USB_TIMEOUT_DATA = 3000;
    const static int COMMAND_READ_ENDPOINT_SIZE = 16;
    const static short COMMAND_WRITE_ENDPOINT_SIZE = 32;

    libusb_device_handle* _device;
    libusb_device_descriptor _device_desc;
    bool _destroyed;

    const static int SZ_SERIAL = 255;
    unsigned char _serial[SZ_SERIAL];

    void SendCommand(PicCommands cmd);
    static int UsbCommandHeader(Controller ctrl, Operation op, uint address, int length, uint8_t* buffer);
    static int UsbCommandHeaderI2c(uint8_t I2cAddress, Operation op, uint address, int length, uint8_t* buffer);
};
}

#endif //_LABNATION_SMARTSCOPEUSB_H
