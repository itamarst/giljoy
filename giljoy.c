#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/time.h>

#include <Python.h>
#include <pythread.h>

/*
We want to track the amount of time spent with the GIL released. Unfortunately
Python acquires and releases the GIL in a number of different functions, so we
need to intercept them all.

Source: http://hg.python.org/cpython/file/5395f96588d4/Python/ceval.c
*/
static void (*real_PyEval_AcquireLock)(void) = 0;
static void (*real_PyEval_ReleaseLock)(void) = 0;
static void (*real_PyEval_AcquireThread)(PyThreadState *tstate) = 0;
static void (*real_PyEval_ReleaseThread)(PyThreadState *tstate) = 0;
static PyThreadState* (*real_PyEval_SaveThread)(void) = 0;
static void (*real_PyEval_RestoreThread)(PyThreadState *tstate) = 0;

/*
Load a symbol from the actual place that provides it.
*/
static void* loadsym(const char* symbol)
{
    void* result = dlsym( RTLD_NEXT , symbol);
    if (!result) {
        fprintf(stderr, "Error loading symbol: %s\n", dlerror());
        exit(1);
    }
    return result;
}


/*
For each pair of acquire/release functions, we store the start time on
release, and store the finish time after GIL re-acquisition finishes. (Elapsed
time measured before acquisition finished might erroneously and unfairly count
time waiting for GIL to be available as time running with GIL unreleased.)

We use thread-specific storage to make sure different threads don't interfere
with each other.
*/
static __thread struct timeval tss_release_time;
static __thread struct timeval tss_acquire_time;

static void store_release_time()
{
    fprintf(stderr, "Release!\n");
    gettimeofday(&tss_release_time, NULL);
}

static void store_acquire_time()
{
    fprintf(stderr, "Acquired!\n");
    gettimeofday(&tss_acquire_time, NULL);
}


/*
The wrapper functions for the Python GIL APIs.
*/
void PyEval_AcquireLock() {
    real_PyEval_AcquireLock();
    store_acquire_time();
}

void PyEval_ReleaseLock() {
    store_release_time();
    real_PyEval_ReleaseLock();
}

void PyEval_AcquireThread(PyThreadState *tstate) {
    real_PyEval_AcquireThread(tstate);
    store_acquire_time();
}

void PyEval_ReleaseThread(PyThreadState *tstate) {
    store_release_time();
    real_PyEval_ReleaseThread(tstate);
}

PyThreadState* PyEval_SaveThread()
{
    store_release_time();
    return real_PyEval_SaveThread();
}

void PyEval_RestoreThread(PyThreadState *tstate)
{
    real_PyEval_RestoreThread(tstate);
    store_acquire_time();
}


/*
Public API used by _giljoy.c to clear and get elapsed time.
*/

/*
Clear the elapsed time value.
*/
void giljoy_clear_released_time()
{
    /* Same time means elapsed time is zero. */
    tss_acquire_time = tss_release_time;
}

/*
Return the elapsed microseconds during which GIL was released.

We assume 64 bits are enough, since presumably the program being profiled
won't run for very long.
*/
uint64_t giljoy_released_time()
{
    uint64_t result = 0;
    result += (tss_acquire_time.tv_sec - tss_release_time.tv_sec) * 1000000;
    result += tss_acquire_time.tv_usec - tss_release_time.tv_usec;
    return result;
}


int main(int argc, char *argv[])
{
    PyEval_InitThreads();
    /* Get references to underlying functions that we are proxying. */
    real_PyEval_AcquireLock = loadsym("PyEval_AcquireLock");
    real_PyEval_ReleaseLock = loadsym("PyEval_ReleaseLock");
    real_PyEval_AcquireThread = loadsym("PyEval_AcquireThread");
    real_PyEval_ReleaseThread = loadsym("PyEval_ReleaseThread");
    real_PyEval_SaveThread = loadsym("PyEval_SaveThread");
    real_PyEval_RestoreThread = loadsym("PyEval_RestoreThread");

    return Py_Main(argc, argv);
}
