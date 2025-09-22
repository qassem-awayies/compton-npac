cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(Na22Compton)

# ---- Geant4 package ----
find_package(Geant4 REQUIRED ui_all vis_all)

# ---- C++ standard ----
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Include directories ----
include(${Geant4_USE_FILE})
include_directories(${PROJECT_SOURCE_DIR}/src)

# ---- Source files ----
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/src/*.hh)

# ---- Executable ----
add_executable(Na22Compton ${sources} ${headers})

# ---- Link Geant4 libraries ----
target_link_libraries(Na22Compton ${Geant4_LIBRARIES})

# ---- Compiler options ----
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(Na22Compton PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ---- Install rules (optional) ----
install(TARGETS Na22Compton DESTINATION bin)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/ DESTINATION include FILES_MATCHING PATTERN "*.hh")
#----- ROOT configuration ____
find_package(ROOT REQUIRED COMPONENTS Core Hist RIO)
include_directories(${ROOT_INCLUDE_DIRS})
link_directories(${ROOT_LIBRARY_DIRS})
target_link_libraries(Na22Compton ${ROOT_LIBRARIES})

