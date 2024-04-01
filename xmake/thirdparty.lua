sdk_libs_dir = "$(projectdir)/SDKs/libs/"

-- this must be on the top because other thirdparty libs may depend on it
includes("thirdparty/mimalloc.lua")
includes("thirdparty/platform_sdks.lua")
includes("thirdparty/gamenetworkingsockets.lua")