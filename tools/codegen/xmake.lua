
generator_list["json"] = 
        {
            os.projectdir().."/tools/codegen/serialize_json.py",
            os.projectdir().."/tools/codegen/json_reader.h.mako",
            os.projectdir().."/tools/codegen/json_reader.cpp.mako",
            os.projectdir().."/tools/codegen/json_writer.h.mako",
            os.projectdir().."/tools/codegen/json_writer.cpp.mako"
        };
generator_list["serialize"] = 
        {
            os.projectdir().."/tools/codegen/serialize.py",
            os.projectdir().."/tools/codegen/serialize.h.mako",
        };
generator_list["typeid"] = 
        {
            os.projectdir().."/tools/codegen/typeid.py",
            os.projectdir().."/tools/codegen/typeid.hpp.mako",
        };
generator_list["config"] =
        {
            os.projectdir().."/tools/codegen/config_resource.py",
            os.projectdir().."/tools/codegen/config_resource.cpp.mako",
        };
generator_list["rtti"] = 
        {
            os.projectdir().."/tools/codegen/rtti.py",
            os.projectdir().."/tools/codegen/rtti.cpp.mako",
            os.projectdir().."/tools/codegen/rtti.hpp.mako",
        };
generator_list["config_asset"] = 
        {
            os.projectdir().."/tools/codegen/config_asset.py",
            os.projectdir().."/tools/codegen/config_asset.cpp.mako",
            gendir = toolgendir
        };
generator_list["importer"] =
        {
            os.projectdir().."/tools/codegen/importer.py",
            os.projectdir().."/tools/codegen/importer.cpp.mako",
        };