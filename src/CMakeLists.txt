# find packages ----------------------------
# find vcpkg packages
find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")

# Boost package
# install correct version https://www.appsloveworld.com/cplus/100/197/cmake-new-boost-version-may-have-incorrect-or-missing-dependencies-and-imported
find_package(Boost 1.81.0 COMPONENTS program_options filesystem REQUIRED)
message("📚 boost lib: ${Boost_LIBRARIES}")
message("📚 boost inc: ${Boost_INCLUDE_DIR}")

# Termcolor package
find_path(TERMCOLOR_INCLUDE_DIRS "termcolor/termcolor.hpp")
#-------------------------------------------

include_directories(${BOOST_INCLUDE_DIRS})

include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists-interface.txt)
include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists-node.txt)
include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists-user.txt)
