#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <dlfcn.h>
#include <pthread.h>
#include <cstdlib>
#include <unistd.h>
#include <sys/syscall.h>
#include "PapiManager.hpp"
#include "MonitorManager.hpp"
#include "LoggerManager.hpp"

// ----------------------------------------------------------------------------
// LD_PRELOAD hook: Intercepts pthread_create to auto-register threads with URJA
// ----------------------------------------------------------------------------

/**
 * @brief Real pthread_create function pointer (resolved via dlsym).
 *
 * This holds the address of the original pthread_create function so we can
 * call it after injecting URJA-specific logic.
 */
static int (*real_pthread_create)(pthread_t *, const pthread_attr_t *, void *(*)(void *), void *);

/**
 * @brief Ensures the real pthread_create is resolved only once.
 */
static pthread_once_t once_control = PTHREAD_ONCE_INIT;


/**
 * @brief Resolves the real pthread_create symbol using dlsym(RTLD_NEXT).
 */
static void init_real_pthread() {
    real_pthread_create = (decltype(real_pthread_create))dlsym(RTLD_NEXT, "pthread_create");
}

/**
 * @brief Wrapper entry point for new threads created via pthread_create.
 *
 * This function registers the thread with URJA's PapiManager, then delegates
 * execution to the original thread function.
 *
 * @param arg Pointer to an array of two void pointers:
 *            [0] → original start_routine function
 *            [1] → argument to start_routine
 * @return Whatever the original thread function returns
 */
static void* thread_entry(void* arg) {
    auto* real = reinterpret_cast<void **>(arg);
    auto* fn = reinterpret_cast<void *(*)(void *)>(real[0]);
    void* fn_arg = real[1];
    free(arg);

    pid_t tid = syscall(SYS_gettid);
    PapiManager::getInstance().registerThread(tid, pthread_self());
    void* result = fn(fn_arg);
    PapiManager::getInstance().markThreadFinished(pthread_self());
    return result;
}

/**
 * @brief Overridden version of pthread_create that injects URJA monitoring hooks.
 *
 * This function wraps the original start_routine to ensure the thread is tracked by URJA.
 *
 * @param thread Pointer to pthread_t where the thread ID will be stored
 * @param attr Optional thread attributes
 * @param start_routine Original function the thread will execute
 * @param arg Argument to be passed to start_routine
 * @return Result of the original pthread_create
 */
extern "C" int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                              void *(*start_routine)(void *), void *arg) {
    pthread_once(&once_control, init_real_pthread);
    // Package the real function and its argument into a wrapper
    void** wrapper_args = (void**)malloc(2 * sizeof(void*));
    wrapper_args[0] = (void*)start_routine;
    wrapper_args[1] = arg;

    // Call the original pthread_create with our custom thread_entry
    return real_pthread_create(thread, attr, thread_entry, wrapper_args);
}

/**
 * @brief Constructor hook that runs when the library is loaded via LD_PRELOAD.
 *
 * Initializes URJA subsystems:
 * - LoggerManager
 * - PapiManager
 * - Registers the main thread
 * - Starts background monitoring thread
 */
__attribute__((constructor)) static void init_urja() {
    LoggerManager::getInstance().initialize();
    pid_t tid = syscall(SYS_gettid);
    PapiManager::getInstance().initialize();
    PapiManager::getInstance().registerThread(tid, pthread_self());
    MonitorManager::start();
}

/**
 * @brief Destructor hook that runs automatically on program exit.
 *
 * Stops the monitoring thread and prints the final summary.
 */
__attribute__((destructor)) static void cleanup_urja() {
    MonitorManager::stop();
}
