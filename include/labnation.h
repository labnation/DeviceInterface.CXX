#ifndef _LABNATION_H
#define _LABNATION_H

#include <exception>
#include <cstring>
#include <cstdio>
#include <cstdlib>

#ifdef DEBUG
#define debug(fmt, ...) \
            do { std::fprintf(stdout, "[DBG ] " fmt "\n", ##__VA_ARGS__); } while (0)
#else
#define debug(fmt, ...)
#endif

#define warn(fmt, ...) \
            do { std::fprintf(stdout, "[WARN] " fmt "\n", ##__VA_ARGS__); } while (0)
#define info(fmt, ...) \
            do { std::fprintf(stdout, "[INFO] " fmt "\n", ##__VA_ARGS__); } while (0)
#define error(fmt, ...) \
            do { std::fprintf(stderr, "[ERR ] " fmt "\n", ##__VA_ARGS__); } while (0)


#if TARGET_DARWIN
    #define PTHREAD_NAME(name) \
        pthread_setname_np(name);
#else
#ifdef TARGET_OPENWRT
    #define PTHREAD_NAME(name)
#else
    #define PTHREAD_NAME(name) \
        pthread_setname_np(pthread_self(), name);
#endif
#endif

#endif //_LABNATION_H
