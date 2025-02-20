#include "FastMath.h"
#include <iostream>
#include <cassert>
#include <cmath>

void testRandom() {
    std::cout << "Testing random number generation..." << std::endl;
    
    FastMath::Random::initialize();
    FastMath::Random rng(12345);
    
    // Test range of random floats
    for (int i = 0; i < 1000; i++) {
        float r = rng.getFloat();
        assert(r >= 0.0f && r < 1.0f);
    }
    
    // Test array generation
    float randomArray[10];
    rng.getFloats(randomArray, 10);
    for (int i = 0; i < 10; i++) {
        assert(randomArray[i] >= 0.0f && randomArray[i] < 1.0f);
    }
    
    std::cout << "Random number generation test passed!" << std::endl;
    
    FastMath::Random::shutdown();
}

void testInvSqrt() {
    std::cout << "Testing inverse square root..." << std::endl;
    
    FastMath::Vector::initialize();
    
    // Test accuracy against standard library
    const float test_values[] = { 1.0f, 2.0f, 4.0f, 16.0f, 100.0f, 10000.0f };
    const float max_error = 0.002f;  // Allow 0.2% error
    
    for (float x : test_values) {
        float fast_result = FastMath::Vector::inv_sqrt(x);
        float std_result = 1.0f / std::sqrt(x);
        float error = std::abs(fast_result - std_result) / std_result;
        
        // Print values for debugging
        std::cout << "x = " << x 
                  << ", fast = " << fast_result 
                  << ", std = " << std_result 
                  << ", error = " << (error * 100.0f) << "%" << std::endl;
        
        assert(error < max_error);
        assert(std::isfinite(fast_result));
    }
    
    std::cout << "Inverse square root test passed!" << std::endl;
    
    FastMath::Vector::shutdown();
}

int main() {
    testRandom();
    testInvSqrt();
    std::cout << "\nAll tests passed successfully!" << std::endl;
    return 0;
} 