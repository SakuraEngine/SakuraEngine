sdl2_includes_dir = "$(projectdir)/thirdparty/SDL2"

add_requires("libsdl", {configs = {shared = true}})
table.insert(include_dir_list, sdl2_includes_dir)
table.insert(packages_list, "libsdl")