#ifndef _LABNATION_UTILS_H
#define _LABNATION_UTILS_H

int pthread_join_timeout(pthread_t wid, unsigned int msecs);

#define member_size(type, member) sizeof(((type *)0)->member)

#endif // _LABNATION_UTILS_H
