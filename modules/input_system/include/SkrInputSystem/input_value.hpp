#pragma once
#include "SkrInputSystem/module.configure.h"
#include "SkrInput/input.h"
#include "platform/atomic.h"
#include "containers/detail/sptr.hpp"

namespace skr {
namespace input {

struct InputReading;
struct InputLayer;

enum class EValueType
{
    kFloat,
    kFloat2,
    kFloat3,
    kBool
};

struct SKR_INPUTSYSTEM_API InputValueStorage
{
    InputValueStorage() SKR_NOEXCEPT
    {
        
    }
    InputValueStorage(float v) SKR_NOEXCEPT;
    InputValueStorage(skr_float2_t v) SKR_NOEXCEPT;
    InputValueStorage(skr_float3_t v) SKR_NOEXCEPT;
    InputValueStorage(bool v) SKR_NOEXCEPT;
    InputValueStorage(EValueType type, skr_float4_t raw) SKR_NOEXCEPT;
    InputValueStorage(const InputValueStorage& rhs) SKR_NOEXCEPT;
    
    ~InputValueStorage() SKR_NOEXCEPT;

    void reset() SKR_NOEXCEPT;

    EValueType get_type() const SKR_NOEXCEPT;

    bool get_float(float& out_f) const SKR_NOEXCEPT;
    bool get_float2(skr_float2_t& out_f2) const SKR_NOEXCEPT;
    bool get_float3(skr_float3_t& out_f3) const SKR_NOEXCEPT;
    bool get_bool(bool& out_bool) const SKR_NOEXCEPT;

    float get_magnitude_val() const SKR_NOEXCEPT;
    
protected:
    EValueType type;
    skr_float4_t v;
    struct
    {
        InputReading* reading = nullptr;
        InputLayer* layer = nullptr;
    } lowlevel;
};

struct SKR_INPUTSYSTEM_API RC : public skr::SInterface
{
    virtual ~RC() SKR_NOEXCEPT;

    uint32_t add_refcount() SKR_NOEXCEPT;
    uint32_t release() SKR_NOEXCEPT;
    uint32_t use_count() const SKR_NOEXCEPT;
    
    SAtomicU32 rc = 0;
};

} }