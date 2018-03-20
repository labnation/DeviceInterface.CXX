#ifndef _LABNATIONMEMORY_H
#define _LABNATIONMEMORY_H

#include "register.h"

namespace labnation {

class Memory {

public:

  std::map<uint32_t, Register> registers;

  /*!
   * \brief Write register specified by address
   * \param address the register address to write
   */
  virtual void write(uint32_t address) = 0;
  /*!
   * \brief Read register specified by address
   * \param address the register address to read
   */
  virtual void read(uint32_t address) = 0;
  /*!
   * \brief Write register in range [from, until]
   * \param from first address to write
   * \param until last address to write
   */
  virtual void write_range(uint32_t from, uint32_t until);

  virtual Register operator[](uint32_t address);

};

}

#endif // _LABNATIONMEMORY_H
