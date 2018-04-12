cmake_minimum_required (VERSION 2.6 FATAL_ERROR)
if (NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected, default to Release")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo" FORCE)
endif()

project(stator) #Project name

enable_testing() #Enable build of test executables and 'make test' command
include(CTest)


IF(WIN32)
ENDIF(WIN32)

##########   RELEASE MODE
if(MSVC)
  #MSVC has crazy warnings for -Wall, we'll build up support to the higher warning levels
  add_compile_options(-W1)
  #Enable the math defines (like M_PI!)
  add_definitions(-D_USE_MATH_DEFINES)
  #MSVC Crazily has "min" and "max" (yes, lowercase!) Macros defined
  #which conflict with the C++ standard algorithms! This define
  #instructs the compiler to not be an idiot and define those macros
  add_definitions(-DNOMINMAX)
  #MSVC is scared of long type names (sigh). Disable the warning.
  add_definitions("/wd4503")
  #MSVC actually needs to be told to use big object files, when
  #there's a lot of template shennanigans to store.
  ADD_DEFINITIONS(/bigobj)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /bigobj")
  SET(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /bigobj")
  SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /bigobj")
SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /bigobj")
else()
  add_compile_options(-Wall)
endif()

##########   DEBUG MODE
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DEIGEN_INTERNAL_DEBUGGING -D_GLIBCXX_DEBUG")

######################################################################
########## Function to add includes to the check_*** tests
######################################################################
function(add_required_include include)
  if(CMAKE_REQUIRED_INCLUDES)
    set(CMAKE_REQUIRED_INCLUDES "${CMAKE_REQUIRED_INCLUDES};${include}" PARENT_SCOPE)
  else(CMAKE_REQUIRED_INCLUDES)
    set(CMAKE_REQUIRED_INCLUDES "${include}" PARENT_SCOPE)
  endif(CMAKE_REQUIRED_INCLUDES)
endfunction()

#Ensure the stator library itself is available
add_required_include(${CMAKE_SOURCE_DIR})

######################################################################
########## Eigen library detection and configuration
######################################################################
include(CheckIncludeFileCXX)

check_include_file_cxx(Eigen/Dense HAS_SYSTEM_EIGEN)
if(HAS_SYSTEM_EIGEN)
  message(STATUS "Found installed Eigen library, using this.")
else()
  message(STATUS "Could not find installed Eigen library, searching for Eigen git submodule.")
  add_required_include(${CMAKE_SOURCE_DIR}/extern/eigen)
  check_include_file_cxx(Eigen/Dense HAS_LOCAL_EIGEN)
  if (HAS_LOCAL_EIGEN)
    message(STATUS "Using git submodule Eigen library.")
    include_directories(${CMAKE_SOURCE_DIR}/extern/eigen)
  else()
    message(SEND_ERROR "Could not find the Eigen library. Either install it to the system, or use the git submodule to download a local copy.")
  endif()
endif()


######################################################################
########## COMPILER C++11/C++0x SUPPORT TESTS
######################################################################
### First check if the compiler supports C++11 or C++0x and enable the build flag
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORT_CXX11)
if(COMPILER_SUPPORT_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
  check_cxx_compiler_flag("-std=c++0x" COMPILER_SUPPORT_CXX0X)
  if(COMPILER_SUPPORT_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  else()
    message(SEND_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 or C++0X support. You must install a more modern C++ compiler.")
  endif()
endif()

#Test for different C++11 functionality
if(NOT MSVC)
  set(CMAKE_REQUIRED_FLAGS -Wfatal-errors)
endif()

check_cxx_source_compiles("int main(int, const char**) {int array[5] = {1,2,3,4,5}; for(int& x: array) x *= 2;}" CXX11_RANGE_BASED_FOR)
check_cxx_source_compiles("template <class T> class A {}; template <class T> using B = A<T>; int main(int, const char**) { B<double> b; }" CXX11_TEMPLATE_TYPEDEFS)
check_cxx_source_compiles("template <typename T> struct wrapped{ typedef T type;};
template <typename T> typename T::type unwrap1(T a) { return typename T::type();}
int unwrap(int a) { return 1; }
template <typename T> auto unwrap(T t) -> decltype(unwrap(unwrap1(t))) { return unwrap(unwrap1(t)); }
int main() { unwrap(wrapped<wrapped<int>>()); }" CXX11_TYPE_MANGLING)

#Complain about missing functionality
if(NOT CXX11_TYPE_MANGLING)
  message(SEND_ERROR "C++11 decltype seems not to be working properly, please install a more modern C++ compiler.")
endif()

unset(CMAKE_REQUIRED_FLAGS)
unset(CMAKE_REQUIRED_INCLUDES)

######################################################################
########## HEADER DIRECTORIES
######################################################################
include_directories(${PROJECT_SOURCE_DIR}/)

######################################################################
######### DOCUMENTATION TARGET
######################################################################
find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} COMMENT "Generating API documentation with Doxygen" VERBATIM)
else()
  message(WARNING "Cannot find doxygen, disabling documentation output build target.")
endif(DOXYGEN_FOUND)

######################################################################
######### TEST TARGETS
######################################################################
function(stator_test name) #Registers a unit-test
  add_executable(${name} ${CMAKE_CURRENT_SOURCE_DIR}/testing/${name}.cpp)
  add_test(${name} ${name})
endfunction(stator_test)

add_executable(symbolic_example ${CMAKE_CURRENT_SOURCE_DIR}/examples/symbolic_example.cpp)


#stator_test(orphan_static_list)
stator_test(stack_vector)

if(CXX11_TEMPLATE_TYPEDEFS)
  #stator_test(geometry_shapes)
  stator_test(symbolic_generic)
  stator_test(symbolic_polynomial)
  stator_test(symbolic_poly_solve_roots)
  stator_test(symbolic_poly_taylor)
  stator_test(symbolic_runtime)
  stator_test(symbolic_numeric)
  stator_test(symbolic_parser)
  stator_test(symbolic_ad)
  #stator_test(symbolic_integration)
endif()
