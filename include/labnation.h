#ifndef _LABNATION_H
#define _LABNATION_H

#include <exception>

#ifdef DEBUG
#define debug(fmt, ...) \
            do { if (DEBUG) std::fprintf(stderr, "[DBG ] " fmt "\n", ##__VA_ARGS__); } while (0)
#else
#define debug(fmt, ...)
#endif

#define warn(fmt, ...) \
            do { std::fprintf(stderr, "[WARN] " fmt "\n", ##__VA_ARGS__); } while (0)
#define info(fmt, ...) \
            do { std::fprintf(stderr, "[INFO] " fmt "\n", ##__VA_ARGS__); } while (0)
#define error(fmt, ...) \
            do { std::fprintf(stderr, "[ERR ] " fmt "\n", ##__VA_ARGS__); } while (0)


#if DARWIN
    #define PTHREAD_NAME(name) \
        pthread_setname_np(name);
#else
    #define PTHREAD_NAME(name) \
        pthread_setname_np(pthread_self(), name);
#endif

#endif //_LABNATION_H
