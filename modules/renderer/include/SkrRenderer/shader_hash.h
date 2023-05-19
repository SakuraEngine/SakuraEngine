#pragma once
#include "SkrRenderer/fwd_types.h"
#include "cgpu/flags.h"
#ifdef __cplusplus
#include "containers/string.hpp"
#endif

#ifndef __meta__
    #include "SkrRenderer/shader_hash.generated.h"
#endif

sreflect_struct("guid" : "5a54720c-34b2-444c-8e3a-5977c94136c3")
sattr("serialize" : ["json", "bin"], "rtti" : true)
skr_stable_shader_hash_t 
{

    uint32_t valuea = 0;
    uint32_t valueb = 0;
    uint32_t valuec = 0;
    uint32_t valued = 0;
    
#ifdef __cplusplus
    inline constexpr skr_stable_shader_hash_t() = default;

    sattr("no-rtti" : true)
    inline bool operator==(const skr_stable_shader_hash_t& other) const
    {
        return valuea == other.valuea && valueb == other.valueb && valuec == other.valuec && valued == other.valued;
    }

    struct hasher
    {
        SKR_RENDERER_API size_t operator()(const skr_stable_shader_hash_t& hash) const;
    };

    sattr("no-rtti" : true)
    SKR_RENDERER_API static skr_stable_shader_hash_t hash_string(const char* str, uint32_t size) SKR_NOEXCEPT;

    sattr("no-rtti" : true)
    SKR_RENDERER_API static skr_stable_shader_hash_t from_string(const char* str) SKR_NOEXCEPT;
    
    SKR_RENDERER_API skr_stable_shader_hash_t(uint32_t a, uint32_t b, uint32_t c, uint32_t d) SKR_NOEXCEPT;
    SKR_RENDERER_API explicit operator skr::string() const SKR_NOEXCEPT;
#endif
};
static SKR_CONSTEXPR skr_stable_shader_hash_t kZeroStableShaderHash = skr_stable_shader_hash_t();

sreflect_struct("guid" : "0291f512-747e-4b64-ba5c-5fdc412220a3")
sattr("serialize" : ["json", "bin"], "rtti" : true)
skr_platform_shader_hash_t 
{
    uint32_t flags;
    uint32_t encoded_digits[4];

#ifdef __cplusplus
    sattr("no-rtti" : true)
    inline bool operator==(const skr_platform_shader_hash_t& other) const
    {
        return encoded_digits[0] == other.encoded_digits[0] && encoded_digits[1] == other.encoded_digits[1]
            && encoded_digits[2] == other.encoded_digits[2] && encoded_digits[3] == other.encoded_digits[3]
            && flags == other.flags;
    }

    struct hasher
    {
        SKR_RENDERER_API size_t operator()(const skr_platform_shader_hash_t& hash) const;
    };
#endif
};

sreflect_struct("guid" : "b0b69898-166f-49de-a675-7b04405b98b1")
sattr("serialize" : ["json", "bin"], "rtti" : true)
skr_platform_shader_identifier_t 
{
#ifdef __cplusplus
    skr::TEnumAsByte<ECGPUShaderBytecodeType> bytecode_type;
    skr::TEnumAsByte<ECGPUShaderStage> shader_stage;
#else
    ECGPUShaderBytecodeType bytecode_type;
    ECGPUShaderStage shader_stage;
#endif
    skr_platform_shader_hash_t hash;

#ifdef __cplusplus
    sattr("no-rtti" : true)
    inline bool operator==(const skr_platform_shader_identifier_t& other) const
    {
        return hash == other.hash && bytecode_type == other.bytecode_type && shader_stage == other.shader_stage;
    }

    struct hasher
    {
        SKR_RENDERER_API size_t operator()(const skr_platform_shader_identifier_t& hash) const;
    };
#endif
};