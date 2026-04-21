cmake_minimum_required(VERSION 3.10)
project(BachelorarbeitUG4 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#  UG4 root
set(UG4_ROOT "$ENV{HOME}/ug4")

#  find UG4’s shared library
find_library(UG4_LIBRARY
    NAMES ug4
    HINTS "${UG4_ROOT}/lib"
)
if(NOT UG4_LIBRARY)
    message(FATAL_ERROR "libug4.so not found in ${UG4_ROOT}/lib")
endif()

#  pull in UG4 headers
include_directories("${UG4_ROOT}/ugcore/ugbase")

#  find MPI
find_package(MPI REQUIRED)

#  your executable
add_executable(main src/main.cpp)

#  make assert() visible to UG4 headers
target_compile_options(main PRIVATE -include cassert)

#  link against MPI and UG4
target_link_libraries(main
    PRIVATE
      MPI::MPI_CXX
      ${UG4_LIBRARY}
)

#  so that at runtime your libug4.so is found under $HOME/ug4/lib
set_target_properties(main PROPERTIES
    BUILD_WITH_INSTALL_RPATH  TRUE
    INSTALL_RPATH            "${UG4_ROOT}/lib"
)
