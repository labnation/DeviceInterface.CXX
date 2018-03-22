#ifndef _LABNATION_MEMORY_H
#define _LABNATION_MEMORY_H

#include <cstdint>
#include <string>
#include <map>
#include "register.h"
#include "../hardware/smartscopeusb.h"

namespace labnation {

class Memory {

public:

  Memory(const char* name);
  virtual ~Memory();
  std::string name;
  std::map<uint32_t, Register*> registers;

  /*!
   * \brief Write register specified by address
   * \param address the register address to write
   */
  virtual void Write(uint32_t address);
  /*!
   * \brief Read register specified by address
   * \param address the register address to read
   */
  virtual void Read(uint32_t address);
  /*!
   * \brief Write register in range [from, until]
   * \param from first address to write
   * \param until last address to write
   */
  virtual void WriteRange(uint32_t from, uint32_t until);

  /*!
   * \brief Writes all dirty registers
   * \return True if at least 1 register was written
   */
  virtual bool Commit();

  virtual Register* operator[](uint32_t address);

};

class PicMemory : public Memory {

public:
  PicMemory(SmartScopeUsb* interface);
  void Write(uint32_t address);
  void Read(uint32_t address);

private:
  SmartScopeUsb* _interface;
};

class FpgaMemory : public Memory {

public:
  FpgaMemory(SmartScopeUsb* interface, uint8_t i2c_address, const char* name, uint32_t size = 0, bool read_only = false);
  void Write(uint32_t address);
  void Read(uint32_t address);

private:
  SmartScopeUsb* _interface;
  uint8_t _i2c_address;
  bool _read_only;
};

class FpgaSettings : public FpgaMemory {
public:
  FpgaSettings(SmartScopeUsb* interface, uint8_t i2c_address);
};

class FpgaRom: public FpgaMemory {
public:
  FpgaRom(SmartScopeUsb* interface, uint8_t i2c_address);
};

class FpgaStrobes : public Memory {
public:
  FpgaStrobes(FpgaSettings* write_memory, FpgaRom* read_memory);
  void Write(uint32_t address);
  void Read(uint32_t address);

private:
  FpgaSettings* _write_memory;
  FpgaRom* _read_memory;
};

class Adc : public Memory {
public:
  Adc(FpgaSettings* fpga_settings, FpgaStrobes* fpga_strobes, FpgaRom* fpga_rom);
  void Write(uint32_t address);
  void Read(uint32_t address);

private:
  FpgaSettings* _fpga_settings;
  FpgaStrobes* _fpga_strobes;
  FpgaRom* _fpga_rom;
};

}

#endif // _LABNATION_MEMORY_H
