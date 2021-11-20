cmake_minimum_required(VERSION 3.4.0)
project(gui_plugin DESCRIPTION "gui_plugin shared library")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "$ENV{QT_PATH}//")

set(src_top "${CMAKE_CURRENT_SOURCE_DIR}/../../src")

if(UNIX)
	set(LIBRARY_OUTPUT_PATH "../linuxbuild/bin/plugins")
else()
	add_definitions(-D_UNICODE)
	add_definitions(-DUNICODE)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()

# It is neccessary to include QtGui to workaroung error in moc_ file
include_directories(
    ${src_top}/common
    ${src_top}/gui_plugin
    ${CMAKE_PREFIX_PATH}include
    ${CMAKE_PREFIX_PATH}include/QtGui
)


file(GLOB_RECURSE gui_plugin_src
    LIST_DIRECTORIES false
    ${src_top}/common/*.cpp
    ${src_top}/common/*.h
    ${src_top}/gui_plugin/*.cpp
    ${src_top}/gui_plugin/*.h
    ${src_top}/gui_plugin/*.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/exportmap.def
)


if(UNIX)
    add_compile_options(-std=c++17)
else()
    foreach(_source IN ITEMS ${gui_plugin_src})
        get_filename_component(_source_path "${_source}" PATH)
        # form proper filter name from the path to source file:
        string(REPLACE "${CMAKE_SOURCE_DIR}" "" _group_path "${_source_path}")
        string(REPLACE "/" "\\" _group_path "${_group_path}")
        string(REPLACE "cmake\\gui_plugin" "" _group_path "${_group_path}")
        string(REPLACE "..\\..\\" "" _group_path "${_group_path}")
 
        source_group("${_group_path}" FILES "${_source}")
    endforeach()
endif()

find_package(Qt6 COMPONENTS Widgets Network REQUIRED)

add_library(gui_plugin SHARED
    ${gui_plugin_src}
)

if(UNIX)
    target_link_libraries(gui_plugin libdbg64g Qt::Core Qt::Gui Qt::Widgets Qt::Network)	
    add_custom_command(TARGET gui_plugin  
	        POST_BUILD
	        COMMAND /bin/bash ${CMAKE_CURRENT_SOURCE_DIR}/post_build_event.sh "${LIBRARY_OUTPUT_PATH}"
	        )

else()
    set_target_properties(gui_plugin PROPERTIES RUNTIME_OUTPUT_DIRECTORY "../winbuild/bin/plugins")
    set_target_properties(gui_plugin PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "../winbuild/bin/plugins")
    set_target_properties(gui_plugin PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "../winbuild/bin/plugins")

    target_link_libraries(gui_plugin libdbg64g Qt::Core Qt::Gui Qt::Widgets Qt::Network)	
    add_custom_command(TARGET gui_plugin 
	        POST_BUILD
	        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/post_build_event.bat "\"$(SolutionDir)winbuild/bin\""
	        )

endif()

 