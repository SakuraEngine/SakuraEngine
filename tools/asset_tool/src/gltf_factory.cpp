#include "SkrAssetTool/gltf_factory.h"
#include "cgltf/cgltf.h"
#include "utils/make_zeroed.hpp"
#include "SkrImGui/skr_imgui.h"

namespace skd::asset
{
class SGLTFImporterFactoryImpl : public SImporterFactory
{
public:
    virtual ~SGLTFImporterFactoryImpl() = default;
    bool CanImport(const skr::string& path) const override;
    int Import(const skr::string& path) override;
    int Update() override;
    skr::string GetName() const override { return "GLTF Importer"; }
    skr::string GetDescription() const override { return "GLTF Importer"; }
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
    return skr::string_view(path).ends_with(".gltf") || skr::string_view(path).ends_with(".glb");
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