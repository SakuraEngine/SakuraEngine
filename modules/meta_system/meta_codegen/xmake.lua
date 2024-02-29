shared_module("SkrMetaCodegen", "SKR_META_CODEGEN", engine_version)
    public_dependency("SkrCore", engine_version)
    add_files("src/**.cpp")