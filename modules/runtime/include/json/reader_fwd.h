#pragma once
#include "platform/configure.h"
#include <type_traits>

struct skr_json_reader_t;

#if defined(__cplusplus)

    #define SKR_SIMDJSON_PLAT fallback
namespace simdjson
{
namespace SKR_SIMDJSON_PLAT
{
namespace ondemand
{
class value;
}
} // namespace SKR_SIMDJSON_PLAT
} // namespace simdjson

namespace skr
{
namespace json
{
enum error_code
{
    SUCCESS = 0,                ///< No error
    CAPACITY,                   ///< This parser can't support a document that big
    MEMALLOC,                   ///< Error allocating memory, most likely out of memory
    TAPE_ERROR,                 ///< Something went wrong while writing to the tape (stage 2), this is a generic error
    DEPTH_ERROR,                ///< Your document exceeds the user-specified depth limitation
    STRING_ERROR,               ///< Problem while parsing a string
    T_ATOM_ERROR,               ///< Problem while parsing an atom starting with the letter 't'
    F_ATOM_ERROR,               ///< Problem while parsing an atom starting with the letter 'f'
    N_ATOM_ERROR,               ///< Problem while parsing an atom starting with the letter 'n'
    NUMBER_ERROR,               ///< Problem while parsing a number
    UTF8_ERROR,                 ///< the input is not valid UTF-8
    UNINITIALIZED,              ///< unknown error, or uninitialized document
    EMPTY,                      ///< no structural element found
    UNESCAPED_CHARS,            ///< found unescaped characters in a string.
    UNCLOSED_STRING,            ///< missing quote at the end
    UNSUPPORTED_ARCHITECTURE,   ///< unsupported architecture
    INCORRECT_TYPE,             ///< JSON element has a different type than user expected
    NUMBER_OUT_OF_RANGE,        ///< JSON number does not fit in 64 bits
    INDEX_OUT_OF_BOUNDS,        ///< JSON array index too large
    NO_SUCH_FIELD,              ///< JSON field not found in object
    IO_ERROR,                   ///< Error reading a file
    INVALID_JSON_POINTER,       ///< Invalid JSON pointer reference
    INVALID_URI_FRAGMENT,       ///< Invalid URI fragment
    UNEXPECTED_ERROR,           ///< indicative of a bug in simdjson
    PARSER_IN_USE,              ///< parser is already in use.
    OUT_OF_ORDER_ITERATION,     ///< tried to iterate an array or object out of order
    INSUFFICIENT_PADDING,       ///< The JSON doesn't have enough padding for simdjson to safely parse it.
    INCOMPLETE_ARRAY_OR_OBJECT, ///< The document ends early.
    SCALAR_DOCUMENT_AS_VALUE,   ///< A scalar document is treated as a value.
    OUT_OF_BOUNDS,              ///< Attempted to access location outside of document.
    NUM_JSON_ERROR_CODES,
    ENUMERATOR_ERROR,
    VARIANT_ERROR,
    GUID_ERROR,
    NUM_ERROR_CODES,
};

template <class T, class = void>
struct ReadHelper;

} // namespace json
} // namespace skr

#endif // defined(__cplusplus)