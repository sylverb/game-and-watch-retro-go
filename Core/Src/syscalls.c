#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include "rg_rtc.h"

extern uint32_t log_idx;
extern char logbuf[1024 * 4];

int _open(const char *name, int flags, int mode)
{
    errno = ENOSYS; // Not implemented
    return -1;
}

int _close(int file)
{
    return -1; // Fail
}

int _write(int file, char *ptr, int len)
{
    if (file == STDOUT_FILENO || file == STDERR_FILENO)
    {
        uint32_t idx = log_idx;
        if (idx + len + 1 > sizeof(logbuf))
        {
            idx = 0;
        }

        memcpy(&logbuf[idx], ptr, len);
        idx += len;
        logbuf[idx] = '\0';

        log_idx = idx;

        return len;
    }
    return -1;
}

int _read(int file, char *ptr, int len)
{
    return 0; // end of file
}

int _lseek(int file, int ptr, int dir)
{
    return 0; // success
}

int _fstat(int file, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int file)
{
    return 1;
}

int _gettimeofday(struct timeval *tv, void *tzvp)
{
    if (tv)
    {
        // get epoch UNIX time from RTC
        time_t unixTime = GW_GetUnixTime();
        tv->tv_sec = unixTime;

        // get millisecondes from rtc and convert them to microsecondes
        uint64_t millis = GW_GetCurrentMillis();
        tv->tv_usec = (millis % 1000) * 1000;
        return 0;
    }

    errno = EINVAL;
    return -1;
}
