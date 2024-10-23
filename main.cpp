
/**
 * Inspired by https://blog.bearcats.nl/accurate-sleep-function/
 */

#include <stdint.h>
#include <thread>
#include <chrono>
#include <iostream>
#include <fstream>
#include <functional>

#include "precise-sleep-until.h"

const uint64_t ns_per_ms = 1e6;

using test_function = std::function<void (std::chrono::time_point<std::chrono::steady_clock>)>;

using result_function = std::function<void (uint64_t target_ns, uint64_t actual_ns)>;

// Measure the difference in sleep duration of the test_function vs. the target 'ns_duration'
void test_duration(int64_t ns_duration,
                   test_function tf,
                   double &avg_difference_ns,
                   result_function result,
                   bool output_details = false)
{
    const uint64_t ns_per_ms = 1e6;

    uint64_t difference_sum_ns = 0;
    uint64_t count = 0;

    for(auto i = 0; i < 250; i++)
    {
        auto start = std::chrono::high_resolution_clock::now();
        auto until_time = start + std::chrono::nanoseconds(ns_duration);

        tf(until_time);

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = end - start;

        if(output_details)
        {
            printf("duration %fms\n", ((double)duration.count() / ns_per_ms));
        }

        result(ns_duration, duration.count());

        difference_sum_ns += labs(ns_duration - duration.count());
        count++;

        until_time += std::chrono::nanoseconds(ns_duration);
    }

    avg_difference_ns = (double)difference_sum_ns / count;
}

int main()
{
    std::ofstream csv_output("results.csv");

    // NOTE: there can't be any spaces here in the CSV header line
    csv_output << "\"sleep_type\",\"target_ns\",\"actual_ns\"" << std::endl;

    double average_difference_ns;

    uint64_t ns_duration[] = { 1000, 10000, 100000, 1 * ns_per_ms, 2 * ns_per_ms, 5 * ns_per_ms, 10 * ns_per_ms,
                                20 * ns_per_ms, 50 * ns_per_ms, 100 * ns_per_ms };

    for(auto duration : ns_duration)
    {
        test_duration(duration,
            [](std::chrono::time_point<std::chrono::steady_clock> until_time) {
                preciseSleepUntil(until_time);
            },
            average_difference_ns,
           [&csv_output](uint64_t target, uint64_t actual) {
               csv_output << "\"precise\"," << target << "," << actual << std::endl;
           }
        );

        printf("target %lld ns (%f ms), error %f%%, average diff %f ns, %f ms\n",
            duration, (double)duration / ns_per_ms,
            (average_difference_ns / duration) * 100.0,
            average_difference_ns, average_difference_ns / ns_per_ms);
    }

    printf("\n\n\n");

    for(auto duration : ns_duration)
    {
        test_duration(duration,
            [](std::chrono::time_point<std::chrono::steady_clock> until_time) {
                std::this_thread::sleep_until(until_time);
            },
            average_difference_ns,
            [&csv_output](uint64_t target, uint64_t actual) {
                csv_output << "\"sleep_until\"," << target << "," << actual << std::endl;
            }
        );

        printf("target %lld ns (%f ms), error %f%%,  average diff %f ns, %f ms\n",
            duration, (double)duration / ns_per_ms,
            (average_difference_ns / duration) * 100.0,
            average_difference_ns, average_difference_ns / ns_per_ms);
    }

    printf("\n\n\n");
}

