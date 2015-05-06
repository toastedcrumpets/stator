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

function(add_include include)
  if(CMAKE_REQUIRED_INCLUDES)
    set(CMAKE_REQUIRED_INCLUDES "${CMAKE_REQUIRED_INCLUDES};${include}" PARENT_SCOPE)
  else(CMAKE_REQUIRED_INCLUDES)
    set(CMAKE_REQUIRED_INCLUDES "${include}" PARENT_SCOPE)
  endif(CMAKE_REQUIRED_INCLUDES)
endfunction()

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

include(CheckIncludeFileCXX)
check_include_file_cxx(Eigen/Dense HAS_SYSTEM_EIGEN)
if(HAS_SYSTEM_EIGEN)
  message(STATUS "Found installed Eigen library, using this.")
else()
  message(STATUS "Could not find installed Eigen library, searching for Eigen git submodule.")
  add_include(${CMAKE_SOURCE_DIR}/extern/eigen)
  check_include_file_cxx(Eigen/Dense HAS_LOCAL_EIGEN)
  if (HAS_LOCAL_EIGEN)
    message(STATUS "Using git submodule Eigen library.")
    include_directories(${CMAKE_SOURCE_DIR}/extern/eigen)
  else()
    message(SEND_ERROR "Could not find the Eigen library. Either install it to the system, or use the git submodule to download a local copy.")
  endif()
endif()

add_include(${CMAKE_SOURCE_DIR})
set(CMAKE_REQUIRED_FLAGS -Wfatal-errors)
check_cxx_source_compiles("template <typename T> struct wrapped{ typedef T type;};
template <typename T> typename T::type unwrap1(T a) { return typename T::type();}
int unwrap(int a) { return 1; }
template <typename T> auto unwrap(T t) -> decltype(unwrap(unwrap1(t))) { return unwrap(unwrap1(t)); }
int main() { unwrap(wrapped<wrapped<int>>()); }" CXX11_TYPE_MANGLING)
if(NOT CXX11_TYPE_MANGLING)
  message(WARNING "Although the compiler advertises support for C++0x/C++11, this support is limited (e.g., gcc-4.7 or lower). You can only test a reduced section of stator (no symbolic math).")
  else()
    set(CMAKE_REQUIRED_FLAGS -Wfatal-errors)
    check_cxx_source_compiles("#include <stator/symbolic/symbolic.hpp>
    int main() { using namespace stator::symbolic; Variable<'x'> x; simplify(x+(x-1)); }" CXX11_decltype_works)
    if(NOT CXX11_decltype_works)
      message(WARNING "Although the compiler advertises support for C++0x/11, this support is broken (e.g., gcc-4.8.2). You can only test a reduced section of stator (no symbolic math).")
    endif()  
endif()
unset(CMAKE_REQUIRED_FLAGS)


unset(CMAKE_REQUIRED_INCLUDES)

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
stator_test(geometry_shapes)

if(CXX11_TYPE_MANGLING AND CXX11_decltype_works)
  stator_test(symbolic_generic)
  stator_test(symbolic_polynomial)
endif()
