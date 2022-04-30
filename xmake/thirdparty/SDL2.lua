sdl2_includes_dir = "$(projectdir)/thirdparty/SDL2"
table.insert(include_dir_list, sdl2_includes_dir)

if (is_os("windows")) then 
    table.insert(links_list, "SDL2")
else
    add_requires("libsdl", {configs = {shared = true}})
    table.insert(packages_list, "libsdl")
end