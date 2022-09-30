qtad_include_dir = "$(projectdir)/thirdparty/QtAdvancedDocking/include"
qtad_include_dir_private = "$(projectdir)/thirdparty/QtAdvancedDocking/include/QtAdvancedDocking"
qtad_source_dir = "$(projectdir)/thirdparty/QtAdvancedDocking"

target("QtAdvancedDocking")
    set_group("00.thirdparty")
    add_rules("qt.shared")
    add_defines("ADS_SHARED_EXPORT")
    add_frameworks("QtGui", "QtCore", "QtWidgets", {public = true})
    add_files(qtad_include_dir_private.."/*.h")
    add_files(qtad_source_dir.."/*.cpp")
    add_files(qtad_source_dir.."/*.qrc")
    if (is_plat("linux")) then
        add_files(qtad_source_dir.."/linux/*.cpp")
    end
    add_includedirs(qtad_include_dir, {public = true})
    add_includedirs(qtad_include_dir_private, {public = false})