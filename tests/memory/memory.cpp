#ifdef RUNTIME_SHARED
#define EA_DLL
#endif

#include <EASTL/string.h>
#include <EASTL/vector.h>

int main(void)
{
    eastl::vector<int> veci(100);
    eastl::string str = "!@@#dasddadas?";
    printf("Hello World! %s\n", str.c_str());
    return 0;
}