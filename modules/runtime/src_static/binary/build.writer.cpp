#include "resource/resource_handle.h"
#include "binary/writer.h"
#include "binary/blob.h"
#include "utils/bits.hpp"
#include "rtm/scalarf.h"
#include "rtm/scalard.h"
#include "utils/log.h"


namespace skr::binary
{
int WriteTrait<const bool&>::Write(skr_binary_writer_t* writer, bool value)
{
    return WriteTrait<const uint32_t&>::Write(writer, (uint32_t)value);
}

int WriteTrait<const uint8_t&>::Write(skr_binary_writer_t* writer, uint8_t value)
{
    return WriteBytes(writer, &value, sizeof(value));
}

int WriteTrait<const uint16_t&>::Write(skr_binary_writer_t* writer, uint16_t value)
{
    return WriteBytes(writer, &value, sizeof(value));
}

int WriteTrait<const uint32_t&>::Write(skr_binary_writer_t* writer, uint32_t value)
{
    return WriteBytes(writer, &value, sizeof(value));
}

int WriteTrait<const uint64_t&>::Write(skr_binary_writer_t* writer, uint64_t value)
{
    return WriteBytes(writer, &value, sizeof(value));
}

int WriteTrait<const int32_t&>::Write(skr_binary_writer_t* writer, int32_t value)
{
    return WriteBytes(writer, &value, sizeof(value));
}

int WriteTrait<const int64_t&>::Write(skr_binary_writer_t* writer, int64_t value)
{
    return WriteBytes(writer, &value, sizeof(value));
}

template<class T>
int WriteBitpacked(skr_binary_writer_t* writer, T value, IntegerSerdeConfig<T> config)
{
    SKR_ASSERT(config.min <= config.max);
    bool valid = value >= config.min && value <= config.max;
    SKR_ASSERT(valid);
    SKR_ASSERT(writer->vwrite_bits);

    if(!writer->vwrite_bits)
    {
        SKR_LOG_ERROR("vwrite_bits is not implemented. falling back to vwrite");
        return writer->write(&value, sizeof(T));
    }
    auto bits = 64 - skr::CountLeadingZeros64(config.max - config.min);
    if(!valid)
    {
        SKR_LOG_ERROR("value %d is not in range [%d, %d]", value, config.min, config.max);
        value = std::clamp(value, config.min, config.max);
    }
    T compressed = (value - config.min) & ((1 << bits) - 1);
    return writer->write_bits(&compressed, bits);
}

int WriteTrait<const uint8_t&>::Write(skr_binary_writer_t* writer, uint8_t value, IntegerSerdeConfig<uint8_t> config)
{
    return WriteBitpacked(writer, value, config);
}

int WriteTrait<const uint16_t&>::Write(skr_binary_writer_t* writer, uint16_t value, IntegerSerdeConfig<uint16_t> config)
{
    return WriteBitpacked(writer, value, config);
}

int WriteTrait<const uint32_t&>::Write(skr_binary_writer_t* writer, uint32_t value, IntegerSerdeConfig<uint32_t> config)
{
    return WriteBitpacked(writer, value, config);
}

int WriteTrait<const uint64_t&>::Write(skr_binary_writer_t* writer, uint64_t value, IntegerSerdeConfig<uint64_t> config)
{
    return WriteBitpacked(writer, value, config);
}

int WriteTrait<const int32_t&>::Write(skr_binary_writer_t* writer, int32_t value, IntegerSerdeConfig<int32_t> config)
{
    return WriteBitpacked(writer, value, config);
}

int WriteTrait<const int64_t&>::Write(skr_binary_writer_t* writer, int64_t value, IntegerSerdeConfig<int64_t> config)
{
    return WriteBitpacked(writer, value, config);
}

int WriteTrait<const float&>::Write(skr_binary_writer_t* writer, float value)
{
    return WriteBytes(writer, &value, sizeof(value));
}

int WriteTrait<const double&>::Write(skr_binary_writer_t* writer, double value)
{
    return WriteBytes(writer, &value, sizeof(value));
}

// template<class T>
// int WriteBitpacked(skr_binary_writer_t* writer, T value, FloatingSerdeConfig<T> config)
// {
//     T scaled = value * config.scale;
//     bool valid = scaled < config.max;
//     SKR_ASSERT(valid);
//     SKR_ASSERT(writer->vwrite_bits);
    
//     if(!writer->vwrite_bits)
//     {
//         SKR_LOG_ERROR("vwrite_bits is not implemented. falling back to vwrite");
//         return writer->write(&value, sizeof(T));
//     }
//     if(!valid)
//     {
//         SKR_LOG_ERROR("Value %f is out of range for bitpacked serialization", value);
//         scaled = config.max;
//     }
//     auto integer = (typename FloatingSerdeConfig<T>::Integer) rtm::scalar_round_bankers(scaled);
//     bool sign = integer < 0;
//     if(sign)
//         integer = -integer;
//     auto bits = 65 - skr::CountLeadingZeros64(config.max);
//     auto compressed = integer | (sign ? 0 : (1 << bits));
//     return writer->write_bits(&compressed, bits);
// }

// int WriteTrait<const float&>::Write(skr_binary_writer_t* writer, float value, FloatingSerdeConfig<float> config)
// {
//     return WriteBitpacked(writer, value, config);
// }

// int WriteTrait<const double&>::Write(skr_binary_writer_t* writer, double value, FloatingSerdeConfig<double> config)
// {
//     return WriteBitpacked(writer, value, config);
// }

int WriteTrait<const skr_float2_t&>::Write(skr_binary_writer_t* writer, const skr_float2_t& value)
{
    return WriteBytes(writer, &value, sizeof(value));
}

int WriteTrait<const skr_float3_t&>::Write(skr_binary_writer_t* writer, const skr_float3_t& value)
{
    return WriteBytes(writer, &value, sizeof(value));
}

int WriteTrait<const skr_float4_t&>::Write(skr_binary_writer_t* writer, const skr_float4_t& value)
{
    return WriteBytes(writer, &value, sizeof(value));
}

int WriteTrait<const skr_rotator_t&>::Write(skr_binary_writer_t* writer, const skr_rotator_t& value)
{
    return WriteBytes(writer, &value, sizeof(value));
}

int WriteTrait<const skr_quaternion_t&>::Write(skr_binary_writer_t* writer, const skr_quaternion_t& value)
{
    return WriteBytes(writer, &value, sizeof(value));
}

int WriteTrait<const skr_float4x4_t&>::Write(skr_binary_writer_t* writer, const skr_float4x4_t& value)
{
    return WriteBytes(writer, &value, sizeof(value));
}

template<class T, class ScalarType>
int WriteBitpacked(skr_binary_writer_t* writer, T value, VectorSerdeConfig<ScalarType> config)
{
    ScalarType* array = (ScalarType*)&value;
    static constexpr size_t size = sizeof(T) / sizeof(ScalarType);
	using IntType = std::conditional_t<sizeof(ScalarType) == 4, int32_t, int64_t>;
    // Beyond 2^MaxExponentForScaling scaling cannot improve the precision as the next floating point value is at least 1.0 more. 
	constexpr uint32_t MaxExponentForScaling = sizeof(ScalarType) == 4 ? 23U : 52U;
	constexpr ScalarType MaxValueToScale = ScalarType(IntType(1) << MaxExponentForScaling);

	// Rounding of large values can introduce additional precision errors and the extra cost to serialize with full precision is small.
	constexpr uint32_t MaxExponentAfterScaling = sizeof(ScalarType) == 4 ? 30U : 62U;
	constexpr ScalarType MaxScaledValue = ScalarType(IntType(1) << MaxExponentAfterScaling);

	// NaN values can be properly serialized using the full precision path, but they typically cause lots of errors
	// for the typical engine use case.
    bool containsNan = false;
    for(size_t i = 0; i < size; ++i)
    {
        if(std::isnan(array[i]))
        {
            containsNan = true;
            break;
        }
    }
    SKR_ASSERT(!containsNan);
	if (containsNan)
	{
        SKR_LOG_ERROR("WriteBitpacked: Value isn't finite. Clearing for safety.");
		for(size_t i = 0; i < size; ++i)
        {
            array[i] = 0;
        }
	}

	const ScalarType Factor = config.scale;
	ScalarType ScaledValue[size];
    for(size_t i = 0; i < size; ++i)
    {
        ScaledValue[i] = array[i]*Factor;
    }
    ScalarType absMax = 0;
    ScalarType absMin = 0;
    for(size_t i = 0; i < size; ++i)
    {
        absMax = std::max(absMax, std::abs(ScaledValue[i]));
        absMin = std::min(absMin, std::abs(ScaledValue[i]));
    }

	// If the component values are within bounds then we optimize the bandwidth, otherwise we use full precision.
	if (absMax < MaxScaledValue)
	{
		const bool bUseScaledValue = absMin < MaxValueToScale;

		IntType finalValue[size];
        for(size_t i = 0; i < size; ++i)
        {
            if (bUseScaledValue)
            {
                finalValue[i] = (IntType)rtm::scalar_round_bankers(ScaledValue[i]);
            }
            else
            {
                finalValue[i] = (IntType)rtm::scalar_round_bankers(array[i]);
            }
        }
        uint8_t componentBitCount = 0;
        for(size_t i = 0; i < size; ++i)
        {
            uint8_t bitsNeeded = 65 - skr::CountLeadingZeros64((uint64_t)std::abs(finalValue[i]));
            componentBitCount = std::max(componentBitCount, bitsNeeded);
        }
        for(size_t i = 0; i < size; ++i)
        {
            if (finalValue[i] < 0)
                finalValue[i] |= (IntType(1) << componentBitCount);
        }
		uint8_t ComponentBitCountAndScaleInfo = (bUseScaledValue ? (1U << 6U) : 0U) | componentBitCount;
		auto ret = writer->write(&ComponentBitCountAndScaleInfo, 1);
        if(ret != 0)
            return ret;
        for(size_t i = 0; i < size; ++i)
        {
            ret = writer->write_bits(&finalValue[i], componentBitCount);
            if(ret != 0)
                return ret;
        }
	}
	else
	{
		// A component bit count of 0 indicates full precision.
		constexpr uint32_t ComponentBitCount = 0;
		uint8_t ComponentBitCountAndTypeInfo = ComponentBitCount;
		auto ret = writer->write(&ComponentBitCountAndTypeInfo, 1);
        if(ret != 0)
            return ret;
        ret = writer->write(array, sizeof(T));
        if(ret != 0)
            return ret;
	}
    return 0;  
}

int WriteTrait<const skr_float2_t&>::Write(skr_binary_writer_t* writer, const skr_float2_t& value, VectorSerdeConfig<float> config)
{
    return WriteBitpacked(writer, value, config);
}

int WriteTrait<const skr_float3_t&>::Write(skr_binary_writer_t* writer, const skr_float3_t& value, VectorSerdeConfig<float> config)
{
    return WriteBitpacked(writer, value, config);
}

int WriteTrait<const skr_float4_t&>::Write(skr_binary_writer_t* writer, const skr_float4_t& value, VectorSerdeConfig<float> config)
{
    return WriteBitpacked(writer, value, config);
}

int WriteTrait<const skr_rotator_t&>::Write(skr_binary_writer_t* writer, const skr_rotator_t& value, VectorSerdeConfig<float> config)
{
    return WriteBitpacked(writer, value, config);
}

int WriteTrait<const skr_quaternion_t&>::Write(skr_binary_writer_t* writer, const skr_quaternion_t& value, VectorSerdeConfig<float> config)
{
    return WriteBitpacked(writer, value, config);
}

int WriteTrait<const skr_float4x4_t&>::Write(skr_binary_writer_t* writer, const skr_float4x4_t& value, VectorSerdeConfig<float> config)
{
    return WriteBitpacked(writer, value, config);
}

int WriteTrait<const skr::string&>::Write(skr_binary_writer_t* writer, const skr::string& str)
{
    int ret = WriteTrait<const uint32_t&>::Write(writer, (uint32_t)str.size());
    if (ret != 0)
        return ret;
    return WriteBytes(writer, str.u8_str(), str.size());
}

int WriteTrait<const skr::string_view&>::Write(skr_binary_writer_t* writer, const skr::string_view& str)
{
    int ret = WriteTrait<const uint32_t&>::Write(writer, (uint32_t)str.size());
    if (ret != 0)
        return ret;
    return WriteBytes(writer, str.u8_str(), str.size());
}

int WriteTrait<const skr::string_view&>::Write(skr_binary_writer_t* writer, skr_blob_arena_t& arena, const skr::string_view& str)
{
    auto ptr = (ochar8_t*)str.u8_str();
    auto buffer = (ochar8_t*)arena.get_buffer();
    SKR_ASSERT(ptr >= buffer);
    auto offset = (uint32_t)(ptr - buffer);
    SKR_ASSERT(offset < arena.get_size());
    int ret = skr::binary::Write(writer, (uint32_t)str.size());
    if (ret != 0) {
        return ret;
    }
    if(str.size() == 0)
        return 0;
    ret = skr::binary::Write(writer, offset);
    if (ret != 0) {
        return ret;
    }
    return WriteBytes(writer, str.u8_str(), str.size());
}

int WriteTrait<const skr_guid_t&>::Write(skr_binary_writer_t* writer, const skr_guid_t& guid)
{
    return WriteBytes(writer, &guid, sizeof(guid));
}

int WriteTrait<const skr_md5_t&>::Write(skr_binary_writer_t* writer, const skr_md5_t& md5)
{
    return WriteBytes(writer, &md5, sizeof(md5));
}

int WriteTrait<const skr_resource_handle_t&>::Write(skr_binary_writer_t* writer, const skr_resource_handle_t& handle)
{
    return WriteTrait<const skr_guid_t&>::Write(writer, handle.get_serialized());
}

int WriteTrait<const skr_blob_t&>::Write(skr_binary_writer_t* writer, const skr_blob_t& blob)
{
    int ret = WriteTrait<const uint32_t&>::Write(writer, (uint32_t)blob.size);
    if (ret != 0)
        return ret;
    return WriteBytes(writer, blob.bytes, (uint32_t)blob.size);
}

int WriteTrait<const skr_blob_arena_t&>::Write(skr_binary_writer_t* writer, const skr_blob_arena_t& blob)
{
    int ret = WriteTrait<const uint32_t&>::Write(writer, (uint32_t)blob.get_size());
    if (ret != 0)
        return ret;
    if(blob.get_size() == 0)
        return 0;
    ret = WriteTrait<const uint32_t&>::Write(writer, (uint32_t)blob.get_align());
    if (ret != 0)
        return ret;
    return 0;
}

void BlobTrait<skr::string_view>::BuildArena(skr_blob_arena_builder_t& arena, skr::string_view& dst, const skr::string& src)
{
    auto offset = arena.allocate((src.size() + 1) * sizeof(ochar8_t), alignof(ochar8_t));
    auto buffer = (ochar8_t*)arena.get_buffer() + offset;
    memcpy(buffer, src.u8_str(), (src.size() + 1) * sizeof(ochar8_t));
    memset(buffer + src.size(), 0, sizeof(ochar8_t)); //tailing zero
    dst = skr::string_view((const ochar8_t*)offset, src.size());
}

void BlobTrait<skr::string_view>::Remap(skr_blob_arena_t& arena, skr::string_view& dst)
{
    const auto unmapped_addr = (std::uintptr_t)dst.raw().u8_str();
    const auto offset_in_arena = (std::uintptr_t)arena.base();
    const auto base_ptr = arena.get_buffer();
    const auto str_ptr = reinterpret_cast<const char8_t*>(base_ptr);
    const auto _size = dst.raw().size();
    const auto final_addr = str_ptr + (unmapped_addr - offset_in_arena);
    dst = skr::string_view((char8_t*)final_addr, _size);
}

} // namespace skr::binary