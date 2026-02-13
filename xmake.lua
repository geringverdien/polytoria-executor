set_allowedarchs("x64")
add_rules("mode.debug", "mode.release")

add_requires("nativefiledialog-extended","microsoft-detours", "imgui", "boost", {configs = {dx11 = true, win32 =true, regex=true}})
add_requires("spdlog", {configs = {header_only=true}})

target("wowiezz")
    set_kind("shared")
    set_languages("c++23")
    add_includedirs(".")
    add_files("**.cpp|injector/*.cpp")
    add_packages("spdlog")
    add_links("user32", "dbghelp")

target("injector")
    set_kind("binary")
    set_languages("c++20")
    add_files("injector/main.cpp")