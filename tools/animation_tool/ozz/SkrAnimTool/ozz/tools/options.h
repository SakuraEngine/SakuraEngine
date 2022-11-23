#pragma once
#include "SkrAnimTool/ozz/tools/export.h"
#include "SkrAnim/ozz/base/containers/string.h"
#include "SkrAnim/ozz/base/endianness.h"
#include "tools/jsoncpp/dist/json/json-forwards.h"

namespace ozz {
namespace animation {
namespace offline {
    struct Options
    {
        virtual ~Options() {}
        virtual Json::Value& GetConfig() = 0;
        virtual ozz::Endianness GetEndian() = 0;
        virtual ozz::string GetFile() = 0;
        virtual ozz::string GetOutput() = 0;
        struct Params
        {
            ozz::string config; //config content
            ozz::string endian;
            ozz::string file; //input file
            ozz::string name; //imported animation name
            ozz::string output; //output file
        };
        static Options* Create(const Params& _params);
    }; 
}
}
}