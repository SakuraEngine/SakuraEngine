#include "SkrInspector/inspect_value.h"
#ifdef INSPECT_USABLE
#include "module/subsystem.hpp"
#include "platform/filesystem.hpp"
#include "containers/hashmap.hpp"
#include "platform/thread.h"
#include "containers/btree.hpp"
#include "containers/optional.hpp"
#include <containers/variant.hpp>
#include <containers/string.hpp>
#include <chrono>
#include "SkrImGui/skr_imgui.h"
#include "utils/format.hpp"


namespace skr::inspect
{
struct tweak_value_t
{
    std::chrono::time_point<std::chrono::milliseconds> last_modified;
    //TODO: scriptable override & assert
    skr_value_t value;
    skr_value_t override;
    uint32_t ident;
    const char* expr;
    SMutexObject _mutex;
};
static struct inspect_system* _system = nullptr;
struct inspect_system : public skr::ModuleSubsystem
{
    using tweak_value_map_t = skr::btree_multimap<uint32_t, tweak_value_t>;
    skr::flat_hash_map<eastl::string, tweak_value_map_t, eastl::hash<eastl::string>> _tweak_files;
    uint32_t counter = 0;

    void Initialize() override
    {
        _system = this;
    }

    void Finalize() override
    {
        _system = nullptr;
    }

    template<class T>
    tweak_value_t* AddInspect(const T& value, const char* str, const char* fileName, int lineNumber)
    {
        SMutexLock lock(_mutex.mMutex);
        skr::filesystem::path path = skr::filesystem::path(fileName).lexically_normal();
        auto& tweak_file = _tweak_files[path.u8string().c_str()]; 
        auto result = &tweak_file.emplace(lineNumber, tweak_value_t{})->second;
        result->value = value;
        result->ident = counter++;
        result->expr = str;
        return result;
    }

    void UpdateInspect(tweak_value_t* tweak, void* value)
    {
        SMutexLock lock(tweak->_mutex.mMutex);
        tweak->value = skr_value_ref_t{value, tweak->value.type};
        if(tweak->override)
        {
            tweak->override.type->Copy(value, tweak->override.Ptr());
        }
    }

    void DrawInspect(tweak_value_t& tweak, skr_value_t& value)
    {
        switch(value.type->type)
        {
            case SKR_TYPE_CATEGORY_BOOL:
            {
                ImGui::Checkbox(skr::format("##Value{}", tweak.ident).c_str(), &value.As<bool>());
            }
            break;
            case SKR_TYPE_CATEGORY_F32:
            {
                ImGui::DragFloat(skr::format("##Value{}", tweak.ident).c_str(), &value.As<float>());
            }
            break;
            case SKR_TYPE_CATEGORY_I32:
            case SKR_TYPE_CATEGORY_I64:
            case SKR_TYPE_CATEGORY_U32:
            case SKR_TYPE_CATEGORY_U64:
            case SKR_TYPE_CATEGORY_F64:
            case SKR_TYPE_CATEGORY_F32_2:
            case SKR_TYPE_CATEGORY_F32_3:
            case SKR_TYPE_CATEGORY_F32_4:
            case SKR_TYPE_CATEGORY_F32_4x4:
            case SKR_TYPE_CATEGORY_ROT:
            case SKR_TYPE_CATEGORY_QUAT:
            case SKR_TYPE_CATEGORY_GUID:
            case SKR_TYPE_CATEGORY_MD5:
            case SKR_TYPE_CATEGORY_HANDLE:
            case SKR_TYPE_CATEGORY_STR:
            case SKR_TYPE_CATEGORY_STRV:
            case SKR_TYPE_CATEGORY_ARR:
            case SKR_TYPE_CATEGORY_DYNARR:
            case SKR_TYPE_CATEGORY_ARRV:
            case SKR_TYPE_CATEGORY_OBJ:
            case SKR_TYPE_CATEGORY_ENUM:
            case SKR_TYPE_CATEGORY_REF:
            case SKR_TYPE_CATEGORY_VARIANT:
            case SKR_TYPE_CATEGORY_INVALID:
                SKR_UNIMPLEMENTED_FUNCTION();
            break;
        }
    }

    void DrawInspects()
    {
        ImGui::Begin("ValueInspector");
        for(auto& pair : _tweak_files)
        {
            if(ImGui::CollapsingHeader(pair.first.c_str()))
            {
                if(ImGui::BeginTable(skr::format("##{}", pair.first).c_str(), 3))
                {
                    ImGui::TableSetupScrollFreeze(0, 1);
                    ImGui::TableSetupColumn("Override", ImGuiTableColumnFlags_WidthFixed, 100);
                    ImGui::TableSetupColumn("Expr", ImGuiTableColumnFlags_NoClip);
                    ImGui::TableSetupColumn("Value");
                    ImGui::TableHeadersRow();
                    ImGui::TableNextColumn();

                    for(auto& p : pair.second)
                    {
                        auto& value = p.second;
                        bool override = value.override.HasValue();
                        ImGui::Checkbox(skr::format("##Override{}", value.ident).c_str(), &override);
                        ImGui::TableNextColumn();
                        ImGui::Text("%d: \"%s\"", p.first, value.expr);
                        ImGui::TableNextColumn();
                        if(override)
                        {
                            if(!value.override)
                                value.override = value.value;
                            DrawInspect(value, value.override);
                        }
                        else
                        {
                            value.override.Reset();
                            ImGui::BeginDisabled(true);
                            DrawInspect(value, value.value);
                            ImGui::EndDisabled();
                        }
                        ImGui::TableNextColumn();
                    }
                    ImGui::EndTable();
                }
            }
        }
        ImGui::End();
    }
    
    SMutexObject _mutex;
};

inspected_object* add_inspected_object(void* value, const skr_type_t* type, const char* name, const char* file, int line)
{
    return (inspected_object*)_system->AddInspect(skr_value_ref_t{value, type}, name, file, line);
}
void update_inspected_value(inspected_object* tweak, void* value)
{
    return _system->UpdateInspect((tweak_value_t*)tweak, value);
}
void update_value_inspector()
{
    _system->DrawInspects();
}
}
SKR_MODULE_SUBSYSTEM(skr::inspect::inspect_system, SkrInspector);
#endif