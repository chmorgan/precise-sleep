#pragma once

#include <chrono>
#include <math.h>

template <class T>
void preciseSleepUntil(std::chrono::time_point<T> until_time)
{
    const uint64_t ns_per_ms = 1e6;

    // how long is it until the time?
    auto now = std::chrono::steady_clock::now();
    auto sleep_duration_ns = (until_time - now).count();

    // the shortest time the thread can sleep accurately
    static double thread_sleep_estimate_ns = 5 * ns_per_ms;

    static double mean = 5;
    static double m2 = 0;
    static int64_t count = 1;

    while(sleep_duration_ns > thread_sleep_estimate_ns)
    {
        auto start = std::chrono::steady_clock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        auto end = std::chrono::steady_clock::now();

        double observed_ns_sleep = (end - start).count();
        sleep_duration_ns -= observed_ns_sleep;

        ++count;
        double delta = observed_ns_sleep - mean;
        mean += delta / count;
        m2   += delta * (observed_ns_sleep - mean);
        double stddev = sqrt(m2 / (count - 1));
        thread_sleep_estimate_ns = mean + stddev;
    }

    // spin lock until the exact time
    while (std::chrono::steady_clock::now() < until_time);
}
