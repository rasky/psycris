cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(psycris VERSION 0.1 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

# with cmake >= 3.13 we can use add_link_options
function(add_linker_flag_all_variants flag)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${flag}" PARENT_SCOPE)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${flag}" PARENT_SCOPE)
    # What is a module?
    # set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${flag}" PARENT_SCOPE)
endfunction()

add_compile_options(-fsanitize=undefined)
add_linker_flag_all_variants(-fsanitize=undefined)

add_subdirectory(src)