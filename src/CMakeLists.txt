add_executable(psycris
    main.cpp
    logging.cpp
    cpu/cpu.cpp
    cpu/disassembly.cpp
)

target_compile_features(psycris PUBLIC cxx_std_17)
target_compile_options(psycris PRIVATE -Wall -Wextra)
target_link_libraries(psycris
    CONAN_PKG::fmt
    CONAN_PKG::gsl_microsoft
    CONAN_PKG::spdlog
)