
#include "util.h"
#include "log.h"
#include <sys/time.h>
void get_current_timestamp(unsigned long long *timestamp)
{
    struct timeval tv;

    if (NULL == timestamp)
    {
        log_error("get_current_timestamp: bad timestamp");
        return;
    }

    gettimeofday(&tv, NULL);
    *timestamp = tv.tv_sec * 1000 + tv.tv_usec / 1000;

    return;
}
