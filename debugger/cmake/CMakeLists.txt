cmake_minimum_required(VERSION 3.4.0)
project(riscvdebugger)


set(CMAKE_BUILD_TYPE Debug)

if(UNIX)
	set(EXECUTABLE_OUTPUT_PATH "linuxbuild/bin")
	set(INSTALL_DIR "linuxbuild/bin")
	add_compile_options(-rdynamic)
else()
	add_definitions(-D_UNICODE)
	add_definitions(-DUNICODE)
	set(CMAKE_INSTALL_PREFIX ".")
	set(EXECUTABLE_OUTPUT_PATH "./winbuild/bin")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()

add_subdirectory(libdbg64g)


include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/common
)


file(GLOB _riscvdebugger_src
	${CMAKE_CURRENT_SOURCE_DIR}/../src/common/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../src/appdbg64g/main.cpp
	)


add_executable(
   riscvdebugger
   ${_riscvdebugger_src}
)


if(UNIX)
    target_link_libraries(riscvdebugger libdbg64g pthread rt dl)
else()

    foreach(_source IN ITEMS ${_riscvdebugger_src})
        get_filename_component(_source_path "${_source}" PATH)
        # form proper filter name from the path to source file:
        string(REPLACE "${CMAKE_SOURCE_DIR}" "" _group_path "${_source_path}")
        string(REPLACE "/" "\\" _group_path "${_group_path}")
        string(REPLACE "..\\simulator_src\\" "" _group_path "${_group_path}")
 
        source_group("${_group_path}" FILES "${_source}")
    endforeach()


    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT riscvdebugger)
    set_target_properties(riscvdebugger PROPERTIES
       VS_DEBUGGER_COMMAND_ARGUMENTS "-c ${CMAKE_CURRENT_SOURCE_DIR}/../targets/sysc_river_gui.json")
    target_link_libraries(riscvdebugger libdbg64g)
endif()
  
