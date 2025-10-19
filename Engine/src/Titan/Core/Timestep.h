#pragma once

#include "Titan/PCH.h"

namespace Titan
{

    class TI_API Timestep
    {
    public:
        Timestep(float seconds = 0.0f) : m_Time(seconds) {}

        operator float() const { return m_Time; }

        float GetSeconds() const { return m_Time; }
        float GetMilliseconds() const { return m_Time * 1000.0f; }

        // --- Arithmetic Operators ---
        Timestep operator+(const Timestep& other) const { return Timestep(m_Time + other.m_Time); }
        Timestep operator-(const Timestep& other) const { return Timestep(m_Time - other.m_Time); }
        Timestep operator*(float scalar) const { return Timestep(m_Time * scalar); }
        Timestep operator/(float scalar) const { return Timestep(m_Time / scalar); }

        // --- Compound Assignment Operators ---
        Timestep& operator+=(const Timestep& other)
        {
            m_Time += other.m_Time;
            return *this;
        }

        Timestep& operator-=(const Timestep& other)
        {
            m_Time -= other.m_Time;
            return *this;
        }

        Timestep& operator*=(float scalar)
        {
            m_Time *= scalar;
            return *this;
        }

        Timestep& operator/=(float scalar)
        {
            m_Time /= scalar;
            return *this;
        }

        // --- Comparison Operators ---
        bool operator==(const Timestep& other) const { return m_Time == other.m_Time; }
        bool operator!=(const Timestep& other) const { return m_Time != other.m_Time; }
        bool operator<(const Timestep& other) const { return m_Time < other.m_Time; }
        bool operator>(const Timestep& other) const { return m_Time > other.m_Time; }
        bool operator<=(const Timestep& other) const { return m_Time <= other.m_Time; }
        bool operator>=(const Timestep& other) const { return m_Time >= other.m_Time; }

    private:
        float m_Time;
    };

    inline Timestep operator*(float scalar, const Timestep& ts)
    {
        return Timestep(ts.GetSeconds() * scalar);
    }

    template <glm::length_t L, typename T, glm::qualifier Q>
    inline glm::vec<L, T, Q> operator*(const glm::vec<L, T, Q>& v, const Timestep& ts)
    {
        return v * ts.GetSeconds();
    }

    template <glm::length_t L, typename T, glm::qualifier Q>
    inline glm::vec<L, T, Q> operator*(const Timestep& ts, const glm::vec<L, T, Q>& v)
    {
        return ts.GetSeconds() * v;
    }

    template <glm::length_t L, typename T, glm::qualifier Q>
    inline glm::vec<L, T, Q> operator/(const glm::vec<L, T, Q>& v, const Timestep& ts)
    {
        return v / ts.GetSeconds();
    }

} // namespace Titan
