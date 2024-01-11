#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/concepts.h"

namespace skr::container
{
template <typename It>
struct IteratorAsRange {
    struct Dummy {
    };
    struct Adaptor {
        using DataType = typename It::DataType;

        It iter;

        // compare
        inline friend bool operator!=(const Adaptor& lhs, const Dummy& rhs) { return lhs.iter.has_next(); }

        // move
        inline Adaptor& operator++()
        {
            iter.move_next();
            return *this;
        }

        // dereference
        inline DataType& operator*() { return iter.ref(); }
    };

    SKR_INLINE IteratorAsRange(It&& iter)
        : _iter(std::move(iter))
    {
    }
    SKR_INLINE IteratorAsRange(const It& iter)
        : _iter(iter)
    {
    }

    // begin & end
    inline Adaptor       begin() { return { _iter }; }
    inline Dummy         end() { return {}; }
    inline const Adaptor begin() const { return { _iter }; }
    inline Dummy         end() const { return {}; }

private:
    It _iter;
};
} // namespace skr::container