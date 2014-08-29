#include "Benchmark.h"

#include <iostream>

/*
 * simple benchmarking
 *
 * text is a description which is used within end()
 */
Benchmark::Benchmark(const std::string &text)
    : m_text(text)
    , m_begin()
    , m_end()
{
    begin();
}

Benchmark::~Benchmark()
{
}

void Benchmark::begin()
{
    m_begin = std::chrono::high_resolution_clock::now();
}

void Benchmark::end()
{
    m_end = std::chrono::high_resolution_clock::now();

    std::cout << m_text << ": "
        << std::chrono::duration_cast<
            std::chrono::milliseconds>(m_end-m_begin).count()
        << " ms" << std::endl;
}

