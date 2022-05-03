#include "simdjson.h"

#if defined(__cplusplus)
struct skr_json_reader_t {
    simdjson::ondemand::value* json;
};
// utils for codegen
namespace skr
{
namespace json
{
template <class T>
void Read(simdjson::ondemand::value&& json, T& value);
} // namespace json
} // namespace skr
#else
typedef struct skr_json_reader_t skr_json_reader_t;
#endif