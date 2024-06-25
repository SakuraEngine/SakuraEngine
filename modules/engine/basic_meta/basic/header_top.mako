//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!! THIS FILE IS GENERATED, ANY CHANGES WILL BE LOST !!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#pragma once
#include "SkrBase/config.h"
#include <inttypes.h>

#ifdef __meta__
#error "this file should not be inspected by meta"
#endif

#ifdef SKR_FILE_ID
    #undef SKR_FILE_ID
#endif
#define SKR_FILE_ID ${header_db.file_id}

// BEGIN forward declarations
%for record in header_db.get_records():
%if record.namespace:
namespace ${record.namespace} { struct ${record.short_name}; }
%else:
struct ${record.short_name};
%endif
%endfor
%for enum in header_db.get_enums():
<% 
    prefix = "class" if enum.is_scoped else ""
    underlying_type = f": {enum.underlying_type}" if enum.underlying_type != "unfixed" else ""
%>\
%if enum.namespace:
namespace ${enum.namespace} { enum ${prefix} ${enum.short_name} ${underlying_type}; }
%else:
enum ${prefix} ${enum.short_name} ${underlying_type};
%endif
%endfor
// END forward declarations

// BEGIN Generated Body

// END Generated Body
