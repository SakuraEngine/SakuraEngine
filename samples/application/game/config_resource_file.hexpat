#pragma endian little
#include <type/guid.pat>

struct config_resource
{
	type::GUID configType;
};

enum ECGPUBackend : u32 {
	Vulkan,
	DX12,
};

struct backend_config
{
	ECGPUBackend backend;
};

struct backend_config_file {
	config_resource resource;
	backend_config config;
};

backend_config_file file @ 0x00;
