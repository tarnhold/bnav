#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <chrono>
#include <cstdint>
#include <string>

#define BENCH_LOOP(count) for (uint64_t bench_i = 0; bench_i < count; ++bench_i)

/**
 * @brief The Benchmark class
 *
 * Usage example:
 *
 * Benchmark bench("asciireader");
 * BENCH_LOOP(10000)
 * {
 *     // do something
 * }
 * bench.end();
 */

class Benchmark
{
    std::string m_text;
    std::chrono::high_resolution_clock::time_point m_begin;
    std::chrono::high_resolution_clock::time_point m_end;

public:
    Benchmark(const std::string &text);
    ~Benchmark();

    void begin();
    void end();

};

#endif // BENCHMARK_H
