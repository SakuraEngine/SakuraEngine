sdk_libs_dir = "$(projectdir)/SDKs/libs/"

-- this must be on the top because other thirdparty libs may depend on it
-- once a lib uses EASTL, it must link to SkrRT runtime
includes("thirdparty/OpenString.lua") 
includes("thirdparty/mimalloc.lua")
includes("thirdparty/boost.lua")
includes("thirdparty/SDL2.lua")
includes("thirdparty/tracy.lua")
includes("thirdparty/platform_sdks.lua")
includes("thirdparty/FiberTaskingLib.lua")
includes("thirdparty/gamenetworkingsockets.lua")