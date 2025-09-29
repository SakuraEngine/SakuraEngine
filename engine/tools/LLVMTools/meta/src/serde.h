#pragma once
#include "llvm/Support/JSON.h"

// serde macro
#define META_SERDE(__F) serde(s, #__F, v.__F);
#define META_SERDE_N(__F, __N) serde(s, __N, v.__F);
#define META_SERDE_FUNCTION(__Type) inline void serde(llvm::json::OStream &s, std::string_view key, __Type &v)
#define META_SERDE_FWD(__Type) void serde(llvm::json::OStream &s, std::string_view key, __Type &v);

namespace meta {
// serde helper function
template <typename Func>
void serde_obj(llvm::json::OStream &s, std::string_view key, Func &&func) {
  if (key.empty()) {
    s.object(func);
  } else {
    s.attributeObject(key, func);
  }
}

// serde primitive type
template <typename T>
concept SerdePrimitiveType =
    std::is_integral_v<T> ||
    std::is_floating_point_v<T> ||
    std::is_same_v<T, bool> ||
    std::is_same_v<T, std::string>;
template <SerdePrimitiveType T>
void serde(llvm::json::OStream &s, std::string_view key, T &v) {
  if (key.empty()) {
    s.value(v);
  } else {
    s.attribute(key, v);
  }
}

// serde vector type
template <typename T>
void serde(llvm::json::OStream &s, std::string_view key, std::vector<T> &v) {
  assert(!key.empty() && "json array cannot naested");
  s.attributeArray(key, [&] {
    for (auto &i : v) {
      serde(s, "", i);
    }
  });
}

// serde string map
template <typename T>
void serde(llvm::json::OStream &s, std::string_view key, std::unordered_map<std::string, T> &v) {
  serde_obj(s, key, [&] {
    for (auto &[k, i] : v) {
      serde(s, k, i);
    }
  });
}
} // namespace meta