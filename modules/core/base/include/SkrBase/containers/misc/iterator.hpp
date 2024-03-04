#pragma once
#include "SkrBase/config.h"
#include <concepts>

namespace skr::container
{
// concepts
template <typename It>
concept Iterator = requires(It t) {
    t.ref();
    t.reset();
    t.move_next();
    t.has_next();
};

template <typename It, typename T>
concept IteratorOfType = requires(It t) {
    {
        t.ref()
    } -> std::same_as<T>;
    {
        t.reset()
    } -> std::same_as<void>;
    {
        t.move_next()
    } -> std::same_as<void>;
    {
        t.has_next()
    } -> std::convertible_to<bool>;
};

template <typename It>
concept StlStyleIterator = requires(It t) {
    *t;
    t != t;
    ++t;
    t++;
};

// Cursor -> Range
template <typename TCursor, bool kInverse>
struct CursorRange {
    struct EndType {
    };
    struct Adaptor {
        TCursor cursor;

        // compare
        inline bool operator!=(const EndType& rhs) const
        {
            if constexpr (kInverse)
            {
                return !cursor.reach_begin();
            }
            else
            {
                return !cursor.reach_end();
            }
        }

        // move
        inline Adaptor& operator++()
        {
            if constexpr (kInverse)
            {
                cursor.move_prev();
            }
            else
            {
                cursor.move_next();
            }
            return *this;
        }

        // dereference
        inline decltype(auto) operator*() { return cursor.ref(); }
    };

    SKR_INLINE CursorRange(TCursor&& cursor)
        : _cursor(std::move(cursor))
    {
    }
    SKR_INLINE CursorRange(const TCursor& cursor)
        : _cursor(cursor)
    {
    }

    // begin & end
    inline Adaptor       begin() { return { _cursor }; }
    inline EndType       end() { return {}; }
    inline const Adaptor begin() const { return { _cursor }; }
    inline EndType       end() const { return {}; }

private:
    TCursor _cursor;
};

// Cursor -> Iter
template <typename TCursor, bool kInverse>
struct CursorIter : protected TCursor {
    // ctor & copy & move & assign & move assign
    inline CursorIter(TCursor&& rhs)
        : TCursor(std::move(rhs))
    {
    }
    inline CursorIter(const TCursor& rhs)
        : TCursor(rhs)
    {
    }
    inline CursorIter(const CursorIter& rhs)            = default;
    inline CursorIter(CursorIter&& rhs)                 = default;
    inline CursorIter& operator=(const CursorIter& rhs) = default;
    inline CursorIter& operator=(CursorIter&& rhs)      = default;

    // getter
    inline decltype(auto) ref() const { return TCursor::ref(); }

    // move & validator
    inline void reset()
    {
        if constexpr (kInverse)
        {
            TCursor::reset_to_end();
        }
        else
        {
            TCursor::reset_to_begin();
        }
    }
    inline void move_next()
    {
        if constexpr (kInverse)
        {
            TCursor::move_prev();
        }
        else
        {
            TCursor::move_next();
        }
    }
    inline bool has_next() const
    {
        if constexpr (kInverse)
        {
            return !TCursor::reach_begin();
        }
        else
        {
            return !TCursor::reach_end();
        }
    }

    // cast
    inline const TCursor&                 cursor() const { return *this; }
    inline TCursor&                       cursor() { return *this; }
    inline CursorRange<TCursor, kInverse> as_range() const { return { *this }; }
};

// Cursor -> Stl style iter
template <typename TCursor, bool kInverse>
struct CursorIterStl : protected TCursor {
    // ctor & copy & move & assign & move assign
    inline CursorIterStl(TCursor&& rhs)
        : TCursor(std::move(rhs))
    {
    }
    inline CursorIterStl(const TCursor& rhs)
        : TCursor(rhs)
    {
    }
    inline CursorIterStl(const CursorIterStl& rhs)            = default;
    inline CursorIterStl(CursorIterStl&& rhs)                 = default;
    inline CursorIterStl& operator=(const CursorIterStl& rhs) = default;
    inline CursorIterStl& operator=(CursorIterStl&& rhs)      = default;

    // compare
    inline bool operator!=(const CursorIterStl& rhs)
    {
        return TCursor::operator!=(rhs);
    }

    // move
    inline CursorIterStl& operator++()
    {
        if constexpr (kInverse)
        {
            TCursor::move_prev();
        }
        else
        {
            TCursor::move_next();
        }
        return *this;
    }

    // dereference
    inline decltype(auto) operator*() { return TCursor::ref(); }
};
} // namespace skr::container
