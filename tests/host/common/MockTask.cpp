/*
 Arduino emulator task internals
 Copyright (c) 2018 david gauchard. All rights reserved.

 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal with the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 - Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimers.

 - Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimers in the
   documentation and/or other materials provided with the distribution.

 - The names of its contributors may not be used to endorse or promote
   products derived from this Software without specific prior written
   permission.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 THE CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS WITH THE SOFTWARE.
*/

#include <Arduino.h>
#include <Schedule.h>

#include <chrono>
#include <mutex>
#include <thread>
#include <functional>

#include <ets_sys.h>
#include <sched.h>

namespace
{

// TODO ets_task(), ets_post(), etc.? afaik, no real messages are passed between tasks, just execution requests
// TODO only -std=c++20 has nice barrier api and jthread. using semaphore.h for now and plain threads
//      (i.e. helping out with 'stop conditon', and a much nicer task waiting api)

// by default, main thread is running sdk-related tasks and setup() / loop() is scheduled right after
// - every sdk task loop iteration, notify user task when it is scheduled via esp_schedule()
//   and wait for the specified thread token cv until the next sdk task iteration
// - when loop() is ready to yield via esp_suspend() (either at the end of the func, or via yield()),
//   notify the second thread token cv to repeat the conditions above

std::thread user_task;

std::atomic<bool> user_task_is_done { false };
std::atomic<bool> scheduled { false };

sem_t sdk_token {};
sem_t user_token {};

void notify(sem_t& token)
{
    sem_post(&token);
}

void wait(sem_t& token)
{
    sem_wait(&token);
}

ETSTimer delay_timer;

void mock_task_wrapper()
{
    std::once_flag setup_done;

    for (;;)
    {
        wait(user_token);

        std::call_once(setup_done, setup);
        loop();
        loop_end();

        esp_schedule();
        notify(sdk_token);

        if (user_task_is_done)
        {
            break;
        }
    }
}

}  // namespace

extern "C" bool can_yield()
{
    return std::this_thread::get_id() == user_task.get_id();
}

extern "C" void esp_suspend()
{
    notify(sdk_token);
    wait(user_token);
}

extern "C" void esp_schedule()
{
    scheduled = true;
}

extern "C" void esp_yield()
{
    esp_schedule();
    esp_suspend();
}

extern "C" void esp_delay(unsigned long ms)
{
    if (ms)
    {
        ets_timer_setfn(
            &delay_timer,
            [](void*)
            {
                esp_schedule();
            },
            nullptr);
        ets_timer_arm_new(&delay_timer, ms, 0, 1);
    }
    else
    {
        esp_schedule();
    }

    esp_suspend();
    if (ms)
    {
        ets_timer_disarm(&delay_timer);
    }
}

void mock_stop_task()
{
    user_task_is_done = true;
    notify(sdk_token);
    notify(user_token);
}

void mock_loop_task(void (*system_task)(), std::chrono::milliseconds interval,
                    const bool& user_exit)
{
    sem_init(&user_token, 0, 0);
    sem_init(&sdk_token, 0, 0);

    user_task = std::thread(mock_task_wrapper);

    esp_schedule();
    for (;;)
    {
        system_task();
        if (interval.count() > 0)
        {
            std::this_thread::sleep_for(interval);
        }

        if (scheduled)
        {
            scheduled = false;
            notify(user_token);
            wait(sdk_token);
        }

        if (user_exit)
        {
            break;
        }
    }

    mock_stop_all();
}
