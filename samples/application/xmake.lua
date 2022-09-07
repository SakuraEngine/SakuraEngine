target("GameRT")
    add_rules("skr.module", {api = "GAMERT"})
    add_rules("c++.codegen", {
        files = {"game/**.h", "game/**.hpp"},
        rootdir = "game/"
    })
    add_includedirs("game/include", {public=true})
    add_deps("SkrRT", "SkrScene", "SkrRenderer", "SkrImGui", "SkrInputSystem")
    add_files("game/src/**.cpp")

target("Game")
    set_kind("binary")
    add_deps("GameRT")
    add_rules("utils.install-resources", {
        extensions = {".gltf", ".bin", ".png"},
        outdir = "/../resources", _png_outdir = "/../resources/textures"})
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/Game",
        dxil_outdir = "/../resources/shaders/Game"})
    add_files("game/main.cpp", "game/render_resources.cpp", "game/render_effects.cpp",  "game/game_module.cpp")
    add_files("game/shaders/**.hlsl")
    add_files("game/**.bin", "game/**.gltf")
    if (is_os("windows")) then 
        add_files("/../../resources/windows/sakura.rc")
    end

target("VMemController")
    set_kind("binary")
    add_deps("SkrImGui", "SkrRenderGraph")
    add_files("vmem_controller/**.cpp")
    if (is_os("windows")) then 
        add_files("/../../resources/windows/sakura.rc")
    end

target("GameTool")
    add_rules("skr.module", {api = "GAMETOOL"})
    add_rules("c++.codegen", {
        files = {"gametool/**.h", "gametool/**.hpp"},
        rootdir = "gametool/"
    })
    add_includedirs("gametool/include", {public=true})
    add_deps("SkrTool", "GameRT")
    add_files("gametool/src/**.cpp")
    on_config(function (target, opt)
        local dep = target:dep("GameRT");
        local toolgendir = path.join(dep:autogendir({root = true}), dep:plat(), "tool/generated", dep:name())
        if os.exists(toolgendir) then
            target:add("includedirs", toolgendir)
            local cppfiles = os.files(path.join(toolgendir, "/**.cpp"))
            for _, file in ipairs(cppfiles) do
                target:add("files", file)
            end
        end
    end)

if (os.host() == "windows" and has_config("build_chat")) then
    includes("chat/xmake.lua")
end

if (os.host() == "windows") then
    target("Live2DViewer")
        set_kind("binary")
        add_deps("SkrLive2D", "SkrImGui")
        add_rules("utils.install-resources", {
            extensions = {".json", ".moc3", ".png"},
            outdir = "/../resources/Live2DViewer", 
            rootdir = os.curdir().."/live2d-viewer/resources"})
        add_rules("utils.dxc", {
            spv_outdir = "/../resources/shaders/Live2DViewer",
            dxil_outdir = "/../resources/shaders/Live2DViewer"})
        add_includedirs("live2d-viewer/include", {public=true})
        add_files("live2d-viewer/src/main.cpp", "live2d-viewer/src/viewer_module.cpp", "live2d-viewer/src/imgui.cpp")
        -- add_files("live2d-viewer/shaders/**.hlsl")
        add_files("live2d-viewer/**.json", "live2d-viewer/**.moc3", "live2d-viewer/**.png")
        if (is_os("windows")) then 
            add_files("/../../resources/windows/sakura.rc")
        end
end