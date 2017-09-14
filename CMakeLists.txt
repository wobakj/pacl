cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project(pacl)

#set the build type if its not set
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE )
endif()

#if user didnt set install dir, override it and write it to the cache -> Type and description necessary,
#overwrite variable, not just write it to cache
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/install" CACHE STRING "Install path prefix, prepended onto install directories." FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

#before adding libraries, define the output paths
# MSVC & Xcode automatically create the build-type folders
if(MSVC OR CMAKE_GENERATOR STREQUAL Xcode)
  SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
  SET(LIBRARY_OUTPUT_PATH  ${PROJECT_BINARY_DIR})
else()
  SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
  SET(LIBRARY_OUTPUT_PATH  ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
endif()
# externals
# include glm, as system header to suppress warnings
include_directories(SYSTEM external/glm-0.9.8.4)

include_directories(include)

# ocl exe
find_package(MPI REQUIRED)
add_executable(hello source/hello.cpp)
install(TARGETS hello DESTINATION .)
# dependencies
target_include_directories(hello SYSTEM PRIVATE ${MPI_CXX_INCLUDE_PATH})
target_link_libraries(hello ${MPI_CXX_LIBRARIES})
# Add a test:
enable_testing()
add_test(SimpleTest ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} 4 ${CMAKE_CURRENT_BINARY_DIR}/mytest)

# set build type dependent flags
if(UNIX)
    set(CMAKE_CXX_FLAGS_RELEASE "-O2")
elseif(MSVC)
  set(CMAKE_CXX_FLAGS_RELEASE "/MD /O2")
  set(CMAKE_CXX_FLAGS_DEBUG "/MDd /Zi")
endif()

# activate C++ 11
if(NOT MSVC)
    add_definitions(-std=c++11)
    # show all warnings
    add_definitions(-Wall -Wconversion -fstrict-aliasing)
    # force linking with c++11 lib
    if(APPLE)
        set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++0x")
        add_definitions(-stdlib=libc++)
    endif()
else()
    # build in parallel
    add_definitions(/MP /W3)
endif()

# SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# installation rules, copy ocl kernels
install(DIRECTORY ${PROJECT_SOURCE_DIR}/kernels
DESTINATION ${CMAKE_INSTALL_PREFIX})