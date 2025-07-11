#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <dlfcn.h>
#include <pthread.h>
#include <cstdlib>
#include <unistd.h>
#include <chrono>
#include <iostream>
#include <sys/syscall.h>
#include "PapiManager.hpp"
#include "MonitorManager.hpp"
#include "LoggerManager.hpp"

static int (*real_pthread_create)(pthread_t *, const pthread_attr_t *, void *(*)(void *), void *);

static pthread_once_t once_control = PTHREAD_ONCE_INIT;


static void init_real_pthread() {
    real_pthread_create = (decltype(real_pthread_create))dlsym(RTLD_NEXT, "pthread_create");
}

static void* thread_entry(void* arg) {
    auto* real = reinterpret_cast<void **>(arg);
    auto* fn = reinterpret_cast<void *(*)(void *)>(real[0]);
    void* fn_arg = real[1];
    free(arg);

    pid_t tid = syscall(SYS_gettid);

    PapiManager::getInstance().registerThread(tid, pthread_self());
    
    void* result = fn(fn_arg);
    PapiManager::getInstance().markThreadFinished(tid);
    return result;
}

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

__attribute__((constructor)) static void init_urja() {
    //const char* skip = std::getenv("URJA_SKIP_INIT");
    //if (skip && std::string(skip) == "1") return;
    unsetenv("LD_PRELOAD");
    LoggerManager::getInstance().initialize();
    pid_t tid = syscall(SYS_gettid);
    PapiManager::getInstance().initialize();
    PapiManager::getInstance().registerThread(tid, pthread_self());
    MonitorManager::start();
}

__attribute__((destructor)) static void cleanup_urja() {
    MonitorManager::stop();
}
