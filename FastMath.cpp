#include "FastMath.h"
#include <string.h>  // For memcpy, memset

namespace FastMath {

    // Implementation classes that contain the assembly code
    class MatrixImpl {
    public:
        static void multiply4x4(const float* a, const float* b, float* result) {
            __asm__ __volatile__ (
                // Load first row of matrix A into ymm0
                "vmovups (%0), %%ymm0\n\t"
                "vmovups 16(%0), %%ymm1\n\t"
                
                // Load first row of matrix B into ymm2
                "vmovups (%1), %%ymm2\n\t"
                "vmovups 16(%1), %%ymm3\n\t"
                
                // Multiply and accumulate
                "vmulps %%ymm0, %%ymm2, %%ymm4\n\t"
                "vaddps %%ymm4, %%ymm1, %%ymm5\n\t"
                
                // Store result
                "vmovups %%ymm5, (%2)\n\t"
                :
                : "r"(a), "r"(b), "r"(result)
                : "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "memory"
            );
        }

        static void transformVector(const float* matrix, const float* vector, float* result) {
            __asm__ __volatile__ (
                // Load vector into xmm0
                "vmovups (%1), %%xmm0\n\t"
                
                // Load matrix rows
                "vmovups (%0), %%xmm1\n\t"
                "vmovups 16(%0), %%xmm2\n\t"
                "vmovups 32(%0), %%xmm3\n\t"
                "vmovups 48(%0), %%xmm4\n\t"
                
                // Multiply and accumulate
                "vmulps %%xmm0, %%xmm1, %%xmm5\n\t"
                "vaddps %%xmm5, %%xmm2, %%xmm6\n\t"
                
                // Store result
                "vmovups %%xmm6, (%2)\n\t"
                :
                : "r"(matrix), "r"(vector), "r"(result)
                : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "memory"
            );
        }
    };

    class RandomImpl {
    public:
        static void generateRandomBytes(void* dest, size_t size) {
            unsigned char* output = (unsigned char*)dest;
            for (size_t i = 0; i < size; i += 8) {
                uint64_t random;
                __asm__ __volatile__ (
                    "rdrand %0"
                    : "=r"(random)
                    :
                    : "cc"
                );
                memcpy(&output[i], &random, 8);
            }
        }

        static uint64_t xoshiro256(uint64_t* state) {
            uint64_t result;
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
            return result;
        }
    };

    // Matrix4x4 implementation
    MatrixImpl* Matrix4x4::s_impl = NULL;
    RandomImpl* Random::s_impl = NULL;

    Matrix4x4::Matrix4x4() {
        setIdentity();
    }

    Matrix4x4::Matrix4x4(const float* data) {
        memcpy(m_data, data, sizeof(m_data));
    }

    Matrix4x4::~Matrix4x4() {
    }

    void Matrix4x4::setIdentity() {
        static const float identity[] = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        memcpy(m_data, identity, sizeof(m_data));
    }

    void Matrix4x4::setZero() {
        memset(m_data, 0, sizeof(m_data));
    }

    void Matrix4x4::set(const float* data) {
        memcpy(m_data, data, sizeof(m_data));
    }

    void Matrix4x4::multiply(const Matrix4x4& other, Matrix4x4& result) const {
        if (s_impl) {
            MatrixImpl::multiply4x4(m_data, other.m_data, result.m_data);
        }
    }

    void Matrix4x4::transform(const float* vector, float* result) const {
        if (s_impl) {
            MatrixImpl::transformVector(m_data, vector, result);
        }
    }

    bool Matrix4x4::initialize() {
        if (!s_impl) {
            s_impl = new MatrixImpl();
        }
        return true;
    }

    void Matrix4x4::shutdown() {
        delete s_impl;
        s_impl = NULL;
    }

    // Random implementation
    Random::Random(unsigned int seed) {
        RandomImpl::generateRandomBytes(m_state, sizeof(m_state));
    }

    Random::~Random() {
    }

    unsigned int Random::getUInt() {
        unsigned int result;
        if (s_impl) {
            result = (unsigned int)RandomImpl::xoshiro256((uint64_t*)m_state);
        }
        return result;
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

} // namespace FastMath
