/*
 * pthread example test app for lldb, from Michael Sartain's post at 
 * http://linux-debugger-bits.blogspot.com/2013/07/linux-pthread-test-app-with-lldb.html
 *
 * clang -Wall -g -o mikesart_pthread -lpthread mikesart_pthread.c
 */

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#if defined(__FreeBSD__)
#include <pthread_np.h>
#endif

__thread pid_t g_tls = -1;
__thread char g_threadname[32];

#if defined(__FreeBSD__)
pid_t gettid()
{
    return pthread_getthreadid_np();
}

void setname(const char *name)
{
    pthread_set_name_np(pthread_self(), g_threadname);
}
#elif defined(__Linux__)
pid_t gettid()
{
    return (pid_t)syscall(SYS_gettid);
}

void setname(const char *name)
{
    pthread_setname_np(pthread_self(), g_threadname);
}
#else
pid_t gettid()
{
    return (pid_t)(-1);
}

void setname(const char *name)
{
}
#endif

void logit(const char *format, ...)
{
    va_list args;
    char buf[1024];

    snprintf(buf, sizeof(buf), "'%s' [#%d LWP:%d 0x%lx] %s", g_threadname, g_tls, gettid(), (unsigned long)pthread_self(), format);

    va_start (args, format);
    vprintf (buf, args);
    va_end (args);
}

void *thread_proc(void *arg)
{
    g_tls = (int)(intptr_t)arg;

    logit("pthread_setname_np('%s')\n", g_threadname);
    snprintf(g_threadname, sizeof(g_threadname), "thread_%d", g_tls);
    setname(g_threadname);
    logit("sleep(5)\n");
    sleep(5);

    pid_t tid = gettid();
    logit("pthread_exit(%d)\n", tid);
    pthread_exit((void *)(intptr_t)tid);
    return 0;
}

int main(int argc, char *argv[])
{
    pthread_t threadids[256];
    static const size_t max_threads = sizeof(threadids) / sizeof(threadids[0]);

    size_t num_threads = (argc > 1) ? atoi(argv[1]) : 2;
    if (num_threads < 2)
        num_threads = 2;
    else if (num_threads > max_threads)
        num_threads = max_threads;

    snprintf(g_threadname, sizeof(g_threadname), "mainthrd");
    setname(g_threadname);

    printf("num_threads:%zu\n", num_threads);

    for(size_t i = 0; i < num_threads; i++)
    {
        int err = pthread_create(&(threadids[i]), NULL, &thread_proc, (void *)(intptr_t)i);
        logit("pthread_create:%d (%s) pthread_t:%lx\n", err, strerror(err), threadids[i]);
    }

    sleep(1);

    for(size_t i = 0; i < num_threads; i++)
    {
        logit("Waiting for thread #%zu\n", i);

        void *status = NULL;
        int rc = pthread_join(threadids[i], &status);
        logit("Thread #%zu rc:%d status:%d\n", i, rc, (int)(intptr_t)status);
    }

    printf("done.\n");
    return 0;
} 
