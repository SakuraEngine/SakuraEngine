#pragma once
#include "cgpu/api.h"
#include "SkrRT/containers/vector.hpp"
#include <catch2/generators/catch_generators.hpp>

class CGPUBackendGenerator : public Catch::Generators::IGenerator<ECGPUBackend>
{
public:
    ECGPUBackend const& get() const override {
        return m_values[m_idx];
    }
    
    bool next() override {
        ++m_idx;
        return m_idx < m_values.size();
    }

    static Catch::Generators::GeneratorWrapper<ECGPUBackend> Create()
    {
        return Catch::Generators::GeneratorWrapper<ECGPUBackend>(
            Catch::Detail::make_unique<CGPUBackendGenerator>()
        );
    }

    skr::vector<ECGPUBackend> m_values = 
    {
#ifdef CGPU_USE_VULKAN
        CGPU_BACKEND_VULKAN
#endif
#ifdef CGPU_USE_D3D12
        , CGPU_BACKEND_D3D12
#endif 
    };
    size_t m_idx = 0;
};
