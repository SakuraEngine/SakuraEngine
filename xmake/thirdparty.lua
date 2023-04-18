sdk_libs_dir = "$(projectdir)/SDKs/libs/"

-- this must be on the top because other thirdparty libs may depend on it
-- once a lib uses EASTL, it must link to SkrRT runtime
includes("thirdparty/OpenString.lua") 
includes("thirdparty/EASTL.lua") 
includes("thirdparty/lmdb.lua")
includes("thirdparty/boost.lua")
includes("thirdparty/SDL2.lua")
includes("thirdparty/cgltf.lua")
includes("thirdparty/tracy.lua")
includes("thirdparty/platform_sdks.lua")
includes("thirdparty/parallel_hashmap.lua")
includes("thirdparty/FiberTaskingLib.lua")
includes("thirdparty/simdjson.lua")
includes("thirdparty/fmt.lua")
includes("thirdparty/usd.lua")
includes("thirdparty/lua.lua")
includes("thirdparty/gamenetworkingsockets.lua")