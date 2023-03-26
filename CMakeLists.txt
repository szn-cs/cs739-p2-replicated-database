cmake_minimum_required(VERSION 3.10.2)
set(CMAKE_CXX_STANDARD 20)

## When using vcpkg as a submodule of your project, you can add the following to your CMakeLists.txt before the first project() call, instead of passing CMAKE_TOOLCHAIN_FILE to the cmake invocation. This will still allow people to not use vcpkg, by passing the CMAKE_TOOLCHAIN_FILE directly, but it will make the configure-build step slightly easier.
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/dependency/vcpkg/scripts/buildsystems/vcpkg.cmake"
CACHE STRING "Vcpkg toolchain file")

project (replicated_db 
          VERSION 1.0.0
          LANGUAGES C CXX
         DESCRIPTION "Replicated Database Project.")

# debug
set(CMAKE_BUILD_TYPE RelWithDebInfo)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/config/cmake;${CMAKE_MODULE_PATH}")

# find vcpkg packages
find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")

# Boost package
# find_package(Boost REQUIRED regex date_time system filesystem thread graph)
# message("boost lib: ${Boost_LIBRARIES}")
# message("boost inc:${Boost_INCLUDE_DIR}")

# Termcolor package
find_path(TERMCOLOR_INCLUDE_DIRS "termcolor/termcolor.hpp")

# print debug information 
# include(./config/PrintVariables.cmake) # log cmake variables

# include(CTest)
# enable_testing()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)

# CPACK steers build files to proper paths (maybe used later in build process)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

option(ENABLE_ASAN "Enable AddressSanitizer, a fast memory error detector based on compiler instrumentation" OFF)
if(ENABLE_ASAN)
    target_compile_options(${PROJECT_NAME} PUBLIC -fsanitize=address)
    target_link_options(${PROJECT_NAME} PUBLIC -fsanitize=address)
endif()

option(ENABLE_UBSAN "Enable UBSanitizer, undefined behaviour detector based on compiler instrumentation" OFF)
if(ENABLE_UBSAN)
    target_compile_options(${PROJECT_NAME} PUBLIC -fsanitize=undefined)
    target_link_options(${PROJECT_NAME} PUBLIC -fsanitize=undefined)
endif()
