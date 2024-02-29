target("MetaTest")
    set_group("05.tests/meta")
    set_kind("headeronly")
    add_generator(
        "test_meta_generator",
        "path",
        {"files"}
    )