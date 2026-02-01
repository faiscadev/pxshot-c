cmake_minimum_required(VERSION 3.14)
project(pxshot VERSION 1.0.0 LANGUAGES C)

# C11 standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Options
option(PXSHOT_BUILD_SHARED "Build shared library" ON)
option(PXSHOT_BUILD_STATIC "Build static library" ON)
option(PXSHOT_BUILD_EXAMPLES "Build examples" ON)
option(PXSHOT_USE_SYSTEM_CJSON "Use system cJSON instead of bundled" OFF)
option(PXSHOT_HEADER_ONLY "Install as header-only (no libraries)" OFF)

# Find dependencies
find_package(CURL REQUIRED)

if(PXSHOT_USE_SYSTEM_CJSON)
    find_package(cJSON REQUIRED)
    add_compile_definitions(PXSHOT_USE_SYSTEM_CJSON)
endif()

# Include directories
set(PXSHOT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(NOT PXSHOT_HEADER_ONLY)
    # Source files
    set(PXSHOT_SOURCES src/pxshot.c)

    # Static library
    if(PXSHOT_BUILD_STATIC)
        add_library(pxshot_static STATIC ${PXSHOT_SOURCES})
        target_include_directories(pxshot_static
            PUBLIC
                $<BUILD_INTERFACE:${PXSHOT_INCLUDE_DIR}>
                $<INSTALL_INTERFACE:include>
        )
        target_link_libraries(pxshot_static PUBLIC CURL::libcurl)
        if(PXSHOT_USE_SYSTEM_CJSON)
            target_link_libraries(pxshot_static PUBLIC cJSON::cJSON)
        endif()
        set_target_properties(pxshot_static PROPERTIES OUTPUT_NAME pxshot)
        
        # Export alias
        add_library(pxshot::static ALIAS pxshot_static)
    endif()

    # Shared library
    if(PXSHOT_BUILD_SHARED)
        add_library(pxshot_shared SHARED ${PXSHOT_SOURCES})
        target_include_directories(pxshot_shared
            PUBLIC
                $<BUILD_INTERFACE:${PXSHOT_INCLUDE_DIR}>
                $<INSTALL_INTERFACE:include>
        )
        target_link_libraries(pxshot_shared PUBLIC CURL::libcurl)
        if(PXSHOT_USE_SYSTEM_CJSON)
            target_link_libraries(pxshot_shared PUBLIC cJSON::cJSON)
        endif()
        set_target_properties(pxshot_shared PROPERTIES
            OUTPUT_NAME pxshot
            VERSION ${PROJECT_VERSION}
            SOVERSION ${PROJECT_VERSION_MAJOR}
        )
        
        # Export alias
        add_library(pxshot::shared ALIAS pxshot_shared)
    endif()

    # Default target alias
    if(PXSHOT_BUILD_STATIC)
        add_library(pxshot ALIAS pxshot_static)
        add_library(pxshot::pxshot ALIAS pxshot_static)
    elseif(PXSHOT_BUILD_SHARED)
        add_library(pxshot ALIAS pxshot_shared)
        add_library(pxshot::pxshot ALIAS pxshot_shared)
    endif()
endif()

# Examples
if(PXSHOT_BUILD_EXAMPLES AND NOT PXSHOT_HEADER_ONLY)
    add_executable(example_basic examples/basic.c)
    target_link_libraries(example_basic PRIVATE pxshot)
    
    add_executable(example_store examples/store.c)
    target_link_libraries(example_store PRIVATE pxshot)
    
    add_executable(example_usage examples/usage.c)
    target_link_libraries(example_usage PRIVATE pxshot)
    
    # Header-only example
    add_executable(example_header_only examples/header_only.c)
    target_include_directories(example_header_only PRIVATE ${PXSHOT_INCLUDE_DIR})
    target_link_libraries(example_header_only PRIVATE CURL::libcurl)
    if(PXSHOT_USE_SYSTEM_CJSON)
        target_link_libraries(example_header_only PRIVATE cJSON::cJSON)
    endif()
endif()

# Installation
include(GNUInstallDirs)

# Install headers
install(FILES
    ${PXSHOT_INCLUDE_DIR}/pxshot.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

if(NOT PXSHOT_USE_SYSTEM_CJSON)
    install(FILES
        ${PXSHOT_INCLUDE_DIR}/cJSON.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
endif()

if(NOT PXSHOT_HEADER_ONLY)
    # Install libraries
    if(PXSHOT_BUILD_STATIC)
        install(TARGETS pxshot_static
            EXPORT pxshotTargets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )
    endif()

    if(PXSHOT_BUILD_SHARED)
        install(TARGETS pxshot_shared
            EXPORT pxshotTargets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
    endif()

    # Export targets
    install(EXPORT pxshotTargets
        FILE pxshotTargets.cmake
        NAMESPACE pxshot::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/pxshot
    )

    # Package config
    include(CMakePackageConfigHelpers)

    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/pxshotConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/pxshotConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/pxshot
    )

    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/pxshotConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/pxshotConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/pxshotConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/pxshot
    )
endif()

# pkg-config
if(NOT PXSHOT_HEADER_ONLY)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/pxshot.pc.in
        ${CMAKE_CURRENT_BINARY_DIR}/pxshot.pc
        @ONLY
    )
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/pxshot.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
    )
endif()

# Summary
message(STATUS "")
message(STATUS "Pxshot C SDK v${PROJECT_VERSION}")
message(STATUS "  Build shared library: ${PXSHOT_BUILD_SHARED}")
message(STATUS "  Build static library: ${PXSHOT_BUILD_STATIC}")
message(STATUS "  Build examples: ${PXSHOT_BUILD_EXAMPLES}")
message(STATUS "  Use system cJSON: ${PXSHOT_USE_SYSTEM_CJSON}")
message(STATUS "  Header-only mode: ${PXSHOT_HEADER_ONLY}")
message(STATUS "")
