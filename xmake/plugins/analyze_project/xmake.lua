task("analyze_project")
    on_run("main")
    
    set_menu {
        usage = "xmake analyze_project [options]",
        description = "run tests",
        options = {}
    }
