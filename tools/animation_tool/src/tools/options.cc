#include "SkrAnimTool/ozz/tools/options.h"
#include "SkrAnim/ozz/base/log.h"
#include "tools/jsoncpp/dist/json/json.h"
#include "import2ozz_config.h"

static bool ValidateEndianness(const ozz::animation::offline::Options::Params& option) {
  bool valid = std::strcmp(option.endian.c_str(), "native") == 0 ||
               std::strcmp(option.endian.c_str(), "little") == 0 ||
               std::strcmp(option.endian.c_str(), "big") == 0;
  if (!valid) {
    ozz::log::Err() << "Invalid endianness option \"" << option.endian << "\""
                    << std::endl;
  }
  return valid;
}

ozz::Endianness InitializeEndianness(const ozz::animation::offline::Options::Params& option) {
  // Initializes output endianness from options.
  ozz::Endianness endianness = ozz::GetNativeEndianness();
  if (std::strcmp(option.endian.c_str(), "little") == 0) {
    endianness = ozz::kLittleEndian;
  } else if (std::strcmp(option.endian.c_str(), "big") == 0) {
    endianness = ozz::kBigEndian;
  }
  ozz::log::LogV() << (endianness == ozz::kLittleEndian ? "Little" : "Big")
                   << " endian output binary format selected." << std::endl;
  return endianness;
}

// static bool ValidateLogLevel(const ozz::animation::offline::Options::Params& option) {
//   bool valid = std::strcmp(option.value(), "verbose") == 0 ||
//                std::strcmp(option.value(), "standard") == 0 ||
//                std::strcmp(option.value(), "silent") == 0;
//   if (!valid) {
//     ozz::log::Err() << "Invalid log level option \"" << option << "\""
//                     << std::endl;
//   }
//   return valid;
// }

// void InitializeLogLevel(const ozz::animation::offline::Options::Params& option) {
//   ozz::log::Level log_level = ozz::log::GetLevel();
//   if (std::strcmp(OPTIONS_log_level, "silent") == 0) {
//     log_level = ozz::log::kSilent;
//   } else if (std::strcmp(OPTIONS_log_level, "standard") == 0) {
//     log_level = ozz::log::kStandard;
//   } else if (std::strcmp(OPTIONS_log_level, "verbose") == 0) {
//     log_level = ozz::log::kVerbose;
//   }
//   ozz::log::SetLevel(log_level);
//   ozz::log::LogV() << "Verbose log level activated." << std::endl;
// }

namespace ozz {
namespace animation {
namespace offline {

struct OptionsImpl : Options
{
    OptionsImpl(Json::Value _config, ozz::Endianness _endianness, ozz::string _file, ozz::string _output)
        : config(std::move(_config))
        , endianness(_endianness)
        , file(std::move(_file))
        , output(std::move(_output))
    {
    }
    virtual Json::Value& GetConfig() override { return config; }
    virtual ozz::Endianness GetEndian() override { return endianness; }
    virtual ozz::string GetFile() override { return file; }
    virtual ozz::string GetOutput() override { return output; }

    Json::Value config;
    ozz::Endianness endianness;
    ozz::string file;
    ozz::string output;
};

Options* Options::Create(const Options::Params& params)
{
    Json::Value config;
    
    ozz::log::SetLevel(ozz::log::kStandard);
    ozz::Endianness endian = ozz::GetNativeEndianness();
    if(!ProcessConfiguration(&config, params.config))
        return nullptr;
    return new OptionsImpl(std::move(config), endian, params.file, params.output);
}
}
}
}