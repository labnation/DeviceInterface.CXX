#ifndef _LABNATION_H
#define _LABNATION_H

#include <exception>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <string>

#ifdef DEBUG
#define debug(fmt, ...) \
            do { std::fprintf(stdout, "[DBG][" __FILE__":%d] " fmt "\n", __LINE__, ##__VA_ARGS__); } while (0)
#else
#define debug(fmt, ...)
#endif

#define warn(fmt, ...) \
            do { std::fprintf(stdout, "[WARN] " fmt "\n", ##__VA_ARGS__); } while (0)
#define info(fmt, ...) \
            do { std::fprintf(stdout, "[INFO] " fmt "\n", ##__VA_ARGS__); } while (0)
#define error(fmt, ...) \
            do { std::fprintf(stderr, "[ERR ] " fmt "\n", ##__VA_ARGS__); } while (0)


#if __MACH__
    #define PTHREAD_NAME(name) \
        pthread_setname_np(name);
#else
#ifdef __UCLIBC__
    #define PTHREAD_NAME(name)
#else
    #define PTHREAD_NAME(name) \
        pthread_setname_np(pthread_self(), name);
#endif
#endif

namespace labnation {

class Exception: public std::exception {
private:
    std::string _message;
public:
    explicit Exception(const char* message, ...);
    virtual const char* what() const throw() {
        return _message.c_str();
    }
};

}

#endif //_LABNATION_H
