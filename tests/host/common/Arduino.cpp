/*
 Arduino.cpp - Mocks for common Arduino APIs
 Copyright © 2016 Ivan Grokhotkov

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
*/

#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>

#include <Arduino.h>
#include <Schedule.h>

#include <thread>
#include <stdexcept>

#include <ets_sys.h>
#include <sched.h>

namespace {

timeval gtod0 = { 0, 0 };

} // namespace

extern "C" unsigned long millis()
{
    timeval time;
    gettimeofday(&time, NULL);
    if (gtod0.tv_sec == 0)
        memcpy(&gtod0, &time, sizeof gtod0);
    return ((time.tv_sec - gtod0.tv_sec) * 1000) + ((time.tv_usec - gtod0.tv_usec) / 1000);
}

extern "C" unsigned long micros()
{
    timeval time;
    gettimeofday(&time, NULL);
    if (gtod0.tv_sec == 0)
        memcpy(&gtod0, &time, sizeof gtod0);
    return ((time.tv_sec - gtod0.tv_sec) * 1000000) + time.tv_usec - gtod0.tv_usec;
}

extern "C" void yield()
{
    run_scheduled_recurrent_functions();
    if (!can_yield()) {
        throw std::runtime_error("should only yield from loop()!");
    }

    esp_yield();
}

extern "C" void loop_end()
{
    run_scheduled_functions();
    run_scheduled_recurrent_functions();
}

extern "C" void optimistic_yield(uint32_t interval_us)
{
    static auto last = std::chrono::steady_clock::now();

    const auto now = std::chrono::steady_clock::now();
    if (last - now > std::chrono::microseconds{ interval_us }) {
        last = now;
        yield();
    }
}

bool esp_try_delay(const uint32_t start_ms, const uint32_t timeout_ms, const uint32_t intvl_ms)
{
    uint32_t expired = millis() - start_ms;
    if (expired >= timeout_ms)
    {
        return true;
    }
    esp_delay(std::min((timeout_ms - expired), intvl_ms));
    return false;
}

extern "C" void __panic_func(const char* file, int line, const char* func)
{
    (void)file;
    (void)line;
    (void)func;
    abort();
}

extern "C" void delay(unsigned long ms)
{
    esp_delay(ms);
}

extern "C" void delayMicroseconds(unsigned int us)
{
    std::this_thread::sleep_for(
        std::chrono::microseconds{ us });
}

#include "cont.h"
cont_t*         g_pcont = NULL;
extern "C" void cont_suspend(cont_t*) { }

extern "C" int __mockverbose(const char* fmt, ...)
{
    (void)fmt;
    return 0;
}

int mockverbose(const char* fmt, ...)
    __attribute__((weak, alias("__mockverbose"), format(printf, 1, 2)));
