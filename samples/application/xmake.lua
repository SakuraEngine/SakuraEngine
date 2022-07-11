target("GameRT")
    add_rules("skr.module", {api = "GAMERT"})
    add_rules("c++.reflection", {
        files = {"game/**.h", "game/**.hpp"},
        rootdir = "game/"
    })
    add_includedirs("game/include", {public=true})
    add_deps("SkrRT", "SkrScene", "SkrGAInput", "SkrRenderer", "SkrImGui")
    add_files("game/src/**.cpp", "game/src/**.c")

target("Game")
    set_kind("binary")
    add_deps("GameRT")
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/Game",
        dxil_outdir = "/../resources/shaders/Game"})
    add_files("game/main.cpp", "game/render_resources.cpp", "game/render_effects.cpp",  "game/game_module.cpp")
    add_files("game/shaders/**.hlsl")

target("GameTool")
    add_rules("skr.module", {api = "GAMETOOL"})
    add_rules("c++.reflection", {
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