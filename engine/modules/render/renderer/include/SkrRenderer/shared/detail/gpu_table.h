#pragma once
#ifdef __CPPSL__
    #include "table_accessors.hxx" // IWYU pragma: export
#else
    #include "table_accessors.hpp" // IWYU pragma: export
#endif

namespace skr::gpu
{

static_assert(sizeof(Row<float>) == 3 * sizeof(uint32_t));
static_assert(sizeof(FixedRange<float, 1>) == 3 * sizeof(uint32_t));
static_assert(sizeof(Range<float>) == 4 * sizeof(uint32_t));

#define GPU_DATABLOCK_SCALAR(T, S)\
template <> struct GPUDatablock<T> {\
    inline static constexpr uint32_t Size = 4;\
    GPUDatablock<T>() = default;\
    GPUDatablock<T>(const T& v)\
        : u0(v)\
    { static_assert(sizeof(S) == sizeof(float), "PRIMITIVE SCALAR TYPES MUST BE 32BIT TYPES"); }\
    operator T() const { return T(u0); }\
private:\
    S u0 = 0;\
};

#define GPU_DATABLOCK_VEC2(T, S)\
template <> struct GPUDatablock<T##2> {\
    inline static constexpr uint32_t Size = 4 * 2;\
    GPUDatablock<T##2>() = default;\
    GPUDatablock<T##2>(const T##2& v)\
        : u0(v.x), u1(v.y)\
    { static_assert(sizeof(S) == sizeof(float), "PRIMITIVE VEC2 TYPES MUST BE 32BIT TYPES"); }\
    operator T##2() const { return T##2(u0, u1); }\
private:\
    S u0 = 0, u1 = 0;\
};

#define GPU_DATABLOCK_VEC3(T, S)\
template <> struct GPUDatablock<T##3> {\
    inline static constexpr uint32_t Size = 4 * 3;\
    GPUDatablock<T##3>() = default;\
    GPUDatablock<T##3>(const T##3& v)\
        : u0(v.x), u1(v.y), u2(v.z)\
    { static_assert(sizeof(S) == sizeof(float), "PRIMITIVE VEC3 TYPES MUST BE 32BIT TYPES"); }\
    operator T##3() const { return T##3(u0, u1, u2); }\
private:\
    S u0 = 0, u1 = 0, u2 = 0;\
};

#define GPU_DATABLOCK_VEC4(T, S)\
template <> struct GPUDatablock<T##4> {\
    inline static constexpr uint32_t Size = 4 * 4;\
    GPUDatablock<T##4>() = default;\
    GPUDatablock<T##4>(const T##4& v)\
        : u0(v.x), u1(v.y), u2(v.z), u3(v.w)\
    { static_assert(sizeof(S) == sizeof(float), "PRIMITIVE VEC4 TYPES MUST BE 32BIT TYPES"); }\
    operator T##4() const { return T##4(u0, u1, u2, u3); }\
private:\
    S u0 = 0, u1 = 0, u2 = 0, u3 = 0;\
};

#define GPU_DATABLOCK_MAT3(T)\
template <> struct GPUDatablock<T##3x3> {\
    inline static constexpr uint32_t Size = 3 * 3 * 4;\
    GPUDatablock<T##3x3>() = default;\
    GPUDatablock<T##3x3>(const T##3x3& v)\
        : u0(v[0]), u1(v[1]), u2(v[2])\
    { static_assert(sizeof(T) == sizeof(float), "PRIMITIVE MAT3 TYPES MUST BE 32BIT TYPES"); }\
    operator T##3x3() const { return T##3x3(u0, u1, u2); }\
private:\
    GPUDatablock<T##4> u0, u1, u2;\
};

#define GPU_DATABLOCK_MAT4(T)\
template <> struct GPUDatablock<T##4x4> {\
    inline static constexpr uint32_t Size = 4 * 4 * 4;\
    GPUDatablock<T##4x4>() = default;\
    GPUDatablock<T##4x4>(const T##4x4& v)\
        : u0(v[0]), u1(v[1]), u2(v[2]), u3(v[3])\
    { static_assert(sizeof(T) == sizeof(float), "PRIMITIVE MAT4 TYPES MUST BE 32BIT TYPES"); }\
    operator T##4x4() const { return T##4x4(u0, u1, u2, u3); }\
private:\
    GPUDatablock<T##4> u0, u1, u2, u3;\
};

using uint = uint32_t;
GPU_DATABLOCK_SCALAR(bool, uint);
GPU_DATABLOCK_VEC2(bool, uint);
GPU_DATABLOCK_VEC3(bool, uint);
GPU_DATABLOCK_VEC4(bool, uint);

GPU_DATABLOCK_SCALAR(int, int);
GPU_DATABLOCK_VEC2(int, int);
GPU_DATABLOCK_VEC3(int, int);
GPU_DATABLOCK_VEC4(int, int);

GPU_DATABLOCK_SCALAR(uint, uint);
GPU_DATABLOCK_VEC2(uint, uint);
GPU_DATABLOCK_VEC3(uint, uint);
GPU_DATABLOCK_VEC4(uint, uint);

GPU_DATABLOCK_SCALAR(float, float);
GPU_DATABLOCK_VEC2(float, float);
GPU_DATABLOCK_VEC3(float, float);
GPU_DATABLOCK_VEC4(float, float);

// TODO: 4x4
// GPU_DATABLOCK_MAT3(float);
GPU_DATABLOCK_MAT4(float);

template <typename T, uint32_t N>
struct GPUDatablock<skr::gpu_array<T, N>>
{
public:
    inline static constexpr uint32_t Size = sizeof(GPUDatablock<T>) * N;

    GPUDatablock() = default;
    GPUDatablock<skr::gpu_array<T, N>>(const skr::gpu_array<T, N>& r)
    {
        for (uint32_t i = 0; i < N; i++)
        {
            data[i] = r[i];
        }
    }

    operator skr::gpu_array<T, N>() const 
    { 
        skr::gpu_array<T, N> r; 
        for (uint32_t i = 0; i < N; i++)
        {
            r[i] = data[i];
        }
        return r; 
    }

private:
    skr::gpu_array<GPUDatablock<T>, N> data;
};

template <typename T>
struct GPUDatablock<Row<T>>
{
public:
    using ThisType = GPUDatablock<Row<T>>;
    using InnerType = Row<T>;
    inline static constexpr uint32_t Size = 4 + 4 + 4;

    GPUDatablock() = default;
    GPUDatablock<Row<T>>(const InnerType& row)
        : instance_index(row._instance_index), buffer_offset(row._buffer_offset), bindless_index(row._bindless_index)
    {

    }

    operator Row<T>() const 
    { 
        InnerType row; 
        row._instance_index = instance_index;
        row._buffer_offset = buffer_offset;
        row._bindless_index = bindless_index;
        return row; 
    }

private:
    uint32_t instance_index = 0;
    uint32_t buffer_offset = 0;
    uint32_t bindless_index = ~0;
};

template <typename T, uint32_t N>
struct GPUDatablock<FixedRange<T, N>>
{
public:
    using ThisType = GPUDatablock<FixedRange<T, N>>;
    using InnerType = FixedRange<T, N>;
    inline static constexpr uint32_t Size = 4 + 4 + 4;

    GPUDatablock() = default;
    GPUDatablock<FixedRange<T, N>>(const InnerType& range)
        : bindless_index(range._bindless_index), buffer_offset(range._buffer_offset), first_instance(range._first_instance)
    {

    }

    operator FixedRange<T, N>() const 
    { 
        InnerType range;
        range._bindless_index = bindless_index;
        range._buffer_offset = buffer_offset;
        range._first_instance = first_instance;
        return range; 
    }

private:
    uint32_t first_instance = 0;
    uint32_t buffer_offset = 0;
    uint32_t bindless_index = ~0;
};

template <typename T>
struct GPUDatablock<Range<T>>
{
public:
    using ThisType = GPUDatablock<Range<T>>;
    using InnerType = Range<T>;
    inline static constexpr uint32_t Size = 4 + 4 + 4 + 4;

    GPUDatablock() = default;
    GPUDatablock<Range<T>>(const InnerType& range)
        : first_instance(range._first_instance), count(range._count), 
          buffer_offset(range._buffer_offset), bindless_index(range._bindless_index)
    {

    }

    operator Range<T>() const 
    { 
        Range<T> range; 
        range._first_instance = first_instance;
        range._count = count;
        range._buffer_offset = buffer_offset;
        range._bindless_index = bindless_index;
        return range; 
    }

private:
    uint32_t first_instance = 0;
    uint32_t count = 0;
    uint32_t buffer_offset = 0;
    uint32_t bindless_index = ~0;
};

static_assert(sizeof(GPUDatablock<Row<float>>) == 3 * sizeof(uint32_t));
static_assert(sizeof(GPUDatablock<FixedRange<float, 2>>) == 3 * sizeof(uint32_t));
static_assert(sizeof(GPUDatablock<Range<float>>) == 4 * sizeof(uint32_t));

} // namespace skr::gpu