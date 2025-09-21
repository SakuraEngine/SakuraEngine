#pragma once

#ifdef _EXPORT

#define export [[clang::annotate("skr-shader", "export")]]
#define kernel_1d(x)
#define kernel_2d(x, y)
#define kernel_3d(x, y, z)

#else

#define export
#define kernel_1d(x) clang::annotate("skr-shader", "kernel", (x), (1), (1))
#define kernel_2d(x, y) clang::annotate("skr-shader", "kernel", (x), (y), (1))
#define kernel_3d(x, y, z) clang::annotate("skr-shader", "kernel", (x), (y), (z))
#define numthreads(x, y, z) clang::annotate("skr-shader", "kernel", (x), (y), (z))

#endif

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

#define groupshared [[clang::annotate("skr-shader", "groupshared")]]