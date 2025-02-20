#ifndef FAST_MATH_H
#define FAST_MATH_H

#include <cstddef>

namespace FastMath {

    // Forward declaration of implementation class
    class MatrixImpl;
    class RandomImpl;

    // 4x4 Matrix class
    class Matrix4x4 {
    private:
        float m_data[16];
        static MatrixImpl* s_impl;  // Static implementation pointer

    public:
        Matrix4x4();
        Matrix4x4(const float* data);
        ~Matrix4x4();

        // Basic operations
        void setIdentity();
        void setZero();
        void set(const float* data);
        const float* getData() const { return m_data; }
        float* getData() { return m_data; }

        // Assembly-optimized operations
        void multiply(const Matrix4x4& other, Matrix4x4& result) const;
        void transform(const float* vector, float* result) const;

        // Static initialization/cleanup
        static bool initialize();
        static void shutdown();
    };

    // Random number generator
    class Random {
    private:
        unsigned char m_state[32];  // 256-bit state for xoshiro256**
        static RandomImpl* s_impl;  // Static implementation pointer

    public:
        Random(unsigned int seed = 12345);
        ~Random();

        // Generate random numbers
        unsigned int getUInt();
        float getFloat();  // Returns [0.0f, 1.0f)
        void getFloats(float* array, size_t count);

        // Static initialization/cleanup
        static bool initialize();
        static void shutdown();
    };

} // namespace FastMath

#endif // FAST_MATH_H
