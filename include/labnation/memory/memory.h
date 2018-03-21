#ifndef _LABNATION_MEMORY_H
#define _LABNATION_MEMORY_H

#include <cstdint>
#include <string>
#include <map>
#include "register.h"
#include "../smartscopeusb.h"

namespace labnation {

class Memory {

public:

  virtual ~Memory();
  std::string name;
  std::map<uint, Register*> registers;

  /*!
   * \brief Write register specified by address
   * \param address the register address to write
   */
  virtual void Write(uint address);
  /*!
   * \brief Read register specified by address
   * \param address the register address to read
   */
  virtual void Read(uint address);
  /*!
   * \brief Write register in range [from, until]
   * \param from first address to write
   * \param until last address to write
   */
  virtual void WriteRange(uint from, uint until);

  /*!
   * \brief Writes all dirty registers
   * \return True if at least 1 register was written
   */
  virtual bool Commit();

  virtual Register* operator[](uint address);

};

class PicMemory : public Memory {

public:
  PicMemory(SmartScopeUsb* interface);
  void Write(uint address);
  void Read(uint address);

private:
  SmartScopeUsb* _interface;
};




}

#endif // _LABNATION_MEMORY_H
