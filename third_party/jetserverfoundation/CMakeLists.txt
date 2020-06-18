# Copyright 2019 The Jet Server foundation Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file. See the AUTHORS file for names of contributors.

cmake_minimum_required(VERSION 3.9)

project(jsflib VERSION 1.22.0 LANGUAGES C CXX)

# C standard can be overridden when this is used as a sub-project.
if(NOT CMAKE_C_STANDARD)
  # This project can use C11, but will gracefully decay down to C89.
  set(CMAKE_C_STANDARD 11)
  set(CMAKE_C_STANDARD_REQUIRED OFF)
  set(CMAKE_C_EXTENSIONS OFF)
endif(NOT CMAKE_C_STANDARD)

# C++ standard can be overridden when this is used as a sub-project.
if(NOT CMAKE_CXX_STANDARD)
  # This project requires C++11.
  set(CMAKE_CXX_STANDARD 11)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif(NOT CMAKE_CXX_STANDARD)

if (WIN32)
  set(JSF_PLATFORM_NAME JSF_PLATFORM_WINDOWS)
  # TODO(cmumford): Make UNICODE configurable for Windows.
  add_definitions(-D_UNICODE -DUNICODE)
else (WIN32)
  set(JSF_PLATFORM_NAME JSF_PLATFORM_POSIX)
endif (WIN32)

option(JSF_BUILD_TESTS "Build jsf's unit tests" ON)
#option(JSF_BUILD_BENCHMARKS "Build LevelDB's benchmarks" ON)
option(JSF_INSTALL "Install jsf's header and library" ON)

include(CheckIncludeFile)
check_include_file("unistd.h" HAVE_UNISTD_H)

include(CheckCXXSymbolExists)
# Using check_cxx_symbol_exists() instead of check_c_symbol_exists() because
# we're including the header from C++, and feature detection should use the same
# compiler language that the project will use later. Principles aside, some
# versions of do not expose fdatasync() in <unistd.h> in standard C mode
# (-std=c11), but do expose the function in standard C++ mode (-std=c++11).
check_cxx_symbol_exists(fdatasync "unistd.h" HAVE_FDATASYNC)
check_cxx_symbol_exists(F_FULLFSYNC "fcntl.h" HAVE_FULLFSYNC)
check_cxx_symbol_exists(O_CLOEXEC "fcntl.h" HAVE_O_CLOEXEC)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # Disable C++ exceptions.
  string(REGEX REPLACE "/EH[a-z]+" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHs-c-")
  add_definitions(-D_HAS_EXCEPTIONS=0)

  # Disable RTTI.
  string(REGEX REPLACE "/GR" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR-")
else(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # Enable strict prototype warnings for C code in clang and gcc.
  if(NOT CMAKE_C_FLAGS MATCHES "-Wstrict-prototypes")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wstrict-prototypes")
  endif(NOT CMAKE_C_FLAGS MATCHES "-Wstrict-prototypes")

  # Disable C++ exceptions.
  string(REGEX REPLACE "-fexceptions" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")

  # Disable RTTI.
  string(REGEX REPLACE "-frtti" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

# Test whether -Wthread-safety is available. See
# https://clang.llvm.org/docs/ThreadSafetyAnalysis.html
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-Wthread-safety HAVE_CLANG_THREAD_SAFETY)

# Used by googletest.
check_cxx_compiler_flag(-Wno-missing-field-initializers
                        JSF_HAVE_NO_MISSING_FIELD_INITIALIZERS)

include(CheckCXXSourceCompiles)

# Test whether C++17 __has_include is available.
check_cxx_source_compiles("
#if defined(__has_include) &&  __has_include(<string>)
#include <string>
#endif
int main() { std::string str; return 0; }
" HAVE_CXX17_HAS_INCLUDE)

set(JSF_PUBLIC_INCLUDE_DIR "include")

include_directories(
  "${PROJECT_BINARY_DIR}/include"
  "${JSF_PUBLIC_INCLUDE_DIR}"
)

if(BUILD_SHARED_LIBS)
  # Only export JSF_EXPORT symbols from the shared library.
  add_compile_options(-fvisibility=hidden)
endif(BUILD_SHARED_LIBS)

# Must be included before CMAKE_INSTALL_INCLUDEDIR is used.
include(GNUInstallDirs)

add_library(jsflib "")
target_sources(jsflib
  PRIVATE
    "${JSF_PUBLIC_INCLUDE_DIR}/jsf_memory_pool.h"
    "${JSF_PUBLIC_INCLUDE_DIR}/jsf_client_property.h"
    "${JSF_PUBLIC_INCLUDE_DIR}/jsf_onpacket_thread.h"
    "${JSF_PUBLIC_INCLUDE_DIR}/jsf_packet.h"
    "${JSF_PUBLIC_INCLUDE_DIR}/jsf_server.h"
    "${JSF_PUBLIC_INCLUDE_DIR}/jsf_server_sockets_mgr.h"
    "${JSF_PUBLIC_INCLUDE_DIR}/jsf_socket.h"
    "${JSF_PUBLIC_INCLUDE_DIR}/jsf_socket_ring_buffer.h"
    "${JSF_PUBLIC_INCLUDE_DIR}/jsf_socket_wrapper.h"
    "${JSF_PUBLIC_INCLUDE_DIR}/jsf_thread.h"
    "${JSF_PUBLIC_INCLUDE_DIR}/jsf_thread_lock.h"
    "${JSF_PUBLIC_INCLUDE_DIR}/jsf_type_defs.h"
    "src/jsf_client_property.cpp"
    "src/jsf_onpacket_thread.cpp"
    "src/jsf_server.cpp"
    "src/jsf_server_sockets_mgr.cpp"
    "src/jsf_socket.cpp"
    "src/jsf_socket_wrapper.cpp"
    "src/jsf_thread.cpp"
    
  # Only CMake 3.3+ supports PUBLIC sources in targets exported by "install".
  $<$<VERSION_GREATER:CMAKE_VERSION,3.2>:PUBLIC>
    "${JSF_PUBLIC_INCLUDE_DIR}/jsf_client_property.h"
    "${JSF_PUBLIC_INCLUDE_DIR}/jsf_onpacket_thread.h"
    "${JSF_PUBLIC_INCLUDE_DIR}/jsf_packet.h"
    "${JSF_PUBLIC_INCLUDE_DIR}/jsf_server.h"
    "${JSF_PUBLIC_INCLUDE_DIR}/jsf_thread.h"
    "${JSF_PUBLIC_INCLUDE_DIR}/jsf_thread_lock.h"
    "${JSF_PUBLIC_INCLUDE_DIR}/export.h"
    "${JSF_PUBLIC_INCLUDE_DIR}/jsf_type_defs.h"
)

target_include_directories(jsflib
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set_target_properties(jsflib
  PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})

target_compile_definitions(jsflib
  PRIVATE
    # Used by include/export.h when building shared libraries.
    JSF_COMPILE_LIBRARY
    # Used by port/port.h.
    ${JSF_PLATFORM_NAME}=1
)

if(BUILD_SHARED_LIBS)
  target_compile_definitions(jsflib
    PUBLIC
      # Used by include/export.h.
      JSF_SHARED_LIBRARY
  )
endif(BUILD_SHARED_LIBS)

if(HAVE_CLANG_THREAD_SAFETY)
  target_compile_options(jsflib
    PUBLIC
      -Werror -Wthread-safety)
endif(HAVE_CLANG_THREAD_SAFETY)


if(JSF_BUILD_TESTS)
  enable_testing()

  # Prevent overriding the parent project's compiler/linker settings on Windows.
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  set(install_gtest OFF)
  set(install_gmock OFF)
  set(build_gmock ON)

  # This project is tested using GoogleTest.
  #add_subdirectory("../googletest")
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../googletest ${CMAKE_CURRENT_BINARY_DIR}/googletest)

  # GoogleTest triggers a missing field initializers warning.
  if(JSF_HAVE_NO_MISSING_FIELD_INITIALIZERS)
    set_property(TARGET gtest
        APPEND PROPERTY COMPILE_OPTIONS -Wno-missing-field-initializers)
    set_property(TARGET gmock
        APPEND PROPERTY COMPILE_OPTIONS -Wno-missing-field-initializers)
  endif(JSF_HAVE_NO_MISSING_FIELD_INITIALIZERS)

  function(jsflib_test test_file)
    get_filename_component(test_target_name "${test_file}" NAME_WE)

    add_executable("${test_target_name}" "")
    target_sources("${test_target_name}"
      PRIVATE
        "${test_file}"
    )
    target_link_libraries("${test_target_name}" gmock gtest)
    target_compile_definitions("${test_target_name}"
      PRIVATE
        ${JSF_PLATFORM_NAME}=1
    )
    add_test(NAME "${test_target_name}" COMMAND "${test_target_name}")
  endfunction(jsflib_test)


  if(NOT BUILD_SHARED_LIBS)
    jsflib_test("test/jsf_test.cc")
  endif(NOT BUILD_SHARED_LIBS)
endif(JSF_BUILD_TESTS)


if(JSF_INSTALL)
  install(TARGETS jsflib
    EXPORT jsflibTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )
  install(
    FILES
    "${JSF_PUBLIC_INCLUDE_DIR}/jsf_client_property.h"
    "${JSF_PUBLIC_INCLUDE_DIR}/jsf_onpacket_thread.h"
    "${JSF_PUBLIC_INCLUDE_DIR}/jsf_packet.h"
    "${JSF_PUBLIC_INCLUDE_DIR}/jsf_server.h"
    "${JSF_PUBLIC_INCLUDE_DIR}/jsf_thread.h"
    "${JSF_PUBLIC_INCLUDE_DIR}/jsf_thread_lock.h"
    "${JSF_PUBLIC_INCLUDE_DIR}/export.h"
    "${JSF_PUBLIC_INCLUDE_DIR}/jsf_type_defs.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/jsflib"
  )

  include(CMakePackageConfigHelpers)
  configure_package_config_file(
    "cmake/${PROJECT_NAME}Config.cmake.in"
    "${PROJECT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
  )
  write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
  )
  install(
    EXPORT jsflibTargets
    NAMESPACE jsflib::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
  )
  install(
    FILES
      "${PROJECT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake"
      "${PROJECT_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
  )
endif(JSF_INSTALL)
