#pragma once

#define ignore clang::annotate("skr-shader", "ignore")
#define noignore clang::annotate("skr-shader", "noignore")
#define dump clang::annotate("skr-shader", "dump")
#define bypass clang::annotate("skr-shader", "bypass")
#define swizzle clang::annotate("skr-shader", "swizzle")
#define access clang::annotate("skr-shader", "access")
#define builtin(name) clang::annotate("skr-shader", "builtin", (name))
#define unaop(name) clang::annotate("skr-shader", "unaop", (name))
#define binop(name) clang::annotate("skr-shader", "binop", (name))
#define callop(name) clang::annotate("skr-shader", "callop", (name))
#define ext_call(name) clang::annotate("skr-shader", "ext_call", (name))
#define expr(name) clang::annotate("skr-shader", "expr", (name))

#define numthreads(x, y, z) clang::annotate("skr-shader", "kernel", (x), (y), (z))
#define groupshared [[clang::annotate("skr-shader", "groupshared")]]
#define globallycoherent [[clang::annotate("skr-shader", "globallycoherent")]]
#define unorm __attribute__((annotate_type("unorm")))
#define snorm __attribute__((annotate_type("snorm")))