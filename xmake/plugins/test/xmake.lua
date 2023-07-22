task("test")
    set_category("plugin")
    on_run("main")

    set_menu {
        usage = "xmake test [options]",
        description = "run tests",
        options = {}
    }

    set_menu {
                -- usage
                usage = "xmake test|r [options] [target] [arguments]"

                -- description
            ,   description = "test the project target."

                -- xmake r
            ,   shortname = 'r'

                -- options
            ,   options =
                {
                    {'d', "debug",      "k",   nil  , "Run and debug the given target."                                    }
                ,   {'a', "all",        "k",   nil  , "Run all targets."                                                   }
                ,   {'g', "group",      "kv",  nil  , "Run all targets of the given group. It support path pattern matching.",
                                                      "e.g.",
                                                      "    xmake run -g test",
                                                      "    xmake run -g test_*",
                                                      "    xmake run --group=benchmark/*"                                  }
                ,   {'w', "workdir",    "kv",  nil  , "Work directory of running targets, default is folder of targetfile",
                                                      "e.g.",
                                                      "    xmake run -w .",
                                                      "    xmake run --workdir=`pwd`"                                      }
                ,   {'j', "jobs",       "kv", "1",    "Set the number of parallel compilation jobs."                       }
                ,   {nil, "detach",     "k", nil,     "Run targets in detached processes."                                 }
                ,   {}
                ,   {nil, "target",     "v",   nil  , "The target name. It will run all default targets if this parameter is not specified."
                                                    , values = function (complete, opt)
                                                            return import("private.utils.complete_helper.runable_targets")(complete, opt)
                                                        end }

                ,   {nil, "arguments",  "vs",  nil  , "The target arguments"                                               }
                }
            }