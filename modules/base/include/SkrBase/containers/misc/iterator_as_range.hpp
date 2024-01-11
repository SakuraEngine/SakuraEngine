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
        inline friend bool operator==(const Adaptor& lhs, const Dummy& rhs) { return lhs.iter.has_next(); }
        inline friend bool operator==(const Dummy& lhs, const Adaptor& rhs) { return rhs.iter.has_next(); }
        inline friend bool operator!=(const Adaptor& lhs, const Dummy& rhs) { return !(lhs == rhs); }
        inline friend bool operator!=(const Dummy& lhs, const Adaptor& rhs) { return !(lhs == rhs); }

        // move
        inline Adaptor& operator++()
        {
            iter.move_next();
            return *this;
        }
        inline Adaptor operator++(int)
        {
            auto tmp = *this;
            iter.move_next();
            return tmp;
        }

        // dereference
        inline DataType&       operator*() { return iter.ref(); }
        inline const DataType& operator*() const { return iter.ref(); }
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