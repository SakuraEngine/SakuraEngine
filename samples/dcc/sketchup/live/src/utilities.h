#pragma once

#include <string>

#include "ruby/framework.h"


/*
 * VALUEFUNC(f) is a macro used to typecast a C function that implements
 * a Ruby method so that it can be passed as an argument to API functions
 * like rb_define_method() and rb_define_singleton_method().
 *
 * VOIDFUNC(f) is a macro used to typecast a C function that implements
 * either the "mark" or "free" stuff for a Ruby Data object, so that it
 * can be passed as an argument to API functions like Data_Wrap_Struct()
 * and Data_Make_Struct().
 */
#define VALUEFUNC(f) ((VALUE (*)(ANYARGS)) f)
#define VOIDFUNC(f)  ((RUBY_DATA_FUNC) f)


namespace example {
namespace ruby {


VALUE GetVALUE(bool boolean);
VALUE GetVALUE(const char* string);
VALUE GetVALUE(const std::string& string);
VALUE GetVALUE(int number);
VALUE GetVALUE(unsigned int number);
VALUE GetVALUE(long number);
VALUE GetVALUE(long long number);
VALUE GetVALUE(unsigned long long number);
VALUE GetVALUE(double number);


} // namespace ruby
} // namespace example
