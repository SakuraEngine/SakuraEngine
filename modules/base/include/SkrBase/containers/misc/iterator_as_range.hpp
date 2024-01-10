#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/concepts.h"

namespace skr::container
{
template <Iterator It>
struct IteratorAsRange {
    struct Dummy {
    };
    struct Adaptor {
        It iter;

        // compare
        inline friend bool operator==(const Adaptor& lhs, const Dummy& rhs) { return lhs.iter.has_next(); }
        inline friend bool operator==(const Dummy& lhs, const Adaptor& rhs) { return rhs.iter.has_next(); }
        inline friend bool operator!=(const Adaptor& lhs, const Dummy& rhs) { return !(lhs == rhs); }
        inline friend bool operator!=(const Dummy& lhs, const Adaptor& rhs) { return !(lhs == rhs); }

        // move
        inline IteratorAsRange& operator++()
        {
            iter.move_next();
            return *this;
        }
        inline IteratorAsRange operator++(int)
        {
            auto tmp = *this;
            iter.move_next();
            return tmp;
        }

        // dereference
        inline decltype(auto) operator*() { return iter.ref(); }
        inline decltype(auto) operator*() const { return iter.ref(); }
    };

    // begin & end
    inline Adaptor       begin() { return { _iter }; }
    inline Dummy         end() { return {}; }
    inline const Adaptor begin() const { return { _iter }; }
    inline Dummy         end() const { return {}; }

private:
    It _iter;
};
} // namespace skr::container