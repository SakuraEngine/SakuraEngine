#include "SkrTweak/module.h"

#ifdef TWEAK_USABLE
#include <fstream>
#include "efsw/efsw.hpp"
#include "platform/filesystem.hpp"
#include "platform/thread.h"
#include "utils/hash.h"
#include "utils/log.h"
#include <containers/hashmap.hpp>
#include <EASTL/vector.h>
#include <containers/variant.hpp>
#include <containers/string.hpp>

struct skr_tweak_value_t
{
    skr::variant<float, int, bool, skr::string> value;
};

class SkrTweakModule : public skr::IDynamicModule, public efsw::FileWatchListener
{
public:
    static SkrTweakModule* instance;
    virtual void on_load(int argc, char8_t** argv) override
    {
        instance = this;
        _watcher.watch();
    }
    virtual void on_unload() override
    {
    }

    struct TweakLine
    {
        int line_number;
        eastl::vector<skr_tweak_value_t*> tweaks;
    };

    void handleFileAction(efsw::WatchID watchid, const std::string& dir, const std::string& filename, efsw::Action action, std::string oldFilename = "" ) override
    {
        if (action == efsw::Actions::Modified)
        {
            skr::filesystem::path dirPath(dir);
            skr::filesystem::path filePath(filename);
            auto fullPath = dirPath / filePath;
            fullPath = fullPath.lexically_normal();
            SMutexLock lock(_mutex.mMutex);
            UpdateTweaks(fullPath);
        }
    }

    void UpdateTweaks(skr::filesystem::path path)
    {
        std::ifstream file( path, std::ios::in );

        if ( !file.is_open() ) {

            SKR_LOG_ERROR( "Unable to open file for tweak: '%'", path.c_str() );
            return;
        }

        size_t current_line_num = 1;
        auto& tweak_file = _tweak_files[path.u8string().c_str()];
        size_t tweak_line_index = 0;
        while ( file.good() && tweak_line_index < tweak_file.size() ) {
            std::string line;
            //skip lines until we get to the line we want
            while(file.good() && current_line_num < tweak_file[tweak_line_index].line_number)
            {
                file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                current_line_num++;
            }
            if(!file.good())
                break;
            std::getline(file, line);
            //parse the line
            if(file.good() && current_line_num == tweak_file[tweak_line_index].line_number)
            {
                size_t tweak_index = 0;
                std::string_view sv = {line.c_str(), line.size()};
                
                auto pos = sv.find("TWEAK");
                while(pos != std::string_view::npos && tweak_index < tweak_file[tweak_line_index].tweaks.size())
                {
                    auto end = sv.find(')', pos);
                    SKR_ASSERT(end != std::string_view::npos);
                    auto stdValueStr = sv.substr(pos + 6, end - pos - 6);
                    auto valueStr = skr::string_view((const char8_t*)stdValueStr.data(), stdValueStr.size());
                    auto tweak = tweak_file[tweak_line_index].tweaks[tweak_index];
                    
                    std::visit([&](auto& value)
                    {
                        ParseTweak(value, valueStr);
                    }, tweak->value);  
                    tweak_index++;
                    pos = sv.find("TWEAK", end + 1);
                }
                tweak_line_index++;
            }     
            current_line_num++;          
        }
    }

    template<class T>
    skr_tweak_value_t* TweakValue(T value, const char* str, const char* fileName, int lineNumber)
    {
        SMutexLock lock(_mutex.mMutex);
        static skr::filesystem::path root = SKR_SOURCE_ROOT;
        skr::filesystem::path path = (root / fileName).lexically_normal();
        auto directory = path.parent_path().lexically_normal().u8string();
        if(!_watched_directories.contains(directory.c_str()))
        {
            _watcher.addWatch((const char*)directory.c_str(), this, false);
            _watched_directories.insert(directory.c_str());
        }
        auto& file = _tweak_files[path.u8string().c_str()];
        TweakLine* line = nullptr;
        for(auto& lines : file)
        {
            if(lines.line_number == lineNumber)
            {
                line = &lines;
            }
        }
        if(line == nullptr)
        {
            line = &file.emplace_back();
            line->line_number = lineNumber;
        }

        auto tweak =  line->tweaks.emplace_back(new skr_tweak_value_t{value});
        //initialize the tweak
        UpdateTweaks(path);
        return tweak;
    }

    void ParseTweak(int& value, skr::string_view str)
    {
        value = std::stoi({str.c_str(), (size_t)str.size()});
    }

    void ParseTweak(float& value, skr::string_view str)
    {
        value = std::stof({str.c_str(), (size_t)str.size()});
    }

    void ParseTweak(bool& value, skr::string_view str)
    {
        value = str == skr::string_view(u8"true");
    }

    void ParseTweak(skr::string& value, skr::string_view str)
    {
        value = str;
    }

    SMutexObject _mutex;
    efsw::FileWatcher _watcher;
    skr::flat_hash_set<skr::string, skr::hash<skr::string>> _watched_directories;
    skr::flat_hash_map<skr::string, eastl::vector<TweakLine>, skr::hash<skr::string>> _tweak_files;
};
SkrTweakModule* SkrTweakModule::instance = nullptr;

IMPLEMENT_DYNAMIC_MODULE(SkrTweakModule, SkrTweak);

skr_tweak_int_t* skr_tweak_value(int value, const char* str, const char* fileName, int lineNumber)
{
    return (skr_tweak_int_t*)SkrTweakModule::instance->TweakValue(value, str, fileName, lineNumber);
}
int skr_get_tweak(skr_tweak_int_t* tweak)
{
    return std::get<int>(((skr_tweak_value_t*)tweak)->value);
}
skr_tweak_float_t* skr_tweak_value(float value, const char* str, const char* fileName, int lineNumber)
{
    return (skr_tweak_float_t*)SkrTweakModule::instance->TweakValue(value, str, fileName, lineNumber);
}
float skr_get_tweak(skr_tweak_float_t* tweak)
{
    return std::get<float>(((skr_tweak_value_t*)tweak)->value);
}
skr_tweak_bool_t* skr_tweak_value(bool value, const char* str, const char* fileName, int lineNumber)
{
    return (skr_tweak_bool_t*)SkrTweakModule::instance->TweakValue(value, str, fileName, lineNumber);
}
bool skr_get_tweak(skr_tweak_bool_t* tweak)
{
    return std::get<bool>(((skr_tweak_value_t*)tweak)->value);
}
skr_tweak_string_t* skr_tweak_value(const char8_t* value, const char* str, const char* fileName, int lineNumber)
{
    return (skr_tweak_string_t*)SkrTweakModule::instance->TweakValue(skr::string(value), str, fileName, lineNumber);
}
const char* skr_get_tweak(skr_tweak_string_t* tweak)
{
    return std::get<skr::string>(((skr_tweak_value_t*)tweak)->value).c_str();
}
#else
SKR_TWEAK_API skr_tweak_int_t* skr_tweak_value(int value, const char* str, const char* fileName, int lineNumber)
{
    return nullptr;
}
SKR_TWEAK_API int skr_get_tweak(skr_tweak_int_t* tweak)
{
    return INT_MAX;
}
SKR_TWEAK_API skr_tweak_float_t* skr_tweak_value(float value, const char* str, const char* fileName, int lineNumber)
{
    return nullptr;
}
SKR_TWEAK_API float skr_get_tweak(skr_tweak_float_t* tweak)
{
    return FLT_MAX;
}
SKR_TWEAK_API skr_tweak_bool_t* skr_tweak_value(bool value, const char* str, const char* fileName, int lineNumber)
{
    return nullptr;
}
SKR_TWEAK_API bool skr_get_tweak(skr_tweak_bool_t* tweak)
{
    return false;
}
SKR_TWEAK_API skr_tweak_string_t* skr_tweak_value(const char* value, const char* str, const char* fileName, int lineNumber)
{
    return nullptr;
}
SKR_TWEAK_API const char* skr_get_tweak(skr_tweak_string_t* tweak)
{
    return nullptr;
}
#endif