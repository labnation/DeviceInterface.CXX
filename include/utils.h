#ifndef _LABNATION_UTILS_H
#define _LABNATION_UTILS_H

#include <string>
#include <sys/socket.h>

#include <algorithm>
#include <cctype>
#include <locale>
int pthread_join_timeout(pthread_t wid, unsigned int msecs);

std::string execute_cmd(const char* cmd);

#define member_size(type, member) sizeof(((type *)0)->member)

bool iface_has_addr(const char* interface);

static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}
// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}
// trim from both ends (in place)
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

#endif // _LABNATION_UTILS_H
