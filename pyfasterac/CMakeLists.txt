cmake_minimum_required(VERSION 3.16)
project(pyfasterac)
#cmake_policy( SET CMP0148 NEW ) # pybind11
#cmake_policy( SET CMP0135 NEW ) # DOWNLOAD_EXTRACT_TIMESTAMP

set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_STANDARD 17)

option(BUILTIN_FASTERAC "Internally build fasterac from source" ON)
option(BUILTIN_PYBIND11 "Internally download pybind11 from source" ON)

if (BUILTIN_PYBIND11)
    message(STATUS "Fetching pybind11")
    include(FetchContent)
    FetchContent_Declare(
            pybind11
            GIT_REPOSITORY https://github.com/pybind/pybind11
            GIT_TAG        v3.0.1
    )
    FetchContent_GetProperties(pybind11)
    if(NOT pybind11_POPULATED)
        FetchContent_MakeAvailable(pybind11)
    endif()
    include_directories(${pybind11_SOURCE_DIR}/include)
else()
    message(STATUS "Searching pybind11")
    find_package(pybind11 REQUIRED)
    include_directories(${pybind11_INCLUDE_DIRS})
endif ()

set(EXTERNAL_LIBRARIES)

pybind11_add_module( pyfasterac fastreader.cpp pybind_pyfasterac.cpp)

# Use Pkg config to find fasterac
#
if (NOT ${BUILTIN_FASTERAC})
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(fasterac libfasterac)
endif()

if (${fasterac_FOUND})
    message(STATUS "--> fasterac include: ${fasterac_INCLUDE_DIRS}")
    message(STATUS "--> fasterac libraries: ${fasterac_LINK_LIBRARIES}")
    include_directories(${fasterac_INCLUDE_DIRS})

    list(APPEND EXTERNAL_LIBRARIES ${fasterac_LINK_LIBRARIES})

else ()

    message(STATUS "Failed to find fasterac; will build internally")
    # add dependencies
    include(ExternalProject)

    ExternalProject_Add(fasterac_lib
        URL https://faster.in2p3.fr/distribution/fasterac-2.20.1.tar.gz
        DOWNLOAD_NO_PROGRESS TRUE
        BUILD_IN_SOURCE TRUE
        CONFIGURE_COMMAND ./configure --prefix=${CMAKE_INSTALL_PREFIX}
        BUILD_COMMAND $(MAKE)
        INSTALL_COMMAND $(MAKE) install
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    )
    include_directories(${CMAKE_INSTALL_PREFIX}/include)
    # Less portable, but works if you know the exact file name
    if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set(LIB_FILE_NAME "libfasterac.dylib")
    else()
        set(LIB_FILE_NAME "libfasterac.so")
    endif()
    list(APPEND EXTERNAL_LIBRARIES ${CMAKE_INSTALL_PREFIX}/lib/${LIB_FILE_NAME})
    message(STATUS "--> fasterac libraries:  ${CMAKE_INSTALL_PREFIX}/lib/${LIB_FILE_NAME}")

    add_dependencies(pyfasterac fasterac_lib)
endif()


target_link_libraries(pyfasterac PRIVATE ${EXTERNAL_LIBRARIES})


install(TARGETS
        pyfasterac
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
)

# Add a install target for the CLion IDE
add_custom_target(install_${PROJECT_NAME}
        "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target install
        COMMENT "Installing ${PROJECT_NAME}")
