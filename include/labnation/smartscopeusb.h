#ifndef _LABNATION_SMARTSCOPEUSB_H
#define _LABNATION_SMARTSCOPEUSB_H

#include "smartscope.h"
namespace labnation {

class SmartScopeUsb : public SmartScope {
public:
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
    
    /*
    [StructLayout(LayoutKind.Explicit, Size = Constants.SZ_HDR, Pack = 1)]
    unsafe public struct SmartScopeHeader
    {
        [FieldOffset(0)]
        public fixed byte magic[2];
        [FieldOffset(2)]
        public byte header_offset;
        [FieldOffset(3)]
        public byte bytes_per_burst;
        [FieldOffset(4)]
        public ushort n_bursts;
        [FieldOffset(6)]
        public ushort offset;
        [FieldOffset(10)]
        public HeaderFlags flags;
        [FieldOffset(11)]
        public byte acquisition_id;
        [FieldOffset(Constants.HDR_OFFSET)]
        public fixed byte regs[Constants.N_HDR_REGS];
        [FieldOffset(Constants.HDR_OFFSET + Constants.N_HDR_REGS)]
        public fixed byte strobes[(Constants.N_HDR_STROBES + 7) / 8];
    }
    */

    const static uint8_t HEADER_CMD_BYTE = 0xC0; //C0 as in Command
    const static uint8_t HEADER_RESPONSE_BYTE = 0xAD; //AD as in Answer Dude
    const static int FLASH_USER_ADDRESS_MASK = 0x0FFF;
    const static uint8_t FPGA_I2C_ADDRESS_AWG = 0x0E;
    const static int I2C_MAX_WRITE_LENGTH = 27;
    const static int I2C_MAX_WRITE_LENGTH_BULK = 29;

    enum Operation { READ, WRITE, WRITE_BEGIN, WRITE_BODY, WRITE_END };
    void WriteControlBytes(uint8_t* message, bool async);
    void WriteControlBytesBulk(uint8_t* message, bool async);
    void WriteControlBytesBulk(uint8_t* message, int offset, int length, bool async);
    void ReadControlBytes(uint8_t* buffer, int offset, int length);
    void LoadBootLoader();

private:
    void ReadControlBytes(int length, uint8_t* buffer);
    static void UsbCommandHeader(Controller ctrl, Operation op, uint address, uint length, uint8_t* buffer);
    static void UsbCommandHeaderI2c(uint8_t I2cAddress, Operation op, uint address, uint length, uint8_t* buffer);
};
}

#endif //_LABNATION_SMARTSCOPEUSB_H