#pragma once
#define SKR_STRINGIZING(x) #x
#define SKR_MAKE_STRING(x) SKR_STRINGIZING(x)
#define SKR_FILE_LINE __FILE__ ":" SKR_MAKE_STRING(__LINE__)