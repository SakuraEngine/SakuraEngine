#ifdef WITH_USDTOOL
#include "SkrUsdTool/scene_asset.hpp"
#include "SkrAssetTool/usd_factory.h"
#include "UsdCore/stage.hpp"
#include "SkrUsdTool/mesh_asset.hpp"
#include "SkrImGui/imgui_utils.h"
#include "platform/filesystem.hpp"
#include "utils/defer.hpp"
#include "json/reader.h"
#include "json/writer.h"
#include "utils/log.h"
#include "ecs/dual.h"
#include "UsdCore/core.hpp"
#include "SkrRenderer/resources/mesh_resource.h"
#include "nfd.h"
#include <fstream>

namespace skd::asset
{
class SUsdImporterFactoryImpl : public SImporterFactory
{
public:
    virtual ~SUsdImporterFactoryImpl() = default;
    bool CanImport(const skr::string& path) const override;
    int Import(const skr::string& path) override;
    int Update() override;
    skr::string GetName() const override { return "USD Importer"; }
    skr::string GetDescription() const override { return "USD Importer"; }

    int TraversePrim(const skd::SUSDPrimId& prim);
    void Clear();

    SUSDStageId _stage;
    bool _createNewAsset = false;
    SUSDSceneImporter* _importer = nullptr;
    SImporter* _childImporter = nullptr;
    skr::flat_hash_map<skr::string, SUSDPrimId, skr::hash<skr::string>> _assetPrims;
    SUSDPrimId _selectedPrim;
    skr::string _assetPath;
    skr::string _assetFolder;
    skr::string _filePath;
};

void SUsdImporterFactoryImpl::Clear()
{
    _stage = nullptr;
    _createNewAsset = false;
    _importer = nullptr;
    _childImporter = nullptr;
    _assetPrims.clear();
    _selectedPrim = nullptr;
    _assetPath.clear();
    _assetFolder.clear();
    _filePath.clear();
}

bool SUsdImporterFactoryImpl::CanImport(const skr::string& path) const
{
    if (skd::USDCoreSupportFile(path.c_str()))
    {
        return true;
    }
    return false;
}

int SUsdImporterFactoryImpl::TraversePrim(const SUSDPrimId& prim)
{
    //check if prim is mesh
    

    auto children = prim->GetFilteredChildren(true);
    for (auto& child : children)
        TraversePrim(child);

    return 0;
}

int SUsdImporterFactoryImpl::Import(const skr::string& path)
{
    _filePath = path;
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
        ImGui::SameLine();
        if(ImGui::Button("browse"))
        {
            nfdchar_t* outPath = nullptr;
            nfdresult_t result = NFD_OpenDialog("meta", nullptr, &outPath);
            if(result == NFD_OKAY)
            {
                _assetPath = outPath;
                free(outPath);
            }
        }
        if(ImGui::Button("import"))
        {
            if(_createNewAsset)
            {
                _importer = SkrNew<SUSDSceneImporter>();
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
                if(auto error = importer["importerType"].get(importerType); error != simdjson::SUCCESS)
                {
                    SKR_LOG_ERROR("no importer type in asset json %s", simdjson::error_message(error));
                    return -1;
                }
                skr_guid_t _importerType;
                skr::json::Read(std::move(importerType), _importerType);
                if(!(_importerType == skr::type::type_id<SUSDSceneImporter>::get()))
                {
                    SKR_LOG_ERROR("importer type not match");
                    return -1;
                }
                _importer = SkrNew<SUSDSceneImporter>();
                if(auto error = skr::json::Read(std::move(importer), *_importer); error != skr::json::SUCCESS)
                {
                    SKR_LOG_ERROR("read importer failed %s", skr::json::error_message(error));
                    return -1;
                }
                _assetFolder = assetPath.parent_path().u8string().c_str();
            }
            
            auto root = _stage->GetPseudoRoot();
            auto allMeshPrims = root->GetAllPrimsOfType(true, "UsdGeomMesh", nullptr);
            for(auto& prim : allMeshPrims)
            {
                auto iter = _importer->redirectors.find(prim->GetPrimPath()->GetString());
                if(iter != _importer->redirectors.end())
                    continue;
                auto protoPrim = prim->IsInstanceProxy() ? prim->GetPrimInPrototype() : prim;
                _assetPrims.insert(std::make_pair(prim->GetPrimPath()->GetString(), protoPrim));
            }
        }
    }
    else
    {
        if(ImGui::BeginTable("AssetList", 4, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY, {0, 1000}))
        {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Path");
            ImGui::TableSetupColumn("ProtoPath");
            ImGui::TableSetupColumn("Type");
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableHeadersRow();
            for(auto& [path, prim] : _assetPrims)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                if(ImGui::Selectable(prim->GetName().c_str(), _selectedPrim == prim, ImGuiSelectableFlags_SpanAllColumns))
                {
                    _selectedPrim = prim;
                }
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", path.c_str());
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%s", prim->GetPrimPath()->GetString().c_str());
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%s", prim->GetTypeName().c_str());
            }
            ImGui::EndTable();
        }
        ImGui::InputText("Asset Folder", &_assetFolder);
        ImGui::SameLine();
        if(ImGui::Button("browse"))
        {
            nfdchar_t* outPath = nullptr;
            nfdresult_t result = NFD_PickFolder(nullptr, &outPath);
            if(result == NFD_OKAY)
            {
                _assetPath = outPath;
                free(outPath);
            }
        }
        if(ImGui::Button("Import All"))
        {
            skr::flat_hash_map<skr::string, skr_guid_t, skr::hash<skr::string>> _protoPrims;
            for(auto& [path, prim] : _assetPrims)
            {
                auto protoPath = prim->GetPrimPath()->GetString();
                auto iter = _protoPrims.find(protoPath);
                if(iter == _protoPrims.end())
                {
                    skr_guid_t newGuid;
                    dual_make_guid(&newGuid);
                    _protoPrims.insert(std::make_pair(protoPath, newGuid));
                    skr_json_writer_t writer{3};
                    writer.StartObject();
                    writer.Key("guid");
                    json::Write(&writer, newGuid);
                    writer.Key("type");
                    json::Write(&writer, skr::type::type_id<skr_mesh_resource_t>::get());
                    writer.Key("importer");
                    writer.StartObject();
                    writer.Key("importerType");
                    json::Write(&writer, skr::type::type_id<SUSDMeshImporter>::get());
                    writer.Key("assetPath");
                    writer.String(_filePath.c_str());
                    writer.Key("primPath");
                    writer.String(path.c_str());
                    writer.EndObject();
                    writer.EndObject();
                    skr::filesystem::path assetPath(_assetFolder.c_str());
                    assetPath /= skr::filesystem::path(path.c_str());
                    assetPath.replace_extension(".mesh.meta");
                    skr::filesystem::create_directories(assetPath.parent_path());
                    std::ofstream file(assetPath.u8string());
                    file << writer.Str().c_str();
                }
            }
            for(auto& [path, prim] : _assetPrims)
            {
                auto& protoguid = _protoPrims[prim->GetPrimPath()->GetString()];
                _importer->redirectors[path] = protoguid;
            }
            skr_json_writer_t writer{3};
            skr::json::Write(&writer, *_importer);
            std::ofstream file(_assetPath.c_str());
            file << writer.Str().c_str();
            Clear();
            return 1;
        }
        if(ImGui::Button("Go Back"))
        {
            Clear();
            return 1;
        }
    }
    return 0;
}
SImporterFactory* GetUsdImporterFactory()
{
    USDCoreInitialize();
    static SUsdImporterFactoryImpl factory;
    return &factory;
}
} // namespace skd::asset
#endif