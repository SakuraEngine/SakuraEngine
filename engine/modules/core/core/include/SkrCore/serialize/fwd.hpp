#pragma once
#include <SkrBase/config.h>

// fwd
namespace skr
{
template <typename T>
struct Serialize;

struct Archive;
struct ArchiveWrite;
struct ArchiveRead;
} // namespace skr

// serialize template and concepts
// serialize template and concepts
namespace skr::concepts
{
template <typename T>
concept HasSerdeWrite = requires(ArchiveWrite& w, const T& v) {
    { ::skr::Serialize<T>::write(w, v) } -> std::same_as<void>;
};
template <typename T>
concept HasSerdeRead = requires(ArchiveRead& r, T& v) {
    { ::skr::Serialize<T>::read(r, v) } -> std::same_as<void>;
};
template <typename T>
concept HasSerdeWriteFields = requires(ArchiveWrite& w, const T& v) {
    { ::skr::Serialize<T>::write_fields(w, v) } -> std::same_as<void>;
};
template <typename T>
concept HasSerdeReadFields = requires(ArchiveRead& r, T& v) {
    { ::skr::Serialize<T>::read_fields(r, v) } -> std::same_as<void>;
};
template <typename T>
concept IsSerdePrimitive = std::is_same_v<T, bool> ||

    std::is_same_v<T, int8_t> || std::is_same_v<T, int16_t> ||
    std::is_same_v<T, int32_t> || std::is_same_v<T, int64_t> ||

    std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> ||
    std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t> ||

    std::is_same_v<T, float> || std::is_same_v<T, double>;
} // namespace skr::concepts