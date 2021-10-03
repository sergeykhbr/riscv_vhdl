cmake_minimum_required(VERSION 3.4.0)
project(libdbg64g DESCRIPTION "libdbg64g shared library")

set(src_top "${CMAKE_CURRENT_SOURCE_DIR}/../../src")

#set(CMAKE_BUILD_TYPE Debug)

if(UNIX)
	set(LIBRARY_OUTPUT_PATH "../linuxbuild/bin")
else()
	add_definitions(-D_UNICODE)
	add_definitions(-DUNICODE)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()


include_directories(
    ${src_top}/common
)
if(UNIX)
else()
  # include dirent.h directory
  include_directories(
      ${src_top}/libdbg64g/include
  )
endif()


file(GLOB_RECURSE libdbg64g_src
    LIST_DIRECTORIES false
    ${src_top}/common/*.cpp
    ${src_top}/common/*.h
    ${src_top}/libdbg64g/*.cpp
    ${src_top}/libdbg64g/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/exportmap.def
)

if(UNIX)
    list(FILTER libdbg64g_src EXCLUDE REGEX "[/]comport[/]com_win.cpp")
else()
    list(FILTER libdbg64g_src EXCLUDE REGEX "[/]comport[/]com_linux.cpp")

    foreach(_source IN ITEMS ${libdbg64g_src})
        get_filename_component(_source_path "${_source}" PATH)
        # form proper filter name from the path to source file:
        string(REPLACE "${CMAKE_SOURCE_DIR}" "" _group_path "${_source_path}")
        string(REPLACE "/" "\\" _group_path "${_group_path}")
        string(REPLACE "cmake\\libdbg64g" "" _group_path "${_group_path}")
        string(REPLACE "..\\..\\" "" _group_path "${_group_path}")
 
        source_group("${_group_path}" FILES "${_source}")
    endforeach()
endif()


add_library(libdbg64g SHARED
    ${libdbg64g_src}
)

if(UNIX)
    set_target_properties(libdbg64g PROPERTIES PREFIX "")
    target_link_libraries(libdbg64g pthread dl rt)	
else()
    set_target_properties(libdbg64g PROPERTIES RUNTIME_OUTPUT_DIRECTORY "../winbuild/bin")
    set_target_properties(libdbg64g PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "../winbuild/bin")
    set_target_properties(libdbg64g PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "../winbuild/bin")

    target_link_libraries(libdbg64g ws2_32)	
endif()
  