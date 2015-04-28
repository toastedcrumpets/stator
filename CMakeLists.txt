cmake_minimum_required (VERSION 2.6 FATAL_ERROR)
if (NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected, default to Release")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release" FORCE)
endif()

project(stator) #Project name

enable_testing() #Enable build of test executables and 'make test' command
include(CTest)

##########   RELEASE MODE
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")
##########   DEBUG MODE
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g -DEIGEN_INTERNAL_DEBUGGING")

######################################################################
########## COMPILER C++11/C++0x SUPPORT TESTS
######################################################################
### First check if the compiler supports C++11 or C++0x at all
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORT_CXX11)
if(COMPILER_SUPPORT_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
  check_cxx_compiler_flag("-std=c++0x" COMPILER_SUPPORT_CXX0X)
  if(COMPILER_SUPPORT_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    ##Check here for required functionality, if we want to support
    ##C++0x implementations

    #Range based for loops
    check_cxx_source_compiles("int main(int, const char**) {int array[5] = {1,2,3,4,5}; for(int& x: array) x *= 2;}" CXX0X_RANGE_BASED_FOR)
    if(NOT CXX0X_RANGE_BASED_FOR)
      message(SEND_ERROR "The compiler ${CMAKE_CXX_COMPILER} has limited C++11 support. You must install a more modern C++ compiler.")
    endif()

  else()
    message(SEND_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. You must install a more modern C++ compiler.")
  endif()
endif()

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.46 COMPONENTS "unit_test_framework")
if(NOT Boost_FOUND)
  message(STATUS "Cannot find a static installation of boost, trying a dynamic version")
  set(Boost_USE_STATIC_LIBS OFF)
  find_package(Boost 1.46 REQUIRED COMPONENTS "unit_test_framework")
  if(NOT Boost_FOUND)
    message(SEND_ERROR "Cannot find a dynamic installation of boost, failing")
  endif()
endif()
include_directories(${Boost_INCLUDE_DIRS})
link_libraries(${Boost_SYSTEM_LIBRARY})

######################################################################
########## HEADER DIRECTORIES
######################################################################
include_directories(${PROJECT_SOURCE_DIR}/)
include_directories(${PROJECT_SOURCE_DIR}/extern/eigen)

######################################################################
######### DOCUMENTATION TARGET
######################################################################
find_package(Doxygen)
if(DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} COMMENT "Generating API documentation with Doxygen" VERBATIM)
endif(DOXYGEN_FOUND)

######################################################################
######### TEST TARGETS
######################################################################
function(stator_test name) #Registers a unit-test
  add_executable(${name} ${CMAKE_CURRENT_SOURCE_DIR}/testing/${name}.cpp)
  add_test(${name} ${name})
endfunction(stator_test)

stator_test(orphan_static_list)
stator_test(symbolic_generic)
stator_test(symbolic_polynomial)