//BEGIN STATIC_CTOR GENERATED
#include "SkrRT/platform/debug.h"
#include "SkrRT/platform/guid.hpp"
#include "SkrRT/platform/memory.h"
#include "SkrRT/type/type.hpp"
#if defined __has_include
    #if  __has_include ("binary/reader_fwd.h")
    #include "SkrRT/serde/binary/reader.h"
    #endif
    #if  __has_include ("binary/writer_fwd.h")
    #include "SkrRT/serde/binary/writer.h"
    #endif
    #if  __has_include ("json/reader_fwd.h")
    #include "SkrRT/serde/json/reader.h"
    #endif
    #if  __has_include ("json/writer_fwd.h")
    #include "SkrRT/serde/json/writer.h"
    #endif
#else
    #include "SkrRT/serde/binary/reader.h"
    #include "SkrRT/serde/binary/writer.h"
    #include "SkrRT/serde/json/reader.h"
    #include "SkrRT/serde/json/writer.h"
#endif


%for record in generator.filter_records(db.records):
static struct StaticConstructor${record.id}Helper
{
    StaticConstructor${record.id}Helper()
    {
        %if hasattr(record, "namespace"):
        using namespace ${record.namespace};
        %endif

        %for realized_expr in record.realized_expr:
        ${realized_expr};
        %endfor
    }
} _StaticConstructor${record.id}Helper;
%endfor


%for enum in generator.filter_enums(db.enums):
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
//END STATIC_CTOR GENERATED