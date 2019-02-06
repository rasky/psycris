add_library(psycris_emu STATIC
    logging.cpp
    scheduler.cpp
    cpu/cpu.cpp
    cpu/cop0.cpp
    cpu/disassembly.cpp
    gpu/gpu.cpp
    gpu/disassembly.cpp
    hw/bus.cpp
    hw/devices/dma.cpp
    hw/devices/interrupt_control.cpp
    hw/devices/spu.cpp
)

target_compile_features(psycris_emu PUBLIC cxx_std_17)
target_compile_options(psycris_emu PRIVATE -Wall -Wextra)

target_link_libraries(psycris_emu
    CONAN_PKG::fmt
    CONAN_PKG::gsl_microsoft
    CONAN_PKG::spdlog
    CONAN_PKG::boost
)

add_executable(psycris
    main.cpp
    config.cpp
    loader.cpp
    psx.cpp
)

target_compile_options(psycris PRIVATE -Wall -Wextra)
target_link_libraries(psycris psycris_emu CONAN_PKG::cli11)

add_executable(tests
    test_runner.cpp
    test_bus.cpp
    test_bitmask.cpp
)
target_compile_options(tests PRIVATE -Wall -Wextra)
target_link_libraries(tests psycris_emu CONAN_PKG::catch2)

add_test(tests tests)