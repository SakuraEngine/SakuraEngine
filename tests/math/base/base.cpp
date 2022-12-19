#include "string.h"
#include "stdlib.h"

int main()
{
    char *p = (char*)::malloc(5);
    p[25] = 'c';
    
    return 0;
}