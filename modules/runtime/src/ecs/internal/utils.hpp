#pragma once
#include <type_traits> // std::decay_t

#ifndef forloop
#define forloop(i, z, n) for (auto i = std::decay_t<decltype(n)>(z); i < (n); ++i)
#endif