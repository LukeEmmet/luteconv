cmake_minimum_required(VERSION 3.10)

project(luteconv CXX)

# Semantic versioning
set(VERSION "1.0.0")

if(NOT ${CMAKE_BUILD_TYPE} MATCHES "Debug")
	set(CMAKE_BUILD_TYPE "Release")
endif()

# Harden
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -fPIE -fstack-protector-strong -Wformat -Wformat-security -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now")

if (CMAKE_BUILD_TYPE MATCHES "Release")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -o2")
	add_definitions(-D_FORTIFY_SOURCE=2)
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif()

add_definitions(-DVERSION=${VERSION})
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie")

# Set target directories for executables and libraries
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib64)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib64)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

enable_testing()

find_library(PUGIXML_LIBRARY NAMES pugixml REQUIRED)
find_library(ZIP_LIBRARY NAMES zip REQUIRED)
include(FindZLIB)
include(FindGTest)

add_subdirectory(src)
add_subdirectory(test)

# Packaging
set(CPACK_PACKAGE_VERSION ${VERSION})
set(CPACK_GENERATOR "RPM" "DEB" "TGZ")
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_CONTACT "Paul Overell")
set(CPACK_PACKAGE_VENDOR "bayleaf.org.uk")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Luteconv, converts between lute tablature file formats")
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")

set(CPACK_DEBIAN_PACKAGE_DEPENDS "libz" "libpugixml" "libzip")

include(CPack)
