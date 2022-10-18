//DO NOT MODIFY THIS FILE
#include "platform/debug.h"
#include "platform/guid.hpp"
#include "platform/memory.h"
#include "type/type_registry.h"
%for header in data.headers:
#include "${header}"
%endfor

%for record in data.records:
static struct StaticConstructor${record.id}Helper
{
    StaticConstructor${record.id}Helper()
    {
        %for realized_expr in record.realized_expr:
        ${realized_expr};
        %endfor
    }
} _StaticConstructor${record.id}Helper;
%endfor


%for enum in data.enums:
static struct StaticConstructor${enum.id}Helper
{
    StaticConstructor${enum.id}Helper()
    {
        %for realized_expr in enum.realized_expr:
        ${realized_expr};
        %endfor
    }
} _StaticConstructor${enum.id}Helper;
%endfor