#pragma once
#include <tuple>
#include "SkrContainers/vector.hpp"

namespace skr
{
template <typename T>
class cartesian_product
{
public:
    template <typename... Args, typename = typename std::enable_if<(sizeof...(Args) > 1)>::type>
    cartesian_product(Args&&... args)
        : count(1)
    {
        static_assert(std::is_arithmetic<T>::value, "T must be an arithmetic type.");
        append(args...);
    }

    cartesian_product(skr::Vector<skr::Vector<T>> const& sequences)
        : count(1)
    {
        tuples.reserve(sequences.size());
        for (auto&& s : sequences)
        {
            tuples.add(E{ s.begin(), 0ul, s.size() });
            count *= (int64_t)s.size();
        }
    }

    cartesian_product(skr::Vector<skr::Vector<T>>&& sequences)
        : cartesian_product(sequences)
    {
    }

    bool has_next() { return count > 0; }

    // next combination
    skr::Vector<T> next()
    {
        skr::Vector<T> res(tuples.size());
        auto           s = res.end() - 1;

        for (auto p = tuples.end() - 1; p >= tuples.begin(); --p, --s)
        {
            *s = *std::get<0>(*p);

            if (p > (tuples.end() - 1))
            {
                auto q = p - 1;
                if (std::get<1>(*q) == std::get<2>(*q))
                {
                    std::get<0>(*q) -= std::get<2>(*q);
                    std::get<1>(*q) = 0;

                    ++std::get<0>(*p);
                    ++std::get<1>(*p);
                }
            }
            else
            {
                ++std::get<0>(*p);
                ++std::get<1>(*p);
            }
        }

        --count;
        return res;
    }

private:
    using I = const T*;
    using E = std::tuple<I, std::size_t, std::size_t>;
    skr::Vector<E> tuples;
    int64_t        count;

    template <typename... Args>
    void append(skr::Vector<T> const& vec, Args&&... args)
    {
        append(vec);
        append(args...);
    }

    void append(skr::Vector<T> const& vec)
    {
        count *= vec.size();
        tuples.emplace_back(std::cbegin(vec), 0ul, vec.size());
    }
};
} // namespace skr