# Copyright (c) 2016-2017 Taylor C. Richberger <taywee@gmx.com> and Pavel
# Belikov
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

cmake_minimum_required(VERSION 3.2)

include(GNUInstallDirs)

project(args VERSION 6.4.0 LANGUAGES CXX)
set(PROJECT_DESCRIPTION "A simple, small, flexible, single-header C++11 argument parsing library. that is designed to appear somewhat similar to Python's argparse.")
set(PROJECT_HOMEPAGE_URL "https://github.com/Taywee/args")
set(PROJECT_VERSION 6.4.0)

set(MAIN_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(MAIN_PROJECT ON)
endif()

include(ExternalProject)

set(TAYWEE_ARGS_TARGET_NAME               args)
set(TAYWEE_ARGS_CONFIG_INSTALL_DIR        "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}" CACHE INTERNAL "")
set(TAYWEE_ARGS_INCLUDE_INSTALL_DIR       "${CMAKE_INSTALL_INCLUDEDIR}")
set(TAYWEE_ARGS_TARGETS_EXPORT_NAME       "${PROJECT_NAME}Targets")
set(TAYWEE_ARGS_CMAKE_CONFIG_TEMPLATE     "cmake/config.cmake.in")
set(TAYWEE_ARGS_CMAKE_CONFIG_DIR          "${CMAKE_CURRENT_BINARY_DIR}")
set(TAYWEE_ARGS_CMAKE_VERSION_CONFIG_FILE "${TAYWEE_ARGS_CMAKE_CONFIG_DIR}/${PROJECT_NAME}ConfigVersion.cmake")
set(TAYWEE_ARGS_CMAKE_PROJECT_CONFIG_FILE "${TAYWEE_ARGS_CMAKE_CONFIG_DIR}/${PROJECT_NAME}Config.cmake")
set(TAYWEE_ARGS_CMAKE_PROJECT_TARGETS_FILE "${TAYWEE_ARGS_CMAKE_CONFIG_DIR}/${PROJECT_NAME}Targets.cmake")
set(TAYWEE_ARGS_PKGCONFIG_INSTALL_DIR     "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
set(TAYWEE_ARGS_INCLUDE_BUILD_DIR "${PROJECT_SOURCE_DIR}")
set(TAYWEE_ARGS_INCLUDE_FILE "${PROJECT_SOURCE_DIR}/args.hxx")

option(ARGS_BUILD_EXAMPLE "Build example" ON)
option(ARGS_BUILD_UNITTESTS "Build unittests" ON)
option(ARGS_Install             "Install CMake targets during install step." ${MAIN_PROJECT})
option(ARGS_SystemInclude       "Include as system headers (skip for clang-tidy)." OFF)

set(CMAKE_CONFIG_DEST "${CMAKE_INSTALL_LIBDIR_ARCHIND}/cmake/${PROJECT_NAME}")
set(PackagingTemplatesDir "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if (ARGS_SystemInclude)
    set(TAYWEE_ARGS_SYSTEM_INCLUDE "SYSTEM")
endif()

add_library(${TAYWEE_ARGS_TARGET_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${TAYWEE_ARGS_TARGET_NAME} ALIAS ${TAYWEE_ARGS_TARGET_NAME})

target_include_directories(
    ${TAYWEE_ARGS_TARGET_NAME}
    ${TAYWEE_ARGS_SYSTEM_INCLUDE} INTERFACE
    $<BUILD_INTERFACE:${TAYWEE_ARGS_INCLUDE_BUILD_DIR}>
    $<INSTALL_INTERFACE:include>
    )

if (ARGS_BUILD_EXAMPLE)
    add_executable(gitlike examples/gitlike.cxx)
    target_link_libraries(gitlike args)
    set_property(TARGET gitlike PROPERTY CXX_STANDARD 11)

    add_executable(completion examples/completion.cxx)
    target_link_libraries(completion args)
    set_property(TARGET completion PROPERTY CXX_STANDARD 11)
endif()

if (ARGS_BUILD_UNITTESTS)
    add_executable(argstest test.cxx)
    target_link_libraries(argstest args)
    set_property(TARGET argstest PROPERTY CXX_STANDARD 11)

    if (MSVC)
        target_compile_options(argstest PRIVATE /W4 /WX /bigobj)
    else ()
        target_compile_options(argstest PRIVATE -Wall -Wextra -Werror -pedantic -Wshadow -Wunused-parameter)
    endif ()

    add_executable(argstest-multiple-inclusion test/multiple_inclusion_1.cxx test/multiple_inclusion_2.cxx)

    target_link_libraries(argstest-multiple-inclusion args)
    set_property(TARGET argstest-multiple-inclusion PROPERTY CXX_STANDARD 11)

    add_executable(argstest-windows-h test/windows_h.cxx)
    target_link_libraries(argstest-windows-h args)
    set_property(TARGET argstest-windows-h PROPERTY CXX_STANDARD 11)

    enable_testing()
    add_test(NAME "test" COMMAND argstest)
    add_test(NAME "test-multiple-inclusion" COMMAND argstest-multiple-inclusion)
endif()

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VENDOR "${PROJECT_NAME} developers")
set(CPACK_PACKAGE_DESCRIPTION "${PROJECT_DESCRIPTION}")
set(CPACK_DEBIAN_PACKAGE_NAME "${CPACK_PACKAGE_NAME}")
set(CPACK_RPM_PACKAGE_NAME "${CPACK_PACKAGE_NAME}")
set(CPACK_PACKAGE_HOMEPAGE_URL "${PROJECT_HOMEPAGE_URL}")
set(CPACK_PACKAGE_MAINTAINER "${CPACK_PACKAGE_VENDOR}")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_MAINTAINER}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

set(CPACK_DEBIAN_PACKAGE_NAME "lib${PROJECT_NAME}-dev")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6-dev")
set(CPACK_DEBIAN_PACKAGE_SUGGESTS "cmake, pkg-config, pkg-conf")

set(CPACK_RPM_PACKAGE_NAME "lib${PROJECT_NAME}-devel")
set(CPACK_RPM_PACKAGE_SUGGESTS "${CPACK_DEBIAN_PACKAGE_SUGGESTS}")

set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_RPM_COMPONENT_INSTALL ON)
set(CPACK_NSIS_COMPONENT_INSTALL ON)
set(CPACK_DEBIAN_COMPRESSION_TYPE "xz")

include(CPack)

include(CMakePackageConfigHelpers)
configure_file(
    "cmake/argsConfigVersion.cmake.in"
    ${TAYWEE_ARGS_CMAKE_VERSION_CONFIG_FILE}
    @ONLY
)
configure_file(
    ${TAYWEE_ARGS_CMAKE_CONFIG_TEMPLATE}
    ${TAYWEE_ARGS_CMAKE_PROJECT_CONFIG_FILE}
    @ONLY
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/pkg-config.pc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc"
)

if(ARGS_Install)
    install(
        FILES ${TAYWEE_ARGS_INCLUDE_FILE}
        DESTINATION ${TAYWEE_ARGS_INCLUDE_INSTALL_DIR}
    )
    install(
        FILES ${TAYWEE_ARGS_CMAKE_PROJECT_CONFIG_FILE} ${TAYWEE_ARGS_CMAKE_VERSION_CONFIG_FILE}
        DESTINATION ${TAYWEE_ARGS_CONFIG_INSTALL_DIR}
    )
    if (NLOHMANN_ADD_NATVIS)
        install(
            FILES ${NLOHMANN_NATVIS_FILE}
            DESTINATION .
    )
    endif()
    export(
        TARGETS ${TAYWEE_ARGS_TARGET_NAME}
        NAMESPACE ${PROJECT_NAME}::
        FILE ${TAYWEE_ARGS_CMAKE_PROJECT_TARGETS_FILE}
    )
    install(
        TARGETS ${TAYWEE_ARGS_TARGET_NAME}
        EXPORT ${TAYWEE_ARGS_TARGETS_EXPORT_NAME}
        INCLUDES DESTINATION ${TAYWEE_ARGS_INCLUDE_INSTALL_DIR}
    )
    install(
        EXPORT ${TAYWEE_ARGS_TARGETS_EXPORT_NAME}
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${TAYWEE_ARGS_CONFIG_INSTALL_DIR}
    )
    install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc"
        DESTINATION ${TAYWEE_ARGS_PKGCONFIG_INSTALL_DIR}
    )
endif()
