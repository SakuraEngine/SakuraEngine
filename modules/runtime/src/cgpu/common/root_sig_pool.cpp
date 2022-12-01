#include "common_utils.h"
#include <EASTL/vector.h>
#include <containers/string.hpp>
#include <containers/hashmap.hpp>
#include "platform/atomic.h"

struct RSCharacteristic
{
    // table count & hash
    uint32_t table_count;
    size_t table_hash;
    uint32_t push_constant_count;
    size_t push_constant_hash;
    // static samplers tie with root signature
    uint32_t static_sampler_count;
    size_t static_samplers_hash;
    ECGPUPipelineType pipeline_type;
    operator size_t() const
    {
        return cgpu_hash(this, sizeof(RSCharacteristic), (size_t)pipeline_type);
    }
    struct hasher { inline size_t operator()(const RSCharacteristic& val) const { return (size_t)val; } };
    struct RSTResource
    {
        ECGPUResourceType type;
        ECGPUTextureDimension dim;
        uint32_t set;
        uint32_t binding;
        uint32_t size;
        uint32_t offset;
        CGPUShaderStages stages;
    };
    struct StaticSampler
    {
        uint32_t set;
        uint32_t binding;
        CGPUSamplerId id;
    };
    struct PushConstant
    {
        uint32_t set;
        uint32_t binding;
        uint32_t size;
        uint32_t offset;
        CGPUShaderStages stages;
    };
};

class CGPURootSignaturePoolImpl : public CGPURootSignaturePool
{
public:
    CGPURootSignaturePoolImpl(const char8_t* name)
        :name(name)
    {

    }
    FORCEINLINE RSCharacteristic calculaeCharacteristic(CGPURootSignature* RSTables, const struct CGPURootSignatureDescriptor* desc)
    {
        // calculate characteristic
        RSCharacteristic newCharacteristic = {};
        newCharacteristic.table_count = RSTables->table_count;
        newCharacteristic.table_hash = (size_t)this;
        for(uint32_t i = 0; i < RSTables->table_count; i++)
        {
            for(uint32_t j = 0; j < RSTables->tables[i].resources_count; j++)
            {
                const auto& res = RSTables->tables[i].resources[j];
                RSCharacteristic::RSTResource r = {};
                r.type = res.type;
                r.dim = res.dim;
                r.set = res.set;
                r.binding = res.binding;
                r.size = res.size;
                r.offset = res.offset;
                r.stages = res.stages;
                newCharacteristic.table_hash = cgpu_hash(&r, sizeof(r), newCharacteristic.table_hash);
            }
        }
        newCharacteristic.push_constant_count = RSTables->push_constant_count;
        newCharacteristic.push_constant_hash = (size_t)this;
        for(uint32_t i = 0; i < desc->push_constant_count; i++)
        {
            RSCharacteristic::PushConstant p = {};
            p.set = RSTables->push_constants[i].set;
            p.binding = RSTables->push_constants[i].binding;
            p.size = RSTables->push_constants[i].size;
            p.offset = RSTables->push_constants[i].offset;
            p.stages = RSTables->push_constants[i].stages;
            newCharacteristic.push_constant_hash = cgpu_hash(&p, sizeof(p), newCharacteristic.push_constant_hash);
        }
        newCharacteristic.static_sampler_count = desc->static_sampler_count;
        newCharacteristic.static_samplers_hash = ~0;
        // static samplers are well stable-sorted during RSTable intiialization
        for(uint32_t i = 0; i < desc->static_sampler_count; i++)
        {
            for (uint32_t j = 0; j < desc->static_sampler_count; j++)
            {
                if(strcmp(desc->static_sampler_names[j], RSTables->static_samplers[i].name) == 0)
                {
                    RSCharacteristic::StaticSampler s = {};
                    s.set = RSTables->static_samplers[i].set;
                    s.binding = RSTables->static_samplers[i].binding;
                    s.id = desc->static_samplers[j];
                    newCharacteristic.static_samplers_hash =
                        cgpu_hash(&s, sizeof(s), newCharacteristic.static_samplers_hash);
                }
            }
        }
        newCharacteristic.pipeline_type = RSTables->pipeline_type;
        return newCharacteristic;
    }
    CGPURootSignatureId try_allocate(CGPURootSignature* RSTables, const struct CGPURootSignatureDescriptor* desc)
    {
        const auto character = calculaeCharacteristic(RSTables, desc);
        const auto iter = characterMap.find(character);
        if (iter != characterMap.end())
        {
            counterMap[iter->second]++;
            return iter->second;
        }
        return nullptr;
    }
    bool deallocate(CGPURootSignatureId rootsig)
    {
        auto trueSig = rootsig;
        while(rootsig->pool && trueSig->pool_sig) { 
            trueSig = trueSig->pool_sig;
        }
        auto&& iter = counterMap.find(trueSig);
        if (iter != counterMap.end())
        {
            const auto oldCounterVal = iter->second;
            if(oldCounterVal <= 1)
            {
                counterMap.erase(trueSig);
                const auto& character = biCharacterMap[trueSig];
                characterMap.erase(character);
                biCharacterMap.erase(trueSig);
                CGPURootSignature* enforceDestroy = (CGPURootSignature*)trueSig;
                enforceDestroy->pool = nullptr;
                enforceDestroy->pool_sig = nullptr;
                cgpu_free_root_signature(enforceDestroy);
                return true;
            }
            iter->second--;
            return true;
        }
        return false;
    }
    bool insert(CGPURootSignature* sig, const CGPURootSignatureDescriptor* desc)
    {
        const auto character = calculaeCharacteristic(sig, desc);
        const auto iter = characterMap.find(character);
        if (iter != characterMap.end())
        {
            SKR_UNREACHABLE_CODE();
            return false;
        }
        characterMap[character] = sig;
        biCharacterMap[sig] = character;
        counterMap[sig] = 1;
        sig->pool = this;
        sig->pool_sig = nullptr;
        return true;
    }
    ~CGPURootSignaturePoolImpl()
    {
        for(auto& iter : counterMap)
        {
            CGPURootSignature* enforceDestroy = (CGPURootSignature*)iter.first;
            enforceDestroy->pool = nullptr;
            enforceDestroy->pool_sig = nullptr;
            cgpu_free_root_signature(enforceDestroy);
        }
    }
protected:
    const skr::string name;
    // TODO: replace with skr::hash_map
    skr::flat_hash_map<RSCharacteristic, CGPURootSignatureId, RSCharacteristic::hasher> characterMap;
    skr::flat_hash_map<CGPURootSignatureId, RSCharacteristic> biCharacterMap;
    skr::flat_hash_map<CGPURootSignatureId, uint32_t> counterMap;
};

CGPURootSignaturePoolId CGPUUtil_CreateRootSignaturePool(const CGPURootSignaturePoolDescriptor* desc)
{
    return SkrNew<CGPURootSignaturePoolImpl>(desc->name);
}
CGPURootSignatureId CGPUUtil_TryAllocateSignature(CGPURootSignaturePoolId pool, CGPURootSignature* RSTables, const struct CGPURootSignatureDescriptor* desc)
{
    auto P = (CGPURootSignaturePoolImpl*)pool;
    return P->try_allocate(RSTables, desc);
}
bool CGPUUtil_AddSignature(CGPURootSignaturePoolId pool, CGPURootSignature* sig, const CGPURootSignatureDescriptor* desc)
{
    auto P = (CGPURootSignaturePoolImpl*)pool;
    return P->insert(sig, desc);
}
void CGPUUtil_AllSignatures(CGPURootSignaturePoolId pool, CGPURootSignatureId* signatures, uint32_t* count)
{

}
bool CGPUUtil_PoolFreeSignature(CGPURootSignaturePoolId pool, CGPURootSignatureId sig)
{
    auto P = (CGPURootSignaturePoolImpl*)pool;
    return P->deallocate(sig);
}
void CGPUUtil_FreeRootSignaturePool(CGPURootSignaturePoolId pool)
{
    auto P = (CGPURootSignaturePoolImpl*)pool;
    SkrDelete(P);
}