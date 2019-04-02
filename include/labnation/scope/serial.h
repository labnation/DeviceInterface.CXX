#ifndef _LABNATION_SERIAL_H
#define _LABNATION_SERIAL_H

#include <string>

#define LIST_PLANT \
  X(UNKNOWN, -1) \
  X(TOLSTRAAT, 0) \
  X(POPERINGE, 1) \
  X(ORADEA, 2)

#define X(name, value) PLANT_##name = value,
enum Plant { LIST_PLANT };
#undef X

#define MODEL_OFFSET      (36*36)
#define MODEL_SMARTSCOPE  (0xA)
#define GENERATION_OFFSET (36)

#define LIST_MODEL \
    X(UNKNOWN,-1) \
    X(SMARTSCOPE_A00, (MODEL_SMARTSCOPE * MODEL_OFFSET) + (0 * GENERATION_OFFSET) + 0) \
    X(SMARTSCOPE_A10, (MODEL_SMARTSCOPE * MODEL_OFFSET) + (1 * GENERATION_OFFSET) + 0) \
    X(SMARTSCOPE_A11, (MODEL_SMARTSCOPE * MODEL_OFFSET) + (1 * GENERATION_OFFSET) + 1) \
    X(SMARTSCOPE_A12, (MODEL_SMARTSCOPE * MODEL_OFFSET) + (1 * GENERATION_OFFSET) + 2) \
    X(SMARTSCOPE_A14, (MODEL_SMARTSCOPE * MODEL_OFFSET) + (1 * GENERATION_OFFSET) + 4) \
    X(SMARTSCOPE_A15, (MODEL_SMARTSCOPE * MODEL_OFFSET) + (1 * GENERATION_OFFSET) + 5) \
    X(SMARTSCOPE_A16, (MODEL_SMARTSCOPE * MODEL_OFFSET) + (1 * GENERATION_OFFSET) + 6) \
    X(SMARTSCOPE_A17, (MODEL_SMARTSCOPE * MODEL_OFFSET) + (1 * GENERATION_OFFSET) + 7) \
    X(SMARTSCOPE_A18, (MODEL_SMARTSCOPE * MODEL_OFFSET) + (1 * GENERATION_OFFSET) + 8)

#define X(name, value) MODEL_##name = value,
enum Model { LIST_MODEL};
#undef X

namespace labnation
{

class Serial {

public:
  Serial(std::string serial);
  static void Explain(Serial s);

  Plant plant;
  Model model;
  int week;
  int year;
  long number;

private:
  std::string source;

};

}

#endif // _LABNATION_SERIAL_H
