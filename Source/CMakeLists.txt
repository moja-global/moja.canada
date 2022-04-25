cmake_minimum_required(VERSION 3.10.0)
project(CBM VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Turn on using solution folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Turn on parallel builds.
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)

# Set the path for custom CMake scripts.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Include some common macros to simpilfy the Moja CMake files
include(MojaMacros)

# Collect the built libraries and include dirs - they will be used to create the moja.cmake file.
set(${CMAKE_PROJECT_NAME}_COMPONENTS "")

# Allow enabling and disabling components.
option(ENABLE_MOJA.MODULES.CBM "moja.modules.cbm" ON)
option(ENABLE_SAWTOOTH "sawtooth" OFF)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "C:/Development/Software/${PROJECT_NAME}" CACHE PATH "..." FORCE)
endif()

# Put the libaries and binaries that get built into directories at the
# top of the build tree rather than in hard-to-find leaf
# directories. This simplifies manual testing and the use of the build
# tree rather than installed Boost libraries.
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# For Debug build types, append a "d" to the library names.
set(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "Set debug library postfix" FORCE)

option(ENABLE_TESTS "Set to OFF|ON (default is ON) to control build of tests" ON)
option(RUN_UNIT_TESTS_ON_BUILD "Set to OFF|ON (default is OFF) to control automatic running of tests at build time" OFF)
option(${CMAKE_PROJECT_NAME}_STATIC "Set to OFF|ON (default is OFF) to control build as STATIC library" OFF)

# Uncomment from next two lines to force statitc or dynamic library, default is autodetection.
if(${CMAKE_PROJECT_NAME}_STATIC)
    add_definitions(-D${CMAKE_PROJECT_NAME}_STATIC -DMOJA_NO_AUTOMATIC_LIBS)    
    set(LIB_MODE STATIC)
    message(STATUS "Building static libraries")
else(${CMAKE_PROJECT_NAME}_STATIC)
    set(LIB_MODE SHARED)
    message(STATUS "Building dynamic libraries")
endif(${CMAKE_PROJECT_NAME}_STATIC)

if(ENABLE_TESTS)
    include(CTest)
    set(BOOST_TEST_REPORTING_LEVEL "SHORT" CACHE STRING "Boost unit test reporting level")
    set_property(CACHE BOOST_TEST_REPORTING_LEVEL PROPERTY STRINGS "SHORT" "DETAILED")
    enable_testing()
    message(STATUS "Building with unittests")
else()
    message(STATUS "Building without tests")
endif()

# Boost
set(Boost_USE_STATIC_LIBS ${USE_STATIC_BOOST})
set(Boost_USE_MULTITHREADED ON)
unset(Boost_INCLUDE_DIR CACHE)
unset(Boost_LIBRARY_DIRS CACHE)

if(${CMAKE_PROJECT_NAME}_STATIC)
	add_definitions(-DUSE_STATIC_BOOST)
    set(Boost_USE_STATIC_LIBS ON)
	set(Boost_USE_STATIC_RUNTIME ON)
else(${CMAKE_PROJECT_NAME}_STATIC)
	add_definitions(-DBOOST_ALL_DYN_LINK)
    set(Boost_USE_STATIC_LIBS OFF)
	set(Boost_USE_STATIC_RUNTIME OFF)
endif(${CMAKE_PROJECT_NAME}_STATIC)

if(USE_STATIC_BOOST)
   message(STATUS "Linking against boost static libraries")
else(USE_STATIC_BOOST)
   message(STATUS "Linking against boost dynamic libraries")
endif(USE_STATIC_BOOST)

# OS Detection
message(STATUS "The compiler in use is ${CMAKE_CXX_COMPILER}.")
message(STATUS "The compiler flags in use is ${CMAKE_CXX_FLAGS}.")
if(CMAKE_SYSTEM MATCHES "Windows")
    add_definitions(-DMOJA_OS_FAMILY_WINDOWS)
 
    if(CMAKE_C_COMPILER_ID MATCHES "MSVC")
        message(STATUS "XXX: MS Visual Compiler detected")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /bigobj")
    endif(CMAKE_C_COMPILER_ID MATCHES "MSVC")
endif(CMAKE_SYSTEM MATCHES "Windows")

if (CMAKE_SYSTEM MATCHES "Linux")
    add_definitions(-DMOJA_OS_FAMILY_UNIX)
  
    # Standard 'must be' defines.
    add_definitions(-D_XOPEN_SOURCE=500 -D_REENTRANT -D_THREAD_SAFE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64)
    set(SYSLIBS pthread dl rt)

    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("-std=c++17" COMPILER_SUPPORTS_CXX17)

    if(COMPILER_SUPPORTS_CXX17)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fPIC")
    else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++17 support. Please use a different C++ compiler.")
    endif()
endif(CMAKE_SYSTEM MATCHES "Linux")

if(ENABLE_MOJA.MODULES.CBM)
    add_subdirectory(moja.modules.cbm)
    list(APPEND ${CMAKE_PROJECT_NAME}_COMPONENTS "moja.modules.cbm")
endif()

if(ENABLE_SAWTOOTH)
    add_subdirectory(sawtooth)
    list(APPEND ${CMAKE_PROJECT_NAME}_COMPONENTS "sawtooth")
	add_subdirectory(moja.modules.sawtooth)
	list(APPEND ${CMAKE_PROJECT_NAME}_COMPONENTS "moja.modules.sawtooth")
endif()


foreach(component ${${CMAKE_PROJECT_NAME}_COMPONENTS})
    message(STATUS "Building: ${component}")
endforeach()

#############################################################
# Uninstall stuff see: http://www.vtk.org/Wiki/CMake_FAQ

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY
)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
)

#############################################################
# Enable packaging

set(CPACK_PACKAGE_VERSION_MAJOR "${MOJA_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${MOJA_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${MOJA_VERSION_PATCH}")

set(CPACK_SOURCE_PACKAGE_FILE_NAME
    "${CMAKE_PROJECT_NAME}-${MOJA_VERSION}"
)

set(CPACK_SOURCE_GENERATOR "ZIP")
set(CPACK_SOURCE_IGNORE_FILES
    "/.hg/"
    "/.hgignore$"
    "build/"
    "ipch/"
    "/resources/"
    ".sdf$;.suo$;.tss$"
)

include(CPack)
add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

#############################################################
# cmake config files

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}ConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

# Set config script install location in a location that find_package() will
# look for, which is different on MS Windows than for UNIX
# Note: also set in MOJA_GENERATE_PACKAGE macro in cmake/MojaMacros.cmake
if (WIN32)
  set(MojaConfigPackageLocation "cmake")
else()
  set(MojaConfigPackageLocation "lib/cmake/${PROJECT_NAME}")
endif()

configure_file(cmake/${PROJECT_NAME}Config.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}Config.cmake" @ONLY)
install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}ConfigVersion.cmake
    DESTINATION
        "${MojaConfigPackageLocation}"
    COMPONENT
        Devel
)

# Install pkg-config files
install(FILES ${MOJA_PKG_CONFIG_FILES}
        DESTINATION lib${LIB_SUFFIX}/pkgconfig)

export(PACKAGE ${PROJECT_NAME})
		
message(STATUS "CMake ${CMAKE_VERSION} successfully configured ${PROJECT_NAME} using ${CMAKE_GENERATOR} generator")
message(STATUS "Installation target path: ${CMAKE_INSTALL_PREFIX}")

message(STATUS "C_FLAGS:  =${CMAKE_C_FLAGS}")
message(STATUS "CXX_FLAGS:=${CMAKE_CXX_FLAGS}")

# clang-format and tidy

find_package(ClangFormat)

if(CLANG_FORMAT_FOUND)
        message("clang-format executable: ${CLANG_FORMAT_EXECUTABLE}")
        message("clang-format version: ${CLANG_FORMAT_VERSION}")
else()
        message(WARNING "clang-format executable not found")
endif()

include(cmake/ClangDevTools.cmake)

# Install githooks directory

message("Installing git hooks at ${CMAKE_CURRENT_SOURCE_DIR} with `git config --local core.hooksPath githooks`")
execute_process(
    COMMAND git config --local core.hooksPath githooks
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
