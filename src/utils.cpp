#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/select.h>

#include <labnation.h>
#include <utils.h>
#include <stdexcept>

#include <sys/types.h>
#include <ifaddrs.h>
#include <arpa/inet.h>


#define PTHREAD_JOIN_POLL_INTERVAL 10
#define false 0
#define true (!false)

typedef struct _waitData waitData;

struct _waitData
{
  pthread_t waitID;
  pthread_t helpID;
  int done;
};

void sleep_msecs(int msecs)
{
  struct timeval tv;

  tv.tv_sec = msecs/1000;
  tv.tv_usec = (msecs % 1000) * 1000;
  select (0,NULL,NULL,NULL,&tv);
}
unsigned int get_ticks()
{
  struct timeval tv;

  gettimeofday(&tv, NULL);
  return (tv.tv_usec/1000 + tv.tv_sec * 1000);
}

void * join_timeout_helper(void *arg)
{
  waitData *data = (waitData*)arg;

  pthread_join(data->waitID, NULL);
  data->done = true;
  return (void *)0;
}

int pthread_join_timeout(pthread_t wid, unsigned int msecs)
{
  pthread_t id;
  waitData data;
  unsigned int start = get_ticks();
  int timedOut = false;

  data.waitID = wid;
  data.done = false;

  if (pthread_create(&id, NULL, join_timeout_helper, &data) != 0)
    return (-1);
  do {
    if (data.done)
      break;
    /* you could also yield to your message loop here... */
    sleep_msecs(PTHREAD_JOIN_POLL_INTERVAL);
  } while ((get_ticks() - start) < msecs);
  if (!data.done)
  {
    pthread_cancel(id);
    timedOut = true;
  }
  /* free helper thread resources */
  pthread_join(id, NULL);
  return (timedOut);
}

std::string execute_cmd(const char* cmd) {
  debug("Executing command [%s]", cmd);
  char buffer[128];
  std::string result = "";
  FILE* pipe = popen(cmd, "r");
  if (!pipe) throw std::runtime_error("popen() failed!");
  try {
    while (!feof(pipe)) {
      if (fgets(buffer, 128, pipe) != NULL)
        result += buffer;
    }
  } catch (...) {
    pclose(pipe);
    throw;
  }
  pclose(pipe);
  debug("Output: \n%s", result.c_str());
  return result;
}

bool iface_has_addr() {
  char interface[] = "wlan0";
  char * cmd;
  debug("Checkinf if interface %s has an IP", interface);
  struct ifaddrs *ifaddr, *ifa;
  struct sockaddr_in *sa;

  if (getifaddrs(&ifaddr) == -1) {
    error("Failed to get IP addresses");
    return false;
  }

  for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    if (strcmp(ifa->ifa_name, interface) != 0)
      continue;
    if (ifa->ifa_addr->sa_family != AF_INET) {
      continue;
    }
    sa = (struct sockaddr_in *) ifa->ifa_addr;
    debug("IP found for interface: %s = %s\n", ifa->ifa_name, inet_ntoa(sa->sin_addr));
    freeifaddrs(ifaddr);
    return true;
  }
  freeifaddrs(ifaddr);
  return false;
}
