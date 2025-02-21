#include "FastMath.h"
#include <string.h>  // For memcpy, memset
#include <cstdint>   // For uint64_t
#include <cstdlib>   // For rand()
#include <cmath>     // For std::sqrt - for comparison

#if defined(__x86_64__)
#include <immintrin.h>
#elif defined(__ARM_NEON)
#include <arm_neon.h>
#endif

namespace FastMath {

    class RandomImpl {
    public:
        static void generateRandomBytes(void* dest, size_t size) {
            unsigned char* output = (unsigned char*)dest;
            size_t remaining = size;
            while (remaining > 0) {
                uint64_t random;
                #if defined(__x86_64__)
                __asm__ __volatile__ (
                    "rdrand %0"
                    : "=r"(random)
                    :
                    : "cc"
                );
                #else
                // Fallback for non-x86_64 systems
                random = rand(); // Note: not cryptographically secure
                #endif
                
                // Only copy as many bytes as needed
                size_t to_copy = (remaining < sizeof(random)) ? remaining : sizeof(random);
                memcpy(output, &random, to_copy);
                output += to_copy;
                remaining -= to_copy;
            }
        }

        static uint64_t xoshiro256(uint64_t* state) {
            uint64_t result;
            #if defined(__x86_64__)
            __asm__ __volatile__ (
                // Load state
                "movq (%1), %%rax\n\t"
                "movq 8(%1), %%rbx\n\t"
                "movq 16(%1), %%rcx\n\t"
                "movq 24(%1), %%rdx\n\t"
                
                // xoshiro256** algorithm
                "rolq $23, %%rax\n\t"
                "xorq %%rbx, %%rax\n\t"
                "xorq %%rcx, %%rax\n\t"
                "xorq %%rdx, %%rax\n\t"
                
                // Store result
                "movq %%rax, %0\n\t"
                : "=r"(result)
                : "r"(state)
                : "rax", "rbx", "rcx", "rdx", "memory"
            );
            #else
            // Fallback implementation for non-x86_64 systems
            result = state[0];
            uint64_t t = state[1] << 17;
            state[2] ^= state[0];
            state[3] ^= state[1];
            state[1] ^= state[2];
            state[0] ^= state[3];
            state[2] ^= t;
            state[3] = (state[3] << 45) | (state[3] >> 19);
            #endif
            return result;
        }
    };

    RandomImpl* Random::s_impl = NULL;

    Random::Random(unsigned int seed) {
        if (!s_impl) {
            initialize();  // Ensure impl is initialized
        }
        
        // Initialize state with better seed mixing
        uint64_t* state = (uint64_t*)m_state;
        state[0] = static_cast<uint64_t>(seed) * UINT64_C(0x2545F4914F6CDD1D);
        state[1] = (state[0] ^ UINT64_C(0x1234567890ABCDEF)) + seed;
        state[2] = (state[1] ^ UINT64_C(0xFEDCBA0987654321)) - seed;
        state[3] = (state[2] ^ UINT64_C(0x0F1E2D3C4B5A6978)) * seed;
    }

    Random::~Random() {
    }

    unsigned int Random::getUInt() {
        if (!s_impl) {
            return 0;
        }
        
        // Use xoshiro256 instead of generateRandomBytes for better random numbers
        return (unsigned int)RandomImpl::xoshiro256((uint64_t*)m_state);
    }

    float Random::getFloat() {
        return getUInt() * (1.0f / 4294967296.0f);
    }

    void Random::getFloats(float* array, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            array[i] = getFloat();
        }
    }

    bool Random::initialize() {
        if (!s_impl) {
            s_impl = new RandomImpl();
        }
        return true;
    }

    void Random::shutdown() {
        delete s_impl;
        s_impl = NULL;
    }

    class VectorImpl {
    private:
        static constexpr float half = 0.5f;
        static constexpr float three_halves = 1.5f;

    public:
        static float inv_sqrt(float x) {
            #if defined(__x86_64__)
                // Use SSE intrinsics
                __m128 x_reg = _mm_set_ss(x);
                __m128 y_reg = _mm_rsqrt_ss(x_reg);  // Initial approximation
                
                // One Newton iteration: y = y * (1.5 - 0.5 * x * y * y)
                __m128 half_reg = _mm_set_ss(half);
                __m128 three_halves_reg = _mm_set_ss(three_halves);
                
                __m128 y2 = _mm_mul_ss(y_reg, y_reg);
                __m128 xy2 = _mm_mul_ss(x_reg, y2);
                __m128 hxy2 = _mm_mul_ss(half_reg, xy2);
                __m128 thm = _mm_sub_ss(three_halves_reg, hxy2);
                y_reg = _mm_mul_ss(y_reg, thm);
                
                float result;
                _mm_store_ss(&result, y_reg);
                return result;
            #elif defined(__ARM_NEON)
                float result;
                __asm__ __volatile__ (
                    // Load value and constants
                    "fmov s0, %w1\n\t"         // s0 = x
                    "fmov s2, #0.5\n\t"        // s2 = 0.5
                    "fmov s3, #1.5\n\t"        // s3 = 1.5
                    
                    // Initial estimate
                    "frsqrte s1, s0\n\t"       // s1 = y0 ~= 1/sqrt(x)
                    
                    // First Newton iteration
                    "fmul s4, s1, s1\n\t"      // s4 = y0 * y0
                    "fmul s4, s0, s4\n\t"      // s4 = x * y0 * y0
                    "fmul s4, s2, s4\n\t"      // s4 = 0.5 * x * y0 * y0
                    "fsub s4, s3, s4\n\t"      // s4 = 1.5 - 0.5 * x * y0 * y0
                    "fmul s1, s1, s4\n\t"      // s1 = y1 = y0 * (1.5 - 0.5 * x * y0 * y0)
                    
                    // Second Newton iteration
                    "fmul s4, s1, s1\n\t"      // s4 = y1 * y1
                    "fmul s4, s0, s4\n\t"      // s4 = x * y1 * y1
                    "fmul s4, s2, s4\n\t"      // s4 = 0.5 * x * y1 * y1
                    "fsub s4, s3, s4\n\t"      // s4 = 1.5 - 0.5 * x * y1 * y1
                    "fmul s0, s1, s4\n\t"      // s0 = y2 = y1 * (1.5 - 0.5 * x * y1 * y1)
                    
                    // Store result
                    "fmov %w0, s0\n\t"
                    : "=w"(result)
                    : "w"(x)
                    : "s0", "s1", "s2", "s3", "s4"
                );
                return result;
            #else
                // Fallback to standard implementation
                return 1.0f / std::sqrt(x);
            #endif
        }
    };

    // Static member initialization
    VectorImpl* Vector::s_impl = NULL;

    // Vector member function implementations
    float Vector::inv_sqrt(float x) {
        if (s_impl) {
            return VectorImpl::inv_sqrt(x);
        }
        return 1.0f / std::sqrt(x);
    }

    bool Vector::initialize() {
        if (!s_impl) {
            s_impl = new VectorImpl();
        }
        return true;
    }

    void Vector::shutdown() {
        delete s_impl;
        s_impl = NULL;
    }

} // namespace FastMath
