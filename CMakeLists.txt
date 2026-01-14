cmake_minimum_required(VERSION 3.10)

# Project settings
project(WiFiBruteForcer VERSION 1.0 LANGUAGES CXX)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Include directories
include_directories(include)

# Source files
set(SOURCES
    src/main.cpp
    src/wifi.cpp
    src/args.cpp
)

# Add executable
add_executable(wifi_brute ${SOURCES})

# Windows specific settings
if(WIN32)
    # Link against necessary libraries
    target_link_libraries(wifi_brute wlanapi.lib)
    
    # Set static linking for Windows
    if(MINGW)
        # For MinGW-w64: statically link standard libraries
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
    endif()
    
    # Resource file is temporarily disabled due to compilation issues
    # It can be re-enabled by uncommenting the following lines:
    # set(RES_FILE ${CMAKE_CURRENT_BINARY_DIR}/resources.rc)
    # configure_file(
    #     ${CMAKE_SOURCE_DIR}/resources.rc.in
    #     ${RES_FILE}
    #     @ONLY
    # )
    # target_sources(wifi_brute PRIVATE ${RES_FILE})
endif()

# Compiler specific settings
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # GCC settings
    target_compile_options(wifi_brute PRIVATE -Wall -Wextra -pedantic)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(wifi_brute PRIVATE -O2 -s)
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Clang settings
    target_compile_options(wifi_brute PRIVATE -Wall -Wextra -pedantic)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(wifi_brute PRIVATE -O2 -s)
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # MSVC settings (should not be used as per requirements)
    message(WARNING "MSVC compiler detected. This project is designed for MinGW-w64 or Clang.")
    target_compile_options(wifi_brute PRIVATE /W4 /permissive-)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(wifi_brute PRIVATE /O2 /MT)
    endif()
endif()

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
