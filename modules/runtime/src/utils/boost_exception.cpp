#include <exception>
#include "utils/DAG.boost.hpp"

namespace boost
{
BOOST_NORETURN void throw_exception(std::exception const& e, const struct boost::source_location&)
{
    abort();
}
BOOST_NORETURN void throw_exception(std::exception const& e)
{
    abort();
}
} // namespace boost