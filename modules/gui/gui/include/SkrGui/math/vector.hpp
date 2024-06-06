#pragma once
#include "SkrGui/fwd_config.hpp"

// vector types
namespace skr::gui
{
struct Vector3 {
    // ctor
    inline constexpr Vector3() SKR_NOEXCEPT
        : x(0.0f),
          y(0.0f),
          z(0.0f)
    {
    }
    inline constexpr Vector3(float all) SKR_NOEXCEPT
        : x(all),
          y(all),
          z(all)
    {
    }
    inline constexpr Vector3(float x, float y, float z) SKR_NOEXCEPT
        : x(x),
          y(y),
          z(z)
    {
    }

    // assign
    inline constexpr Vector3& operator=(float all) SKR_NOEXCEPT
    {
        x = all;
        y = all;
        z = all;
        return *this;
    }

    // copy & move & assign & move assign
    inline constexpr Vector3(const Vector3& other) SKR_NOEXCEPT            = default;
    inline constexpr Vector3(Vector3&& other) SKR_NOEXCEPT                 = default;
    inline constexpr Vector3& operator=(const Vector3& other) SKR_NOEXCEPT = default;
    inline constexpr Vector3& operator=(Vector3&& other) SKR_NOEXCEPT      = default;

    // compare
    inline constexpr bool operator==(const Vector3& other) const SKR_NOEXCEPT { return x == other.x && y == other.y && z == other.z; }
    inline constexpr bool operator!=(const Vector3& other) const SKR_NOEXCEPT { return !(*this == other); }

    // arithmetic ops
    inline Vector3 operator-() const SKR_NOEXCEPT { return Vector3(-x, -y, -z); }

    friend inline Vector3 operator+(const Vector3& lhs, float rhs) SKR_NOEXCEPT { return Vector3(lhs.x + rhs, lhs.y + rhs, lhs.z + rhs); }
    friend inline Vector3 operator-(const Vector3& lhs, float rhs) SKR_NOEXCEPT { return Vector3(lhs.x - rhs, lhs.y - rhs, lhs.z - rhs); }
    friend inline Vector3 operator*(const Vector3& lhs, float rhs) SKR_NOEXCEPT { return Vector3(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs); }
    friend inline Vector3 operator/(const Vector3& lhs, float rhs) SKR_NOEXCEPT { return Vector3(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs); }
    friend inline Vector3 operator%(const Vector3& lhs, float rhs) SKR_NOEXCEPT { return Vector3(::std::fmod(lhs.x, rhs), ::std::fmod(lhs.y, rhs), ::std::fmod(lhs.z, rhs)); }

    friend inline Vector3 operator+(float lhs, const Vector3& rhs) SKR_NOEXCEPT { return Vector3(lhs + rhs.x, lhs + rhs.y, lhs + rhs.z); }
    friend inline Vector3 operator-(float lhs, const Vector3& rhs) SKR_NOEXCEPT { return Vector3(lhs - rhs.x, lhs - rhs.y, lhs - rhs.z); }
    friend inline Vector3 operator*(float lhs, const Vector3& rhs) SKR_NOEXCEPT { return Vector3(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z); }
    friend inline Vector3 operator/(float lhs, const Vector3& rhs) SKR_NOEXCEPT { return Vector3(lhs / rhs.x, lhs / rhs.y, lhs / rhs.z); }
    friend inline Vector3 operator%(float lhs, const Vector3& rhs) SKR_NOEXCEPT { return Vector3(::std::fmod(lhs, rhs.x), ::std::fmod(lhs, rhs.y), ::std::fmod(lhs, rhs.z)); }

    inline Vector3 operator+(const Vector3& other) const SKR_NOEXCEPT { return Vector3(x + other.x, y + other.y, z + other.z); }
    inline Vector3 operator-(const Vector3& other) const SKR_NOEXCEPT { return Vector3(x - other.x, y - other.y, z - other.z); }
    inline Vector3 operator*(const Vector3& other) const SKR_NOEXCEPT { return Vector3(x * other.x, y * other.y, z * other.z); }
    inline Vector3 operator/(const Vector3& other) const SKR_NOEXCEPT { return Vector3(x / other.x, y / other.y, z / other.z); }
    inline Vector3 operator%(const Vector3& other) const SKR_NOEXCEPT { return Vector3(::std::fmod(x, other.x), ::std::fmod(y, other.y), ::std::fmod(z, other.z)); }

    inline Vector3 operator+=(float rhs) SKR_NOEXCEPT { return *this = *this + rhs; }
    inline Vector3 operator-=(float rhs) SKR_NOEXCEPT { return *this = *this - rhs; }
    inline Vector3 operator*=(float rhs) SKR_NOEXCEPT { return *this = *this * rhs; }
    inline Vector3 operator/=(float rhs) SKR_NOEXCEPT { return *this = *this / rhs; }
    inline Vector3 operator%=(float rhs) SKR_NOEXCEPT { return *this = *this % rhs; }

    inline Vector3 operator+=(const Vector3& other) SKR_NOEXCEPT { return *this = *this + other; }
    inline Vector3 operator-=(const Vector3& other) SKR_NOEXCEPT { return *this = *this - other; }
    inline Vector3 operator*=(const Vector3& other) SKR_NOEXCEPT { return *this = *this * other; }
    inline Vector3 operator/=(const Vector3& other) SKR_NOEXCEPT { return *this = *this / other; }
    inline Vector3 operator%=(const Vector3& other) SKR_NOEXCEPT { return *this = *this % other; }

    // visitor
    inline float& operator[](size_t n) SKR_NOEXCEPT
    {
        SKR_ASSERT(n > 0 && n < 3 && "Vector3 index out of range");
        return (&x)[n];
    }
    inline float operator[](size_t n) const SKR_NOEXCEPT
    {
        SKR_ASSERT(n > 0 && n < 3 && "Vector3 index out of range");
        return (&x)[n];
    }

    // hash
    inline static constexpr size_t _skr_hash(const Vector3& v) SKR_NOEXCEPT
    {
        auto hasher = ::skr::Hash<float>{};
        auto hash   = hasher(v.x);
        hash        = ::skr::hash_combine(hash, hasher(v.y));
        hash        = ::skr::hash_combine(hash, hasher(v.z));
        return hash;
    }

    float x, y, z;
};

struct Vector4 {
    // ctor
    inline constexpr Vector4() SKR_NOEXCEPT
        : x(0.0f),
          y(0.0f),
          z(0.0f),
          w(0.0f)
    {
    }
    inline constexpr Vector4(float all) SKR_NOEXCEPT
        : x(all),
          y(all),
          z(all),
          w(all)
    {
    }
    inline constexpr Vector4(float x, float y, float z, float w) SKR_NOEXCEPT
        : x(x),
          y(y),
          z(z),
          w(w)
    {
    }

    // copy & move & assign & move assign
    inline constexpr Vector4(const Vector4& other) SKR_NOEXCEPT            = default;
    inline constexpr Vector4(Vector4&& other) SKR_NOEXCEPT                 = default;
    inline constexpr Vector4& operator=(const Vector4& other) SKR_NOEXCEPT = default;
    inline constexpr Vector4& operator=(Vector4&& other) SKR_NOEXCEPT      = default;

    // integrate to vector3
    inline constexpr Vector4(const Vector3& xyz, float w) SKR_NOEXCEPT
        : x(xyz.x),
          y(xyz.y),
          z(xyz.z),
          w(w)
    {
    }
    inline constexpr operator Vector3()
    {
        return Vector3(x, y, z);
    }

    // compare
    inline constexpr bool operator==(const Vector4& other) const SKR_NOEXCEPT { return x == other.x && y == other.y && z == other.z && w == other.w; }
    inline constexpr bool operator!=(const Vector4& other) const SKR_NOEXCEPT { return !(*this == other); }

    // arithmetic ops
    inline Vector4 operator-() const SKR_NOEXCEPT { return Vector4(-x, -y, -z, -w); }

    friend inline Vector4 operator+(const Vector4& lhs, float rhs) SKR_NOEXCEPT { return Vector4(lhs.x + rhs, lhs.y + rhs, lhs.z + rhs, lhs.w + rhs); }
    friend inline Vector4 operator-(const Vector4& lhs, float rhs) SKR_NOEXCEPT { return Vector4(lhs.x - rhs, lhs.y - rhs, lhs.z - rhs, lhs.w - rhs); }
    friend inline Vector4 operator*(const Vector4& lhs, float rhs) SKR_NOEXCEPT { return Vector4(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs); }
    friend inline Vector4 operator/(const Vector4& lhs, float rhs) SKR_NOEXCEPT { return Vector4(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs, lhs.w / rhs); }
    friend inline Vector4 operator%(const Vector4& lhs, float rhs) SKR_NOEXCEPT { return Vector4(::std::fmod(lhs.x, rhs), ::std::fmod(lhs.y, rhs), ::std::fmod(lhs.z, rhs), ::std::fmod(lhs.w, rhs)); }

    friend inline Vector4 operator+(float lhs, const Vector4& rhs) SKR_NOEXCEPT { return Vector4(lhs + rhs.x, lhs + rhs.y, lhs + rhs.z, lhs + rhs.w); }
    friend inline Vector4 operator-(float lhs, const Vector4& rhs) SKR_NOEXCEPT { return Vector4(lhs - rhs.x, lhs - rhs.y, lhs - rhs.z, lhs - rhs.w); }
    friend inline Vector4 operator*(float lhs, const Vector4& rhs) SKR_NOEXCEPT { return Vector4(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w); }
    friend inline Vector4 operator/(float lhs, const Vector4& rhs) SKR_NOEXCEPT { return Vector4(lhs / rhs.x, lhs / rhs.y, lhs / rhs.z, lhs / rhs.w); }
    friend inline Vector4 operator%(float lhs, const Vector4& rhs) SKR_NOEXCEPT { return Vector4(::std::fmod(lhs, rhs.x), ::std::fmod(lhs, rhs.y), ::std::fmod(lhs, rhs.z), ::std::fmod(lhs, rhs.w)); }

    inline Vector4 operator+(const Vector4& other) const SKR_NOEXCEPT { return Vector4(x + other.x, y + other.y, z + other.z, w + other.w); }
    inline Vector4 operator-(const Vector4& other) const SKR_NOEXCEPT { return Vector4(x - other.x, y - other.y, z - other.z, w - other.w); }
    inline Vector4 operator*(const Vector4& other) const SKR_NOEXCEPT { return Vector4(x * other.x, y * other.y, z * other.z, w * other.w); }
    inline Vector4 operator/(const Vector4& other) const SKR_NOEXCEPT { return Vector4(x / other.x, y / other.y, z / other.z, w / other.w); }
    inline Vector4 operator%(const Vector4& other) const SKR_NOEXCEPT { return Vector4(::std::fmod(x, other.x), ::std::fmod(y, other.y), ::std::fmod(z, other.z), ::std::fmod(w, other.w)); }

    inline Vector4 operator+=(float rhs) SKR_NOEXCEPT { return *this = *this + rhs; }
    inline Vector4 operator-=(float rhs) SKR_NOEXCEPT { return *this = *this - rhs; }
    inline Vector4 operator*=(float rhs) SKR_NOEXCEPT { return *this = *this * rhs; }
    inline Vector4 operator/=(float rhs) SKR_NOEXCEPT { return *this = *this / rhs; }
    inline Vector4 operator%=(float rhs) SKR_NOEXCEPT { return *this = *this % rhs; }

    inline Vector4 operator+=(const Vector4& other) SKR_NOEXCEPT { return *this = *this + other; }
    inline Vector4 operator-=(const Vector4& other) SKR_NOEXCEPT { return *this = *this - other; }
    inline Vector4 operator*=(const Vector4& other) SKR_NOEXCEPT { return *this = *this * other; }
    inline Vector4 operator/=(const Vector4& other) SKR_NOEXCEPT { return *this = *this / other; }
    inline Vector4 operator%=(const Vector4& other) SKR_NOEXCEPT { return *this = *this % other; }

    // visitor
    inline float& operator[](size_t n) SKR_NOEXCEPT
    {
        SKR_ASSERT(n > 0 && n < 4 && "Vector4 index out of range");
        return (&x)[n];
    }
    inline float operator[](size_t n) const SKR_NOEXCEPT
    {
        SKR_ASSERT(n > 0 && n < 4 && "Vector4 index out of range");
        return (&x)[n];
    }

    // hash
    inline static size_t _skr_hash(const Vector4& v) SKR_NOEXCEPT
    {
        auto hasher = ::skr::Hash<float>{};
        auto hash   = hasher(v.x);
        hash        = ::skr::hash_combine(hash, hasher(v.y));
        hash        = ::skr::hash_combine(hash, hasher(v.z));
        hash        = ::skr::hash_combine(hash, hasher(v.w));
        return hash;
    }

    float x, y, z, w;
};
} // namespace skr::gui

// vector ops
namespace skr::gui
{
// length
// length sqrt
// length of
// normalize
// radians to
// dot
// cross
// reflect
// abs
// clamp
// floor
// ceil
// trunc
// round
// distance
// distance sqrt
// min
// max
// is infinite
// is nan

} // namespace skr::gui