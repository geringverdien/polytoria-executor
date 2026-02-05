add_rules("mode.debug", "mode.release")

target("poly")
set_languages("c++23")
    set_kind("shared")
    add_files("src/*.cpp")
