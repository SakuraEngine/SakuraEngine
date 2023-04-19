package("lmdb")
    set_homepage("http://www.lmdb.tech/doc/")
    set_description("LMDB is a Btree-based database management library modeled loosely on the BerkeleyDB API, but much simplified. ")
    set_license("A copy of this license is available in the file LICENSE in the top-level directory of the distribution or, alternatively, at http://www.OpenLDAP.org/license.html.")
    
    add_versions("0.9.29-skr", "ffb022bc2d8174311d3b1fc7473b68b3dc79d6fd173e2498cc4d2860e14f466c")

    on_install(function (package)
        os.mkdir(package:installdir())
        os.cp(path.join(package:scriptdir(), "port", "lmdb"), ".")
        os.cp(path.join(package:scriptdir(), "port", "xmake.lua"), "xmake.lua")

        local configs = {}
        import("package.tools.xmake").install(package, configs)
    end)