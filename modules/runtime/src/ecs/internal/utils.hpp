#pragma once

#ifndef forloop
#define forloop(i, z, n) for (auto i = eastl::decay_t<decltype(n)>(z); i < (n); ++i)
#endif