#include "SkrAssetTool/gltf_factory.h"
#include "cgltf/cgltf.h"
#include "misc/make_zeroed.hpp"
#include "SkrImGui/skr_imgui.h"

#include <EASTL/string_view.h>

namespace skd::asset
{
class SGLTFImporterFactoryImpl : public SImporterFactory
{
public:
    virtual ~SGLTFImporterFactoryImpl() = default;
    bool CanImport(const skr::string& path) const override;
    int Import(const skr::string& path) override;
    int Update() override;
    skr::string GetName() const override { return u8"GLTF Importer"; }
    skr::string GetDescription() const override { return u8"GLTF Importer"; }
    void Clear();

    cgltf_data* data;
};

SImporterFactory* GetGLTFImporterFactory()
{
    static SGLTFImporterFactoryImpl factory;
    return &factory;
}

bool SGLTFImporterFactoryImpl::CanImport(const skr::string& path) const
{
    return eastl::u8string_view(path.u8_str()).ends_with(u8".gltf") || 
        eastl::u8string_view(path.u8_str()).ends_with(u8".glb");
}

int SGLTFImporterFactoryImpl::Import(const skr::string& path)
{
    auto options = make_zeroed<cgltf_options>();
    auto result = cgltf_parse_file(&options, path.c_str(), &data);
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