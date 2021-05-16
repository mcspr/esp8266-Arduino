// Helpers to manage interrupts via schedule_function queue

// TODO: isr may not be able to call code in flash when it is not present in ICACHE (functions, methods, ctors, etc. not marked IRAM_ATTR)
// TODO: does this std function copy really work? (same as old bind)... or is this just a happy accident?

#include <FunctionalInterrupt.h>
#include <Schedule.h>
#include "Arduino.h"

using ScheduledInterruptFunc = std::function<void(InterruptInfo)>;

void attachScheduledInterrupt(uint8_t pin, ScheduledInterruptFunc func, int mode)
{
    if (pin < 16) {
        detachInterrupt(pin);
        attachInterrupt(pin, [pin, func = std::move(func)]() {
            InterruptInfo info;
            info.pin = pin;
            info.value = digitalRead(pin);
            info.micro = micros();
            schedule_function([func, info]() {
                func(info);
            });
        }, mode);
    }
}
