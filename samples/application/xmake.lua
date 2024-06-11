includes("ogui2/xmake.lua")

-- includes("multiplayer/xmake.lua") -- FIXME. lua support
-- includes("game/xmake.lua") -- FIXME. lua support

if os.host() == "windows" and false then
    includes("chat/xmake.lua")
end

executable_module("VMemController", "VMEM_CONTROLLER", engine_version)
    set_group("04.examples/application")
    public_dependency("SkrRenderGraph", engine_version)
    public_dependency("SkrImGui", engine_version)
    set_exceptions("no-cxx")
    add_includedirs("./../common", {public = false})
    add_rules("c++.unity_build", {batchsize = default_unity_batch})
    add_files("vmem_controller/**.cpp")

executable_module("Live2DViewer", "LIVE2D_VIEWER", engine_version)
    set_group("04.examples/application")
    public_dependency("SkrLive2D", engine_version)
    public_dependency("SkrImGui", engine_version)
    add_rules("utils.install_resources", {
        extensions = {".json", ".moc3", ".png"},
        outdir = "/../resources/Live2DViewer", 
        rootdir = os.curdir().."/live2d-viewer/resources"})
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/Live2DViewer",
        dxil_outdir = "/../resources/shaders/Live2DViewer"})
    set_exceptions("no-cxx")
    add_rules("c++.unity_build", {batchsize = default_unity_batch})
    add_includedirs("./../common", {public = false})
    add_includedirs("live2d-viewer/include", {public=true})
    add_files("live2d-viewer/src/main.cpp", "live2d-viewer/src/viewer_module.cpp", "live2d-viewer/src/imgui.cpp")
    -- add_files("live2d-viewer/shaders/**.hlsl")
    add_files("live2d-viewer/**.json", "live2d-viewer/**.moc3", "live2d-viewer/**.png")

--[[
    if(not has_config("shipping_one_archive")) then

    shared_module("GameTool", "GAMETOOL", engine_version)
        set_group("04.examples/application")
        public_dependency("SkrToolCore", "0.1.0")
        public_dependency("GameRuntime", "0.1.0")
        add_includedirs("gametool/include", {public=true})
        add_includedirs("./../common", {public = false})
        add_files("gametool/src/**.cpp")
        add_rules("c++.unity_build", {batchsize = default_unity_batch})
        on_config(function (target, opt)
            local dep = target:dep("GameRuntime");
            local toolgendir = path.join(dep:autogendir({root = true}), dep:plat(), "codegen", dep:name(), "tool")
            if os.exists(toolgendir) then
                target:add("includedirs", toolgendir)
                local cppfiles = os.files(path.join(toolgendir, "/*.cpp"))
                for _, file in ipairs(cppfiles) do
                    target:add("files", file)
                end
            end
        end)

    end
]]--
