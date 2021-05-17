cmake_minimum_required (VERSION 3.0)
if (NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected, default to Release")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo" FORCE)
endif()

project(stator VERSION 1.0) #Project name

enable_testing() #Enable build of test executables and 'make test' command
include(CTest)

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

#set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
#set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

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
    include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/extern/eigen)
  else()
    message(SEND_ERROR "Could not find the Eigen library. Either install it to the system, or use the git submodule to download a local copy.")
  endif()
endif()


######################################################################
########## COMPILER C++11/C++0x SUPPORT TESTS
######################################################################
### First check if the compiler supports C++11 or C++0x and enable the build flag
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
########## PYTHON INTERFACE
######################################################################

add_subdirectory(extern/pybind11)

# Build a Python extension module using pybind11
#   pybindings_add_module(<module>)
# Here <module> should be the fully qualified name for the module,
# e.g. pybindings_add_module(foo.bar._baz)
# <module> becomes the target name in case you wish to do something to it later
# The source for the binding *must* be placed in src/pybindings/{relpath}/py{name}.cc
# E.g. for module=foo.bar._baz -> src/pybindings/bar/py_baz.cc
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "build" CACHE STRING "Where to place the built library")
function(pybindings_add_module module)
    set(target_name ${module})
    string(REPLACE "." "/" modpath ${module})
    string(REPLACE "." ";" modlist ${module})

    # The module name is the last entry
    list(GET modlist -1 modname)

    # Remove everything that is not the root or the module name
    #list(REMOVE_AT modlist 0)
    list(REMOVE_AT modlist -1)

    # Get the relative path
    if(modlist)
        string(REPLACE ";" "/" relpath "${modlist}")
    else()
        set(relpath "")
    endif()

    # Define the binding source file
    set(sources pysrc/${relpath}/${modname}.cpp)

    # Invoke pybind11 and set where the library should go, and what it is called
    pybind11_add_module(${target_name} ${sources})
    set(outdir ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${relpath})
    set_target_properties(${target_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${outdir})
    set_target_properties(${target_name} PROPERTIES OUTPUT_NAME ${modname})
endfunction()

include_directories(${PROJECT_SOURCE_DIR})
pybindings_add_module(stator.core)


######################################################################
######### TEST TARGETS
######################################################################
add_executable(symbolic_example ${CMAKE_CURRENT_SOURCE_DIR}/examples/symbolic_example.cpp)

find_package(GTest REQUIRED)

if(GTest_FOUND)
  function(stator_test name) #Registers a unit-test
    add_executable(${name} ${CMAKE_CURRENT_SOURCE_DIR}/tests/${name}.cpp)
    target_include_directories(${name} PUBLIC ${GTEST_INCLUDE_DIRS})
    target_link_libraries(${name} PUBLIC ${GTEST_LIBRARIES} pthread)
    add_test(${name} ${name})
  endfunction(stator_test)

  #stator_test(orphan_static_list)
  stator_test(stack_vector_test)
  #stator_test(geometry_shapes_test)
  stator_test(symbolic_generic_test)
  stator_test(symbolic_polynomial_test)
  stator_test(symbolic_poly_solve_roots_test)
  stator_test(symbolic_poly_taylor_test)
  stator_test(symbolic_runtime_test)
  stator_test(symbolic_numeric_test)
  stator_test(symbolic_parser_test)
  stator_test(symbolic_ad_test)
  #stator_test(symbolic_integration_test)
else()
  message(WARNING "Cannot find GTest library, disabling unit tests!")
endif()

