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

#include <Arduino.h>
#include <Schedule.h>

#include <functional>
#include <chrono>
#include <thread>

#include <unistd.h>

using clock_type = std::chrono::steady_clock;

extern "C" unsigned long millis()
{
    const auto since_epoch = clock_type::now().time_since_epoch();
    const auto out = std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch);
    return out.count();
}

extern "C" unsigned long micros()
{
    const auto since_epoch = clock_type::now().time_since_epoch();
    const auto out = std::chrono::duration_cast<std::chrono::microseconds>(since_epoch);
    return out.count();
}

extern "C" void loop_end()
{
    run_scheduled_functions();
    run_scheduled_recurrent_functions();
}

extern "C" bool can_yield()
{
    return true;
}

static std::chrono::steady_clock::time_point last_yield{};

extern "C" void optimistic_yield(uint32_t interval_us)
{
    const auto clock_now = clock_type::now();
    if (clock_now - last_yield > std::chrono::microseconds(interval_us))
    {
        last_yield = clock_now;
        esp_yield();
    }
}

extern "C" void esp_suspend_within_cont() {
    run_scheduled_recurrent_functions();
}

extern "C" void esp_suspend() {
    esp_suspend_within_cont();
}

extern "C" void esp_schedule() { }

extern "C" void esp_yield() {
    esp_schedule();
    esp_suspend();
}

extern "C" void yield()
{
    esp_yield();
}

extern "C" void esp_delay(unsigned long ms)
{
    constexpr auto every_ms = std::chrono::milliseconds(14);  // no events to poll, just some small-ish value

    auto now = clock_type::now();
    const auto end = now + std::chrono::milliseconds(ms);

    decltype(now) wake_at;
    do
    {
      wake_at = now + every_ms;
      esp_yield();
      std::this_thread::sleep_until(wake_at);
      now = clock_type::now();
    } while (now < end);
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
    usleep(us);
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
