test_target("ThreadsTest")
    set_group("05.tests/threads")
    public_dependency("SkrRT", engine_version)
    add_files("threads/threads.cpp")

test_target("ServiceThreadTest")
    set_group("05.tests/threads")
    public_dependency("SkrRT", engine_version)
    add_files("threads/service_thread.cpp")

test_target("JobTest")
    set_group("05.tests/task")
    public_dependency("SkrRT", engine_version)
    add_files("threads/job.cpp")

test_target("Task2Test")
    set_group("05.tests/task")
    public_dependency("SkrRT", engine_version)
    add_files("task2/**.cpp")