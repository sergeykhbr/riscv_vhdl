cmake_minimum_required(VERSION 3.6)
project(cpu_arm_plugin DESCRIPTION "cpu_arm_plugin shared library")

set(src_top "${CMAKE_CURRENT_SOURCE_DIR}/../../src")

#set(CMAKE_BUILD_TYPE Debug)

if(UNIX)
	set(LIBRARY_OUTPUT_PATH "../linuxbuild/bin/plugins")
else()
	add_definitions(-D_UNICODE)
	add_definitions(-DUNICODE)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()


include_directories(
    ${src_top}/common
)


file(GLOB_RECURSE cpu_arm_plugin_src
    LIST_DIRECTORIES false
    ${src_top}/common/*.cpp
    ${src_top}/common/*.h
    ${src_top}/cpu_arm_plugin/*.cpp
    ${src_top}/cpu_arm_plugin/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/exportmap.def
)

if(UNIX)
else()
    foreach(_source IN ITEMS ${cpu_arm_plugin_src})
        get_filename_component(_source_path "${_source}" PATH)
        # form proper filter name from the path to source file:
        string(REPLACE "${CMAKE_SOURCE_DIR}" "" _group_path "${_source_path}")
        string(REPLACE "/" "\\" _group_path "${_group_path}")
        string(REPLACE "cmake\\cpu_arm_plugin" "" _group_path "${_group_path}")
        string(REPLACE "..\\..\\" "" _group_path "${_group_path}")
 
        source_group("${_group_path}" FILES "${_source}")
    endforeach()
endif()


add_library(cpu_arm_plugin SHARED
    ${cpu_arm_plugin_src}
)

if(UNIX)
    set_target_properties(cpu_arm_plugin PROPERTIES PREFIX "")
else()
    set_target_properties(cpu_arm_plugin PROPERTIES RUNTIME_OUTPUT_DIRECTORY "../winbuild/bin/plugins")
    set_target_properties(cpu_arm_plugin PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "../winbuild/bin/plugins")
    set_target_properties(cpu_arm_plugin PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "../winbuild/bin/plugins")

endif()

target_link_libraries(cpu_arm_plugin libdbg64g)	
  