Dear Bindings Metadata Format
-----------------------------

The `cimgui.json` file contains JSON metadata about the generated bindings. The format looks like this:

### Top level

```json
{
  "defines": [],
  "enums": [],
  "typedefs": [],
  "structs": [],
  "functions": []
}
```

Each of the top-level keys contains information about one type of object in the generated header.

### Defines

```json
{
  "name": "IMGUI_VERSION",
  "content": "\"1.83 WIP\""
}
```

Defines represent `#define` values.

> Note that the content includes quotes if the define was a string in the original header (as seen above), but _does_ remove brackets from around values (so in the case of `IM_DRAWLIST_TEX_LINES_WIDTH_MAX`, the content is `63` not `(63)`).

|Key|Description|
|---|-----------|
|name|The name of the define|
|content|The textual content of the define|

### Enums

```json
{
  "name": "ImGuiTableRowFlags_",
  "original_fully_qualified_name": "ImGuiTableRowFlags_",
  "storage_type": {
    "declaration": "int"
  },
  "elements": [
    {
      "name": "ImGuiTableRowFlags_None",
      "value": "0"
    },
    {
      "name": "ImGuiTableRowFlags_Headers",
      "value": "1<<0"
    }
  ]
}
```

|Key|Description|
|---|-----------|
|name|The name of the enum|
|original_fully_qualified_name|The name of the enum as it appeared in the original C++ API|
|storage_type|The storage type of the enum (if specified)|
|elements|A list of elements|
|elements.name|The name of the element|
|elements.value|The value of the element (may be a calculation)|

### Typedefs

```json
{
  "name": "ImGuiWindowFlags",
  "type": {
    "declaration": "int"
  }
}
```

A C `typedef`.

|Key|Description|
|---|-----------|
|name|The name of the typedef|
|type|The defined type (as a generic type element)|

### Types

```json
{
  "declaration": "int"
}
```

```json
{
  "declaration": "void*"
}
```

```json
{
  "declaration": "int (*ImGuiInputTextCallback)(ImGuiInputTextCallbackData* data)",
  "type_details": {
    "flavour": "function_pointer",
    "return_type": {
      "declaration": "int"
    },
    "arguments": [
      {
        "name": "data",
        "type": {
          "declaration": "ImGuiInputTextCallbackData*"
        },
        "is_array": false,
        "is_varargs": false
      }
    ]
  }
}
```

These are used in various elements in the JSON data, and represent a C type. Simple cases just have a
single `declaration` key that is the C-style declaration of the type, but more complex examples (currently limited to
function pointers) also have a `type_details` key that contains parsed details of the type in question.

|Key|Description|
|---|-----------|
|declaration|The C-style declaration of the type|
|type|The defined type (as a generic type element)|
|type_details|Parsed details of the type (where applicable)|
|type_details.flavour|The "flavour" (variant) of the type for which the details are supplied|

`type_details.flavour` values:

|Value|Meaning|
|-----|-------|
|function_pointer|The type is a function pointer - the rest of the `type_details` block can be parsed as a normal function pointer type|

Function pointer `type_details` keys:

|Key|Description|
|---|-----------|
|return_type|The function return type|
|arguments|A list of function arguments (see "function arguments")|

### Structs

A C structure. Forward-declarations for structures that do not have definitions provided in the header are included here
for reference, but without any internal details.

```json
{
  "name": "ImVec2",
  "original_fully_qualified_name": "ImVec2",
  "type": "struct",
  "by_value": true,
  "forward_declaration": false,
  "is_anonymous": false,
  "fields": [
    {
      "names": [
        {
          "name": "x",
          "is_array": false
        },
        {
          "name": "y",
          "is_array": false
        }
      ],
      "is_anonymous": false,
      "type": {
        "declaration": "float"
      }
    }
  ]
}

```

|Key|Description|
|---|-----------|
|name|The C name of the structure|
|original_fully_qualified_name|The original C++ name of the structure|
|type|The type of the structure (either `struct` or `union`)|
|by_value|Is this structure normally pass-by-value?|
|forward_declaration|Is this a forward-declaration of the structure?|
|is_anonymous|Is this an anonymous struct?|
|fields|List of contained fields|
|fields.names|List of names (since C allows multiple fields to be declared at once with the same type, each name corresponds to one field instance)|
|fields.names.name|The field name|
|fields.names.is_array|Is this field declared as an array?|
|fields.names.array_bounds|The array bounds, if the field is an array|
|fields.names.width|The bit width of the field, if specified|
|fields.is_anonymous|Is this field anonymous?|
|fields.type|The type of the field (see "types" for more details)|

> The members of anonymous fields should be treated as being part of the owning struct, and thus their name is not relevant, but they are assigned a synthetic name for convenience.

#### Anonymous structs/unions

If a struct or union is anonymous, `is_anonymous` will be `true`, and the struct name will be set to a value of the
form `<anonymous0>`, where the trailing index makes it unique within the file (to avoid clashes, this naming is deliberately chosen so as to not be a valid C++ type name). This name can be used to match
anonymous structures to their point-of-use in the `fields` list, where the `type` will contain the same name (and `is_anonymous` will be set if the field is also anonymous). For example:

```json
{
  "fields": [
    {
      "names": [
        {
          "name": "<anonymous0>",
          "is_array": false
        }
      ],
      "is_anonymous": true,
      "type": {
        "declaration": "<anonymous0>"
      }
    }
  ]
}
```

#### Array fields

Array fields look like this, with the bounds given in `array_bounds`. Note that `array_bounds` can contain non-integer
values (such as enum elements or defines).

```json

{
  "names": [
    {
      "name": "MouseDown",
      "is_array": true,
      "array_bounds": "5"
    }
  ], 
  "is_anonymous": false,
  "type": {
    "declaration": "bool"
  }
}
```

### Functions

Functions provided by the API.

Languages which support default function arguments can probably ignore any functions with `is_default_argument_helper`
set to `true`, as those are additional functions added to support simulating default arguments in C.

When using a version of ImGui with `ImStr` (string view) support, languages which use string views should probably
ignore any functions with `is_imstr_helper` set, as these are generated functions that give an alternative interface
using `const char*` instead of `ImStr`. Conversely, if you are using `const char*` for your strings, then you probably
want to ignore any functions with `has_imstr_helper` set.

```json
{
  "name": "ImGui_CreateContext",
  "original_fully_qualified_name": "ImGui::CreateContext",
  "return_type": {
    "declaration": "ImGuiContext*"
  },
  "arguments": [],
  "is_default_argument_helper": true,
  "is_manual_helper": false
}
```

|Key|Description|
|---|-----------|
|name|The C name of the function|
|original_fully_qualified_name|The original C++ name of the function|
|return_type|The return type of the function (as a type)|
|arguments|A list of the function arguments|
|is_default_argument_helper|Is this function a variant generated to simulate default arguments?|
|is_manual_helper|Is this a manually added function that doesn't exist in the original C++ API but was added specially to the C API? (at present only `ImVector_Construct` and `ImVector_Destruct`)|
|is_imstr_helper|Is this function a helper variant added that takes `const char*` instead of `ImStr` arguments?|
|has_imstr_helper|Is this function one which takes `ImStr` arguments and has had a `const char*` helper variant generated?|

### Function arguments

Function arguments as they appear in function (and function pointer) metadata.

```json
{
  "name": "ctx",
  "type": {
    "declaration": "ImGuiContext*"
  },
  "is_array": false,
  "is_varargs": false,
  "default_value": "NULL"
}
```

```json
{
  "name": "v",
  "type": {
    "declaration": "float"
  },
  "is_array": true,
  "is_varargs": false,
  "array_bounds": "4"
}
```

|Key|Description|
|---|-----------|
|name|The argument name|
|type|The argument type|
|is_array|Is this an array argument?|
|array_bounds|Array bounds, if this is an array argument|
|is_vararges|Is this a varargs argument?|
|default_value|The default value, if present|

### Generic keys

These are generic keys that can appear in the majority of primary elements (defines/typedefs/enums/enum
members/structs/fields/functions).

The `is_internal` key is intended as a broad hint that the function/struct/enum member in question may not be part of
the primary API and should probably be "hidden by default" if such a feature is available in the target language.
However, it is equally possible that advanced users may want or need to access these, so removing them entirely or
completely blocking access is not recommended either.

|Key|Description|
|---|-----------|
|comment|Any comments related to this element (see "comments")|
|is_internal|Is this an internal API member?|

#### Comments

Comment keys contain any comments which are related to an element. There are two types of comment - `preceding` comments
which appear immediately before an element in the source code, `attached` comments which appear immediately after the
element (on the same line). An element can have many preceding comments but only one attached comment.

```json
{
  "preceding": [
    "// Sizing Extra Options"
  ],
  "attached": "// Make outer width auto-fit to columns, overriding outer_size.x value. Only available when ScrollX/ScrollY are disabled and Stretch columns are not used."
}
```

A comment as it appears on an enum element:

```json
{
  "name": "ImGuiWindowFlags_NoTitleBar",
  "value": "1<<0",
  "comments": {
    "attached": "// Disable title-bar"
  }
},
```

|Key|Description|
|---|-----------|
|preceding|An array of preceding comments (if present)|
|attached|The attached comment (if present)|

#### Conditionals

The `conditionals` key contains details on any preprocessor conditionals (`#ifdef`/`#if` blocks) that apply to a given
element.

Conditionals as they appear on two typedefs:

```json
[
  {
    "name": "ImWchar",
    "type": {
      "declaration": "ImWchar32"
    },
    "conditionals": [
      {
        "condition": "ifdef",
        "expression": "IMGUI_USE_WCHAR32"
      }
    ]
  },
  {
    "name": "ImWchar",
    "type": {
      "declaration": "ImWchar16"
    },
    "conditionals": [
      {
        "condition": "ifndef",
        "expression": "IMGUI_USE_WCHAR32"
      }
    ]
  }
]
```

|Key|Description|
|---|-----------|
|conditionals|An array of conditionals for the element|
|conditionals.condition|The condition applied (see below)|
|conditionals.expression|The expression|

Conditional conditions are:

|Condition|Description|
|---------|-----------|
|ifdef|Checks if a define is set (`#ifdef DEFINE`)|
|ifndef|Checks if a define is not set (`#ifndef DEFINE`)|
|if|Checks if the expression evaluates to a non-zero value (`#if EXPRESSION`)|
|ifnot|Checks if the expression evaluates to a zero value (no direct C equivalent, but behaves the same as `#if !(EXPRESSION)`)|

The `ifnot` conditional is used in the case where the element appears in the `#else` block of a `#if`, and thus
indicates that the element is used in the case where the `#if` evaluates to false.  
