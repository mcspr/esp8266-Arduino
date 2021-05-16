#pragma once

#include <stddef.h>
#include <stdint.h>
#include <functional>

struct InterruptInfo {
    uint8_t pin = 0;
    int8_t value = 0;
    uint32_t micro = 0;
};

void attachScheduledInterrupt(uint8_t pin, std::function<void(InterruptInfo)> scheduledIntRoutine, int mode);
