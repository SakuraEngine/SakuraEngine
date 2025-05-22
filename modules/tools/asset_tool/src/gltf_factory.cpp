#include "SkrAssetTool/gltf_factory.h"
#include "SkrBase/misc/make_zeroed.hpp"
#include "SkrContainers/stl_string.hpp"
#include <imgui.h>
#include "cgltf/cgltf.h"

namespace skd::asset
{
class SGLTFImporterFactoryImpl : public SImporterFactory
{
public:
    virtual ~SGLTFImporterFactoryImpl() = default;
    bool CanImport(const skr::String& path) const override;
    int Import(const skr::String& path) override;
    int Update() override;
    skr::String GetName() const override { return u8"GLTF Importer"; }
    skr::String GetDescription() const override { return u8"GLTF Importer"; }
    void Clear();

    cgltf_data* data;
};

SImporterFactory* GetGLTFImporterFactory()
{
    static SGLTFImporterFactoryImpl factory;
    return &factory;
}

bool SGLTFImporterFactoryImpl::CanImport(const skr::String& path) const
{
    return skr::stl_u8string_view(path.u8_str()).ends_with(u8".gltf") || 
        skr::stl_u8string_view(path.u8_str()).ends_with(u8".glb");
}

int SGLTFImporterFactoryImpl::Import(const skr::String& path)
{
    auto options = make_zeroed<cgltf_options>();
    auto result = cgltf_parse_file(&options, path.c_str_raw(), &data);
    if (result != cgltf_result_success)
    {
        return -1;
    }
    return 0;
}

int SGLTFImporterFactoryImpl::Update()
{
    ImGui::Begin("GLTF Importer");
    ImGui::End();
    return 0;
}
}