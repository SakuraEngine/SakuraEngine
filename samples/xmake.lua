set_project("Sakura.Samples")

target("cgpu-mandelbrot")
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/cgpu-mandelbrot",
        dxil_outdir = "/../resources/shaders/cgpu-mandelbrot"})
    set_kind("binary")
    add_deps("SkrRT")
    add_files("cgpu-mandelbrot/*.c")
    add_files("cgpu-mandelbrot/**.hlsl")

target("cgpu-indexed-instance")
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/cgpu-indexed-instance",
        dxil_outdir = "/../resources/shaders/cgpu-indexed-instance"})
    set_kind("binary")
    add_deps("SkrRT")
    add_files("cgpu-indexed-instance/*.c")
    add_files("cgpu-indexed-instance/**.hlsl")

target("cgpu-texture")
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/cgpu-texture",
        dxil_outdir = "/../resources/shaders/cgpu-texture"})
    set_kind("binary")
    add_deps("SkrRT")
    add_files("cgpu-texture/*.c")
    add_files("cgpu-texture/**.hlsl")
    
--[[
if (os.host() == "windows") then
    target("hot-triangle")
        add_rules("utils.dxc", {
            spv_outdir = "/../resources/shaders/hot-triangle",
            dxil_outdir = "/../resources/shaders/hot-triangle"})
        set_kind("binary")
        add_deps("SkrRT")
        add_files("hot-triangle/triangle.c", "hot-triangle/hot_wasm.cpp")
        add_files("hot-triangle/**.hlsl")
end
]]--

target("cgpu-3d")
    add_rules("utils.install-resources", {
        extensions = {".gltf", ".bin", ".png"},
        outdir = "/../resources", _png_outdir = "/../resources/textures"})
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/cgpu-3d",
        dxil_outdir = "/../resources/shaders/cgpu-3d"})
    set_kind("binary")
    add_deps("SkrRT")
    add_files("cgpu-3d/**.cpp")
    add_files("cgpu-3d/**.hlsl")
    add_files("cgpu-3d/**.bin", "cgpu-3d/**.gltf", "cgpu-3d/**.png")

target("rg-triangle")
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/rg-triangle",
        dxil_outdir = "/../resources/shaders/rg-triangle"})
    set_kind("binary")
    add_deps("SkrRT")
    add_files("rg-triangle/*.cpp")
    add_files("rg-triangle/**.hlsl")

target("rg-deferred")
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/rg-deferred",
        dxil_outdir = "/../resources/shaders/rg-deferred"})
    set_kind("binary")
    add_deps("SkrRT")
    add_files("rg-deferred/*.cpp")
    add_files("rg-deferred/**.hlsl")

--独立出来RT以便Tool使用
target("GameRT")
    add_rules("c++.reflection", {
        files = {"game/**.h", "game/**.hpp"},
        rootdir = "game/"
    })
    add_includedirs("game/include", {public=true})
    add_defines("GAMERT_SHARED", {public=true})
    add_defines("GAMERT_IMPL")
    set_kind("shared")
    add_deps("SkrRT")
    add_files("game/src/**.cpp", "game/src/**.c")


target("GameTool")
    set_kind("shared")
    add_rules("c++.reflection", {
        files = {"gametool/**.h", "gametool/**.hpp"},
        rootdir = "gametool/"
    })
    add_includedirs("$(projectdir)/thirdparty/USD/include")
    add_includedirs("$(projectdir)/SDKs/python/include")
    add_includedirs("$(projectdir)/thirdparty/USD/include/boost-1_70")
    add_includedirs("gametool/include", {public=true})
    add_defines("GAMETOOL_SHARED", {public=true})
    add_defines("GAMETOOL_IMPL","BOOST_LIB_TOOLSET=vc141","BOOST_LIB_RT_OPT")
    add_deps("SkrTool", "GameRT")
    add_files("gametool/src/**.cpp")
    add_linkdirs("$(projectdir)/SDKs/python/libs")
    add_linkdirs("$(buildir)/$(os)/$(arch)/$(mode)/USD/lib")
    before_build(function (target)
        import("core.project.task")
        task.run("unzip-usd")
        local binrayDir = vformat("$(buildir)/$(os)/$(arch)/$(mode)")
        local usdlibs = os.files(path.join(binrayDir, "USD/lib/*.lib"))
        local links = {}
        for _, v in ipairs(usdlibs) do 
            table.insert(links, path.basename(v))
        end
        target:add("links", table.unpack(links))
    end)
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


target("Game")
    set_kind("binary")
    add_deps("GameRT")
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/Game",
        dxil_outdir = "/../resources/shaders/Game"})
    add_files("game/main.cpp", "game/render_resources.cpp")
    add_files("game/shaders/**.hlsl")