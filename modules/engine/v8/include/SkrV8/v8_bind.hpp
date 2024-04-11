#pragma once
#include <v8.h>

namespace skr
{
template <typename T>
struct V8Bind {
    inline static v8::Local<v8::Value> to_v8(v8::Local<v8::Context> context, const T& value)
    {
#ifndef __meta__
        static_assert(std::is_same_v<T, T*>, "V8Bind::to_v8 not implemented");
#endif
    }

    inline static T from_v8(v8::Local<v8::Context> context, const v8::Local<v8::Value>& value)
    {
#ifndef __meta__
        static_assert(std::is_same_v<T, T*>, "V8Bind::from_v8 not implemented");
#endif
    }
};
} // namespace skr