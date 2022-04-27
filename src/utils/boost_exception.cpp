#include <exception>
#include "utils/DAG.boost.hpp"

namespace boost
{
void throw_exception(std::exception const& e, const struct boost::source_location&)
{
}
void throw_exception(std::exception const& e)
{
}
} // namespace boost