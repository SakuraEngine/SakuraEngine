target("GameRT")
    add_rules("c++.reflection", {
        files = {"game/**.h", "game/**.hpp"},
        rootdir = "game/"
    })
    add_includedirs("game/include", {public=true})
    add_defines("GAMERT_SHARED", {public=true})
    add_defines("GAMERT_IMPL")
    set_kind("shared")
    add_deps("SkrRT", "SkrGAInput", "SkrRenderer", "SkrImGui")
    add_files("game/src/**.cpp", "game/src/**.c")

target("Game")
    set_kind("binary")
    add_deps("GameRT")
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/Game",
        dxil_outdir = "/../resources/shaders/Game"})
    add_files("game/main.cpp", "game/render_resources.cpp")
    add_files("game/shaders/**.hlsl")

target("GameTool")
    set_kind("shared")
    add_rules("c++.reflection", {
        files = {"gametool/**.h", "gametool/**.hpp"},
        rootdir = "gametool/"
    })
    add_includedirs("gametool/include", {public=true})
    add_defines("GAMETOOL_SHARED", {public=true})
    add_defines("GAMETOOL_IMPL")
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