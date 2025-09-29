#pragma once

#include "meta.h"
#include "serde.h"
#include "clang/AST/Attr.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/PrettyPrinter.h"
#include "llvm/Support/JSON.h"
#include <algorithm>
#include <string>
#include <unordered_map>

// forward
namespace meta {
struct Function;
struct Field;
struct Record;
struct EnumValue;
struct Enum;
struct Database;

META_SERDE_FWD(Function);
META_SERDE_FWD(Field);
META_SERDE_FWD(Record);
META_SERDE_FWD(EnumValue);
META_SERDE_FWD(Enum);
META_SERDE_FWD(Database);
} // namespace meta

namespace meta {
struct Constructor {
  std::string name;
  std::string access = "none";

  std::vector<struct Field> parameters;

  std::string comment;
  std::string file_name;
  int line;

  std::vector<std::string> attrs;
};
META_SERDE_FUNCTION(Constructor) {
  serde_obj(s, key, [&] {
    META_SERDE(name)
    META_SERDE(access)

    META_SERDE(parameters)

    META_SERDE(comment)
    META_SERDE(file_name)
    META_SERDE(line)

    META_SERDE(attrs)
  });
}

struct Function {
  std::string name;
  std::string access = "none";

  bool is_static;
  bool is_const;
  bool is_nothrow;

  std::string ret_type;
  std::string raw_ret_type;
  std::vector<struct Field> parameters;

  std::string comment;
  std::string file_name;
  int line;

  std::vector<std::string> attrs;
};
META_SERDE_FUNCTION(Function) {
  serde_obj(s, key, [&] {
    META_SERDE(name)
    META_SERDE(access)

    META_SERDE(is_static)
    META_SERDE(is_const)
    META_SERDE(is_nothrow)

    META_SERDE(ret_type)
    META_SERDE(raw_ret_type)
    META_SERDE(parameters)

    META_SERDE(comment)
    META_SERDE(file_name)
    META_SERDE(line)

    META_SERDE(attrs)
  });
};

struct Field {
  std::string name;
  std::string access = "none";
  std::string type;
  std::string raw_type;

  size_t array_size = 0;
  std::string default_value;
  bool is_functor = false;
  bool is_callback = false;
  bool is_anonymous = false;
  bool is_static = false;
  Function signature;

  std::string comment;
  int line;

  std::vector<std::string> attrs;
};
META_SERDE_FUNCTION(Field) {
  serde_obj(s, key, [&] {
    META_SERDE(name)
    META_SERDE(access)
    META_SERDE(type)
    META_SERDE(raw_type)

    META_SERDE(array_size)
    META_SERDE(default_value)

    META_SERDE(is_functor)
    META_SERDE(is_callback)
    META_SERDE(is_anonymous)
    META_SERDE(is_static)

    if (v.is_callback) {
      META_SERDE_N(signature, "functor")
    }

    META_SERDE(comment)
    META_SERDE(line)

    META_SERDE(attrs)
  });
}

struct Record {
  std::string name;

  bool is_nested;
  std::vector<std::string> bases;
  std::vector<Field> fields;
  std::vector<Function> methods;
  std::vector<Constructor> ctors;

  std::string file_name;
  std::string comment;
  int line;

  std::vector<std::string> attrs;
};
META_SERDE_FUNCTION(Record) {
  serde_obj(s, key, [&] {
    META_SERDE(name)

    META_SERDE(is_nested)
    META_SERDE(bases)
    META_SERDE(fields)
    META_SERDE(methods)
    META_SERDE(ctors)

    META_SERDE(file_name)
    META_SERDE(comment)
    META_SERDE(line)

    META_SERDE(attrs)
  });
}

struct EnumValue {
  std::string name;

  uint64_t value;

  std::string comment;
  int line;

  std::vector<std::string> attrs;
};
META_SERDE_FUNCTION(EnumValue) {
  serde_obj(s, key, [&] {
    META_SERDE(name)

    META_SERDE(value)

    META_SERDE(comment)
    META_SERDE(line)

    META_SERDE(attrs)
  });
}

struct Enum {
  std::string name;

  std::string underlying_type;
  bool is_scoped;
  std::vector<EnumValue> values;

  std::string file_name;
  std::string comment;
  int line;

  std::vector<std::string> attrs;
};
META_SERDE_FUNCTION(Enum) {
  serde_obj(s, key, [&] {
    META_SERDE(name)

    META_SERDE(underlying_type)
    META_SERDE(is_scoped)
    META_SERDE(values)

    META_SERDE(file_name)
    META_SERDE(comment)
    META_SERDE(line)

    META_SERDE(attrs)
  });
};

struct Database {
  std::vector<Record> records;
  std::vector<Function> functions;
  std::vector<Enum> enums;
  inline bool is_empty() {
    return records.empty() && functions.empty() && enums.empty();
  }
  inline std::string serialize() const {
    std::string str;
    llvm::raw_string_ostream output(str);
    llvm::json::OStream stream(output);

    serde(stream, "", const_cast<Database &>(*this));

    return str;
  }
};
META_SERDE_FUNCTION(Database) {
  serde_obj(s, key, [&] {
    META_SERDE(records)
    META_SERDE(functions)
    META_SERDE(enums)
  });
}
} // namespace meta

// some declarations
namespace meta {
struct Identity {
  std::string fileName;
  unsigned line;
  bool operator==(const Identity &other) const {
    return fileName == other.fileName && line == other.line;
  }
};

struct IdentityHash {
  size_t operator()(const Identity &id) const {
    return std::hash<std::string>{}(id.fileName) + id.line;
  }
};

using FileDataMap = std::unordered_map<std::string, Database>;
} // namespace meta