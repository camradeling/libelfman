cmake_minimum_required(VERSION 3.10)
project(elfman)
option(DEBUG_BUILD "Option description" OFF)
set(CMAKE_CXX_STANDARD 17)
set(PROJECT_NAME elfman)
if(DEBUG_BUILD)
    message(STATUS "DEBUG_BUILD = ON")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fno-omit-frame-pointer -fasynchronous-unwind-tables")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -fno-omit-frame-pointer -fasynchronous-unwind-tables")
else()
    message(STATUS "DEBUG_BUILD = OFF")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Os -s")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s")
    #set(CMAKE_INSTALL_RPATH "/")
endif()

set(Sources
        static_library.cpp
        object_file.cpp
        section.cpp
        symbol.cpp
        rel.cpp
        memory_helpers.cpp
        )

# Add the logger submodule (logger.h / logger.cpp)
add_subdirectory(logger)
add_subdirectory(utils)

message(STATUS "CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")
add_library(elfman ${Sources})

target_include_directories(elfman
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link the logger library into elfman
target_link_libraries(elfman PUBLIC logger_config)
# Link the utils library into elfman
target_link_libraries(elfman PUBLIC utils)

# =========================
# Test programs
# =========================

set(rebuildobj_Sources
        tests/rebuildobj.cpp)
add_executable(rebuildobj ${rebuildobj_Sources})
target_link_libraries(rebuildobj PUBLIC elfman)

set(rebuildar_Sources
        tests/rebuildar.cpp)
add_executable(rebuildar ${rebuildar_Sources})
target_link_libraries(rebuildar PUBLIC elfman)

set(searchsym_Sources
        tests/searchsym.cpp)
add_executable(searchsym ${searchsym_Sources})
target_link_libraries(searchsym PUBLIC elfman)

set(makeglobal_Sources
        tests/makeglobal.cpp)
add_executable(makeglobal ${makeglobal_Sources})
target_link_libraries(makeglobal PUBLIC elfman)

set(getobj_Sources
        tests/getobj.cpp)
add_executable(getobj ${getobj_Sources})
target_link_libraries(getobj PUBLIC elfman)

set(insert_Sources
        tests/insert.cpp)
add_executable(insert ${insert_Sources})
target_link_libraries(insert PUBLIC elfman)

set(insertobj_Sources
        tests/insertobj.cpp)
add_executable(insertobj ${insertobj_Sources})
target_link_libraries(insertobj PUBLIC elfman)

set(moverel_Sources
        tests/moverel.cpp)
add_executable(moverel ${moverel_Sources})
target_link_libraries(moverel PUBLIC elfman)

set(renamesym_Sources
        tests/renamesym.cpp)
add_executable(renamesym ${renamesym_Sources})
target_link_libraries(renamesym PUBLIC elfman)
