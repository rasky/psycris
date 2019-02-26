add_library(psycris_emu STATIC
    libemu/logging.cpp
    libemu/scheduler.cpp
    libemu/cpu/cpu.cpp
    libemu/cpu/cop0.cpp
    libemu/cpu/disassembly.cpp
    libemu/gpu/gpu.cpp
    libemu/gpu/disassembly.cpp
    libemu/bus/bus.cpp
    libemu/devices/dma.cpp
    libemu/devices/interrupt_control.cpp
    libemu/devices/spu.cpp
    libemu/devices/timer.cpp
    libemu/debug.cpp
    libemu/psx.cpp
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
    psycris/main.cpp
    psycris/config.cpp
    psycris/loader.cpp
)

target_compile_options(psycris PRIVATE -Wall -Wextra)
target_link_libraries(psycris psycris_emu CONAN_PKG::cli11)

add_executable(psycris_emu_tests
    libemu/tests/test_runner.cpp
    libemu/tests/test_bus.cpp
    libemu/tests/test_bitmask.cpp
    libemu/tests/test_fifo.cpp
    libemu/tests/test_timer.cpp
)
target_compile_options(psycris_emu_tests PRIVATE -Wall -Wextra)
target_link_libraries(psycris_emu_tests psycris_emu CONAN_PKG::catch2)

add_test(NAME emu_tests COMMAND psycris_emu_tests)