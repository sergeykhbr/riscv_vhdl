cmake_minimum_required(VERSION 3.4.0)
project(cpu_sysc_plugin DESCRIPTION "cpu_sysc_plugin shared library")

if (NOT DEFINED ENV{SYSTEMC_PATH})
    message(FATAL_ERROR "Define variable SYSTEMC_PATH with the proper path to SystemC code")
endif()

set(src_top "${CMAKE_CURRENT_SOURCE_DIR}/../../src")

#set(CMAKE_BUILD_TYPE Debug)

if(UNIX)
	set(LIBRARY_OUTPUT_PATH "../../linuxbuild/bin/plugins")
else()
	add_definitions(-D_UNICODE)
	add_definitions(-DUNICODE)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()


include_directories(
    $ENV{SYSTEMC_PATH}/src
    ${src_top}/common
)


file(GLOB_RECURSE cpu_sysc_plugin_src
    LIST_DIRECTORIES false
    ${src_top}/common/*.cpp
    ${src_top}/common/*.h
    ${src_top}/cpu_sysc_plugin/*.cpp
    ${src_top}/cpu_sysc_plugin/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/exportmap.def
)

if(UNIX)
else()
    foreach(_source IN ITEMS ${cpu_sysc_plugin_src})
        get_filename_component(_source_path "${_source}" PATH)
        # form proper filter name from the path to source file:
        string(REPLACE "${CMAKE_SOURCE_DIR}" "" _group_path "${_source_path}")
        string(REPLACE "/" "\\" _group_path "${_group_path}")
        string(REPLACE "cmake\\cpu_sysc_plugin" "" _group_path "${_group_path}")
        string(REPLACE "..\\..\\" "" _group_path "${_group_path}")
 
        source_group("${_group_path}" FILES "${_source}")
    endforeach()
endif()


if (MSVC)
    add_compile_options(/vmg /wd"4244" /wd"4996")
  if(CMAKE_CL_64)
    link_directories(BEFORE "$ENV{SYSTEMC_PATH}/msvc10/SystemC/x64" "$ENV{SYSTEMC_PATH}/msvc80/SystemC/x64")
  else()
    link_directories(BEFORE "$ENV{SYSTEMC_PATH}/msvc10/SystemC" "$ENV{SYSTEMC_PATH}/msvc80/SystemC")
  endif()
else()
    #add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()


add_library(cpu_sysc_plugin SHARED
    ${cpu_sysc_plugin_src}
)

if(UNIX)
    set_target_properties(cpu_sysc_plugin PROPERTIES PREFIX "")
else()
    set_target_properties(cpu_sysc_plugin PROPERTIES RUNTIME_OUTPUT_DIRECTORY "../winbuild/bin/plugins")
    set_target_properties(cpu_sysc_plugin PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "../winbuild/bin/plugins")
    set_target_properties(cpu_sysc_plugin PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "../winbuild/bin/plugins")

    set_property(TARGET cpu_sysc_plugin PROPERTY
      MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

target_link_libraries(cpu_sysc_plugin libdbg64g SystemC)
  