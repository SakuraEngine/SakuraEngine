#pragma once
#include <type_traits>
#include <EASTL/tuple.h>
#include <EASTL/vector.h>

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

    cartesian_product(eastl::vector<eastl::vector<T>> const& sequences) 
        : count(1)
    {
        tuples.reserve(sequences.size());
        for(auto&& s : sequences) {
            tuples.emplace_back(s.cbegin(), 0ul, s.size());
            count *= (int64_t)s.size();
        }
    }

    cartesian_product(eastl::vector<eastl::vector<T>>&& sequences)
        : cartesian_product(sequences)
    {

    }


    bool has_next() { return count > 0; }

    // next combination
    eastl::vector<T> next()
    {
        eastl::vector<T> res(tuples.size());
        auto s = res.rbegin();

        for (auto p = tuples.rbegin(); p < tuples.rend(); ++p, ++s) {
            *s = *eastl::get<0>(*p);

            if (p > tuples.rbegin()) {
                auto q = p - 1;
                if (eastl::get<1>(*q) == eastl::get<2>(*q)) {
                    eastl::get<0>(*q) -= eastl::get<2>(*q);
                    eastl::get<1>(*q) = 0;

                    ++eastl::get<0>(*p);
                    ++eastl::get<1>(*p);
                }
            } else {
                ++eastl::get<0>(*p);
                ++eastl::get<1>(*p);
            }
        }

        --count;
        return res;
    }

private:
    using I = typename eastl::vector<T>::const_iterator;
    using E = eastl::tuple<I, std::size_t, std::size_t>;
    eastl::vector<E> tuples;
    int64_t count;

    template <typename... Args>
    void append(eastl::vector<T> const& vec, Args&&... args)
    {
        append(vec);
        append(args...);
    }

    void append(eastl::vector<T> const& vec)
    {
        count *= vec.size();
        tuples.emplace_back(eastl::cbegin(vec), 0ul, vec.size());
    }
};
} // namespace skr