#ifndef _LABNATION_UTILS_H
#define _LABNATION_UTILS_H

#include <string>
#include <pthread.h>

int pthread_join_timeout(pthread_t wid, unsigned int msecs);

#define member_size(type, member) sizeof(((type *)0)->member)

namespace labnation {

class Base36 {
public:
  static long Decode(std::string s);

private:
  static std::string charlist;
};

}

#endif // _LABNATION_UTILS_H
