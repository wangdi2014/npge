set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR})

if (${NPGE_STATIC_LINUX})
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a;.so")
    set(CMAKE_EXE_LINKER_FLAGS
        "-static-libgcc -static-libstdc++")
endif ()

include_directories(.)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(Threads REQUIRED)

include(FindZLIB)
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})

set(Boost_USE_STATIC_LIBS ON)
include(FindBoost)
IF (MINGW)
    SET(B_THREAD "thread_win32")
    add_definitions(-DBOOST_THREAD_USE_LIB)
ELSE (MINGW)
    SET(B_THREAD "thread")
ENDIF (MINGW)
FIND_PACKAGE(Boost 1.42
    COMPONENTS program_options filesystem system date_time iostreams ${B_THREAD}
    REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_path(LUAJIT_INCLUDE_DIR luajit.h
    PATH_SUFFIXES include/luajit-2.0)
find_library(LUAJIT_LIBRARY
    NAMES luajit luajit-5.1 luajit-5.2)
if (LUAJIT_INCLUDE_DIR AND LUAJIT_LIBRARY)
    set(LUAJIT_FOUND TRUE)
endif ()

if (LUAJIT_FOUND)
    option(USE_LUAJIT "Use LuaJIT instead of standard Lua" ON)
else ()
    option(USE_LUAJIT "Use LuaJIT instead of standard Lua" OFF)
endif ()

if (USE_LUAJIT)
    if (NOT LUAJIT_FOUND)
        message(FATAL_ERROR "luajit library not found")
    endif()
    set(LUAIMPL_LIBS ${LUAJIT_LIBRARY})
    set(LUAIMPL_INCLUDE ${LUAJIT_INCLUDE_DIR})
else()
    find_package(Lua REQUIRED)
    set(LUAIMPL_INCLUDE ${LUA_INCLUDE_DIR})
    add_definitions(-DLUA_COMPAT_ALL)
    set(LUAIMPL_LIBS ${LUA_LIBRARIES})
endif()

include_directories(${LUAIMPL_INCLUDE})

find_package(Luabind REQUIRED)
include_directories(${LUABIND_INCLUDE_DIRS})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/config.hpp)

message(${Boost_LIBRARIES})
message("Removing pthread from Boost_LIBRARIES")
# https://cmake.org/pipermail/cmake/2006-October/011734.html
FOREACH(item ${Boost_LIBRARIES})
  STRING(REGEX MATCH "pthread" item_test ${item})
  IF (NOT item_test)
    LIST(APPEND My_Boost_LIBRARIES ${item})
  ENDIF ()
ENDFOREACH()
message(${My_Boost_LIBRARIES})

set(COMMON_LIBS
 npge_algo
 npge_model
 npge_util
 ${My_Boost_LIBRARIES}
 ${ZLIB_LIBRARIES}
 ${LUABIND_LIBRARIES}
 ${LUAIMPL_LIBS})
IF (MINGW)
    set(COMMON_LIBS ${COMMON_LIBS} -lws2_32)
ENDIF (MINGW)
IF (UNIX)
    set(COMMON_LIBS ${COMMON_LIBS} ${CMAKE_THREAD_LIBS_INIT})
ENDIF ()
message("CMAKE_SYSTEM_NAME")
message(${CMAKE_SYSTEM_NAME})
IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(COMMON_LIBS ${COMMON_LIBS} dl)
ENDIF ()

subdirs(util)
subdirs(model)
subdirs(algo)
subdirs(test)
subdirs(tool)

FIND_PACKAGE(Qt4)
option(BUILD_GUI "Build graphical user interface" ${QT4_FOUND})
if(BUILD_GUI)
    subdirs(gui)
endif()

