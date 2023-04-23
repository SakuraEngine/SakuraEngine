///////////////////////////////////////////////////////////////////////////////
// EADateTime.h
//
// Copyright (c) Electronic Arts. All rights reserved.
///////////////////////////////////////////////////////////////////////////////

#ifndef EASTDC_EADATETIME_H
#define EASTDC_EADATETIME_H

#include <chrono>

#include "../internal/config.h"


#define EASTDC_API EASTL_EASTDC_API

namespace EA
{
	namespace StdC
	{

		inline EASTDC_API uint64_t GetTime()
		{
			using namespace std::chrono;
			nanoseconds ns = duration_cast<nanoseconds>(system_clock::now().time_since_epoch());
			return ns.count();
		}

	} // namespace StdC
} // namespace EA


#endif // EASTDC_EADATETIME_H
