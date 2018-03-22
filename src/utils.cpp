#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/select.h>
#include <math.h>
#include <utils.h>

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

namespace labnation {

std::string Base36::charlist = "0123456789abcdefghijklmnopqrstuvwxyz";

long Base36::Decode(std::string s) {
  long result = 0;
  int pos = 0;

  for (std::string::reverse_iterator rit=s.rbegin(); rit!=s.rend(); ++rit) {
    result += charlist.find(tolower(*rit)) * (int)pow(36, pos);
    pos++;
  }

  return result;

}

}
