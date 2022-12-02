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
#include "SkrImgui/skr_imgui.h"
#include "utils/format.hpp"


namespace skr::inspect
{
struct tweak_value_t
{
    std::chrono::time_point<std::chrono::milliseconds> last_modified;
    //TODO: scriptable override & assert
    skr::optional<skr::variant<float, int, bool, skr::string>> override;
    skr::optional<skr::variant<float, int, bool, skr::string>> condition;
    skr::variant<float, int, bool, skr::string> value;
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

    template<class T>
    T UpdateInspect(tweak_value_t* tweak, const T& value)
    {
        SMutexLock lock(tweak->_mutex.mMutex);
        tweak->value = value;
        if(tweak->condition)
        {
            SKR_ASSERT(value != skr::get<T>(tweak->condition.value()));
            tweak->condition = skr::nullopt;
        }
        if(tweak->override)
        {
            return skr::get<T>(tweak->override.value());
        }
        return value;
    }

    void DrawInspect(tweak_value_t* value, int& data)
    {
        ImGui::DragInt(skr::format("value##{}", value->ident).c_str(), &data, 1.0f, 0, 0, "%d", ImGuiSliderFlags_AlwaysClamp);
    }

    void DrawInspect(tweak_value_t* value, float& data)
    {
        ImGui::DragFloat(skr::format("value##{}", value->ident).c_str(), &data, 0.01f, 0, 0, "%.3f", ImGuiSliderFlags_AlwaysClamp);
    }

    void DrawInspect(tweak_value_t* value, bool& data)
    {
        ImGui::Checkbox(skr::format("value##{}", value->ident).c_str(), &data);
    }

    void DrawInspect(tweak_value_t* value, skr::string& data)
    {
        ImGui::InputText(skr::format("value##{}", value->ident).c_str(), data.data(), data.size());
    }

    void DrawInspects()
    {
        ImGui::Begin("ValueInspector");
        for(auto& pair : _tweak_files)
        {
            if(ImGui::CollapsingHeader(pair.first.c_str()))
            {
                for(auto& p : pair.second)
                {
                    auto& value = p.second;
                    bool override = value.override.has_value();
                    ImGui::Checkbox(skr::format("Line {}: \"{}\"##{}", p.first, value.expr, value.ident).c_str(), &override);
                    ImGui::SameLine();
                    if(override)
                    {
                        if(!value.override.has_value())
                            value.override = value.value;
                        std::visit([&](auto& arg) { DrawInspect(&value, arg); }, value.override.value());
                    }
                    else
                    {
                        value.override = skr::nullopt;
                        ImGui::BeginDisabled(true);
                        std::visit([&](auto& arg) { DrawInspect(&value, arg); }, value.value);
                        ImGui::EndDisabled();
                    }
                }
            }
        }
        ImGui::End();
    }
    
    SMutexObject _mutex;
};
inspected_int* add_inspected_value(int value, const char* name, const char* file, int line)
{
    return (inspected_int*)_system->AddInspect(value, name, file, line);
}
inspected_float* add_inspected_value(float value, const char* name, const char* file, int line)
{
    return (inspected_float*)_system->AddInspect(value, name, file, line);
}
inspected_bool* add_inspected_value(bool value, const char* name, const char* file, int line)
{
    return (inspected_bool*)_system->AddInspect(value, name, file, line);
}
inspected_string* add_inspected_value(const char* value, const char* name, const char* file, int line)
{
    return (inspected_string*)_system->AddInspect(value, name, file, line);
}
int update_inspected_value(inspected_int* tweak, int value)
{
    return _system->UpdateInspect((tweak_value_t*)tweak, value);
}
float update_inspected_value(inspected_float* tweak, float value)
{
    return _system->UpdateInspect((tweak_value_t*)tweak, value);
}
bool update_inspected_value(inspected_bool* tweak, bool value)
{
    return _system->UpdateInspect((tweak_value_t*)tweak, value);
}
const char* update_inspected_value(inspected_string* tweak, const char* value)
{
    return _system->UpdateInspect((tweak_value_t*)tweak, skr::string(value)).c_str();
}
void update_value_inspector()
{
    _system->DrawInspects();
}
}
SKR_MODULE_SUBSYSTEM(skr::inspect::inspect_system, SkrInspector);
#endif