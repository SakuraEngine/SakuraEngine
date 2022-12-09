
#ifdef WITH_USDTOOL
#include "SkrAssetTool/usd_factory.h"
#include "UsdCore/stage.hpp"
#include "SkrImGui/imgui_utils.h"
#include "platform/filesystem.hpp"
#include "utils/defer.hpp"
#include "json/reader.h"
#include "utils/log.h"
#include <fstream>

namespace skd::asset
{
class SUsdImporterFactoryImpl : public SUsdImporterFactory
{
public:
    virtual ~SUsdImporterFactoryImpl() = default;
    bool CanImport(const skr::string& path) const override;
    int Import(const skr::string& path) override;
    int Update() override;

    int TraversePrim(const skd::SUSDPrimId& prim);

    SUSDStageId _stage;
    bool _createNewAsset = false;
    SSceneImporter* _importer = nullptr;
    skr::string _assetPath;
};

bool SUsdImporterFactoryImpl::CanImport(const skr::string& path) const
{
    if (skd::USDCoreSupportFile(path.c_str()))
    {
        return true;
    }
    return false;
}

int SUsdImporterFactoryImpl::TraversePrim(const skd::SUSDPrimId& prim)
{

    //check if prim is mesh
    auto children = prim->GetChildren();

    for (auto& child : children)
    {
        TraversePrim(child);
    }

    return 0;
}

int SUsdImporterFactoryImpl::Import(const skr::string& path)
{
    _stage = skd::USDCoreOpenStage(path.c_str());
    if (!_stage)
    {
        return -1;
    }
    return 0;
}

int SUsdImporterFactoryImpl::Update()
{
    if(!_importer)
    {
        ImGui::Checkbox("create new asset", &_createNewAsset);
        if(!_createNewAsset)
        {
            ImGui::InputText("existing asset", &_assetPath);
        }
        else
        {
            ImGui::InputText("new asset", &_assetPath);
        }
        if(ImGui::Button("import"))
        {
            if(_createNewAsset)
            {
                _importer = SkrNew<SSceneImporter>();
            }
            else
            {
                skr::filesystem::path assetPath(_assetPath.c_str());
                if(!skr::filesystem::exists(assetPath))
                {
                    SKR_LOG_ERROR("asset path not exist");
                    return -1;
                }
                simdjson::padded_string json;
                if(simdjson::padded_string::load(assetPath.u8string().c_str()).get(json) != simdjson::SUCCESS)
                {
                    SKR_LOG_ERROR("load asset json failed");
                    return -1;
                }
                simdjson::ondemand::parser parser;
                simdjson::ondemand::value doc;
                if(auto error = parser.iterate(json).get(doc); error != simdjson::SUCCESS)
                {
                    SKR_LOG_ERROR("failed to parse json; %s", simdjson::error_message(error));
                    return -1;
                }
                simdjson::ondemand::value importer;
                if(auto error = doc["importer"].get(importer); error != simdjson::SUCCESS)
                {
                    SKR_LOG_ERROR("no importer in asset json %s", simdjson::error_message(error));  
                    return -1;
                }
                simdjson::ondemand::value importerType;
                if(auto error = importer["type"].get(importerType); error != simdjson::SUCCESS)
                {
                    SKR_LOG_ERROR("no importer type in asset json %s", simdjson::error_message(error));
                    return -1;
                }
                skr_guid_t _importerType;
                skr::json::Read(std::move(importerType), _importerType);
                if(!(_importerType == skr::type::type_id<SSceneImporter>::get()))
                {
                    SKR_LOG_ERROR("importer type not match");
                    return -1;
                }
                _importer = SkrNew<SSceneImporter>();
                if(auto error = skr::json::Read(std::move(importer), *_importer); error != skr::json::SUCCESS)
                {
                    SKR_LOG_ERROR("read importer failed %s", skr::json::error_message(error));
                    return -1;
                }
            }
            
            auto root = _stage->GetPseudoRoot();
            if(auto result = TraversePrim(root); result != 0)
            {
                return result;
            }
        }
    }
    else
    {
        SKR_UNIMPLEMENTED_FUNCTION();
    }
    return 0;
}
} // namespace skd::asset
#endif