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
#include "misc/defer.hpp"
#include "SkrImGui/imgui_utils.h"


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
    skr::flat_hash_map<skr::string, tweak_value_map_t, skr::hash<skr::string>> _tweak_files;
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
                ImGui::Checkbox("##Value", &value.As<bool>());
            }
            break;
            case SKR_TYPE_CATEGORY_F32:
            {
                ImGui::DragFloat("##Value", &value.As<float>());
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
                ImGui::PushID(pair.first.c_str());
                if(ImGui::BeginTable("##Table", 3))
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
                        ImGui::PushID(value.ident);
                        ImGui::Checkbox("##Override", &override);
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
                        ImGui::PopID();
                        ImGui::TableNextColumn();
                    }
                    ImGui::EndTable();
                }
                ImGui::PopID();
            }
        }
        ImGui::End();
    }

    void DrawObject(const skr_value_ref_t& object, const skr::string& name);
    void DrawLeaf(const skr_value_ref_t& value, const skr::string& name);
    void DrawPropertyPannel(const skr_value_ref_t& object);
    
    SMutexObject _mutex;
};

void inspect_system::DrawObject(const skr_value_ref_t &object, const skr::string &name)
{
    SKR_ASSERT(object.type->type == SKR_TYPE_CATEGORY_OBJ);
    ImGui::PushID(object.ptr);
    SKR_DEFER({ ImGui::PopID(); });
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    bool nodeOpen = ImGui::TreeNodeEx("Object", ImGuiTreeNodeFlags_SpanFullWidth, "%s", name.c_str());
    if(nodeOpen)
    {
        auto& rtype = *(skr::type::RecordType*)object.type;
        for(auto& field : rtype.fields)
        {
            skr_value_ref_t child = {(char*)object.ptr + field.offset, field.type};
            if(field.type->type == SKR_TYPE_CATEGORY_OBJ)
                DrawObject(child, field.name);
            else
            {
                ImGui::PushID(&field);
                SKR_DEFER({ ImGui::PopID(); });
                DrawLeaf(child, field.name);
            }
        }
    }
}

void inspect_system::DrawLeaf(const skr_value_ref_t &value, const skr::string &name)
{
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;
    ImGui::TreeNodeEx("Field", flags, "%s", name.c_str());
    ImGui::TableSetColumnIndex(1);
    ImGui::SetNextItemWidth(-FLT_MIN);
    switch(value.type->type)
    {
        case SKR_TYPE_CATEGORY_I32:
        {
            ImGui::InputScalar("##value", ImGuiDataType_S32, value.ptr);
            break;
        }
        case SKR_TYPE_CATEGORY_U32:
        {
            ImGui::InputScalar("##value", ImGuiDataType_U32, value.ptr);
            break;
        }
        case SKR_TYPE_CATEGORY_I64:
        {
            ImGui::InputScalar("##value", ImGuiDataType_S64, value.ptr);
            break;
        }
        case SKR_TYPE_CATEGORY_U64:
        {
            ImGui::InputScalar("##value", ImGuiDataType_U64, value.ptr);
            break;
        }
        case SKR_TYPE_CATEGORY_F32:
        {
            ImGui::InputFloat("##value", (float*)value.ptr);
            break;
        }
        case SKR_TYPE_CATEGORY_F64:
        {
            ImGui::InputDouble("##value", (double*)value.ptr);
            break;
        }
        case SKR_TYPE_CATEGORY_BOOL:
        {
            ImGui::Checkbox("##value", (bool*)value.ptr);
            break;
        }
        case SKR_TYPE_CATEGORY_STR:
        {
            skr::string* v = (skr::string*)value.ptr;
            ImGui::InputText("##value", v);
            break;
        }
        case SKR_TYPE_CATEGORY_HANDLE:
            //TODO: draw handle
        default:
        {
            ImGui::Text("Unsupported field type");
            break;
        }
    }
}

void inspect_system::DrawPropertyPannel(const skr_value_ref_t &object)
{
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
    if(ImGui::BeginTable("Properties", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable))
    {
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Value");
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();
        DrawObject(object, u8"Root");
        ImGui::EndTable();
    }
    ImGui::PopStyleVar();
}

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