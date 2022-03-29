cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

cmake_policy(VERSION 3.16..3.21)

if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.19.0")
   # allows spaces in ctest names
   cmake_policy(SET CMP0110 NEW)
endif()

if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.20.0")
   # enable RTTI on MSVC
   cmake_policy(SET CMP0117 OLD)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

project(cs_libguarded)

set(BUILD_MAJOR "1")
set(BUILD_MINOR "2")
set(BUILD_MICRO "0")

set(BUILD_COMPONENTS "cs_libguarded")

option(BUILD_TESTS "Enables building the Catch2 unit tests" OFF)

include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)
include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckTypeSize)
include(FeatureSummary)

# location for install or package
if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
   include(GNUInstallDirs)

elseif (CMAKE_SYSTEM_NAME MATCHES "(Linux|OpenBSD|FreeBSD|NetBSD|DragonFly)")
   include(GNUInstallDirs)

elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")
   set(CMAKE_INSTALL_BINDIR bin)
   set(CMAKE_INSTALL_LIBDIR lib)
   set(CMAKE_INSTALL_INCLUDEDIR include)

endif()

set(PACKAGE           "cs_libguarded")
set(PACKAGE_NAME      "CsLibGuarded")
set(PACKAGE_VERSION   "${BUILD_MAJOR}.${BUILD_MINOR}.${BUILD_MICRO}")
set(PACKAGE_STRING    "cs_libguarded ${BUILD_MAJOR}.${BUILD_MINOR}.${BUILD_MICRO}")
set(PACKAGE_TARNAME   "cs_libguarded")
set(PACKAGE_BUGREPORT "info@copperspice.com")
set(PACKAGE_URL       "https://www.copperspice.com/")

set(CPACK_PACKAGE_NAME          ${PROJECT_NAME} )
set(CPACK_PACKAGE_VENDOR        "CopperSpice")
set(CPACK_PACKAGE_CONTACT       "info@copperspice.com")

set(CPACK_PACKAGE_VERSION_MAJOR ${BUILD_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${BUILD_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${BUILD_MICRO})

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A library for managing shared data")
set(CPACK_PACKAGE_DESCRIPTION_FILE    "${CMAKE_SOURCE_DIR}/README.md")

set(CPACK_SOURCE_IGNORE_FILES         "/build/;/.git;${CPACK_SOURCE_IGNORE_FILES}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY   ${CMAKE_INSTALL_PREFIX})
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY  OFF)

include(CPack)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_DIRECTORIES_BEFORE ON)
set(CMAKE_EXTENSIONS OFF)

if(APPLE)
   set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS}    -Wl,-undefined,error")
   set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-undefined,error")
   set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,-undefined,error")
else()
   set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS}    -Wl,--no-undefined")
   set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
   set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--no-undefined")
endif()

# destination for cmake export files
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
   set(PKG_PREFIX "cmake/CsLibGuarded")

else()
   set(PKG_PREFIX "${CMAKE_INSTALL_LIBDIR}/cmake/CsLibGuarded")

endif()

# file locations for building
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(src/cs_libguarded.cmake)

# catch2 set up
if(BUILD_TESTS)
   enable_testing()
   add_subdirectory(test)
endif()

if (${CMAKE_SIZEOF_VOID_P} EQUAL 4)
   set(TARGETBITS 32)
else()
   set(TARGETBITS 64)
endif()

message("")
message("CsLibGuarded configured to run on:  ${CMAKE_SYSTEM_NAME} ${TARGETBITS} bit, ${CMAKE_BUILD_TYPE} Mode")
message("CsLibGuarded will be built in:      ${CMAKE_BINARY_DIR}")
message("CsLibGuarded will be installed in:  ${CMAKE_INSTALL_PREFIX}")
message("\n")
