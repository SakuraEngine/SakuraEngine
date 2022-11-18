#pragma endian little
#include <type/guid.pat>

struct resource_header
{
	u32 headerVersion;
	u32 Version;
	type::GUID resourceId;
	type::GUID resourceType;
	u8 dependencyCount;
    type::GUID dependencies[dependencyCount];
};

resource_header file @ 0x00;