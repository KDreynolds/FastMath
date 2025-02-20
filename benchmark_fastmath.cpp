#include "FastMath.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <algorithm>
#include <functional>

// Helper for timing measurements
class Timer {
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;
    TimePoint start;
public:
    Timer() : start(Clock::now()) {}
    
    double elapsed() {
        auto end = Clock::now();
        return std::chrono::duration<double, std::milli>(end - start).count();
    }
};

struct BenchmarkResult {
    double min;
    double max;
    double avg;
    float sum;  // Keep the sum to verify consistency
};

BenchmarkResult runBenchmark(int iterations, const std::function<float()>& operation) {
    std::vector<double> times;
    const int samples = 5;
    float final_sum = 0;
    
    for (int sample = 0; sample < samples; sample++) {
        // Cool down between samples
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        Timer timer;
        final_sum = operation();  // Call the provided function
        times.push_back(timer.elapsed());
    }
    
    // Calculate statistics
    double min = *std::min_element(times.begin(), times.end());
    double max = *std::max_element(times.begin(), times.end());
    double avg = 0;
    for (double t : times) avg += t;
    avg /= times.size();
    
    return {min, max, avg, final_sum};
}

void benchmarkRandomGeneration(int iterations = 1000000) {
    std::cout << "\nBenchmarking Random Number Generation (" << iterations << " iterations):" << std::endl;
    
    FastMath::Random::initialize();
    
    // Warm up the CPU
    for (volatile int i = 0; i < 1000000; i++) {}
    
    // Run optimized benchmark
    auto optimized = runBenchmark(iterations, [iterations]() {
        FastMath::Random rng(12345);
        float sum = 0.0f;
        for (int i = 0; i < iterations; i++) {
            sum += rng.getFloat();
        }
        return sum;
    });
    
    std::cout << "Optimized: min=" << optimized.min << "ms, max=" << optimized.max 
              << "ms, avg=" << optimized.avg << "ms (sum=" << optimized.sum << ")" << std::endl;
    
    // Cool down
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Run standard benchmark
    auto standard = runBenchmark(iterations, [iterations]() {
        float sum = 0.0f;
        for (int i = 0; i < iterations; i++) {
            sum += static_cast<float>(rand()) / RAND_MAX;
        }
        return sum;
    });
    
    std::cout << "Standard:  min=" << standard.min << "ms, max=" << standard.max 
              << "ms, avg=" << standard.avg << "ms (sum=" << standard.sum << ")" << std::endl;
    
    FastMath::Random::shutdown();
}

void benchmarkInvSqrt(int iterations = 10000000) {
    std::cout << "\nBenchmarking Inverse Square Root (" << iterations << " iterations):" << std::endl;
    
    FastMath::Vector::initialize();
    
    // Create test data
    std::vector<float> test_values(1000);
    for (size_t i = 0; i < test_values.size(); i++) {
        test_values[i] = 1.0f + static_cast<float>(i) / 100.0f;
    }
    
    // Run optimized benchmark
    auto optimized = [&]() {
        float sum = 0.0f;
        for (int i = 0; i < iterations; i++) {
            sum += FastMath::Vector::inv_sqrt(test_values[i % test_values.size()]);
        }
        return sum;
    };
    
    // Run standard benchmark
    auto standard = [&]() {
        float sum = 0.0f;
        for (int i = 0; i < iterations; i++) {
            sum += 1.0f / std::sqrt(test_values[i % test_values.size()]);
        }
        return sum;
    };
    
    BenchmarkResult opt_result = runBenchmark(iterations, optimized);
    std::cout << "Optimized: min=" << opt_result.min << "ms, max=" << opt_result.max 
              << "ms, avg=" << opt_result.avg << "ms (sum=" << opt_result.sum << ")" << std::endl;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    BenchmarkResult std_result = runBenchmark(iterations, standard);
    std::cout << "Standard:  min=" << std_result.min << "ms, max=" << std_result.max 
              << "ms, avg=" << std_result.avg << "ms (sum=" << std_result.sum << ")" << std::endl;
    
    FastMath::Vector::shutdown();
}

int main() {
    benchmarkRandomGeneration();
    benchmarkInvSqrt();
    return 0;
} 