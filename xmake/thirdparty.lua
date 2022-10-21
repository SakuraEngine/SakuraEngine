-- this must be on the top because other thirdparty libs may depend on it
-- once a lib uses EASTL, it must link to SkrRT runtime
includes("thirdparty/EASTL.lua") 
includes("thirdparty/lmdb.lua")
includes("thirdparty/mimalloc.lua")
includes("thirdparty/boost.lua")
includes("thirdparty/gsl.lua")
includes("thirdparty/DirectXMath.lua")
includes("thirdparty/SDL2.lua")
includes("thirdparty/cgltf.lua")
includes("thirdparty/tracy.lua")
includes("thirdparty/platform_sdks.lua")
if (is_os("windows")) then 
    includes("thirdparty/wasm3.lua")
end
includes("thirdparty/parallel_hashmap.lua")
includes("thirdparty/FiberTaskingLib.lua")
includes("thirdparty/simdjson.lua")
includes("thirdparty/fmt.lua")
includes("thirdparty/ghc_filesys.lua")
includes("thirdparty/zlib.lua")
includes("thirdparty/usd.lua")