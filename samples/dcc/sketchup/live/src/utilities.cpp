#include "utilities.h"

// #include <sstream>

namespace example {
namespace ruby {


VALUE GetVALUE(bool boolean)
{
  return (boolean) ? Qtrue : Qfalse;
}


VALUE GetVALUE(const char* string)
{
  VALUE str_val = rb_str_new_cstr(string);
#ifdef HAVE_RUBY_ENCODING_H
  // Mark all strings as UTF-8 encoded. This makes them play nicer with
  // Ruby 2.0.
  static int enc_index = rb_enc_find_index("UTF-8");
  rb_enc_associate_index(str_val, enc_index);
#endif
  return str_val;
}


VALUE GetVALUE(const std::string& string)
{
  return GetVALUE(string.c_str());
}


VALUE GetVALUE(int number)
{
  return INT2NUM(number);
}


VALUE GetVALUE(unsigned int number)
{
  return UINT2NUM(number);
}


VALUE GetVALUE(long number)
{
  return LONG2NUM(number);
}


VALUE GetVALUE(long long number)
{
  return LL2NUM(number);
}


VALUE GetVALUE(unsigned long long number)
{
  return ULL2NUM(number);
}


VALUE GetVALUE(double number)
{
  return rb_float_new(number);
}


} // namespace ruby
} // namespace example
