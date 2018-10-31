add_executable(psycris
    main.cpp
    cpu/cpu.cpp 
)

target_compile_features(psycris PUBLIC cxx_std_17)
target_compile_options(psycris PRIVATE -Wall -Wextra)