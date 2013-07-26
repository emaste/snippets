#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>

__thread pid_t g_tls = -1;
__thread char g_threadname[32];

pid_t gettid()
{
    return (pid_t)syscall(SYS_gettid);
}

void logf(const char *format, ...)
{
    va_list args;
    char buf[1024];

    snprintf(buf, sizeof(buf), "'%s' [#%d LWP:%d 0x%lx] %s", g_threadname, g_tls, gettid(), pthread_self(), format);

    va_start (args, format);
    vprintf (buf, args);
    va_end (args);
}

void *thread_proc(void *arg)
{
    g_tls = (int)(intptr_t)arg;

    logf("pthread_setname_np('%s')\n", g_threadname);
    snprintf(g_threadname, sizeof(g_threadname), "thread_%d", g_tls);
    pthread_setname_np(pthread_self(), g_threadname);

    logf("sleep(5)\n");
    sleep(5);

    pid_t tid = gettid();
    logf("pthread_exit(%d)\n", tid);
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
    pthread_setname_np(pthread_self(), g_threadname);

    printf("num_threads:%zu\n", num_threads);

    for(size_t i = 0; i < num_threads; i++)
    {
        int err = pthread_create(&(threadids[i]), NULL, &thread_proc, (void *)(intptr_t)i);
        logf("pthread_create:%d (%s) pthread_t:%lx\n", err, strerror(err), threadids[i]);
    }

    sleep(1);

    for(size_t i = 0; i < num_threads; i++)
    {
        logf("Waiting for thread #%zu\n", i);

        void *status = NULL;
        int rc = pthread_join(threadids[i], &status);
        logf("Thread #%zu rc:%d status:%d\n", i, rc, (int)(intptr_t)status);
    }

    printf("done.\n");
    return 0;
} 
