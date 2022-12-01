#pragma once
#include <type_traits>
#include "platform/configure.h"

namespace skr
{
    template<class T>
    struct TEnumAsByte
    {
        static_assert(std::is_enum<T>::value, "T must be an enum type");
        using UT = std::underlying_type_t<T>;
    public:
        inline TEnumAsByte() SKR_NOEXCEPT = default;
        inline TEnumAsByte(T enumeration) SKR_NOEXCEPT
            : value(enumeration)
        {

        }
        inline TEnumAsByte<T>& operator=(T enumeration) SKR_NOEXCEPT
        {
            this->value = enumeration;
            return *this;
        }
        const auto& as_byte() const
        {
            return (const UT&)(value);
        }
        auto& as_byte()
        {
            return (UT&)(value);
        }
        static TEnumAsByte<T> from_byte(UT v)
        {
            return { static_cast<T>(v) };
        }
        operator T() const
        {
            return static_cast<T>(value);
        }
    protected:
        UT value;
    };
}

// binary reader
#include "binary/reader_fwd.h"

namespace skr
{
namespace binary
{
template <class T>
struct ReadHelper<TEnumAsByte<T>>
{
    static int Read(skr_binary_reader_t* reader, TEnumAsByte<T>& value)
    {
        return skr::binary::Archive(reader, value.as_byte());
    }
};
} // namespace binary
} // namespace skr


// binary writer
#include "binary/writer_fwd.h"

namespace skr
{
namespace binary
{

template <class T>
struct WriteHelper<const TEnumAsByte<T>&> {
    static int Write(skr_binary_writer_t* writer, const TEnumAsByte<T>& value)
    {
        return skr::binary::Archive(writer, value.as_byte());
    }
};

} // namespace binary
} // namespace skr