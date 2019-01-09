#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdarg.h>

#define MSG_LOG_BUF_LEN (256)

int msg_log(char *fmt, ...)
{
    va_list ap;
    char buf[MSG_LOG_BUF_LEN] = {0};
    int size = 0;
    struct timeval tv;
    struct tm *p;

    gettimeofday(&tv, NULL);
    p = gmtime(&tv.tv_sec);
    sprintf(buf, "[%02d:%02d:%02d %03d] ", p->tm_hour, p->tm_min, p->tm_sec, (int)tv.tv_usec/1000);
    size = strlen(buf);

    va_start(ap, fmt);
    (void)vsnprintf((char *)&buf[size], sizeof(buf) - size, fmt, ap);
    va_end(ap);

    printf("%s", buf);
    return 0;
}

