#ifndef _LABNATION_MEMORY_H
#define _LABNATION_MEMORY_H

#include <cstdint>
#include <map>
#include "register.h"

namespace labnation {

class Memory {

public:

  std::map<uint32_t, Register> registers;

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

  virtual Register operator[](uint32_t address);

};

}

#endif // _LABNATION_MEMORY_H
