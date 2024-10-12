task("vsc_dbg")
    on_run("main")
    
    set_menu {
        usage = "xmake vsc_dbg [options]",
        description = "generate vscode debug configurations",
        options = {}
    }
